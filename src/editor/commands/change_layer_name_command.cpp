#ifdef TOOLS_ENABLED

#include "change_layer_name_command.h"

using namespace godot;
using namespace editor::commands;

void ChangeLayerNameCommand::redo() {
    context_node->set_layer_name(layer_id, new_layer_name);
    Command::redo();
}

void ChangeLayerNameCommand::undo() {
    context_node->set_layer_name(layer_id, former_layer_name);
    Command::undo();
}

void ChangeLayerNameCommand::set_layer_id(uint32_t p_layer_id) {
    layer_id = p_layer_id;
}

void ChangeLayerNameCommand::set_new_layer_name(const String& p_new_layer_name) {
    new_layer_name = p_new_layer_name;
}

void ChangeLayerNameCommand::set_former_layer_name(const String& p_former_layer_name) {
    former_layer_name = p_former_layer_name;
}

#endif
