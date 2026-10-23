#ifdef TOOLS_ENABLED

#include "outline_drawer.h"

#include "isometric_server.h"
#include "utils/isometric_maths.h"

#include <classes/rendering_server.hpp>
#include <variant/packed_color_array.hpp>

using namespace godot;
using namespace editor;

static PackedColorArray single_color(const Color& p_color) {
    PackedColorArray colors;
    colors.push_back(p_color);
    return colors;
}

void OutlineDrawer::draw_outline(node::IsometricPositionable* positionable) {
    const OutlineData& outline_data = positionable->get_outline_data();
    RID outline_rid = outline_data.rid;
    RenderingServer* rendering_server = RenderingServer::get_singleton();
    rendering_server->canvas_item_clear(outline_rid);
    rendering_server->canvas_item_set_parent(outline_rid, positionable->get_canvas_item());
    rendering_server->canvas_item_set_z_as_relative_to_parent(outline_rid, true);

    RID space_rid = positionable->get_space_rid();
    if (!space_rid.is_valid()) { return; }
    const data::IsometricParameters* space_configuration = ISOMETRIC_SERVER->space_get_configuration(space_rid);
    if (!space_configuration) { return; }

    PackedVector2Array points = utils::get_bounding_box(*space_configuration, positionable->get_size());

    // Each top corner sits either on the upper face or, when the slope lowers it, on the bottom face.
    int no_slope = 0;
    int left_slope = 0;
    int right_slope = 0;
    int forward_slope = 0;
    int backward_slope = 0;
    switch (positionable->get_slope_type()) {
        case node::IsometricPositionable::SlopeType::NONE:
            no_slope = 1;
            break;
        case node::IsometricPositionable::SlopeType::LEFT:
            left_slope = 1;
            break;
        case node::IsometricPositionable::SlopeType::RIGHT:
            right_slope = 1;
            break;
        case node::IsometricPositionable::SlopeType::FORWARD:
            forward_slope = 1;
            break;
        case node::IsometricPositionable::SlopeType::BACKWARD:
            backward_slope = 1;
            break;
        case node::IsometricPositionable::SlopeType::SLOPE_TYPE_MAX:
            break;
    }

    PackedVector2Array up_points;
    up_points.push_back((no_slope + left_slope + forward_slope) * points[4] + (right_slope + backward_slope) * points[0]);
    up_points.push_back((no_slope + right_slope + forward_slope) * points[5] + (left_slope + backward_slope) * points[1]);
    up_points.push_back((no_slope + right_slope + backward_slope) * points[6] + (left_slope + forward_slope) * points[2]);
    up_points.push_back((no_slope + left_slope + backward_slope) * points[7] + (right_slope + forward_slope) * points[3]);

    PackedVector2Array down_points;
    down_points.push_back(points[0]);
    down_points.push_back(points[1]);
    down_points.push_back(points[2]);
    down_points.push_back(points[3]);

    if (outline_data.should_draw_polygons) {
        bool is_dynamic = positionable->get_is_dynamic();
        Color up_color = is_dynamic ? Color(0.5, 0.5, 0.5) : Color(0.9, 0.9, 0.9);
        Color left_color = is_dynamic ? Color(0.2, 0.2, 0.2) : Color(0.6, 0.6, 0.6);
        Color right_color = is_dynamic ? Color(0.4, 0.4, 0.4) : Color(0.8, 0.8, 0.8);

        PackedVector2Array right_points;
        right_points.push_back(up_points[1]);
        right_points.push_back(down_points[1]);
        right_points.push_back(down_points[2]);
        right_points.push_back(up_points[2]);

        PackedVector2Array left_points;
        left_points.push_back(up_points[2]);
        left_points.push_back(down_points[2]);
        left_points.push_back(down_points[3]);
        left_points.push_back(up_points[3]);

        rendering_server->canvas_item_add_polygon(outline_rid, up_points, single_color(up_color));
        rendering_server->canvas_item_add_polygon(outline_rid, left_points, single_color(left_color));
        rendering_server->canvas_item_add_polygon(outline_rid, right_points, single_color(right_color));
    }

    const Color& color = outline_data.color;
    real_t line_size = outline_data.line_size;

    // Upper lines
    rendering_server->canvas_item_add_line(outline_rid, up_points[0], up_points[1], color, line_size);
    rendering_server->canvas_item_add_line(outline_rid, up_points[1], up_points[2], color, line_size);
    rendering_server->canvas_item_add_line(outline_rid, up_points[2], up_points[3], color, line_size);
    rendering_server->canvas_item_add_line(outline_rid, up_points[3], up_points[0], color, line_size);

    // Vertical lines; the back one is hidden.
    rendering_server->canvas_item_add_line(outline_rid, up_points[1], down_points[1], color, line_size);
    rendering_server->canvas_item_add_line(outline_rid, up_points[2], down_points[2], color, line_size);
    rendering_server->canvas_item_add_line(outline_rid, up_points[3], down_points[3], color, line_size);

    // Lower lines; the back ones are hidden.
    rendering_server->canvas_item_add_line(outline_rid, down_points[1], down_points[2], color, line_size);
    rendering_server->canvas_item_add_line(outline_rid, down_points[2], down_points[3], color, line_size);

    rendering_server->canvas_item_set_visible(outline_rid, outline_data.is_visible);
}

void OutlineDrawer::set_outline_visible(node::IsometricPositionable* positionable, bool visible) {
    OutlineData& outline_data = positionable->get_outline_data();
    outline_data.is_visible = visible;
    RenderingServer::get_singleton()->canvas_item_set_visible(outline_data.rid, visible);
}

#endif
