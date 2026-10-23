#ifndef ISOMETRIC_MAPS_SET_LAYER_VISIBILITY_COMMAND_H
#define ISOMETRIC_MAPS_SET_LAYER_VISIBILITY_COMMAND_H

#ifdef TOOLS_ENABLED

#include "command.h"

namespace editor::commands {
    class SetLayerVisibilityCommand : public Command {
        GDCLASS(SetLayerVisibilityCommand, Command)

    protected:
        static void _bind_methods() {}

    private:
        uint32_t layer_id = node::IsometricMap::DEFAULT_LAYER_ID;
        bool is_visible = false;

    public:
        void redo() override;
        void undo() override;

        void set_layer_id(uint32_t p_layer_id);
        void set_visible(bool p_is_visible);
    };
}// namespace editor::commands

#endif
#endif// ISOMETRIC_MAPS_SET_LAYER_VISIBILITY_COMMAND_H
