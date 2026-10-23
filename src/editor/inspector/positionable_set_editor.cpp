#ifdef TOOLS_ENABLED

#include "positionable_set_editor.h"

#include "editor/editor_utils.h"
#include "editor/positionable_scenes_cache_manager.h"

#include <classes/h_box_container.hpp>
#include <classes/label.hpp>
#include <classes/resource_saver.hpp>

using namespace godot;
using namespace editor::inspector;

void PositionableSetEditor::set_positionable_set(const Ref<resource::PositionableSet>& p_positionable_set) {
    current_set = p_positionable_set;
    _refresh_categories();
}

static Button* create_button(const String& p_text) {
    auto* button = memnew(Button);
    button->set_text(p_text);
    return button;
}

static void fill_dialog(Window* p_dialog, Control* p_content) {
    p_content->set_anchor(SIDE_LEFT, 0.01);
    p_content->set_anchor(SIDE_TOP, 0.01);
    p_content->set_anchor(SIDE_RIGHT, 0.99);
    p_content->set_anchor(SIDE_BOTTOM, 0.99);
    p_dialog->add_child(p_content);
    p_dialog->connect("close_requested", callable_mp(p_dialog, &Window::hide));
}

PositionableSetEditor::PositionableSetEditor() :
  category_selector(memnew(OptionButton)),
  contained_tiles_list(memnew(ItemList)),
  fix_path_button(memnew(Button)),
  add_category_dialog(memnew(Window)),
  add_category_dialog_line_edit(memnew(LineEdit)),
  file_dialog(memnew(FileDialog)),
  fix_path_dialog(memnew(FileDialog)),
  alert_popup(memnew(AcceptDialog)),
  remove_tile_alert_popup(memnew(Window)),
  do_not_display_alert_remove_tile(memnew(CheckBox)),
  remove_category_alert_popup(memnew(Window)),
  do_not_display_alert_remove_category(memnew(CheckBox)) {
    set_name("Positionable set");

    auto* select_category_container = memnew(HBoxContainer);
    auto* category_selector_label = memnew(Label);
    category_selector_label->set_text("Categories:");
    category_selector->set_h_size_flags(SIZE_EXPAND_FILL);
    Button* add_category_button = create_button("+");
    Button* remove_category_button = create_button("-");
    Button* refresh_button = create_button("Refresh");

    select_category_container->add_child(category_selector_label);
    select_category_container->add_child(category_selector);
    select_category_container->add_child(add_category_button);
    select_category_container->add_child(remove_category_button);
    select_category_container->add_child(refresh_button);
    add_child(select_category_container);

    auto* contained_tiles_label = memnew(Label);
    contained_tiles_label->set_text("Contained positionables:");
    add_child(contained_tiles_label);

    contained_tiles_list->set_h_size_flags(SIZE_EXPAND_FILL);
    contained_tiles_list->set_v_size_flags(SIZE_EXPAND_FILL);
    add_child(contained_tiles_list);

    auto* add_and_remove_tiles_container = memnew(HBoxContainer);
    fix_path_button->set_text("Fix !");
    fix_path_button->set_visible(false);
    add_and_remove_tiles_container->add_child(fix_path_button);
    Button* add_positionable_button = create_button("+");
    add_and_remove_tiles_container->add_child(add_positionable_button);
    Button* remove_positionable_button = create_button("-");
    add_and_remove_tiles_container->add_child(remove_positionable_button);
    add_and_remove_tiles_container->set_h_size_flags(SIZE_SHRINK_END);
    add_child(add_and_remove_tiles_container);

    Button* save_button = create_button("Save");
    add_child(save_button);

    add_category_dialog->set_title("Add category");
    auto* add_category_dialog_container = memnew(VBoxContainer);
    add_category_dialog_line_edit->set_h_size_flags(SIZE_EXPAND_FILL);
    Button* add_category_dialog_ok_button = create_button("Ok!");
    add_category_dialog_ok_button->set_h_size_flags(SIZE_SHRINK_CENTER);
    add_category_dialog_container->add_child(add_category_dialog_line_edit);
    add_category_dialog_container->add_child(add_category_dialog_ok_button);
    fill_dialog(add_category_dialog, add_category_dialog_container);
    add_category_dialog->set_visible(false);
    add_child(add_category_dialog);

    PackedStringArray scene_filters;
    scene_filters.push_back("*.tscn");

    file_dialog->set_filters(scene_filters);
    file_dialog->set_file_mode(FileDialog::FILE_MODE_OPEN_ANY);
    file_dialog->set_visible(false);
    add_child(file_dialog);

    fix_path_dialog->set_filters(scene_filters);
    fix_path_dialog->set_file_mode(FileDialog::FILE_MODE_OPEN_FILE);
    fix_path_dialog->set_visible(false);
    add_child(fix_path_dialog);

    alert_popup->set_visible(false);
    add_child(alert_popup);

    Button* remove_tile_alert_ok_button = _generate_alert_remove_dialog(remove_tile_alert_popup, do_not_display_alert_remove_tile);
    remove_tile_alert_popup->set_visible(false);
    add_child(remove_tile_alert_popup);

    Button* remove_category_alert_ok_button = _generate_alert_remove_dialog(remove_category_alert_popup, do_not_display_alert_remove_category);
    remove_category_alert_popup->set_visible(false);
    add_child(remove_category_alert_popup);

    category_selector->connect("item_selected", callable_mp(this, &PositionableSetEditor::_on_category_selected));
    add_category_button->connect("pressed", callable_mp(this, &PositionableSetEditor::_on_add_category_button));
    remove_category_button->connect("pressed", callable_mp(this, &PositionableSetEditor::_on_remove_category_button));
    add_category_dialog_ok_button->connect("pressed", callable_mp(this, &PositionableSetEditor::_on_add_category_dialog_ok_button));
    remove_category_alert_ok_button->connect("pressed", callable_mp(this, &PositionableSetEditor::_on_remove_category_alert_ok_button));
    refresh_button->connect("pressed", callable_mp(this, &PositionableSetEditor::_refresh));

    add_positionable_button->connect("pressed", callable_mp(this, &PositionableSetEditor::_on_add_positionable_button));
    remove_positionable_button->connect("pressed", callable_mp(this, &PositionableSetEditor::_on_remove_positionable_button));
    remove_tile_alert_ok_button->connect("pressed", callable_mp(this, &PositionableSetEditor::_on_remove_tile_alert_ok_button));

    fix_path_button->connect("pressed", callable_mp(this, &PositionableSetEditor::_on_fix_path_button));

    file_dialog->connect("dir_selected", callable_mp(this, &PositionableSetEditor::_on_file_dialog_file_or_dir_selected));
    file_dialog->connect("file_selected", callable_mp(this, &PositionableSetEditor::_on_file_dialog_file_or_dir_selected));
    fix_path_dialog->connect("file_selected", callable_mp(this, &PositionableSetEditor::_on_fix_path_dialog_file_selected));

    contained_tiles_list->connect("item_selected", callable_mp(this, &PositionableSetEditor::_on_positionable_selected));

    save_button->connect("pressed", callable_mp(this, &PositionableSetEditor::_on_save_button));

    PositionableScenesCacheManager::get_singleton()->register_control(this, callable_mp(this, &PositionableSetEditor::_refresh_icons));
}

