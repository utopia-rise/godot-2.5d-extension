#ifdef TOOLS_ENABLED

#include "command.h"

#include <classes/editor_interface.hpp>
#include <classes/editor_undo_redo_manager.hpp>

using namespace godot;
using namespace editor::commands;

void Command::redo() {
    _execute_hooks();
}

void Command::undo() {
    _execute_hooks();
}

void Command::set_context_node(node::IsometricMap* p_context_node) {
    context_node = p_context_node;
}

void Command::add_hook(const Callable& p_hook) {
    hooks.push_back(p_hook);
}

void Command::_execute_hooks() const {
    for (const Callable& hook : hooks) {
        hook.call();
    }
}

void Command::commit_as_action(
  const Vector<Ref<Command>>& p_commands,
  node::IsometricMap* p_context,
  const String& p_action_title,
  UndoRedo::MergeMode p_merge_mode,
  bool p_backward_undo_ops
) {
    EditorUndoRedoManager* undo_redo = EditorInterface::get_singleton()->get_editor_undo_redo();
    bool has_valid_command = false;
    for (const Ref<Command>& command : p_commands) {
        if (command.is_null()) { continue; }
        if (!has_valid_command) {
            undo_redo->create_action(p_action_title, p_merge_mode, p_context, p_backward_undo_ops);
            has_valid_command = true;
        }
        command->set_context_node(p_context);
        undo_redo->add_do_method(command.ptr(), "redo");
        undo_redo->add_undo_method(command.ptr(), "undo");
    }

    if (has_valid_command) { undo_redo->commit_action(); }
}

void Command::_bind_methods() {
    ClassDB::bind_method(D_METHOD("redo"), &Command::redo);
    ClassDB::bind_method(D_METHOD("undo"), &Command::undo);
}

#endif
