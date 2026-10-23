#ifndef ISOMETRIC_MAPS_ROTATE_EDITOR_PLANE_COMMAND_H
#define ISOMETRIC_MAPS_ROTATE_EDITOR_PLANE_COMMAND_H

#ifdef TOOLS_ENABLED

#include "command.h"

namespace editor::commands {
    class RotateEditorPlaneCommand : public Command {
        GDCLASS(RotateEditorPlaneCommand, Command)

    protected:
        static void _bind_methods() {}

    private:
        Vector3::Axis new_axis = Vector3::AXIS_X;
        Vector3::Axis former_axis = Vector3::AXIS_X;
        int former_position = 0;

        void set_axis_and_position(Vector3::Axis p_axis, int p_position);

    public:
        void redo() override;
        void undo() override;

        void set_new_axis(Vector3::Axis p_axis);
        void set_former_axis(Vector3::Axis p_axis);
        void set_former_position(int p_position);
    };
}// namespace editor::commands

#endif
#endif// ISOMETRIC_MAPS_ROTATE_EDITOR_PLANE_COMMAND_H