PositionableSetEditor::~PositionableSetEditor() {
    if (PositionableScenesCacheManager* cache_manager = PositionableScenesCacheManager::get_singleton()) {
        cache_manager->unregister_control(this);
    }
}

void PositionableSetEditor::_on_add_category_button() {
    add_category_dialog->popup_centered_ratio(0.5);
}

void PositionableSetEditor::_on_add_category_dialog_ok_button() {
    add_category_dialog->set_visible(false);
    if (current_set.is_valid()) {
        String category_to_add = add_category_dialog_line_edit->get_text();
        if (!current_set->add_category(category_to_add)) {
            _alert("This category already exists !");
            return;
        }
        category_selector->add_item(category_to_add);
    }
    add_category_dialog_line_edit->set_text("");
}

void PositionableSetEditor::_on_remove_category_button() {
    if (_popup_if_no_category_selected()) { return; }

    if (!do_not_display_alert_remove_category->is_pressed()) {
        remove_category_alert_popup->popup_centered_ratio(0.4);
        return;
    }

    _remove_selected_category();
}

void PositionableSetEditor::_on_remove_category_alert_ok_button() {
    remove_category_alert_popup->set_visible(false);
    _remove_selected_category();
}

void PositionableSetEditor::_on_add_positionable_button() {
    if (_popup_if_no_category_selected()) { return; }
    file_dialog->popup_centered_ratio(0.75);
}

void PositionableSetEditor::_on_file_dialog_file_or_dir_selected(const String& path) {
    String category = category_selector->get_text();
    for (const String& positionable_path : EditorUtils::find_all_positionables_in_path(path)) {
        current_set->insert_positionable_if_not_present(category, positionable_path);
    }

    _on_category_selected(category_selector->get_selected());
}

