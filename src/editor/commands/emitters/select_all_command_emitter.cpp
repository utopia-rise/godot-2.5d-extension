#ifdef TOOLS_ENABLED

#include "select_all_command_emitter.h"

#include "editor/commands/select_positionable_command.h"
#include "editor/isometric_editor_plugin.h"
#include "editor/positionable_selector_manager.h"

using namespace godot;
using namespace editor::commands::emitters;

Vector<Ref<editor::commands::Command>> SelectAllCommandEmitter::from_gui_input_to_command(const Ref<InputEventKey>& p_event) {
    Vector<Ref<Command>> commands;

    if (!p_event->is_pressed() || p_event->get_keycode() != KEY_A || !p_event->is_command_or_control_pressed()) {
        return commands;
    }

    IsometricEditorPlugin* isometric_editor_plugin = IsometricEditorPlugin::get_instance();
    node::IsometricMap* map = isometric_editor_plugin->get_selected_map();

    Vector3 view_start;
    Vector3 view_end;
    for (int axis = Vector3::AXIS_X; axis <= Vector3::AXIS_Z; ++axis) {
        EditorPlane::PlaneType min_limiter = EditorPlane::get_min_view_limiter(static_cast<Vector3::Axis>(axis));
        EditorPlane::PlaneType max_limiter = EditorPlane::get_max_view_limiter(static_cast<Vector3::Axis>(axis));
        view_start[axis] = static_cast<real_t>(isometric_editor_plugin->get_editor_plane_for_map(map, min_limiter).get_position());
        view_end[axis] = static_cast<real_t>(isometric_editor_plugin->get_editor_plane_for_map(map, max_limiter).get_position());
    }

    for (node::IsometricPositionable* positionable : map->get_positionables_in(AABB(view_start, view_end))) {
        if (!positionable) { continue; }
        Vector3 position = positionable->get_local_position_3d();
        if (PositionableSelectorManager::get_singleton()->is_position_selected_for_map(map, position)) { continue; }

        Ref<SelectPositionableCommand> command;
        command.instantiate();
        command->set_position(position);
        command->set_should_deselect_first(false);
        commands.push_back(command);
    }

    return commands;
}

#endif
