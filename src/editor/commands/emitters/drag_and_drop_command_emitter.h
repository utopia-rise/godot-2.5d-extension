#ifndef ISOMETRIC_MAPS_DRAG_AND_DROP_COMMAND_EMITTER_H
#define ISOMETRIC_MAPS_DRAG_AND_DROP_COMMAND_EMITTER_H

#ifdef TOOLS_ENABLED

#include "command_emitter.h"

#include <classes/input_event_mouse.hpp>

namespace editor::commands::emitters {
    class DragAndDropCommandEmitter : public CommandEmitter<DragAndDropCommandEmitter, InputEventMouse> {
        friend class CommandEmitter<DragAndDropCommandEmitter, InputEventMouse>;

        Vector<node::IsometricPositionable*> current_preview_nodes;

        Vector3 initial_position = Vector3(-1, -1, -1);
        Vector3 limit_position = Vector3(-1, -1, -1);

        Vector<Ref<Command>> from_gui_input_to_command(const Ref<InputEventMouse>& p_event);

        void _clear_current_preview_nodes(int64_t new_size);

        static AABB _calculate_real_aabb(const Vector3& initial_position, const Vector3& limit_position, const Vector3& positionable_size);

        static Vector<Vector3> _calculate_positionables_positions(const Vector3& initial_position, const Vector3& limit_position, const Vector3& positionable_size);

    public:
        DragAndDropCommandEmitter() : CommandEmitter("Add positionables with drag and drop") {}

        ~DragAndDropCommandEmitter();
    };
}// namespace editor::commands::emitters

#endif
#endif// ISOMETRIC_MAPS_DRAG_AND_DROP_COMMAND_EMITTER_H
