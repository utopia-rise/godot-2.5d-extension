#ifndef ISOMETRIC_MAPS_POSITIONABLE_SELECTOR_MANAGER_H
#define ISOMETRIC_MAPS_POSITIONABLE_SELECTOR_MANAGER_H

#ifdef TOOLS_ENABLED

#include "node/isometric_map.h"
#include "node/isometric_positionable.h"

#include <templates/hash_map.hpp>
#include <templates/vector.hpp>

namespace editor {
    using namespace godot;

    // Positions of the selected positionables, per map. Lives with the extension's editor initialization level.
    class PositionableSelectorManager {
        static PositionableSelectorManager* singleton;

        HashMap<node::IsometricMap*, Vector<Vector3>> map_to_selected_positions;

        static void show_outline(node::IsometricMap* map, node::IsometricPositionable* positionable);

    public:
        static PositionableSelectorManager* get_singleton();
        static void create();
        static void free();

        void select_positionable_at(node::IsometricMap* map, node::IsometricPositionable* positionable);
        void deselect_positionable_at(node::IsometricMap* map, const Vector3& position);
        void deselect_all(node::IsometricMap* map);

        const Vector<Vector3>& get_selected_for_map(node::IsometricMap* map);
        void set_selected_for_map(node::IsometricMap* map, const Vector<Vector3>& selected);
        bool is_position_selected_for_map(node::IsometricMap* map, const Vector3& p_position);
        void refresh_outline_for_selected(node::IsometricMap* map);
    };
}// namespace editor

#endif
#endif// ISOMETRIC_MAPS_POSITIONABLE_SELECTOR_MANAGER_H
