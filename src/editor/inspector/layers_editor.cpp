#ifdef TOOLS_ENABLED

#include "layers_editor.h"

#include "editor/commands/add_layer_command.h"
#include "editor/commands/add_positionable_command.h"
#include "editor/commands/change_layer_name_command.h"
#include "editor/commands/revert_command.h"
#include "editor/commands/select_positionable_command.h"
#include "editor/commands/set_layer_visibility_command.h"
#include "editor/isometric_editor_plugin.h"
#include "editor/positionable_selector_manager.h"

#include <classes/button.hpp>
#include <classes/color_picker_button.hpp>
#include <classes/h_box_container.hpp>
#include <classes/scroll_container.hpp>
#include <variant/typed_array.hpp>

using namespace godot;
using namespace editor::inspector;

static constexpr const char* ADD_LAYER_ACTION_NAME = "Add layer";
static constexpr const char* REMOVE_LAYER_ACTION_NAME = "Remove layer";
static constexpr const char* SET_LAYER_VISIBLE_ACTION_NAME = "Set layer visible";
static constexpr const char* CHANGE_LAYER_NAME_ACTION_NAME_FORMAT = "Change layer name (id: %s)";
static constexpr const char* SELECT_LAYER_CONTENT_ACTION_NAME = "Select layer content";

static Label* create_label(const String& p_text) {
    auto* label = memnew(Label);
    label->set_text(p_text);
    return label;
}

LayersEditor::LayersEditor() :
  layer_line_edit(memnew(LineEdit)),
  layer_controls_container(memnew(GridContainer)),
  remove_layer_popup(memnew(ConfirmationDialog)),
  remove_layer_popup_label(memnew(Label)) {
    set_name("Isometric layers");

    auto* top_bar = memnew(HBoxContainer);
    layer_line_edit->set_h_size_flags(SIZE_EXPAND_FILL);
    auto* add_button = memnew(Button);
    add_button->set_text("+");
    top_bar->add_child(layer_line_edit);
    top_bar->add_child(add_button);
    add_button->connect("pressed", callable_mp(this, &LayersEditor::_add_layer));
    add_child(top_bar);

    auto* scroll_container = memnew(ScrollContainer);
    layer_controls_container->set_columns(6);
    layer_controls_container->set_h_size_flags(SIZE_EXPAND_FILL);
    scroll_container->add_child(layer_controls_container);
    scroll_container->set_h_size_flags(SIZE_EXPAND_FILL);
    scroll_container->set_v_size_flags(SIZE_EXPAND_FILL);
    add_child(scroll_container);

    current_layer_button_group.instantiate();

    set_h_size_flags(SIZE_EXPAND_FILL);
    set_v_size_flags(SIZE_EXPAND_FILL);

    remove_layer_popup->set_exclusive(true);
    remove_layer_popup->add_child(remove_layer_popup_label);
    remove_layer_popup->connect("confirmed", callable_mp(this, &LayersEditor::_remove_layer));
    add_child(remove_layer_popup);
}

