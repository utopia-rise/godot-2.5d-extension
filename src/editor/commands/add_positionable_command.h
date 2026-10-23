#ifndef ISOMETRIC_MAPS_ADD_POSITIONABLE_COMMAND_H
#define ISOMETRIC_MAPS_ADD_POSITIONABLE_COMMAND_H

#ifdef TOOLS_ENABLED

#include "command.h"

namespace editor::commands {
    class AddPositionableCommand : public Command {
        GDCLASS(AddPositionableCommand, Command)

    protected:
        static void _bind_methods() {}

    private:
        AABB aabb;
        int positionable_id = resource::PositionableSet::NONE_POSITIONABLE_ID;
        uint32_t layer_id = node::IsometricMap::DEFAULT_LAYER_ID;
        // Set once this command removed the positionable: re-adding it restores it even over a conflicting one.
        bool restores_removed = false;

    public:
        void redo() override;
        void undo() override;

        void set_aabb(const AABB& p_aabb);
        void set_positionable_id(int p_id);
        void set_layer_id(uint32_t p_layer_id);
    };
}// namespace editor::commands

#endif
#endif// ISOMETRIC_MAPS_ADD_POSITIONABLE_COMMAND_H
