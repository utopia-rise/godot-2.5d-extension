#ifndef ISOMETRIC_MAPS_MOVE_EDITOR_VIEW_LIMITER_COMMAND_EMITTER_H
#define ISOMETRIC_MAPS_MOVE_EDITOR_VIEW_LIMITER_COMMAND_EMITTER_H

#ifdef TOOLS_ENABLED

#include "command_emitter.h"

#ifdef MACOS_ENABLED
#include <classes/input_event_pan_gesture.hpp>
#else
#include <classes/input_event_mouse_button.hpp>
#endif

namespace editor::commands::emitters {
    // Trackpads scroll with pan gestures on macOS, mice with wheel buttons elsewhere.
#ifdef MACOS_ENABLED
    using ScrollInputEvent = InputEventPanGesture;
#else
    using ScrollInputEvent = InputEventMouseButton;
#endif

    class MoveEditorViewLimiterCommandEmitter : public CommandEmitter<MoveEditorViewLimiterCommandEmitter, ScrollInputEvent> {
        friend class CommandEmitter<MoveEditorViewLimiterCommandEmitter, ScrollInputEvent>;

        enum EventMotion {
            FORWARD,
            BACKWARD,
            NONE
        };

#ifdef MACOS_ENABLED
        real_t cumulative_scroll_delta = 0;
        uint64_t cumulative_scroll_last_time = 0;
#endif

        Vector<Ref<Command>> from_gui_input_to_command(const Ref<ScrollInputEvent>& p_event);

        static bool _is_event_activated(const Ref<ScrollInputEvent>& p_event);

        EventMotion _get_event_motion(const Ref<ScrollInputEvent>& p_event);

    public:
        MoveEditorViewLimiterCommandEmitter() : CommandEmitter("Move view limiter") {}
    };
}// namespace editor::commands::emitters

#endif
#endif// ISOMETRIC_MAPS_MOVE_EDITOR_VIEW_LIMITER_COMMAND_EMITTER_H
