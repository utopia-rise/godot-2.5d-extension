#ifndef ISOMETRIC_MAPS_COMPOSITE_COMMAND_H
#define ISOMETRIC_MAPS_COMPOSITE_COMMAND_H

#ifdef TOOLS_ENABLED

#include "command.h"

namespace editor::commands {
    class CompositeCommand : public Command {
        GDCLASS(CompositeCommand, Command)

    protected:
        static void _bind_methods() {}

    private:
        Vector<Ref<Command>> commands;

    public:
        void redo() override;
        void undo() override;
        void set_context_node(node::IsometricMap* p_context_node) override;

        void append_command(const Ref<Command>& p_command);
    };
}// namespace editor::commands

#endif
#endif// ISOMETRIC_MAPS_COMPOSITE_COMMAND_H
