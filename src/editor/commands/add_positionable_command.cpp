#ifdef TOOLS_ENABLED

#include "add_positionable_command.h"

using namespace godot;
using namespace editor::commands;

void AddPositionableCommand::redo() {
    context_node->add_positionable_if_nothing_present(aabb, positionable_id, layer_id);
    Command::redo();
}

void AddPositionableCommand::undo() {
    context_node->remove_positionable(aabb);
    Command::undo();
}

void AddPositionableCommand::set_aabb(const AABB& p_aabb) {
    aabb = p_aabb;
}

void AddPositionableCommand::set_positionable_id(int p_id) {
    positionable_id = p_id;
}

void AddPositionableCommand::set_layer_id(uint32_t p_layer_id) {
    layer_id = p_layer_id;
}

#endif
