#ifndef ISOMETRIC_MAPS_ROTATE_EDITOR_PLANE_COMMAND_EMITTER_H
#define ISOMETRIC_MAPS_ROTATE_EDITOR_PLANE_COMMAND_EMITTER_H

#ifdef TOOLS_ENABLED

#include "command_emitter.h"

#include <classes/input_event_key.hpp>

namespace editor::commands::emitters {
    class RotateEditorPlaneCommandEmitter : public CommandEmitter<RotateEditorPlaneCommandEmitter, InputEventKey> {
        friend class CommandEmitter<RotateEditorPlaneCommandEmitter, InputEventKey>;

        Vector<Ref<Command>> from_gui_input_to_command(const Ref<InputEventKey>& p_event);

    public:
        RotateEditorPlaneCommandEmitter() : CommandEmitter("Rotate editor plane") {}
    };
}// namespace editor::commands::emitters

#endif
#endif// ISOMETRIC_MAPS_ROTATE_EDITOR_PLANE_COMMAND_EMITTER_H
