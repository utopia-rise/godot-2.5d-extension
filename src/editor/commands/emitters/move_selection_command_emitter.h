#ifndef ISOMETRIC_MAPS_MOVE_SELECTION_COMMAND_EMITTER_H
#define ISOMETRIC_MAPS_MOVE_SELECTION_COMMAND_EMITTER_H

#ifdef TOOLS_ENABLED

#include "command_emitter.h"

#include <classes/input_event_mouse.hpp>
#include <templates/hash_map.hpp>

namespace editor::commands::emitters {
    // Shift + drag moves the selected positionables: previews follow the mouse, the release commits the move.
    class MoveSelectionCommandEmitter : public CommandEmitter<MoveSelectionCommandEmitter, InputEventMouse> {
        friend class CommandEmitter<MoveSelectionCommandEmitter, InputEventMouse>;

        HashMap<Vector3, node::IsometricPositionable*> current_preview_nodes;
        HashMap<Vector3, uint32_t> initial_layer_ids;
        Vector3 initial_mouse_position = Vector3(-1, -1, -1);
        bool is_move_valid = true;

        Vector<Ref<Command>> from_gui_input_to_command(const Ref<InputEventMouse>& p_event);

        void _clear_preview_nodes(node::IsometricMap* p_map);

    public:
        MoveSelectionCommandEmitter() : CommandEmitter("Move positionables", UndoRedo::MERGE_DISABLE, true) {}
    };
}// namespace editor::commands::emitters

#endif
#endif// ISOMETRIC_MAPS_MOVE_SELECTION_COMMAND_EMITTER_H
