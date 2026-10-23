#ifdef TOOLS_ENABLED

#include "composite_command.h"

using namespace godot;
using namespace editor::commands;

void CompositeCommand::redo() {
    for (const Ref<Command>& command : commands) {
        command->redo();
    }
    Command::redo();
}

void CompositeCommand::undo() {
    for (int64_t i = commands.size() - 1; i >= 0; --i) {
        commands[i]->undo();
    }
    Command::undo();
}

void CompositeCommand::set_context_node(node::IsometricMap* p_context_node) {
    Command::set_context_node(p_context_node);
    for (const Ref<Command>& command : commands) {
        command->set_context_node(p_context_node);
    }
}

void CompositeCommand::append_command(const Ref<Command>& p_command) {
    commands.push_back(p_command);
}

#endif
