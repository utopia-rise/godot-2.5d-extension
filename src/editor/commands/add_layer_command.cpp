#ifdef TOOLS_ENABLED

#include "add_layer_command.h"

using namespace godot;
using namespace editor::commands;

void AddLayerCommand::redo() {
    layer_id = context_node->add_layer(layer_name, layer_id);
    Command::redo();
}

void AddLayerCommand::undo() {
    if (layer_id == node::IsometricMap::NO_LAYER_ID) {
        context_node->remove_layer(layer_name);
    } else {
        context_node->remove_layer(layer_id);
    }
    Command::undo();
}

void AddLayerCommand::set_layer_id(uint32_t p_layer_id) {
    layer_id = p_layer_id;
}

void AddLayerCommand::set_layer_name(const String& p_layer_name) {
    layer_name = p_layer_name;
}

#endif
