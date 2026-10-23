#ifdef TOOLS_ENABLED

#include "move_selection_command_emitter.h"

#include "editor/commands/add_positionable_command.h"
#include "editor/commands/revert_command.h"
#include "editor/commands/select_positionable_command.h"
#include "editor/isometric_editor_plugin.h"
#include "editor/positionable_selector_manager.h"

#include <classes/input.hpp>

using namespace godot;
using namespace editor::commands::emitters;

static const Color DEFAULT_MODULATE_COLOR(1, 1, 1, 0.5);

Vector<Ref<editor::commands::Command>> MoveSelectionCommandEmitter::from_gui_input_to_command(const Ref<InputEventMouse>& p_event) {
    Vector<Ref<Command>> commands;

    IsometricEditorPlugin* isometric_editor_plugin = IsometricEditorPlugin::get_instance();
    node::IsometricMap* map = isometric_editor_plugin->get_selected_map();

    bool is_activated = initial_mouse_position != Vector3(-1, -1, -1);

    // Shift starts a move, which then goes on without it; a right click cancels it.
    if (p_event->get_button_mask().has_flag(MOUSE_BUTTON_MASK_RIGHT) || (!p_event->is_shift_pressed() && !is_activated)) {
        for (const KeyValue<Vector3, node::IsometricPositionable*>& entry : current_preview_nodes) {
            if (node::IsometricPositionable* original = map->get_positionable_at(entry.key)) {
                original->set_visible(true);
            }
        }
        _clear_preview_nodes(map);
        is_move_valid = true;
        return commands;
    }

    Vector3 mouse_position = isometric_editor_plugin->get_mouse_position_3d();

    if (Input::get_singleton()->is_mouse_button_pressed(MOUSE_BUTTON_LEFT)) {
        is_move_valid = true;

        if (!is_activated) {
            const Vector<Vector3>& selected_positions = PositionableSelectorManager::get_singleton()->get_selected_for_map(map);
            if (selected_positions.is_empty()) { return commands; }

            initial_mouse_position = mouse_position;
            for (const Vector3& position : selected_positions) {
                node::IsometricPositionable* original_positionable = map->get_positionable_at(position);
                if (!original_positionable) { continue; }
                auto* duplicate = Object::cast_to<node::IsometricPositionable>(original_positionable->duplicate());
                original_positionable->set_visible(false);
                duplicate->set_is_dynamic(true);

                map->add_child(duplicate);

                current_preview_nodes[position] = duplicate;
                initial_layer_ids[position] = map->get_layer_id_at(position);
            }
        }

        Vector3 position_delta = mouse_position - initial_mouse_position;

        for (const KeyValue<Vector3, node::IsometricPositionable*>& entry : current_preview_nodes) {
            node::IsometricPositionable* positionable = entry.value;
            Vector3 new_position = entry.key + position_delta;
            AABB new_box(new_position, positionable->get_size());

            bool overlaps_unmoved = map->has_positionable_in(new_box, map->get_positionable_at(entry.key))
                                 && !current_preview_nodes.has(new_position);
            if (overlaps_unmoved || !map->is_aabb_in_map(new_box)) {
                positionable->set_debug_modulate(node::IsometricPositionable::CONFLICT_MODULATE_COLOR);
                is_move_valid = false;
            } else {
                positionable->set_debug_modulate(DEFAULT_MODULATE_COLOR);
            }

            positionable->set_local_position_3d(new_position);
        }
        return commands;
    }

    if (!is_activated || !is_move_valid) { return commands; }

    // Every positionable leaves its cell before any lands, so moves onto cells freed by the same move are valid.
    for (const KeyValue<Vector3, node::IsometricPositionable*>& entry : current_preview_nodes) {
        const Vector3& initial_position = entry.key;

        Ref<SelectPositionableCommand> select_command;
        select_command.instantiate();
        select_command->set_position(initial_position);
        Ref<RevertCommand> deselect_command;
        deselect_command.instantiate();
        deselect_command->set_reverse_command(select_command);
        commands.push_back(deselect_command);

        Ref<AddPositionableCommand> add_command;
        add_command.instantiate();
        add_command->set_aabb(AABB(initial_position, entry.value->get_size()));
        add_command->set_positionable_id(map->get_positionable_id_for_position(initial_position));
        add_command->set_layer_id(initial_layer_ids[initial_position]);
        Ref<RevertCommand> remove_command;
        remove_command.instantiate();
        remove_command->set_reverse_command(add_command);
        commands.push_back(remove_command);
    }

    for (const KeyValue<Vector3, node::IsometricPositionable*>& entry : current_preview_nodes) {
        const Vector3& initial_position = entry.key;
        Vector3 new_position = entry.value->get_local_position_3d();

        Ref<AddPositionableCommand> add_command;
        add_command.instantiate();
        add_command->set_aabb(AABB(new_position, entry.value->get_size()));
        add_command->set_positionable_id(map->get_positionable_id_for_position(initial_position));
        add_command->set_layer_id(initial_layer_ids[initial_position]);
        commands.push_back(add_command);

        Ref<SelectPositionableCommand> select_command;
        select_command.instantiate();
        select_command->set_position(new_position);
        commands.push_back(select_command);
    }

    _clear_preview_nodes(map);
    return commands;
}

void MoveSelectionCommandEmitter::_clear_preview_nodes(node::IsometricMap* p_map) {
    for (const KeyValue<Vector3, node::IsometricPositionable*>& entry : current_preview_nodes) {
        p_map->remove_child(entry.value);
        memdelete(entry.value);
    }
    current_preview_nodes.clear();
    initial_layer_ids.clear();
    initial_mouse_position = Vector3(-1, -1, -1);
}

#endif
