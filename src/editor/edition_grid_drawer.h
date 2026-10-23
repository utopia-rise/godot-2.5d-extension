#ifndef ISOMETRIC_MAPS_EDITION_GRID_DRAWER_H
#define ISOMETRIC_MAPS_EDITION_GRID_DRAWER_H

#ifdef TOOLS_ENABLED

#include "editor_plane.h"
#include "node/isometric_map.h"

namespace editor {
    class EditionGridDrawer {
    public:
        static void draw_grid(const EditorPlane& editor_plane, const node::IsometricMap* map, const Color& p_color);
        static void draw_plane(const EditorPlane& p_editor_plane, const node::IsometricMap* map);
        static void clear_for_editor_plane(const EditorPlane& editor_plane);
    };
}// namespace editor

#endif
#endif// ISOMETRIC_MAPS_EDITION_GRID_DRAWER_H
