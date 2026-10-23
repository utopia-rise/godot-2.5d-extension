#ifdef TOOLS_ENABLED

#include "move_editor_grid_command_emitter.h"

#include "editor/commands/move_editor_plane_command.h"
#include "editor/isometric_editor_plugin.h"

using namespace godot;
using namespace editor::commands::emitters;

Vector<Ref<editor::commands::Command>> MoveEditorGridCommandEmitter::from_gui_input_to_command(const Ref<InputEventKey>& p_event) {
    Vector<Ref<Command>> commands;

    if (!p_event->is_pressed()) { return commands; }

    bool is_forward;
    switch (p_event->get_keycode()) {
        case KEY_UP:
            is_forward = true;
            break;
        case KEY_DOWN:
            is_forward = false;
            break;
        default:
            return commands;
    }

    IsometricEditorPlugin* isometric_editor_plugin = IsometricEditorPlugin::get_instance();
    node::IsometricMap* map = isometric_editor_plugin->get_selected_map();
    EditorPlane& editor_plane = isometric_editor_plugin->get_editor_plane_for_map(map, EditorPlane::PlaneType::EDITOR_DRAWER);

    int current_position = editor_plane.get_position();
    auto map_size_on_axis = static_cast<int>(map->get_size()[editor_plane.get_axis()]);
    if ((current_position == 0 && !is_forward) || (current_position >= map_size_on_axis - 1 && is_forward)) {
        return commands;
    }
    int new_position = MIN(current_position, map_size_on_axis) + (is_forward ? 1 : -1);

    Ref<MoveEditorPlaneCommand> move_command;
    move_command.instantiate();
    move_command->set_plane_type(EditorPlane::PlaneType::EDITOR_DRAWER);
    move_command->set_old_position(current_position);
    move_command->set_new_position(new_position);
    commands.push_back(move_command);

    return commands;
}

#endif
