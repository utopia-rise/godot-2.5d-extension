#ifdef TOOLS_ENABLED

#include "isometric_editor_plugin.h"

#include "constants.h"
#include "edition_grid_drawer.h"
#include "isometric_server.h"
#include "isometric_string_names.h"
#include "outline_drawer.h"
#include "positionable_scenes_cache_manager.h"
#include "positionable_selector_manager.h"
#include "utils/isometric_maths.h"

#include <classes/input_event_key.hpp>
#include <classes/label.hpp>
#include <classes/rendering_server.hpp>
#include <classes/scene_tree.hpp>

using namespace godot;
using namespace editor;

static constexpr const char* POSITIONABLE_PANE_BUTTON_TITLE = "Isometric positionables";
static constexpr const char* GRID_COLOR_PICKER_TITLE = "Grid color:";

// Listed in Mode order.
static constexpr const char* EDITION_MODE_LABELS[] = {"None", "Select", "Paint", "Drag & Drop"};

static constexpr float PLANE_VISIBILITY_DELAY = 1.5;

IsometricEditorPlugin* IsometricEditorPlugin::singleton = nullptr;

IsometricEditorPlugin::IsometricEditorPlugin() {
    singleton = this;
}

IsometricEditorPlugin::~IsometricEditorPlugin() {
    if (singleton == this) { singleton = nullptr; }
}

IsometricEditorPlugin* IsometricEditorPlugin::get_instance() {
    return singleton;
}

void IsometricEditorPlugin::_notification(int p_what) {
    switch (p_what) {
        case NOTIFICATION_ENTER_TREE:
            _setup_ui();
            break;
        case NOTIFICATION_EXIT_TREE:
            _drop();
            RenderingServer::get_singleton()->disconnect("frame_post_draw", callable_mp(this, &IsometricEditorPlugin::_on_frame_post_draw));
            remove_control_from_container(CONTAINER_CANVAS_EDITOR_MENU, toolbar);
            memdelete(toolbar);
            remove_control_from_bottom_panel(positionable_selection_pane);
            memdelete(positionable_selection_pane);
            memdelete(layers_editor);
            break;
        default:
            break;
    }
}

void IsometricEditorPlugin::_setup_ui() {
    toolbar = memnew(HBoxContainer);
    toolbar->hide();
    add_control_to_container(CONTAINER_CANVAS_EDITOR_MENU, toolbar);

    auto* grid_color_label = memnew(Label);
    grid_color_label->set_text(GRID_COLOR_PICKER_TITLE);
    toolbar->add_child(grid_color_label);
    grid_color_picker_button = memnew(ColorPickerButton);
    grid_color_picker_button->set_custom_minimum_size(Vector2(32, 0));
    grid_color_picker_button->connect("color_changed", callable_mp(this, &IsometricEditorPlugin::_on_grid_color_picker_change));
    toolbar->add_child(grid_color_picker_button);

    edition_mode_button = memnew(OptionButton);
    for (const char* label : EDITION_MODE_LABELS) {
        edition_mode_button->add_item(label);
    }
    edition_mode_button->set_flat(true);
    edition_mode_button->connect("item_selected", callable_mp(this, &IsometricEditorPlugin::_on_edition_mode_changed));
    toolbar->add_child(edition_mode_button);

    debug_button = memnew(Button);
    debug_button->set_flat(true);
    debug_button->set_toggle_mode(true);
    debug_button->set_text("Debug");
    debug_button->connect("toggled", callable_mp(this, &IsometricEditorPlugin::set_debug_mode));
    toolbar->add_child(debug_button);

    positionable_selection_pane = memnew(inspector::PositionableSelectionPane);
    positionable_pane_button = add_control_to_bottom_panel(positionable_selection_pane, POSITIONABLE_PANE_BUTTON_TITLE);
    positionable_pane_button->set_visible(false);

    layers_editor = memnew(inspector::LayersEditor);

    RenderingServer::get_singleton()->connect("frame_post_draw", callable_mp(this, &IsometricEditorPlugin::_on_frame_post_draw));
}

