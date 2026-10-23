#ifndef ISOMETRIC_MAPS_REVERT_COMMAND_H
#define ISOMETRIC_MAPS_REVERT_COMMAND_H

#ifdef TOOLS_ENABLED

#include "command.h"

namespace editor::commands {
    // Runs another command backwards: redo undoes it and undo redoes it.
    class RevertCommand : public Command {
        GDCLASS(RevertCommand, Command)

    protected:
        static void _bind_methods() {}

    private:
        Ref<Command> reverse_command;

    public:
        void redo() override;
        void undo() override;
        void set_context_node(node::IsometricMap* p_context_node) override;

        void set_reverse_command(const Ref<Command>& p_reverse_command);
    };
}// namespace editor::commands

#endif
#endif// ISOMETRIC_MAPS_REVERT_COMMAND_H
