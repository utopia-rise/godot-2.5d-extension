#ifndef ISOMETRIC_MAPS_ISOMETRIC_POSITIONABLE_H
#define ISOMETRIC_MAPS_ISOMETRIC_POSITIONABLE_H

#include <classes/collision_object3d.hpp>
#include <classes/node2d.hpp>
#include <variant/node_path.hpp>
#include <variant/rid.hpp>
#include <variant/vector3.hpp>

#ifdef TOOLS_ENABLED
#include "editor/outline_data.h"

#include <classes/box_mesh.hpp>
#include <classes/mesh_instance3d.hpp>
#include <classes/standard_material3d.hpp>
#endif

namespace node {
    using namespace godot;

    class IsometricPositionable : public Node2D {
        GDCLASS(IsometricPositionable, Node2D);

    public:
        enum class SlopeType {
            NONE = 0,
            LEFT = 1,
            RIGHT = 2,
            FORWARD = 3,
            BACKWARD = 4,
            SLOPE_TYPE_MAX = 5
        };

    private:
        Vector3 size = Vector3(1, 1, 1);
        Vector3 local_position;
        int depth = 1;

        SlopeType slope_type = SlopeType::NONE;

        RID self;
        bool is_dynamic = false;
        NodePath collision_object_node_path;
        CollisionObject3D* collision_object = nullptr;

        void set_global_position_3d(const Vector3& p_position);

        void _rebind_collision_object_position() const;

        void on_ready();
        void on_physics_process();
        void on_exit_tree();

#ifdef TOOLS_ENABLED
        bool debug_view = false;
        editor::OutlineData outline_data;

        MeshInstance3D* debug_mesh_instance_3d = nullptr;
        Ref<BoxMesh> debug_mesh;
        Ref<StandardMaterial3D> debug_mesh_material;

        void _rebind_debug_mesh_instance() const;
#endif

    protected:
        RID world;
        bool world_owner = false;
        bool is_container = false;

        void update_position();

        virtual void on_enter_tree();

        void _notification(int p_what);
        static void _bind_methods();

    public:
        IsometricPositionable() = default;
        ~IsometricPositionable() override;

        Vector3 get_local_position_3d() const;
        void set_local_position_3d(const Vector3& p_local);
        Vector3 get_global_position_3d() const;
        Vector3 get_size() const;
        virtual void set_size(const Vector3& p_size);
        int get_depth() const;
        void set_depth(int p_depth);
        RID get_space_rid() const;
        SlopeType get_slope_type() const;
        bool get_is_dynamic() const;
        void set_is_dynamic(bool p_is_dynamic);
        NodePath get_collision_object_node_path() const;
        void set_collision_object_node_path(const NodePath& p_node_path);
        RID get_isometric_element_rid() const;

#ifdef TOOLS_ENABLED
        void set_debug_view(bool p_debug);

        editor::OutlineData& get_outline_data();
        void update_debug_mesh_color(const Color& p_color) const;
#endif

#ifdef DEBUG_ENABLED
        static const Color CONFLICT_MODULATE_COLOR;

        // Tints the node in debug builds, such as a tile overlapping another one in a map.
        virtual void set_debug_modulate(const Color& p_modulate);
#endif
    };
}// namespace node

#endif// ISOMETRIC_MAPS_ISOMETRIC_POSITIONABLE_H
