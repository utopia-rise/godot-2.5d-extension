#ifndef ISOMETRIC_MAPS_POSITIONABLE_SET_EDITOR_H
#define ISOMETRIC_MAPS_POSITIONABLE_SET_EDITOR_H

#ifdef TOOLS_ENABLED

#include "resource/positionable_set.h"

#include <classes/accept_dialog.hpp>
#include <classes/button.hpp>
#include <classes/check_box.hpp>
#include <classes/file_dialog.hpp>
#include <classes/item_list.hpp>
#include <classes/line_edit.hpp>
#include <classes/option_button.hpp>
#include <classes/v_box_container.hpp>
#include <classes/window.hpp>

namespace editor::inspector {
    using namespace godot;

    // Bottom panel editing a PositionableSet: its categories and the positionable scenes in each.
    class PositionableSetEditor : public VBoxContainer {
        GDCLASS(PositionableSetEditor, VBoxContainer)

        OptionButton* category_selector;
        ItemList* contained_tiles_list;
        Button* fix_path_button;
        Window* add_category_dialog;
        LineEdit* add_category_dialog_line_edit;

        FileDialog* file_dialog;
        FileDialog* fix_path_dialog;

        AcceptDialog* alert_popup;

        Window* remove_tile_alert_popup;
        CheckBox* do_not_display_alert_remove_tile;

        Window* remove_category_alert_popup;
        CheckBox* do_not_display_alert_remove_category;

        Ref<resource::PositionableSet> current_set;

        void _on_add_category_button();
        void _on_add_category_dialog_ok_button();
        void _on_remove_category_button();
        void _on_remove_category_alert_ok_button();
        void _on_add_positionable_button();
        void _on_file_dialog_file_or_dir_selected(const String& path);
        void _on_remove_positionable_button();
        void _on_remove_tile_alert_ok_button();
        void _on_fix_path_button();
        void _on_fix_path_dialog_file_selected(const String& path);
        void _refresh_categories();
        void _on_category_selected(int index);
        void _on_positionable_selected(int index);
        void _on_save_button();
        bool _popup_if_no_category_selected();
        void _remove_selected_positionable();
        void _remove_selected_category();
        void _refresh_icons();
        void _refresh();
        void _alert(const String& p_text);

        static Button* _generate_alert_remove_dialog(Window* dialog, CheckBox* do_not_display_alert_check_box);

    protected:
        static void _bind_methods() {}

    public:
        void set_positionable_set(const Ref<resource::PositionableSet>& p_positionable_set);

        PositionableSetEditor();
        ~PositionableSetEditor() override;
    };
}// namespace editor::inspector

#endif
#endif// ISOMETRIC_MAPS_POSITIONABLE_SET_EDITOR_H
