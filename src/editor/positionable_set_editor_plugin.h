#ifndef ISOMETRIC_MAPS_POSITIONABLE_SET_EDITOR_PLUGIN_H
#define ISOMETRIC_MAPS_POSITIONABLE_SET_EDITOR_PLUGIN_H

#ifdef TOOLS_ENABLED

#include "editor/inspector/positionable_set_editor.h"

#include <classes/button.hpp>
#include <classes/editor_plugin.hpp>

namespace editor {
    using namespace godot;

    class PositionableSetEditorPlugin : public EditorPlugin {
        GDCLASS(PositionableSetEditorPlugin, EditorPlugin)

        inspector::PositionableSetEditor* positionable_set_editor = nullptr;
        Button* positionable_set_editor_button = nullptr;

    protected:
        void _notification(int p_what);

        static void _bind_methods() {}

    public:
        bool _handles(Object* p_object) const override;
        void _make_visible(bool p_visible) override;
        void _edit(Object* p_object) override;
    };
}// namespace editor

#endif
#endif// ISOMETRIC_MAPS_POSITIONABLE_SET_EDITOR_PLUGIN_H
