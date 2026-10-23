#ifdef TOOLS_ENABLED

#include "drag_and_drop_command_emitter.h"

#include "editor/commands/add_positionable_command.h"
#include "editor/editor_utils.h"
#include "editor/isometric_editor_plugin.h"

#include <classes/input.hpp>

using namespace godot;
using namespace editor::commands::emitters;

static bool is_aabb_free_and_editable(node::IsometricMap* map, const AABB& aabb) {
    return map->is_aabb_in_map(aabb) && !map->is_overlapping(aabb)
        && editor::IsometricEditorPlugin::get_instance()->is_aabb_in_view_limiters(aabb);
}

Vector<Ref<editor::commands::Command>> DragAndDropCommandEmitter::from_gui_input_to_command([[maybe_unused]] const Ref<InputEventMouse>& p_event) {
    Vector<Ref<Command>> commands;
    bool is_activated = initial_position != Vector3(-1, -1, -1);

    IsometricEditorPlugin* isometric_editor_plugin = IsometricEditorPlugin::get_instance();
    int selected_tile_id = isometric_editor_plugin->get_selection_pane()->get_selected_positionable_id();
    if (selected_tile_id == resource::PositionableSet::NONE_POSITIONABLE_ID) { return commands; }

    node::IsometricMap* map = isometric_editor_plugin->get_selected_map();
    Vector3 mouse_position = isometric_editor_plugin->get_mouse_position_3d();

    Vector3 positionable_size;
    if (!EditorUtils::get_positionable_size(map->get_positionable_set(), selected_tile_id, positionable_size)) {
        return commands;
    }

    if (Input::get_singleton()->is_mouse_button_pressed(MOUSE_BUTTON_LEFT)) {
        if (is_activated) {
            if (!is_aabb_free_and_editable(map, _calculate_real_aabb(initial_position, mouse_position, positionable_size))) {
                return commands;
            }
            limit_position = mouse_position;
        } else {
            if (!is_aabb_free_and_editable(map, _calculate_real_aabb(mouse_position, mouse_position, positionable_size))) {
                return commands;
            }
            initial_position = mouse_position;
            limit_position = mouse_position;
        }

        _update_preview_nodes(
          map,
          map->get_positionable_set()->get_positionable_scene_for_id(selected_tile_id),
          _calculate_positionables_positions(initial_position, limit_position, positionable_size)
        );
        return commands;
    }

    if (!is_activated) { return commands; }

    _clear_preview_nodes();

    for (const Vector3& position : _calculate_positionables_positions(initial_position, limit_position, positionable_size)) {
        Ref<AddPositionableCommand> add_command;
        add_command.instantiate();
        add_command->set_aabb(AABB(position, positionable_size));
        add_command->set_positionable_id(selected_tile_id);
        add_command->set_layer_id(isometric_editor_plugin->get_selected_layer());
        commands.push_back(add_command);
    }

    initial_position = Vector3(-1, -1, -1);
    limit_position = Vector3(-1, -1, -1);

    return commands;
}

// Only previews entering or leaving the dragged area change, which keeps drags cheap on large maps.
void DragAndDropCommandEmitter::_update_preview_nodes(node::IsometricMap* p_map, const Ref<PackedScene>& p_scene, const Vector<Vector3>& p_positions) {
    HashSet<Vector3> wanted_positions;
    for (const Vector3& position : p_positions) {
        wanted_positions.insert(position);
    }

    Vector<Vector3> stale_positions;
    for (const KeyValue<Vector3, node::IsometricPositionable*>& entry : preview_nodes) {
        if (!wanted_positions.has(entry.key)) { stale_positions.push_back(entry.key); }
    }
    for (const Vector3& position : stale_positions) {
        node::IsometricPositionable* preview_node = preview_nodes[position];
        p_map->remove_child(preview_node);
        memdelete(preview_node);
        preview_nodes.erase(position);
    }

    for (const Vector3& position : p_positions) {
        if (preview_nodes.has(position)) { continue; }
        auto* preview_node = Object::cast_to<node::IsometricPositionable>(p_scene->instantiate());
        preview_node->set_modulate(Color(1, 1, 1, 0.5));
        preview_node->set_local_position_3d(position);
        p_map->add_child(preview_node);
        preview_nodes[position] = preview_node;
    }
}

void DragAndDropCommandEmitter::_clear_preview_nodes() {
    for (const KeyValue<Vector3, node::IsometricPositionable*>& entry : preview_nodes) {
        if (Node* parent = entry.value->get_parent()) { parent->remove_child(entry.value); }
        memdelete(entry.value);
    }
    preview_nodes.clear();
}

AABB DragAndDropCommandEmitter::_calculate_real_aabb(const Vector3& initial_position, const Vector3& limit_position, const Vector3& positionable_size) {
    Vector3 position_delta = limit_position - initial_position;
    Vector3 size;
    for (int axis = Vector3::AXIS_X; axis <= Vector3::AXIS_Z; ++axis) {
        size[axis] = position_delta[axis] < positionable_size[axis]
                     ? positionable_size[axis]
                     : Math::floor(position_delta[axis] / positionable_size[axis]) * positionable_size[axis] + 1;
    }

    IsometricEditorPlugin* isometric_editor_plugin = IsometricEditorPlugin::get_instance();
    Vector3::Axis plane_axis = isometric_editor_plugin
                                 ->get_editor_plane_for_map(isometric_editor_plugin->get_selected_map(), EditorPlane::PlaneType::EDITOR_DRAWER)
                                 .get_axis();
    // Nothing spreads across the editor plane: the drag only fills the plane itself.
    size[plane_axis] = positionable_size[plane_axis];
    return {initial_position, size};
}

Vector<Vector3> DragAndDropCommandEmitter::_calculate_positionables_positions(
  const Vector3& initial_position,
  const Vector3& limit_position,
  const Vector3& positionable_size
) {
    AABB real_aabb = _calculate_real_aabb(initial_position, limit_position, positionable_size);

    int elements_count_on_x = static_cast<int>(real_aabb.size.x / positionable_size.x);
    int elements_count_on_y = static_cast<int>(real_aabb.size.y / positionable_size.y);
    int elements_count_on_z = static_cast<int>(real_aabb.size.z / positionable_size.z);

    Vector<Vector3> positions;
    for (int i = 0; i < elements_count_on_x; ++i) {
        for (int j = 0; j < elements_count_on_y; ++j) {
            for (int k = 0; k < elements_count_on_z; ++k) {
                positions.push_back(initial_position + Vector3(i, j, k) * positionable_size);
            }
        }
    }
    return positions;
}

DragAndDropCommandEmitter::~DragAndDropCommandEmitter() {
    _clear_preview_nodes();
}

#endif
