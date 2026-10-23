#ifdef TOOLS_ENABLED

#include "select_positionable_command.h"

#include "editor/positionable_selector_manager.h"

using namespace godot;
using namespace editor::commands;

void SelectPositionableCommand::redo() {
    PositionableSelectorManager* selector_manager = PositionableSelectorManager::get_singleton();
    if (should_deselect_first) {
        selected_cache = selector_manager->get_selected_for_map(context_node);
        selector_manager->deselect_all(context_node);
    }
    if (selector_manager->is_position_selected_for_map(context_node, position)) {
        selector_manager->deselect_positionable_at(context_node, position);
    } else if (node::IsometricPositionable* positionable = context_node->get_positionable_at(position)) {
        selector_manager->select_positionable_at(context_node, positionable);
    }

    Command::redo();
}

void SelectPositionableCommand::undo() {
    PositionableSelectorManager* selector_manager = PositionableSelectorManager::get_singleton();
    if (selector_manager->is_position_selected_for_map(context_node, position)) {
        selector_manager->deselect_positionable_at(context_node, position);
    } else if (node::IsometricPositionable* positionable = context_node->get_positionable_at(position)) {
        selector_manager->select_positionable_at(context_node, positionable);
    }
    if (should_deselect_first) { selector_manager->set_selected_for_map(context_node, selected_cache); }

    Command::undo();
}

void SelectPositionableCommand::set_position(const Vector3& p_position) {
    position = p_position;
}

void SelectPositionableCommand::set_should_deselect_first(bool p_should) {
    should_deselect_first = p_should;
}

#endif
