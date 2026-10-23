#ifndef ISOMETRIC_MAPS_LAYERS_EDITOR_H
#define ISOMETRIC_MAPS_LAYERS_EDITOR_H

#ifdef TOOLS_ENABLED

#include <classes/button_group.hpp>
#include <classes/check_box.hpp>
#include <classes/confirmation_dialog.hpp>
#include <classes/grid_container.hpp>
#include <classes/label.hpp>
#include <classes/line_edit.hpp>
#include <classes/v_box_container.hpp>

namespace editor::inspector {
    using namespace godot;

    class LayersEditor : public VBoxContainer {
        GDCLASS(LayersEditor, VBoxContainer)

        LineEdit* layer_line_edit;
        GridContainer* layer_controls_container;
        ConfirmationDialog* remove_layer_popup;
        Label* remove_layer_popup_label;
        Ref<ButtonGroup> current_layer_button_group;
        uint32_t layer_id_to_remove = 0;
        String layer_name_to_remove;

        void _add_layer();
        void _on_remove_layer_button(uint32_t p_layer_id, const String& p_layer_name);
        void _on_layer_name_submitted(const String& p_text, LineEdit* p_line_edit, uint32_t p_layer_id);
        void _on_layer_name_edited(LineEdit* p_line_edit, uint32_t p_layer_id);
        void _on_layer_select_content(uint32_t p_layer_id);
        void _set_layer_visible(uint32_t p_layer_id, CheckBox* p_check_box);
        void _set_current_layer(uint32_t p_layer_id);
        void _layer_color_changed(const Color& p_color, uint32_t p_layer_id);
        void _remove_layer();

    protected:
        static void _bind_methods() {}

    public:
        void refresh();

        LayersEditor();
    };
}// namespace editor::inspector

#endif
#endif// ISOMETRIC_MAPS_LAYERS_EDITOR_H
