#ifndef ISOMETRIC_MAPS_COMMAND_EMITTER_H
#define ISOMETRIC_MAPS_COMMAND_EMITTER_H

#ifdef TOOLS_ENABLED

#include "editor/commands/command.h"

#include <classes/input_event.hpp>

namespace editor::commands::emitters {
    // Turns canvas input events of type Evt into commands committed as one editor action. Derived implements
    // from_gui_input_to_command.
    template<class Derived, class Evt>
    class CommandEmitter {
        const char* action_title;
        UndoRedo::MergeMode merge_mode;
        bool backward_undo_ops;

    public:
        explicit CommandEmitter(const char* p_action_title, UndoRedo::MergeMode p_merge_mode = UndoRedo::MERGE_DISABLE, bool p_backward_undo_ops = false) :
          action_title(p_action_title),
          merge_mode(p_merge_mode),
          backward_undo_ops(p_backward_undo_ops) {}

        void on_gui_input(const Ref<InputEvent>& p_event, node::IsometricMap* p_context) {
            Ref<Evt> event = p_event;
            if (event.is_null()) { return; }

            Command::commit_as_action(static_cast<Derived*>(this)->from_gui_input_to_command(event), p_context, action_title, merge_mode, backward_undo_ops);
        }
    };
}// namespace editor::commands::emitters

#endif
#endif// ISOMETRIC_MAPS_COMMAND_EMITTER_H