inspector::PositionableSelectionPane* IsometricEditorPlugin::get_selection_pane() const {
    return positionable_selection_pane;
}

void IsometricEditorPlugin::set_debug_mode(bool p_debug) {
    show_debug = p_debug;
    get_tree()->call_group(IsometricStringNames::get_singleton()->debug_group_name, "set_debug_view", p_debug);
    ISOMETRIC_SERVER->set_debug(p_debug);
    if (selected_map) { OutlineDrawer::set_outline_visible(selected_map, p_debug); }
}

void IsometricEditorPlugin::set_should_clear_buffer_on_next_frame(bool should) {
    should_clear_buffer_on_next_frame = should;
}

void IsometricEditorPlugin::_edit(Object* p_object) {
    auto* map = Object::cast_to<node::IsometricMap>(p_object);
    if (!map) { return; }
    if (selected_map && selected_map != map) { _drop(); }
    selected_map = map;

    if (!handling_data_map.has(selected_map)) {
        handling_data_map.insert(selected_map, MapHandlingData(selected_map->get_size()));
        selected_map->connect("draw", callable_mp(this, &IsometricEditorPlugin::refresh).bind(EditorPlane::PlaneType::EDITOR_DRAWER));
        selected_map->connect(
          "positional_set_changed",
          callable_mp(positionable_selection_pane, &inspector::PositionableSelectionPane::set_positionable_set)
        );
        selected_map->connect(
          IsometricStringNames::get_singleton()->size_changed_signal,
          callable_mp(this, &IsometricEditorPlugin::_on_map_size_changed)
        );
    }

    positionable_selection_pane->set_positionable_set(selected_map->get_positionable_set());
    OutlineDrawer::set_outline_visible(selected_map, show_debug);

    _draw_edition_grid();

    PositionableSelectorManager::get_singleton()->refresh_outline_for_selected(selected_map);

    layers_editor->refresh();
    if (!layers_editor->get_parent()) { add_control_to_dock(DOCK_SLOT_RIGHT_BL, layers_editor); }
}

void IsometricEditorPlugin::_drop() {
    if (selected_map) {
        OutlineDrawer::set_outline_visible(selected_map, false);
        const MapHandlingData& map_handling_data = handling_data_map[selected_map];
        for (const EditorPlane& editor_plane : map_handling_data.editor_planes) {
            EditionGridDrawer::clear_for_editor_plane(editor_plane);
        }
    }
    if (layers_editor && layers_editor->get_parent()) { remove_control_from_docks(layers_editor); }
    selected_map = nullptr;
}

bool IsometricEditorPlugin::_handles(Object* p_object) const {
    return Object::cast_to<node::IsometricMap>(p_object) != nullptr;
}

void IsometricEditorPlugin::_clear() {
    if (!selected_map) { return; }
    OutlineDrawer::set_outline_visible(selected_map, false);
    selected_map = nullptr;
}

bool IsometricEditorPlugin::_forward_canvas_gui_input(const Ref<InputEvent>& p_event) {
    if (!selected_map) { return false; }

    // Undo and redo shortcuts stay with the editor.
    Ref<InputEventKey> keyboard_event = p_event;
    if (
      keyboard_event.is_valid() && keyboard_event->is_command_or_control_pressed()
      && (keyboard_event->get_keycode() == KEY_Z || keyboard_event->get_keycode() == KEY_Y)
    ) {
        return false;
    }

    switch (current_mode) {
        case NONE:
            return false;
        case SELECT:
            select_command_emitter.on_gui_input(p_event, selected_map);
            select_all_command_emitter.on_gui_input(p_event, selected_map);
            delete_command_emitter.on_gui_input(p_event, selected_map);
            move_selection_command_emitter.on_gui_input(p_event, selected_map);
            break;
        case PAINT:
            painting_command_emitter.on_gui_input(p_event, selected_map);
            break;
        case DRAG_AND_DROP:
            drag_and_drop_command_emitter.on_gui_input(p_event, selected_map);
            break;
    }
    move_editor_drawer_command_emitter.on_gui_input(p_event, selected_map);
    rotate_editor_plane_command_emitter.on_gui_input(p_event, selected_map);
    plane_view_limiter_command_emitter.on_gui_input(p_event, selected_map);
    return true;
}

