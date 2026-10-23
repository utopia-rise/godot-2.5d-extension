#ifndef ISOMETRIC_MAPS_MOVE_EDITOR_PLANE_COMMAND_H
#define ISOMETRIC_MAPS_MOVE_EDITOR_PLANE_COMMAND_H

#ifdef TOOLS_ENABLED

#include "command.h"
#include "editor/editor_plane.h"

namespace editor::commands {
    class MoveEditorPlaneCommand : public Command {
        GDCLASS(MoveEditorPlaneCommand, Command)

    protected:
        static void _bind_methods() {}

    private:
        EditorPlane::PlaneType plane_type = EditorPlane::PlaneType::SIZE;
        int new_position = 0;
        int old_position = 0;

    public:
        void redo() override;
        void undo() override;

        void set_new_position(int p_new_position);
        void set_old_position(int p_old_position);
        void set_plane_type(EditorPlane::PlaneType p_plane_type);
    };
}// namespace editor::commands

#endif
#endif// ISOMETRIC_MAPS_MOVE_EDITOR_PLANE_COMMAND_H
