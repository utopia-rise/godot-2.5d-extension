#ifndef ISOMETRIC_MAPS_DELETE_COMMAND_EMITTER_H
#define ISOMETRIC_MAPS_DELETE_COMMAND_EMITTER_H

#ifdef TOOLS_ENABLED

#include "command_emitter.h"

#include <classes/input_event_key.hpp>

namespace editor::commands::emitters {
    class DeleteCommandEmitter : public CommandEmitter<DeleteCommandEmitter, InputEventKey> {
        friend class CommandEmitter<DeleteCommandEmitter, InputEventKey>;

        Vector<Ref<Command>> from_gui_input_to_command(const Ref<InputEventKey>& p_event);

    public:
        DeleteCommandEmitter() : CommandEmitter("Delete positionable") {}
    };
}// namespace editor::commands::emitters

#endif
#endif// ISOMETRIC_MAPS_DELETE_COMMAND_EMITTER_H
