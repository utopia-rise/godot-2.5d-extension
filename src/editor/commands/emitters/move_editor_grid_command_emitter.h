#ifndef ISOMETRIC_MAPS_MOVE_EDITOR_GRID_COMMAND_EMITTER_H
#define ISOMETRIC_MAPS_MOVE_EDITOR_GRID_COMMAND_EMITTER_H

#ifdef TOOLS_ENABLED

#include "command_emitter.h"

#include <classes/input_event_key.hpp>

namespace editor::commands::emitters {
    class MoveEditorGridCommandEmitter : public CommandEmitter<MoveEditorGridCommandEmitter, InputEventKey> {
        friend class CommandEmitter<MoveEditorGridCommandEmitter, InputEventKey>;

        Vector<Ref<Command>> from_gui_input_to_command(const Ref<InputEventKey>& p_event);

    public:
        MoveEditorGridCommandEmitter() : CommandEmitter("Move editor grid") {}
    };
}// namespace editor::commands::emitters

#endif
#endif// ISOMETRIC_MAPS_MOVE_EDITOR_GRID_COMMAND_EMITTER_H
