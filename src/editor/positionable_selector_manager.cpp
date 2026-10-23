#ifdef TOOLS_ENABLED

#include "positionable_selector_manager.h"

#include "outline_drawer.h"

using namespace godot;
using namespace editor;

PositionableSelectorManager* PositionableSelectorManager::singleton = nullptr;

PositionableSelectorManager* PositionableSelectorManager::get_singleton() {
    return singleton;
}

void PositionableSelectorManager::create() {
    singleton = memnew(PositionableSelectorManager);
}

void PositionableSelectorManager::free() {
    memdelete(singleton);
    singleton = nullptr;
}

void PositionableSelectorManager::select_positionable_at(node::IsometricMap* map, node::IsometricPositionable* positionable) {
    show_outline(map, positionable);
    map_to_selected_positions[map].push_back(positionable->get_local_position_3d());
}

void PositionableSelectorManager::deselect_positionable_at(node::IsometricMap* map, const Vector3& position) {
    node::IsometricPositionable* selected = map->get_positionable_at(position);
    if (!selected) { return; }
    OutlineDrawer::set_outline_visible(selected, false);
    map_to_selected_positions[map].erase(position);
}

void PositionableSelectorManager::deselect_all(node::IsometricMap* map) {
    Vector<Vector3>& selected_positions = map_to_selected_positions[map];
    for (const Vector3& position : selected_positions) {
        if (node::IsometricPositionable* positionable = map->get_positionable_at(position)) {
            OutlineDrawer::set_outline_visible(positionable, false);
        }
    }
    selected_positions.clear();
}

const Vector<Vector3>& PositionableSelectorManager::get_selected_for_map(node::IsometricMap* map) {
    return map_to_selected_positions[map];
}

void PositionableSelectorManager::set_selected_for_map(node::IsometricMap* map, const Vector<Vector3>& selected) {
    map_to_selected_positions[map] = selected;
    refresh_outline_for_selected(map);
}

bool PositionableSelectorManager::is_position_selected_for_map(node::IsometricMap* map, const Vector3& p_position) {
    return map_to_selected_positions[map].has(p_position);
}

void PositionableSelectorManager::refresh_outline_for_selected(node::IsometricMap* map) {
    for (const Vector3& position : map_to_selected_positions[map]) {
        if (node::IsometricPositionable* positionable = map->get_positionable_at(position)) {
            show_outline(map, positionable);
        }
    }
}

void PositionableSelectorManager::show_outline(node::IsometricMap* map, node::IsometricPositionable* positionable) {
    OutlineData& outline_data = positionable->get_outline_data();
    outline_data.color = map->get_meta(
      vformat(node::IsometricMap::LAYER_COLOR_META_NAME_FORMAT, map->get_layer_id_at(positionable->get_local_position_3d())),
      Color()
    );
    outline_data.line_size = 10;
    OutlineDrawer::draw_outline(positionable);
    OutlineDrawer::set_outline_visible(positionable, true);
}

#endif