void LayersEditor::refresh() {
    TypedArray<Node> children = layer_controls_container->get_children();
    for (int i = 0; i < children.size(); ++i) {
        Node* child = Object::cast_to<Node>(children[i]);
        layer_controls_container->remove_child(child);
        child->queue_free();
    }

    node::IsometricMap* current_map = IsometricEditorPlugin::get_instance()->get_selected_map();
    if (!current_map) { return; }

    layer_controls_container->add_child(create_label("Current"));
    layer_controls_container->add_child(create_label("Layer name"));
    layer_controls_container->add_child(create_label("Color"));
    layer_controls_container->add_child(create_label("Visible"));
    layer_controls_container->add_child(create_label("Select content"));
    layer_controls_container->add_child(create_label("Remove"));

    uint32_t last_layer_edited = current_map->get_meta(node::IsometricMap::LAST_EDITED_LAYER_META_NAME, node::IsometricMap::DEFAULT_LAYER_ID);

    Dictionary layers = current_map->get_layers();
    Array ids = layers.keys();
    for (int i = 0; i < ids.size(); ++i) {
        uint32_t layer_id = ids[i];
        String layer_name = layers[ids[i]];

        auto* current_layer_check_box = memnew(CheckBox);
        current_layer_check_box->connect("pressed", callable_mp(this, &LayersEditor::_set_current_layer).bind(layer_id));
        current_layer_check_box->set_button_group(current_layer_button_group);
        layer_controls_container->add_child(current_layer_check_box);

        if (layer_id == node::IsometricMap::DEFAULT_LAYER_ID) {
            layer_controls_container->add_child(create_label(layer_name));
        } else {
            auto* layer_name_line_edit = memnew(LineEdit);
            layer_name_line_edit->set_text(layer_name);
            Callable on_name_edited = callable_mp(this, &LayersEditor::_on_layer_name_edited).bind(layer_name_line_edit, layer_id);
            layer_name_line_edit->connect(
              "text_submitted",
              callable_mp(this, &LayersEditor::_on_layer_name_submitted).bind(layer_name_line_edit, layer_id)
            );
            layer_name_line_edit->connect("focus_exited", on_name_edited);
            layer_controls_container->add_child(layer_name_line_edit);
        }

        auto* color_picker_button = memnew(ColorPickerButton);
        color_picker_button->connect("color_changed", callable_mp(this, &LayersEditor::_layer_color_changed).bind(layer_id));
        Color layer_color = current_map->get_meta(vformat(node::IsometricMap::LAYER_COLOR_META_NAME_FORMAT, layer_id), Color());
        color_picker_button->set_pick_color(layer_color);
        current_map->set_layer_color(layer_id, layer_color);
        layer_controls_container->add_child(color_picker_button);

        auto* visible_check_box = memnew(CheckBox);
        visible_check_box->set_pressed(true);
        visible_check_box->connect("pressed", callable_mp(this, &LayersEditor::_set_layer_visible).bind(layer_id, visible_check_box));
        layer_controls_container->add_child(visible_check_box);

        auto* select_content_button = memnew(Button);
        select_content_button->set_text("Select");
        select_content_button->connect("pressed", callable_mp(this, &LayersEditor::_on_layer_select_content).bind(layer_id));
        layer_controls_container->add_child(select_content_button);

        auto* layer_remove_button = memnew(Button);
        layer_remove_button->set_text("-");
        layer_remove_button->connect("pressed", callable_mp(this, &LayersEditor::_on_remove_layer_button).bind(layer_id, layer_name));
        layer_controls_container->add_child(layer_remove_button);

        current_layer_check_box->set_pressed(layer_id == last_layer_edited);
    }
}

void LayersEditor::_add_layer() {
    Ref<commands::AddLayerCommand> add_layer_command;
    add_layer_command.instantiate();
    add_layer_command->set_layer_name(layer_line_edit->get_text());
    add_layer_command->add_hook(callable_mp(this, &LayersEditor::refresh));

    Vector<Ref<commands::Command>> commands;
    commands.push_back(add_layer_command);
    commands::Command::commit_as_action(commands, IsometricEditorPlugin::get_instance()->get_selected_map(), ADD_LAYER_ACTION_NAME);
    layer_line_edit->clear();
}

void LayersEditor::_on_layer_name_submitted([[maybe_unused]] const String& p_text, LineEdit* p_line_edit, uint32_t p_layer_id) {
    _on_layer_name_edited(p_line_edit, p_layer_id);
}

// Renames are committed when editing ends, not on each keystroke: the undo history refreshes this editor, which
// recreates the line edits.
void LayersEditor::_on_layer_name_edited(LineEdit* p_line_edit, uint32_t p_layer_id) {
    node::IsometricMap* current_map = IsometricEditorPlugin::get_instance()->get_selected_map();
    if (!current_map) { return; }

    String new_layer_name = p_line_edit->get_text();
    String former_layer_name = current_map->get_layer_name(p_layer_id);
    if (new_layer_name.is_empty() || new_layer_name == former_layer_name) { return; }

    Ref<commands::ChangeLayerNameCommand> change_layer_name_command;
    change_layer_name_command.instantiate();
    change_layer_name_command->set_layer_id(p_layer_id);
    change_layer_name_command->set_new_layer_name(new_layer_name);
    change_layer_name_command->set_former_layer_name(former_layer_name);

    Vector<Ref<commands::Command>> commands;
    commands.push_back(change_layer_name_command);
    commands::Command::commit_as_action(commands, current_map, vformat(CHANGE_LAYER_NAME_ACTION_NAME_FORMAT, p_layer_id), UndoRedo::MERGE_ALL);
}

