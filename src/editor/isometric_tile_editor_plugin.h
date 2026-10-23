#ifndef ISOMETRIC_MAPS_ISOMETRIC_TILE_EDITOR_PLUGIN_H
#define ISOMETRIC_MAPS_ISOMETRIC_TILE_EDITOR_PLUGIN_H

#ifdef TOOLS_ENABLED

#include "node/isometric_positionable.h"

#include <classes/color_picker_button.hpp>
#include <classes/editor_plugin.hpp>
#include <classes/h_box_container.hpp>

namespace editor {
    using namespace godot;

    // Outlines the edited positionable (a tile, not a map) on the canvas.
    class IsometricTileEditorPlugin : public EditorPlugin {
        GDCLASS(IsometricTileEditorPlugin, EditorPlugin)

        node::IsometricPositionable* selected_positionable = nullptr;
        HBoxContainer* toolbar = nullptr;
        ColorPickerButton* color_picker_button = nullptr;

        void _on_size_changed();
        void _on_color_picker_change(const Color& p_color);

    protected:
        void _notification(int p_what);

        static void _bind_methods() {}

    public:
        bool _handles(Object* p_object) const override;
        void _edit(Object* p_object) override;
        void _make_visible(bool p_visible) override;
    };
}// namespace editor

#endif
#endif// ISOMETRIC_MAPS_ISOMETRIC_TILE_EDITOR_PLUGIN_H
