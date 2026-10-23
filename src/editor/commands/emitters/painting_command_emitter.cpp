#ifdef TOOLS_ENABLED

#include "painting_command_emitter.h"

#include "editor/commands/add_positionable_command.h"
#include "editor/editor_utils.h"
#include "editor/isometric_editor_plugin.h"

#include <classes/input.hpp>

using namespace godot;
using namespace editor::commands::emitters;

Vector<Ref<editor::commands::Command>> PaintingCommandEmitter::from_gui_input_to_command([[maybe_unused]] const Ref<InputEventMouse>& p_event) {
    Vector<Ref<Command>> commands;

    _clear_current_preview_node();

    IsometricEditorPlugin* isometric_editor_plugin = IsometricEditorPlugin::get_instance();
    node::IsometricMap* map = isometric_editor_plugin->get_selected_map();

    int selected_tile_id = isometric_editor_plugin->get_selection_pane()->get_selected_positionable_id();
    if (selected_tile_id == resource::PositionableSet::NONE_POSITIONABLE_ID) { return commands; }

    Vector3 size;
    if (!EditorUtils::get_positionable_size(map->get_positionable_set(), selected_tile_id, size)) { return commands; }

    AABB aabb(isometric_editor_plugin->get_mouse_position_3d(), size);
    if (!map->is_aabb_in_map(aabb) || !isometric_editor_plugin->is_aabb_in_view_limiters(aabb) || map->is_overlapping(aabb)) {
        return commands;
    }

    if (!Input::get_singleton()->is_mouse_button_pressed(MOUSE_BUTTON_LEFT)) {
        current_preview_node = Object::cast_to<node::IsometricPositionable>(
          map->get_positionable_set()->get_positionable_scene_for_id(selected_tile_id)->instantiate()
        );
        current_preview_node->set_modulate(Color(1, 1, 1, 0.5));
        current_preview_node->set_is_dynamic(true);
        current_preview_node->set_local_position_3d(aabb.position);
        map->add_child(current_preview_node);
        return commands;
    }

    Ref<AddPositionableCommand> add_command;
    add_command.instantiate();
    add_command->set_aabb(aabb);
    add_command->set_positionable_id(selected_tile_id);
    add_command->set_layer_id(isometric_editor_plugin->get_selected_layer());
    commands.push_back(add_command);
    return commands;
}

void PaintingCommandEmitter::_clear_current_preview_node() {
    if (!current_preview_node) { return; }
    if (Node* parent = current_preview_node->get_parent()) { parent->remove_child(current_preview_node); }
    memdelete(current_preview_node);
    current_preview_node = nullptr;
}

PaintingCommandEmitter::~PaintingCommandEmitter() {
    _clear_current_preview_node();
}

#endif
