#ifdef TOOLS_ENABLED

#include "move_to_layer_command.h"

using namespace godot;
using namespace editor::commands;

void MoveToLayerCommand::redo() {
    context_node->set_layer_id_at(position, new_layer_id);
    Command::redo();
}

void MoveToLayerCommand::undo() {
    context_node->set_layer_id_at(position, former_layer_id);
    Command::undo();
}

void MoveToLayerCommand::set_position(const Vector3& p_position) {
    position = p_position;
}

void MoveToLayerCommand::set_new_layer_id(uint32_t p_new_layer_id) {
    new_layer_id = p_new_layer_id;
}

void MoveToLayerCommand::set_former_layer_id(uint32_t p_former_layer_id) {
    former_layer_id = p_former_layer_id;
}

#endif
