#ifdef TOOLS_ENABLED

#include "fill_plan_command_emitter.h"

#include "editor/commands/add_positionable_command.h"
#include "editor/editor_utils.h"
#include "editor/isometric_editor_plugin.h"

#include <classes/input_event_mouse_button.hpp>

using namespace godot;
using namespace editor::commands::emitters;

Vector<Ref<editor::commands::Command>> FillPlanCommandEmitter::from_gui_input_to_command(const Ref<InputEventMouse>& p_event) {
    Vector<Ref<Command>> commands;

    Ref<InputEventMouseButton> button_event = p_event;
    if (button_event.is_null() || !button_event->is_pressed() || button_event->get_button_index() != MOUSE_BUTTON_LEFT) {
        return commands;
    }

    IsometricEditorPlugin* isometric_editor_plugin = IsometricEditorPlugin::get_instance();
    int selected_tile_id = isometric_editor_plugin->get_selection_pane()->get_selected_positionable_id();
    if (selected_tile_id == resource::PositionableSet::NONE_POSITIONABLE_ID) { return commands; }

    node::IsometricMap* map = isometric_editor_plugin->get_selected_map();
    Vector3 positionable_size;
    if (!EditorUtils::get_positionable_size(map->get_positionable_set(), selected_tile_id, positionable_size)) {
        return commands;
    }

    Vector3::Axis plane_axis =
      isometric_editor_plugin->get_editor_plane_for_map(map, EditorPlane::PlaneType::EDITOR_DRAWER).get_axis();
    uint32_t layer_id = isometric_editor_plugin->get_selected_layer();
    for (const Vector3& position :
         map->get_connected_free_positions(isometric_editor_plugin->get_mouse_position_3d(), plane_axis, positionable_size)) {
        AABB aabb(position, positionable_size);
        if (!isometric_editor_plugin->is_aabb_in_view_limiters(aabb)) { continue; }

        Ref<AddPositionableCommand> add_command;
        add_command.instantiate();
        add_command->set_aabb(aabb);
        add_command->set_positionable_id(selected_tile_id);
        add_command->set_layer_id(layer_id);
        commands.push_back(add_command);
    }

    return commands;
}

#endif
