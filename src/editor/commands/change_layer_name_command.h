#ifndef ISOMETRIC_MAPS_CHANGE_LAYER_NAME_COMMAND_H
#define ISOMETRIC_MAPS_CHANGE_LAYER_NAME_COMMAND_H

#ifdef TOOLS_ENABLED

#include "command.h"

namespace editor::commands {
    class ChangeLayerNameCommand : public Command {
        GDCLASS(ChangeLayerNameCommand, Command)

    protected:
        static void _bind_methods() {}

    private:
        uint32_t layer_id = node::IsometricMap::DEFAULT_LAYER_ID;
        String new_layer_name;
        String former_layer_name;

    public:
        void redo() override;
        void undo() override;

        void set_layer_id(uint32_t p_layer_id);
        void set_new_layer_name(const String& p_new_layer_name);
        void set_former_layer_name(const String& p_former_layer_name);
    };
}// namespace editor::commands

#endif
#endif// ISOMETRIC_MAPS_CHANGE_LAYER_NAME_COMMAND_H
