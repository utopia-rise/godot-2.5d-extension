#ifndef ISOMETRIC_MAPS_PAINTING_COMMAND_EMITTER_H
#define ISOMETRIC_MAPS_PAINTING_COMMAND_EMITTER_H

#ifdef TOOLS_ENABLED

#include "command_emitter.h"

#include <classes/input_event_mouse.hpp>

namespace editor::commands::emitters {
    class PaintingCommandEmitter : public CommandEmitter<PaintingCommandEmitter, InputEventMouse> {
        friend class CommandEmitter<PaintingCommandEmitter, InputEventMouse>;

        node::IsometricPositionable* current_preview_node = nullptr;

        Vector<Ref<Command>> from_gui_input_to_command(const Ref<InputEventMouse>& p_event);

        void _clear_current_preview_node();

    public:
        PaintingCommandEmitter() : CommandEmitter("Paint isometric elements") {}

        ~PaintingCommandEmitter();
    };
}// namespace editor::commands::emitters

#endif
#endif// ISOMETRIC_MAPS_PAINTING_COMMAND_EMITTER_H
