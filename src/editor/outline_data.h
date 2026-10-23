#ifndef ISOMETRIC_MAPS_OUTLINE_DATA_H
#define ISOMETRIC_MAPS_OUTLINE_DATA_H

#ifdef TOOLS_ENABLED

#include <variant/color.hpp>
#include <variant/rid.hpp>

namespace editor {
    struct OutlineData {
        godot::RID rid;
        bool is_visible = false;
        godot::Color color;
        real_t line_size = 0;
        bool should_draw_polygons = false;

        OutlineData();
    };
}// namespace editor

#endif
#endif// ISOMETRIC_MAPS_OUTLINE_DATA_H
