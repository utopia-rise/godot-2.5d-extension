#ifdef TOOLS_ENABLED

#include "isometric_tile_editor_plugin.h"

#include "isometric_string_names.h"
#include "node/isometric_map.h"
#include "outline_drawer.h"

#include <classes/label.hpp>

using namespace godot;
using namespace editor;

static constexpr const char* OUTLINE_COLOR_TITLE = "Outline color:";

void IsometricTileEditorPlugin::_notification(int p_what) {
    if (p_what == NOTIFICATION_ENTER_TREE) {
        toolbar = memnew(HBoxContainer);
        toolbar->hide();
        auto* outline_color_label = memnew(Label);
        outline_color_label->set_text(OUTLINE_COLOR_TITLE);
        toolbar->add_child(outline_color_label);
        color_picker_button = memnew(ColorPickerButton);
        color_picker_button->set_custom_minimum_size(Vector2(32, 0));
        color_picker_button->connect("color_changed", callable_mp(this, &IsometricTileEditorPlugin::_on_color_picker_change));
        toolbar->add_child(color_picker_button);
        add_control_to_container(CONTAINER_CANVAS_EDITOR_MENU, toolbar);
    } else if (p_what == NOTIFICATION_EXIT_TREE) {
        remove_control_from_container(CONTAINER_CANVAS_EDITOR_MENU, toolbar);
        memdelete(toolbar);
        toolbar = nullptr;
    }
}

bool IsometricTileEditorPlugin::_handles(Object* p_object) const {
    return Object::cast_to<node::IsometricPositionable>(p_object) && !Object::cast_to<node::IsometricMap>(p_object);
}

void IsometricTileEditorPlugin::_edit(Object* p_object) {
    auto* positionable = Object::cast_to<node::IsometricPositionable>(p_object);
    if (!positionable) { return; }
    selected_positionable = positionable;

    selected_positionable->get_outline_data().line_size = 1.0;
    _on_color_picker_change(color_picker_button->get_pick_color());
    OutlineDrawer::set_outline_visible(selected_positionable, true);

    Callable on_size_changed = callable_mp(this, &IsometricTileEditorPlugin::_on_size_changed);
    const StringName& size_changed_signal = IsometricStringNames::get_singleton()->size_changed_signal;
    if (!selected_positionable->is_connected(size_changed_signal, on_size_changed)) {
        selected_positionable->connect(size_changed_signal, on_size_changed);
    }
}

void IsometricTileEditorPlugin::_make_visible(bool p_visible) {
    toolbar->set_visible(p_visible);
}

void IsometricTileEditorPlugin::_on_size_changed() {
    if (selected_positionable) { OutlineDrawer::draw_outline(selected_positionable); }
}

void IsometricTileEditorPlugin::_on_color_picker_change(const Color& p_color) {
    if (!selected_positionable) { return; }
    selected_positionable->get_outline_data().color = p_color;
    OutlineDrawer::draw_outline(selected_positionable);
}

#endif
