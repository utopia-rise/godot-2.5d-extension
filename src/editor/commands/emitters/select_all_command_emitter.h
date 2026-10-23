#ifndef ISOMETRIC_MAPS_SELECT_ALL_COMMAND_EMITTER_H
#define ISOMETRIC_MAPS_SELECT_ALL_COMMAND_EMITTER_H

#ifdef TOOLS_ENABLED

#include "command_emitter.h"

#include <classes/input_event_key.hpp>

namespace editor::commands::emitters {
    class SelectAllCommandEmitter : public CommandEmitter<SelectAllCommandEmitter, InputEventKey> {
        friend class CommandEmitter<SelectAllCommandEmitter, InputEventKey>;

        Vector<Ref<Command>> from_gui_input_to_command(const Ref<InputEventKey>& p_event);

    public:
        SelectAllCommandEmitter() : CommandEmitter("Select all isometric elements") {}
    };
}// namespace editor::commands::emitters

#endif
#endif// ISOMETRIC_MAPS_SELECT_ALL_COMMAND_EMITTER_H
