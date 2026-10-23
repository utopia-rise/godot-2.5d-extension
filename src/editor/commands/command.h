#ifndef ISOMETRIC_MAPS_COMMAND_H
#define ISOMETRIC_MAPS_COMMAND_H

#ifdef TOOLS_ENABLED

#include "node/isometric_map.h"

#include <classes/ref_counted.hpp>
#include <classes/undo_redo.hpp>
#include <templates/vector.hpp>
#include <variant/callable.hpp>

namespace editor::commands {
    using namespace godot;

    // An undoable edit of an IsometricMap. Commands are registered in the editor's undo/redo history, which calls
    // their bound redo and undo methods.
    class Command : public RefCounted {
        GDCLASS(Command, RefCounted)

    protected:
        node::IsometricMap* context_node = nullptr;

        static void _bind_methods();

    public:
        virtual void redo();
        virtual void undo();
        virtual void set_context_node(node::IsometricMap* p_context_node);
        void add_hook(const Callable& p_hook);

        // Registers the commands as one action of the editor history, then runs them. Null commands are skipped and no
        // action is created when none remains.
        static void commit_as_action(
          const Vector<Ref<Command>>& p_commands,
          node::IsometricMap* p_context,
          const String& p_action_title,
          UndoRedo::MergeMode p_merge_mode = UndoRedo::MERGE_DISABLE,
          bool p_backward_undo_ops = false
        );

    private:
        Vector<Callable> hooks;

        void _execute_hooks() const;
    };
}// namespace editor::commands

#endif
#endif// ISOMETRIC_MAPS_COMMAND_H
