#ifdef TOOLS_ENABLED

#include "edition_grid_drawer.h"

#include "isometric_server.h"

#include <classes/rendering_server.hpp>
#include <variant/packed_color_array.hpp>
#include <variant/packed_vector2_array.hpp>
#include <variant/transform2d.hpp>

using namespace godot;
using namespace editor;

// Places the plane's canvas item over the map, offset to the plane position along its axis.
static void prepare_plane_canvas_item(RID p_canvas_item, const EditorPlane& p_editor_plane, const node::IsometricMap* p_map, const data::IsometricParameters& p_parameters) {
    RenderingServer* rendering_server = RenderingServer::get_singleton();
    rendering_server->canvas_item_clear(p_canvas_item);
    rendering_server->canvas_item_set_parent(p_canvas_item, p_map->get_canvas_item());

    auto diamond_width = static_cast<float>(p_parameters.diamond_width);
    auto diamond_height = static_cast<float>(p_parameters.diamond_height);
    Vector3 map_size = p_map->get_size();
    Vector3::Axis axis = p_editor_plane.get_axis();
    float plane_position = MIN(static_cast<float>(p_editor_plane.get_position()), map_size[axis]);

    Vector2 offset;
    switch (axis) {
        case Vector3::AXIS_X:
            offset = Vector2(-diamond_width * 0.5f * plane_position, -diamond_height * 0.5f * plane_position);
            break;
        case Vector3::AXIS_Y:
            offset = Vector2(diamond_width * 0.5f * plane_position, -diamond_height * 0.5f * plane_position);
            break;
        case Vector3::AXIS_Z:
            offset = Vector2(0, p_parameters.z_length * plane_position);
            break;
    }
    Vector2 global_offset(0, -diamond_height * 0.5f);
    rendering_server->canvas_item_set_transform(p_canvas_item, Transform2D().translated(global_offset - offset));
}

void EditionGridDrawer::draw_grid(const EditorPlane& editor_plane, const node::IsometricMap* map, const Color& p_color) {
    const data::IsometricParameters* parameters = ISOMETRIC_SERVER->space_get_configuration(map->get_space_rid());
    if (!parameters) { return; }

    RID rid = editor_plane.get_rid();
    prepare_plane_canvas_item(rid, editor_plane, map, *parameters);

    RenderingServer* rendering_server = RenderingServer::get_singleton();
    auto diamond_width = static_cast<float>(parameters->diamond_width);
    auto diamond_height = static_cast<float>(parameters->diamond_height);
    float tile_z_length = parameters->z_length;
    Vector3 map_size = map->get_size();

    switch (editor_plane.get_axis()) {
        case Vector3::AXIS_X:
            // Drawn along the X axis using the map size on Y and Z.
            for (int i = 0; i < static_cast<int>(map_size.y) + 1; i++) {
                auto index = static_cast<float>(i);
                Vector2 from(-diamond_width * 0.5f * index, diamond_height * 0.5f * index);
                Vector2 to(-diamond_width * 0.5f * index, diamond_height * 0.5f * index - tile_z_length * map_size.z);
                rendering_server->canvas_item_add_line(rid, from, to, p_color, 2.0);
            }
            for (int i = 0; i < static_cast<int>(map_size.z) + 1; i++) {
                auto index = static_cast<float>(i);
                Vector2 from(0, -tile_z_length * index);
                Vector2 to(-diamond_width * 0.5f * map_size.y, diamond_height * 0.5f * map_size.y - tile_z_length * index);
                rendering_server->canvas_item_add_line(rid, from, to, p_color, 2.0);
            }
            break;
        case Vector3::AXIS_Y:
            // Drawn along the Y axis using the map size on X and Z.
            for (int i = 0; i < static_cast<int>(map_size.z) + 1; i++) {
                auto index = static_cast<float>(i);
                Vector2 from(0, -tile_z_length * index);
                Vector2 to(diamond_width * 0.5f * map_size.x, diamond_height * 0.5f * map_size.x - tile_z_length * index);
                rendering_server->canvas_item_add_line(rid, from, to, p_color, 2.0);
            }
            for (int i = 0; i < static_cast<int>(map_size.x) + 1; i++) {
                auto index = static_cast<float>(i);
                Vector2 from(diamond_width * 0.5f * index, diamond_height * 0.5f * index);
                Vector2 to(diamond_width * 0.5f * index, diamond_height * 0.5f * index - tile_z_length * map_size.z);
                rendering_server->canvas_item_add_line(rid, from, to, p_color, 2.0);
            }
            break;
        case Vector3::AXIS_Z:
            // Drawn along the Z axis using the map size on X and Y.
            for (int i = 0; i < static_cast<int>(map_size.y) + 1; i++) {
                auto index = static_cast<float>(i);
                Vector2 from(-diamond_width * 0.5f * index, diamond_height * 0.5f * index);
                Vector2 to(diamond_width * 0.5f * (map_size.x - index), diamond_height * 0.5f * (index + map_size.x));
                rendering_server->canvas_item_add_line(rid, from, to, p_color, 2.0);
            }
            for (int i = 0; i < static_cast<int>(map_size.x) + 1; i++) {
                auto index = static_cast<float>(i);
                Vector2 from(diamond_width * 0.5f * index, diamond_height * 0.5f * index);
                Vector2 to(diamond_width * 0.5f * (index - map_size.y), diamond_height * 0.5f * (map_size.y + index));
                rendering_server->canvas_item_add_line(rid, from, to, p_color, 2.0);
            }
            break;
    }
}

