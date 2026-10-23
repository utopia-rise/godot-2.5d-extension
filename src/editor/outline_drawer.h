#ifndef ISOMETRIC_MAPS_OUTLINE_DRAWER_H
#define ISOMETRIC_MAPS_OUTLINE_DRAWER_H

#ifdef TOOLS_ENABLED

#include "node/isometric_positionable.h"

namespace editor {
    class OutlineDrawer {
    public:
        static void draw_outline(node::IsometricPositionable* positionable);
        static void set_outline_visible(node::IsometricPositionable* positionable, bool visible);
    };
}// namespace editor

#endif
#endif// ISOMETRIC_MAPS_OUTLINE_DRAWER_H
