#ifndef ISOMETRIC_MAPS_ISOMETRIC_MATHS_H
#define ISOMETRIC_MAPS_ISOMETRIC_MATHS_H

#include "data/isometric_element.h"
#include "data/isometric_parameters.h"

#include <core/math.hpp>
#include <variant/aabb.hpp>
#include <variant/packed_vector2_array.hpp>
#include <variant/vector2.hpp>
#include <variant/vector3.hpp>

namespace utils {
    using namespace godot;

    struct Hexagone {
        real_t min_x = 0;
        real_t max_x = 0;
        real_t min_y = 0;
        real_t max_y = 0;
        real_t min_h = 0;
        real_t max_h = 0;
    };

    inline Vector2 from_3D_to_screen(const data::IsometricParameters& params, const Vector3& pos) {
        return {
          (pos.x - pos.y) * static_cast<real_t>(params.diamond_width) * 0.5f,
          (pos.x + pos.y) * static_cast<real_t>(params.diamond_height) * 0.5f - params.z_length * pos.z
        };
    }

    inline Vector3 from_screen_to_3D(const data::IsometricParameters& params, const Vector2& pos, Vector3::Axis known_axis, real_t orth_known_axis) {
        real_t screen_x = pos.x;
        real_t screen_y = pos.y;
        auto diamond_width = static_cast<real_t>(params.diamond_width);
        auto diamond_height = static_cast<real_t>(params.diamond_height);

        real_t orth_x = 0;
        real_t orth_y = 0;
        real_t orth_z = 0;
        switch (known_axis) {
            case Vector3::AXIS_X:
                orth_x = orth_known_axis;
                orth_y = orth_x - 2 * screen_x / diamond_width;
                orth_z = (diamond_height * (orth_x - screen_x / diamond_width) - screen_y) / params.z_length;
                break;
            case Vector3::AXIS_Y:
                orth_y = orth_known_axis;
                orth_x = orth_y + 2 * screen_x / diamond_width;
                orth_z = (diamond_height * (orth_y + screen_x / diamond_width) - screen_y) / params.z_length;
                break;
            case Vector3::AXIS_Z:
                orth_z = orth_known_axis;
                orth_x = screen_x / diamond_width + (screen_y + orth_z * params.z_length) / diamond_height;
                orth_y = (screen_y + orth_z * params.z_length) / diamond_height - screen_x / diamond_width;
                break;
        }

        return {Math::round(orth_x), Math::round(orth_y), Math::round(orth_z)};
    }

    inline Hexagone get_hexagone_points(const data::IsometricParameters& params, const AABB& aabb) {
        const Vector3& ortho_position = aabb.position;
        const Vector3& size = aabb.size;
        Vector3 upper_point(ortho_position.x, ortho_position.y, ortho_position.z + params.z_ratio * size.z);
        Vector3 lower_point(ortho_position.x + size.x, ortho_position.y + size.y, ortho_position.z);
        Vector3 left_point(ortho_position.x, ortho_position.y + size.y, ortho_position.z);
        Vector3 right_point(ortho_position.x + size.x, ortho_position.y, ortho_position.z);

        return {
          upper_point.x - upper_point.z,
          lower_point.x - lower_point.z,
          upper_point.y - upper_point.z,
          lower_point.y - lower_point.z,
          left_point.x - left_point.y,
          right_point.x - right_point.y
        };
    }

    inline bool are_boxes_overlapping(const data::IsometricParameters& params, const AABB& box, const AABB& other) {
        Hexagone hex1 = get_hexagone_points(params, box);
        Hexagone hex2 = get_hexagone_points(params, other);
        real_t margin = params.topological_margin;

        return !(margin >= hex2.max_x - hex1.min_x || margin >= hex1.max_x - hex2.min_x)
            && !(margin >= hex2.max_y - hex1.min_y || margin >= hex1.max_y - hex2.min_y)
            && !(margin >= hex2.max_h - hex1.min_h || margin >= hex1.max_h - hex2.min_h);
    }

    inline bool is_box_in_front(const data::IsometricParameters& params, const AABB& box, const AABB& other) {
        Vector3 box_end = box.position + box.size;
        Vector3 other_end = other.position + other.size;
        real_t margin = params.topological_margin;

        if (box_end.x - other.position.x <= margin) {
            return false;
        } else if (other_end.x - box.position.x <= margin) {
            return true;
        }

        if (box_end.y - other.position.y <= margin) {
            return false;
        } else if (other_end.y - box.position.y <= margin) {
            return true;
        }

        if (box_end.z - other.position.z <= margin) {
            return false;
        } else if (other_end.z - box.position.z <= margin) {
            return true;
        }

        Vector3 distance = box.position + box_end - other.position - other_end;
        Vector3 camera_vector(1, 1, params.z_ratio);
        return distance.dot(camera_vector) >= 0;
    }

    inline PackedVector2Array get_bounding_box(const data::IsometricParameters& params, const Vector3& size) {
        real_t w = size.x;
        real_t d = size.y;
        real_t h = size.z;

        auto tile_width_float = static_cast<real_t>(params.diamond_width);
        auto tile_height_float = static_cast<real_t>(params.diamond_height);

        Vector2 offset(0, -tile_height_float * 0.5f);

        PackedVector2Array points;

        // Lower points
        points.push_back(Vector2(0, 0) + offset);
        points.push_back(Vector2(tile_width_float * 0.5f * w, tile_height_float * 0.5f * w) + offset);
        points.push_back(Vector2(tile_width_float * 0.5f * (w - d), tile_height_float * 0.5f * (d + w)) + offset);
        points.push_back(Vector2(-tile_width_float * 0.5f * d, tile_height_float * 0.5f * d) + offset);

        Vector2 height_offset(0, -params.z_length * h);

        // Upper points
        points.push_back(points[0] + height_offset);
        points.push_back(points[1] + height_offset);
        points.push_back(points[2] + height_offset);
        points.push_back(points[3] + height_offset);

        return points;
    }
}// namespace utils

#endif// ISOMETRIC_MAPS_ISOMETRIC_MATHS_H
