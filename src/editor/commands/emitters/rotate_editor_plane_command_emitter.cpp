#ifdef TOOLS_ENABLED

#include "rotate_editor_plane_command_emitter.h"

#include "editor/commands/rotate_editor_plane_command.h"
#include "editor/isometric_editor_plugin.h"

using namespace godot;
using namespace editor::commands::emitters;

Vector<Ref<editor::commands::Command>> RotateEditorPlaneCommandEmitter::from_gui_input_to_command(const Ref<InputEventKey>& p_event) {
    Vector<Ref<Command>> commands;

    if (!p_event->is_pressed()) { return commands; }

    int rotation;
    switch (p_event->get_keycode()) {
        case KEY_LEFT:
            rotation = -1;
            break;
        case KEY_RIGHT:
            rotation = 1;
            break;
        default:
            return commands;
    }

    IsometricEditorPlugin* isometric_editor_plugin = IsometricEditorPlugin::get_instance();
    EditorPlane& editor_plane =
      isometric_editor_plugin->get_editor_plane_for_map(isometric_editor_plugin->get_selected_map(), EditorPlane::PlaneType::EDITOR_DRAWER);
    Vector3::Axis current_axis = editor_plane.get_axis();

    const int axis_count = 3;
    Ref<RotateEditorPlaneCommand> rotate_command;
    rotate_command.instantiate();
    rotate_command->set_former_axis(current_axis);
    rotate_command->set_former_position(editor_plane.get_position());
    rotate_command->set_new_axis(static_cast<Vector3::Axis>(Math::posmod(current_axis + rotation, axis_count)));
    commands.push_back(rotate_command);

    return commands;
}

#endif
