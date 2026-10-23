#ifndef ISOMETRIC_MAPS_ISOMETRIC_EDITOR_PLUGIN_H
#define ISOMETRIC_MAPS_ISOMETRIC_EDITOR_PLUGIN_H

#ifdef TOOLS_ENABLED

#include "editor/commands/emitters/delete_command_emitter.h"
#include "editor/commands/emitters/drag_and_drop_command_emitter.h"
#include "editor/commands/emitters/fill_plan_command_emitter.h"
#include "editor/commands/emitters/move_editor_grid_command_emitter.h"
#include "editor/commands/emitters/move_editor_view_limiter_command_emitter.h"
#include "editor/commands/emitters/move_selection_command_emitter.h"
#include "editor/commands/emitters/painting_command_emitter.h"
#include "editor/commands/emitters/rotate_editor_plane_command_emitter.h"
#include "editor/commands/emitters/select_all_command_emitter.h"
#include "editor/commands/emitters/select_command_emitter.h"
#include "editor/inspector/layers_editor.h"
#include "editor/inspector/positionable_selection_pane.h"
#include "editor_plane.h"
#include "node/isometric_map.h"

#include <classes/button.hpp>
#include <classes/color_picker_button.hpp>
#include <classes/editor_plugin.hpp>
#include <classes/h_box_container.hpp>
#include <classes/option_button.hpp>
#include <classes/scene_tree_timer.hpp>
#include <templates/hash_map.hpp>

namespace editor {
    using namespace godot;

    // Edits IsometricMap nodes on the 2D canvas: edition grid, view limiters, layers, and the select, paint and
    // drag-and-drop modes.
    class IsometricEditorPlugin : public EditorPlugin {
        GDCLASS(IsometricEditorPlugin, EditorPlugin)

    public:
        enum Mode {
            NONE,
            SELECT,
            PAINT,
            DRAG_AND_DROP,
            FILL
        };

    private:
        struct MapHandlingData {
            EditorPlane editor_planes[EditorPlane::PlaneType::SIZE];
            Ref<SceneTreeTimer> plane_timers[EditorPlane::PlaneType::SIZE];
            bool is_grid[EditorPlane::PlaneType::SIZE];

            MapHandlingData();
            explicit MapHandlingData(const Vector3& p_map_size);
        };

        static IsometricEditorPlugin* singleton;

        inspector::PositionableSelectionPane* positionable_selection_pane = nullptr;
        inspector::LayersEditor* layers_editor = nullptr;
        Button* positionable_pane_button = nullptr;
        HBoxContainer* toolbar = nullptr;
        ColorPickerButton* grid_color_picker_button = nullptr;
        OptionButton* edition_mode_button = nullptr;
        Button* debug_button = nullptr;

        HashMap<node::IsometricMap*, MapHandlingData> handling_data_map;
        node::IsometricMap* selected_map = nullptr;

        bool show_debug = false;
        Mode current_mode = Mode::NONE;

        bool should_clear_buffer_on_next_frame = false;

        commands::emitters::PaintingCommandEmitter painting_command_emitter;
        commands::emitters::SelectCommandEmitter select_command_emitter;
        commands::emitters::SelectAllCommandEmitter select_all_command_emitter;
        commands::emitters::DeleteCommandEmitter delete_command_emitter;
        commands::emitters::MoveSelectionCommandEmitter move_selection_command_emitter;
        commands::emitters::DragAndDropCommandEmitter drag_and_drop_command_emitter;
        commands::emitters::FillPlanCommandEmitter fill_plan_command_emitter;
        commands::emitters::MoveEditorGridCommandEmitter move_editor_drawer_command_emitter;
        commands::emitters::RotateEditorPlaneCommandEmitter rotate_editor_plane_command_emitter;
        commands::emitters::MoveEditorViewLimiterCommandEmitter plane_view_limiter_command_emitter;

        void _setup_ui();
        void _drop();
        void _edit_map(const NodePath& p_path);

        void _on_frame_post_draw();
        void _on_edition_mode_changed(int selected_index);
        void _on_plane_visibility_timeout(int p_plane_type);
        void _on_map_size_changed();
        void _on_move_selection_to_current_layer();
        void _on_grid_color_picker_change(const Color& p_color);

        void _draw_edition_grid() const;
        void _draw_plane(EditorPlane::PlaneType p_plane_type);
        void _set_plane_timer(EditorPlane::PlaneType p_plane_type, float p_delay);

    protected:
        void _notification(int p_what);
        static void _bind_methods();

    public:
        IsometricEditorPlugin();
        ~IsometricEditorPlugin() override;

        static IsometricEditorPlugin* get_instance();

        bool _forward_canvas_gui_input(const Ref<InputEvent>& p_event) override;
        void _edit(Object* p_object) override;
        bool _handles(Object* p_object) const override;
        void _clear() override;
        void _make_visible(bool p_visible) override;

        inspector::PositionableSelectionPane* get_selection_pane() const;
        void set_debug_mode(bool p_debug);
        void set_should_clear_buffer_on_next_frame(bool should);
        void refresh(int p_plane_type);

        node::IsometricMap* get_selected_map() const;
        EditorPlane& get_editor_plane_for_map(node::IsometricMap* p_map, EditorPlane::PlaneType p_plane_type);
        Vector3 get_mouse_position_3d();
        bool is_aabb_in_view_limiters(const AABB& p_aabb) const;
        uint32_t get_selected_layer() const;
    };
}// namespace editor

#endif
#endif// ISOMETRIC_MAPS_ISOMETRIC_EDITOR_PLUGIN_H
