#ifdef TOOLS_ENABLED

#include "set_layer_visibility_command.h"

using namespace godot;
using namespace editor::commands;

void SetLayerVisibilityCommand::redo() {
    context_node->set_layer_visible(layer_id, is_visible);
    Command::redo();
}

void SetLayerVisibilityCommand::undo() {
    context_node->set_layer_visible(layer_id, !is_visible);
    Command::undo();
}

void SetLayerVisibilityCommand::set_layer_id(uint32_t p_layer_id) {
    layer_id = p_layer_id;
}

void SetLayerVisibilityCommand::set_visible(bool p_is_visible) {
    is_visible = p_is_visible;
}

#endif
