#ifdef TOOLS_ENABLED

#include "move_editor_plane_command.h"

#include "editor/isometric_editor_plugin.h"
#include "logging.h"

using namespace godot;
using namespace editor::commands;

void MoveEditorPlaneCommand::redo() {
    ISO_DEBUG_FAIL_COND_MSG(plane_type == EditorPlane::PlaneType::SIZE, "PlaneType should be set on MoveEditorPlaneCommand");
    IsometricEditorPlugin* isometric_editor_plugin = IsometricEditorPlugin::get_instance();
    isometric_editor_plugin->get_editor_plane_for_map(context_node, plane_type).set_position(new_position);
    isometric_editor_plugin->refresh(plane_type);
    Command::redo();
}

void MoveEditorPlaneCommand::undo() {
    IsometricEditorPlugin* isometric_editor_plugin = IsometricEditorPlugin::get_instance();
    isometric_editor_plugin->get_editor_plane_for_map(context_node, plane_type).set_position(old_position);
    isometric_editor_plugin->refresh(plane_type);
    Command::undo();
}

void MoveEditorPlaneCommand::set_new_position(int p_new_position) {
    new_position = p_new_position;
}

void MoveEditorPlaneCommand::set_old_position(int p_old_position) {
    old_position = p_old_position;
}

void MoveEditorPlaneCommand::set_plane_type(EditorPlane::PlaneType p_plane_type) {
    plane_type = p_plane_type;
}

#endif
