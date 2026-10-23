#ifdef TOOLS_ENABLED

#include "select_command_emitter.h"

#include "editor/commands/select_positionable_command.h"
#include "editor/isometric_editor_plugin.h"

#include <classes/input_event_mouse_button.hpp>

using namespace godot;
using namespace editor::commands::emitters;

Vector<Ref<editor::commands::Command>> SelectCommandEmitter::from_gui_input_to_command(const Ref<InputEventMouse>& p_event) {
    Vector<Ref<Command>> commands;

    // Shift + click starts moving the selection instead.
    Ref<InputEventMouseButton> button_event = p_event;
    if (
      button_event.is_null() || !button_event->is_pressed() || button_event->get_button_index() != MOUSE_BUTTON_LEFT
      || button_event->is_shift_pressed()
    ) {
        return commands;
    }

    IsometricEditorPlugin* isometric_editor_plugin = IsometricEditorPlugin::get_instance();
    node::IsometricMap* map = isometric_editor_plugin->get_selected_map();
    Vector3 position = isometric_editor_plugin->get_mouse_position_3d();

    if (!isometric_editor_plugin->is_aabb_in_view_limiters(AABB(position, Vector3(1, 1, 1)))) { return commands; }
    if (!map->get_positionable_at(position)) { return commands; }

    Ref<SelectPositionableCommand> command;
    command.instantiate();
    command->set_position(position);
    command->set_should_deselect_first(!button_event->is_command_or_control_pressed());
    commands.push_back(command);

    return commands;
}

#endif