void IsometricEditorPlugin::_make_visible(bool p_visible) {
    if (!p_visible) { _drop(); }
    toolbar->set_visible(p_visible);
    positionable_pane_button->set_visible(p_visible);
}

void IsometricEditorPlugin::refresh(int p_plane_type) {
    if (!selected_map || !handling_data_map.has(selected_map)) { return; }

    MapHandlingData& map_handling_data = handling_data_map[selected_map];
    if (map_handling_data.is_grid[p_plane_type]) {
        _draw_edition_grid();
        return;
    }
    auto plane_type = static_cast<EditorPlane::PlaneType>(p_plane_type);
    _draw_plane(plane_type);
    _set_plane_timer(plane_type, PLANE_VISIBILITY_DELAY);
}

node::IsometricMap* IsometricEditorPlugin::get_selected_map() const {
    return selected_map;
}

EditorPlane& IsometricEditorPlugin::get_editor_plane_for_map(node::IsometricMap* p_map, EditorPlane::PlaneType p_plane_type) {
    return handling_data_map[p_map].editor_planes[p_plane_type];
}

Vector3 IsometricEditorPlugin::get_mouse_position_3d() {
    const data::IsometricParameters* parameters = ISOMETRIC_SERVER->space_get_configuration(selected_map->get_space_rid());
    const EditorPlane& editor_plane = get_editor_plane_for_map(selected_map, EditorPlane::PlaneType::EDITOR_DRAWER);
    return utils::from_screen_to_3D(
      *parameters,
      selected_map->get_local_mouse_position(),
      editor_plane.get_axis(),
      static_cast<real_t>(editor_plane.get_position())
    );
}

bool IsometricEditorPlugin::is_aabb_in_view_limiters(const AABB& p_aabb) const {
    const MapHandlingData* map_handling_data = handling_data_map.getptr(selected_map);
    if (!map_handling_data) { return false; }

    for (int axis = Vector3::AXIS_X; axis <= Vector3::AXIS_Z; ++axis) {
        auto start = static_cast<int>(p_aabb.position[axis]);
        int end = start + static_cast<int>(p_aabb.size[axis]) - 1;
        const EditorPlane& min_limiter =
          map_handling_data->editor_planes[EditorPlane::get_min_view_limiter(static_cast<Vector3::Axis>(axis))];
        const EditorPlane& max_limiter =
          map_handling_data->editor_planes[EditorPlane::get_max_view_limiter(static_cast<Vector3::Axis>(axis))];
        if (start < min_limiter.get_position() || end >= max_limiter.get_position()) { return false; }
    }
    return true;
}

uint32_t IsometricEditorPlugin::get_selected_layer() const {
    if (!selected_map) { return node::IsometricMap::DEFAULT_LAYER_ID; }
    return selected_map->get_meta(node::IsometricMap::LAST_EDITED_LAYER_META_NAME, node::IsometricMap::DEFAULT_LAYER_ID);
}

IsometricEditorPlugin::MapHandlingData::MapHandlingData() : MapHandlingData(Vector3()) {}