void EditionGridDrawer::draw_plane(const EditorPlane& p_editor_plane, const node::IsometricMap* map) {
    const data::IsometricParameters* parameters = ISOMETRIC_SERVER->space_get_configuration(map->get_space_rid());
    if (!parameters) { return; }

    RID rid = p_editor_plane.get_rid();
    prepare_plane_canvas_item(rid, p_editor_plane, map, *parameters);

    auto diamond_width = static_cast<float>(parameters->diamond_width);
    auto diamond_height = static_cast<float>(parameters->diamond_height);
    float tile_z_length = parameters->z_length;
    Vector3 map_size = map->get_size();

    PackedVector2Array polygon_points;
    switch (p_editor_plane.get_axis()) {
        case Vector3::AXIS_X:
            polygon_points.push_back(Vector2(0, 0));
            polygon_points.push_back(Vector2(0, -tile_z_length * map_size.z));
            polygon_points.push_back(
              Vector2(-diamond_width * 0.5f * map_size.y, diamond_height * 0.5f * map_size.y - tile_z_length * map_size.z)
            );
            polygon_points.push_back(Vector2(-diamond_width * 0.5f * map_size.y, diamond_height * 0.5f * map_size.y));
            break;
        case Vector3::AXIS_Y:
            polygon_points.push_back(Vector2(0, 0));
            polygon_points.push_back(Vector2(diamond_width * 0.5f * map_size.x, diamond_height * 0.5f * map_size.x));
            polygon_points.push_back(
              Vector2(diamond_width * 0.5f * map_size.x, diamond_height * 0.5f * map_size.x - tile_z_length * map_size.z)
            );
            polygon_points.push_back(Vector2(0, -tile_z_length * map_size.z));
            break;
        case Vector3::AXIS_Z:
            polygon_points.push_back(Vector2(0, 0));
            polygon_points.push_back(Vector2(-diamond_width * 0.5f * map_size.y, diamond_height * 0.5f * map_size.y));
            polygon_points.push_back(
              Vector2(diamond_width * 0.5f * (map_size.x - map_size.y), diamond_height * 0.5f * (map_size.y + map_size.x))
            );
            polygon_points.push_back(Vector2(diamond_width * 0.5f * map_size.x, diamond_height * 0.5f * map_size.x));
            break;
    }

    PackedColorArray colors;
    colors.push_back(Color(0, 0, 0, 0.2));
    RenderingServer::get_singleton()->canvas_item_add_polygon(rid, polygon_points, colors);
}

void EditionGridDrawer::clear_for_editor_plane(const EditorPlane& editor_plane) {
    RenderingServer::get_singleton()->canvas_item_clear(editor_plane.get_rid());
}

#endif
