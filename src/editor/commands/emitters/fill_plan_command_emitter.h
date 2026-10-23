#ifndef ISOMETRIC_MAPS_FILL_PLAN_COMMAND_EMITTER_H
#define ISOMETRIC_MAPS_FILL_PLAN_COMMAND_EMITTER_H

#ifdef TOOLS_ENABLED

#include "command_emitter.h"

#include <classes/input_event_mouse.hpp>

namespace editor::commands::emitters {
    // Fill bucket: a click fills the free area of the editor plane connected to the clicked cell.
    class FillPlanCommandEmitter : public CommandEmitter<FillPlanCommandEmitter, InputEventMouse> {
        friend class CommandEmitter<FillPlanCommandEmitter, InputEventMouse>;

        Vector<Ref<Command>> from_gui_input_to_command(const Ref<InputEventMouse>& p_event);

    public:
        FillPlanCommandEmitter() : CommandEmitter("Fill plan with elements") {}
    };
}// namespace editor::commands::emitters

#endif
#endif// ISOMETRIC_MAPS_FILL_PLAN_COMMAND_EMITTER_H