void LayersEditor::_on_layer_select_content(uint32_t p_layer_id) {
    node::IsometricMap* current_map = IsometricEditorPlugin::get_instance()->get_selected_map();
    if (!current_map) { return; }

    Vector<Ref<commands::Command>> commands;
    for (const Vector3& position : PositionableSelectorManager::get_singleton()->get_selected_for_map(current_map)) {
        Ref<commands::SelectPositionableCommand> select_command;
        select_command.instantiate();
        select_command->set_position(position);

        Ref<commands::RevertCommand> deselect_command;
        deselect_command.instantiate();
        deselect_command->set_reverse_command(select_command);
        commands.push_back(deselect_command);
    }

    for (const Vector3& position : current_map->get_layer_positions(p_layer_id)) {
        Ref<commands::SelectPositionableCommand> select_command;
        select_command.instantiate();
        select_command->set_position(position);
        commands.push_back(select_command);
    }

    commands::Command::commit_as_action(commands, current_map, SELECT_LAYER_CONTENT_ACTION_NAME, UndoRedo::MERGE_DISABLE, true);
}

void LayersEditor::_on_remove_layer_button(uint32_t p_layer_id, const String& p_layer_name) {
    remove_layer_popup_label->set_text(vformat(
      "You're going to remove layer %s.\n"
      "This will remove all tiles included in layer from map.\n"
      "Please confirm removal.",
      p_layer_name
    ));
    layer_id_to_remove = p_layer_id;
    layer_name_to_remove = p_layer_name;
    remove_layer_popup->popup_centered();
}

void LayersEditor::_set_layer_visible(uint32_t p_layer_id, CheckBox* p_check_box) {
    Ref<commands::SetLayerVisibilityCommand> visibility_command;
    visibility_command.instantiate();
    visibility_command->set_layer_id(p_layer_id);
    visibility_command->set_visible(p_check_box->is_pressed());

    Vector<Ref<commands::Command>> commands;
    commands.push_back(visibility_command);
    commands::Command::commit_as_action(commands, IsometricEditorPlugin::get_instance()->get_selected_map(), SET_LAYER_VISIBLE_ACTION_NAME);
}

void LayersEditor::_set_current_layer(uint32_t p_layer_id) {
    if (node::IsometricMap* current_map = IsometricEditorPlugin::get_instance()->get_selected_map()) {
        current_map->set_meta(node::IsometricMap::LAST_EDITED_LAYER_META_NAME, p_layer_id);
    }
}

void LayersEditor::_layer_color_changed(const Color& p_color, uint32_t p_layer_id) {
    if (node::IsometricMap* map = IsometricEditorPlugin::get_instance()->get_selected_map()) {
        map->set_layer_color(p_layer_id, p_color);
    }
}

void LayersEditor::_remove_layer() {
    node::IsometricMap* current_map = IsometricEditorPlugin::get_instance()->get_selected_map();
    if (!current_map) { return; }

    Vector<Ref<commands::Command>> commands;

    Vector3 map_size = current_map->get_size();
    for (int x = 0; x < static_cast<int>(map_size.x); ++x) {
        for (int y = 0; y < static_cast<int>(map_size.y); ++y) {
            for (int z = 0; z < static_cast<int>(map_size.z); ++z) {
                Vector3 position_in_map(x, y, z);
                if (current_map->get_layer_id_at(position_in_map) != layer_id_to_remove) { continue; }

                node::IsometricPositionable* positionable = current_map->get_positionable_at(position_in_map);
                if (!positionable) { continue; }

                Ref<commands::AddPositionableCommand> add_command;
                add_command.instantiate();
                add_command->set_aabb(AABB(position_in_map, positionable->get_size()));
                add_command->set_positionable_id(current_map->get_positionable_id_for_position(position_in_map));
                add_command->set_layer_id(layer_id_to_remove);

                Ref<commands::RevertCommand> delete_command;
                delete_command.instantiate();
                delete_command->set_reverse_command(add_command);
                commands.push_back(delete_command);
            }
        }
    }

    Ref<commands::AddLayerCommand> add_layer_command;
    add_layer_command.instantiate();
    add_layer_command->set_layer_id(layer_id_to_remove);
    add_layer_command->set_layer_name(layer_name_to_remove);

    Ref<commands::RevertCommand> remove_layer_command;
    remove_layer_command.instantiate();
    remove_layer_command->set_reverse_command(add_layer_command);
    remove_layer_command->add_hook(callable_mp(this, &LayersEditor::refresh));
    commands.push_back(remove_layer_command);

    commands::Command::commit_as_action(commands, current_map, REMOVE_LAYER_ACTION_NAME);
}

#endif
