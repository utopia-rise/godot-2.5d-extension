#ifndef ISOMETRIC_MAPS_SELECT_POSITIONABLE_COMMAND_H
#define ISOMETRIC_MAPS_SELECT_POSITIONABLE_COMMAND_H

#ifdef TOOLS_ENABLED

#include "command.h"

namespace editor::commands {
    // Toggles the selection of the positionable at a position.
    class SelectPositionableCommand : public Command {
        GDCLASS(SelectPositionableCommand, Command)

    protected:
        static void _bind_methods() {}

    private:
        Vector3 position;
        bool should_deselect_first = false;

        Vector<Vector3> selected_cache;

    public:
        void redo() override;
        void undo() override;

        void set_position(const Vector3& p_position);
        void set_should_deselect_first(bool p_should);
    };
}// namespace editor::commands

#endif
#endif// ISOMETRIC_MAPS_SELECT_POSITIONABLE_COMMAND_H
