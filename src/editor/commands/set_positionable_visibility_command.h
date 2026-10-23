#ifndef ISOMETRIC_MAPS_SET_POSITIONABLE_VISIBILITY_COMMAND_H
#define ISOMETRIC_MAPS_SET_POSITIONABLE_VISIBILITY_COMMAND_H

#ifdef TOOLS_ENABLED

#include "command.h"

namespace editor::commands {
    // Toggles the visibility of the positionable at a position, so undo is the same operation as redo.
    class SetPositionableVisibilityCommand : public Command {
        GDCLASS(SetPositionableVisibilityCommand, Command)

    protected:
        static void _bind_methods() {}

    private:
        Vector3 position;

    public:
        void redo() override;
        void undo() override;

        void set_position(const Vector3& p_position);
    };
}// namespace editor::commands

#endif
#endif// ISOMETRIC_MAPS_SET_POSITIONABLE_VISIBILITY_COMMAND_H
