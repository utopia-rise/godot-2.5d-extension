#ifndef ISOMETRIC_MAPS_SELECT_COMMAND_EMITTER_H
#define ISOMETRIC_MAPS_SELECT_COMMAND_EMITTER_H

#ifdef TOOLS_ENABLED

#include "command_emitter.h"

#include <classes/input_event_mouse.hpp>

namespace editor::commands::emitters {
    class SelectCommandEmitter : public CommandEmitter<SelectCommandEmitter, InputEventMouse> {
        friend class CommandEmitter<SelectCommandEmitter, InputEventMouse>;

        Vector<Ref<Command>> from_gui_input_to_command(const Ref<InputEventMouse>& p_event);

    public:
        SelectCommandEmitter() : CommandEmitter("Select isometric element") {}
    };
}// namespace editor::commands::emitters

#endif
#endif// ISOMETRIC_MAPS_SELECT_COMMAND_EMITTER_H