void PositionableSetEditor::_on_remove_positionable_button() {
    if (_popup_if_no_category_selected()) { return; }

    if (contained_tiles_list->get_selected_items().is_empty()) {
        _alert("You must select a tile !");
        return;
    }

    if (!do_not_display_alert_remove_tile->is_pressed()) {
        remove_tile_alert_popup->popup_centered_ratio(0.4);
        return;
    }

    _remove_selected_positionable();
}

void PositionableSetEditor::_on_remove_tile_alert_ok_button() {
    remove_tile_alert_popup->set_visible(false);
    _remove_selected_positionable();
}

void PositionableSetEditor::_on_fix_path_button() {
    fix_path_dialog->popup_centered_ratio();
}

void PositionableSetEditor::_on_fix_path_dialog_file_selected(const String& path) {
    PackedStringArray positionables_in_path = EditorUtils::find_all_positionables_in_path(path);
    PackedInt32Array selected_items = contained_tiles_list->get_selected_items();
    if (positionables_in_path.is_empty() || selected_items.is_empty()) { return; }

    auto* metadata = Object::cast_to<PositionableItemListMetadata>(contained_tiles_list->get_item_metadata(selected_items[0]));
    if (metadata && !current_set->add_or_update_positionable(metadata->positionable_id, positionables_in_path[0])) {
        _alert("Tile already exists in set !");
    }

    _on_category_selected(category_selector->get_selected());
}

void PositionableSetEditor::_refresh_categories() {
    category_selector->clear();
    if (current_set.is_valid()) {
        for (const String& category : current_set->get_categories()) {
            category_selector->add_item(category);
        }
    }

    _on_category_selected(category_selector->get_selected());
}

void PositionableSetEditor::_on_category_selected(int index) {
    PositionableScenesCacheManager::get_singleton()->clear(this);
    contained_tiles_list->clear();

    if (index < 0) { return; }

    EditorUtils::refresh_item_list_containing_tiles_for_category(category_selector->get_item_text(index), contained_tiles_list, current_set, this);
}

void PositionableSetEditor::_on_positionable_selected(int index) {
    fix_path_button->set_visible(!EditorUtils::item_from_list_has_valid_tile_metadata(contained_tiles_list, index));
}

void PositionableSetEditor::_on_save_button() {
    if (current_set.is_valid()) { ResourceSaver::get_singleton()->save(current_set); }
}

bool PositionableSetEditor::_popup_if_no_category_selected() {
    int selected_category = category_selector->get_selected();
    if (selected_category < 0 || category_selector->get_item_text(selected_category).is_empty()) {
        _alert("You must select a category !");
        return true;
    }
    return false;
}

void PositionableSetEditor::_remove_selected_positionable() {
    int selected_item_index = contained_tiles_list->get_selected_items()[0];
    Ref<PositionableItemListMetadata> metadata = contained_tiles_list->get_item_metadata(selected_item_index);
    current_set->remove_positionable(metadata->positionable_id);
    _on_category_selected(category_selector->get_selected());
}

void PositionableSetEditor::_remove_selected_category() {
    current_set->remove_category(category_selector->get_text());
    _refresh_categories();
}

void PositionableSetEditor::_refresh_icons() {
    EditorUtils::refresh_positionable_icons_for_item_list(contained_tiles_list, this);
}

void PositionableSetEditor::_refresh() {
    _on_category_selected(category_selector->get_selected());
}

void PositionableSetEditor::_alert(const String& p_text) {
    alert_popup->set_text(p_text);
    alert_popup->popup_centered();
}

Button* PositionableSetEditor::_generate_alert_remove_dialog(Window* dialog, CheckBox* do_not_display_alert_check_box) {
    dialog->set_title("Remove from positionable set");
    auto* remove_alert_container = memnew(VBoxContainer);
    auto* alert_remove_text = memnew(Label);
    alert_remove_text->set_text(
      "You're going to remove tile(s) from set.\n"
      "Make sure those tiles are not used anymore in maps.\n"
      "If those are still in use, this will break your map and you'll have to fix it."
    );
    alert_remove_text->set_h_size_flags(SIZE_EXPAND_FILL);
    alert_remove_text->set_v_size_flags(SIZE_EXPAND_FILL);
    remove_alert_container->add_child(alert_remove_text);
    do_not_display_alert_check_box->set_text("Do not display this message anymore.");
    remove_alert_container->add_child(do_not_display_alert_check_box);
    Button* remove_alert_ok_button = create_button("Remove !");
    remove_alert_ok_button->set_h_size_flags(SIZE_SHRINK_CENTER);
    remove_alert_container->add_child(remove_alert_ok_button);
    fill_dialog(dialog, remove_alert_container);
    return remove_alert_ok_button;
}

#endif
