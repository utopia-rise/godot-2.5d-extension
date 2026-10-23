#ifdef TOOLS_ENABLED

#include "revert_command.h"

using namespace godot;
using namespace editor::commands;

void RevertCommand::redo() {
    reverse_command->undo();
    Command::redo();
}

void RevertCommand::undo() {
    reverse_command->redo();
    Command::undo();
}

void RevertCommand::set_context_node(node::IsometricMap* p_context_node) {
    Command::set_context_node(p_context_node);
    reverse_command->set_context_node(p_context_node);
}

void RevertCommand::set_reverse_command(const Ref<Command>& p_reverse_command) {
    reverse_command = p_reverse_command;
}

#endif
