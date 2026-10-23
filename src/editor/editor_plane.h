#ifndef ISOMETRIC_MAPS_EDITOR_PLANE_H
#define ISOMETRIC_MAPS_EDITOR_PLANE_H

#ifdef TOOLS_ENABLED

#include <variant/rid.hpp>
#include <variant/vector2.hpp>
#include <variant/vector3.hpp>

namespace editor {
    using namespace godot;

    // A plane drawn over a map in the editor: the edition grid, or one of the six view limiters.
    class EditorPlane {
    public:
        enum PlaneType {
            EDITOR_DRAWER,
            X_MIN_VIEW_LIMITER,
            X_MAX_VIEW_LIMITER,
            Y_MIN_VIEW_LIMITER,
            Y_MAX_VIEW_LIMITER,
            Z_MIN_VIEW_LIMITER,
            Z_MAX_VIEW_LIMITER,
            SIZE
        };

        static PlaneType get_min_view_limiter(Vector3::Axis p_axis) {
            return static_cast<PlaneType>(X_MIN_VIEW_LIMITER + 2 * p_axis);
        }

        static PlaneType get_max_view_limiter(Vector3::Axis p_axis) {
            return static_cast<PlaneType>(X_MAX_VIEW_LIMITER + 2 * p_axis);
        }

        int get_position() const;
        void set_position(int p_position);
        Vector3::Axis get_axis() const;
        void set_axis(Vector3::Axis p_axis);
        RID get_rid() const;
        const Vector2& get_size() const;

        EditorPlane(int p_position, Vector3::Axis p_axis, const Vector2& p_size);

    private:
        int position;
        Vector3::Axis axis;
        Vector2 size;
        RID self;
    };
}// namespace editor

#endif
#endif// ISOMETRIC_MAPS_EDITOR_PLANE_H
