#ifndef ISOMETRIC_MAPS_MOVE_TO_LAYER_COMMAND_H
#define ISOMETRIC_MAPS_MOVE_TO_LAYER_COMMAND_H

#ifdef TOOLS_ENABLED

#include "command.h"

namespace editor::commands {
    class MoveToLayerCommand : public Command {
        GDCLASS(MoveToLayerCommand, Command)

    protected:
        static void _bind_methods() {}

    private:
        Vector3 position;
        uint32_t new_layer_id = node::IsometricMap::NO_LAYER_ID;
        uint32_t former_layer_id = node::IsometricMap::NO_LAYER_ID;

    public:
        void redo() override;
        void undo() override;

        void set_position(const Vector3& p_position);
        void set_new_layer_id(uint32_t p_new_layer_id);
        void set_former_layer_id(uint32_t p_former_layer_id);
    };
}// namespace editor::commands

#endif
#endif// ISOMETRIC_MAPS_MOVE_TO_LAYER_COMMAND_H
