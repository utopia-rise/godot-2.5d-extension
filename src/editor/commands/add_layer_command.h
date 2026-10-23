#ifndef ISOMETRIC_MAPS_ADD_LAYER_COMMAND_H
#define ISOMETRIC_MAPS_ADD_LAYER_COMMAND_H

#ifdef TOOLS_ENABLED

#include "command.h"

namespace editor::commands {
    class AddLayerCommand : public Command {
        GDCLASS(AddLayerCommand, Command)

    protected:
        static void _bind_methods() {}

    private:
        String layer_name;
        uint32_t layer_id = node::IsometricMap::NO_LAYER_ID;

    public:
        void redo() override;
        void undo() override;

        void set_layer_id(uint32_t p_layer_id);
        void set_layer_name(const String& p_layer_name);
    };
}// namespace editor::commands

#endif
#endif// ISOMETRIC_MAPS_ADD_LAYER_COMMAND_H