IsometricEditorPlugin::MapHandlingData::MapHandlingData(const Vector3& p_map_size) :
  editor_planes {
    {0, Vector3::AXIS_Z, {p_map_size.x, p_map_size.y}},
    {0, Vector3::AXIS_X, {p_map_size.y, p_map_size.z}},
    {Constants::int_max, Vector3::AXIS_X, {p_map_size.y, p_map_size.z}},
    {0, Vector3::AXIS_Y, {p_map_size.x, p_map_size.z}},
    {Constants::int_max, Vector3::AXIS_Y, {p_map_size.x, p_map_size.z}},
    {0, Vector3::AXIS_Z, {p_map_size.x, p_map_size.y}},
    {Constants::int_max, Vector3::AXIS_Z, {p_map_size.x, p_map_size.y}}
  },
  is_grid {true, false, false, false, false, false, false} {}

void IsometricEditorPlugin::_on_frame_post_draw() {
    // Icon viewports render on the frame after they are added: their textures are copied one frame later.
    PositionableScenesCacheManager* cache_manager = PositionableScenesCacheManager::get_singleton();
    if (cache_manager->is_adding()) {
        should_clear_buffer_on_next_frame = false;
    } else if (should_clear_buffer_on_next_frame) {
        cache_manager->copy_current_viewports_textures();
        cache_manager->refresh_all_icons();
        cache_manager->clear_current_viewports();
        should_clear_buffer_on_next_frame = false;
    } else {
        should_clear_buffer_on_next_frame = true;
    }
}

void IsometricEditorPlugin::_on_edition_mode_changed(int selected_index) {
    current_mode = static_cast<Mode>(selected_index);
}

void IsometricEditorPlugin::_on_plane_visibility_timeout(int p_plane_type) {
    if (!selected_map) { return; }
    MapHandlingData& map_handling_data = handling_data_map[selected_map];
    EditionGridDrawer::clear_for_editor_plane(map_handling_data.editor_planes[p_plane_type]);
    map_handling_data.plane_timers[p_plane_type] = Ref<SceneTreeTimer>();
}

void IsometricEditorPlugin::_on_map_size_changed() {
    refresh(EditorPlane::PlaneType::EDITOR_DRAWER);
}

void IsometricEditorPlugin::_on_grid_color_picker_change([[maybe_unused]] const Color& p_color) {
    if (selected_map) { _draw_edition_grid(); }
}

void IsometricEditorPlugin::_draw_edition_grid() const {
    const MapHandlingData* map_handling_data = handling_data_map.getptr(selected_map);
    if (!map_handling_data) { return; }
    EditionGridDrawer::draw_grid(
      map_handling_data->editor_planes[EditorPlane::PlaneType::EDITOR_DRAWER],
      selected_map,
      grid_color_picker_button->get_pick_color()
    );
}

void IsometricEditorPlugin::_draw_plane(EditorPlane::PlaneType p_plane_type) {
    EditionGridDrawer::draw_plane(handling_data_map[selected_map].editor_planes[p_plane_type], selected_map);
}

void IsometricEditorPlugin::_set_plane_timer(EditorPlane::PlaneType p_plane_type, float p_delay) {
    MapHandlingData& map_handling_data = handling_data_map[selected_map];
    Callable on_timeout = callable_mp(this, &IsometricEditorPlugin::_on_plane_visibility_timeout).bind(p_plane_type);

    // A newer move restarts the countdown: the previous timer must not hide the plane early.
    Ref<SceneTreeTimer> existing_timer = map_handling_data.plane_timers[p_plane_type];
    if (existing_timer.is_valid() && existing_timer->is_connected("timeout", on_timeout)) {
        existing_timer->disconnect("timeout", on_timeout);
    }

    Ref<SceneTreeTimer> timer = get_tree()->create_timer(p_delay, false);
    timer->connect("timeout", on_timeout);
    map_handling_data.plane_timers[p_plane_type] = timer;
}

void IsometricEditorPlugin::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_debug_mode", "debug"), &IsometricEditorPlugin::set_debug_mode);
    ClassDB::bind_method(D_METHOD("refresh", "plane_type"), &IsometricEditorPlugin::refresh);
}

#endif
