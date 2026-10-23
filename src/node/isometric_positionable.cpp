#include "isometric_positionable.h"

#include "isometric_server.h"
#include "isometric_string_names.h"
#include "logging.h"
#include "utils/isometric_maths.h"

#include <classes/engine.hpp>
#include <variant/transform2d.hpp>
#include <variant/transform3d.hpp>

#ifdef TOOLS_ENABLED
#include "editor/outline_drawer.h"

#include <classes/rendering_server.hpp>
#endif

using namespace godot;
using namespace node;

void IsometricPositionable::on_enter_tree() {
    add_to_group(IsometricStringNames::get_singleton()->debug_group_name);
    if (!collision_object_node_path.is_empty()) {
        if (auto* collider = Object::cast_to<CollisionObject3D>(get_node_or_null(collision_object_node_path))) {
            collision_object = collider;
            _rebind_collision_object_position();
        } else {
            LOG_ERROR(vformat("Positionable %s collision_object_node_path does not point to a valid CollisionObject3D", get_path()));
        }
    }

    if (auto* parent = Object::cast_to<IsometricPositionable>(get_parent())) {
        if (parent->world.is_valid()) {
            world = parent->world;
            world_owner = false;
        }
    }

    if (!world.is_valid()) {
        world = ISOMETRIC_SERVER->space_create();
        world_owner = true;
    }

    if (!is_container) {
        self = ISOMETRIC_SERVER->isometric_element_create(is_dynamic, AABB(get_global_position_3d(), size));
        ISOMETRIC_SERVER->isometric_element_attach_canvas_item(self, get_canvas_item(), depth);
        ISOMETRIC_SERVER->space_attach_isometric_element(world, self);
    }
    update_position();

#ifdef TOOLS_ENABLED
    if (Engine::get_singleton()->is_editor_hint() && !is_container) {
        debug_mesh_material.instantiate();
        debug_mesh_material->set_albedo(Color());

        debug_mesh.instantiate();
        debug_mesh->set_material(debug_mesh_material);

        debug_mesh_instance_3d = memnew(MeshInstance3D);
        debug_mesh_instance_3d->set_mesh(debug_mesh);

        add_child(debug_mesh_instance_3d);

        _rebind_debug_mesh_instance();
    }
#endif
}

void IsometricPositionable::on_ready() {
    if (is_dynamic && collision_object && !Engine::get_singleton()->is_editor_hint()) { set_physics_process(true); }
}

void IsometricPositionable::on_physics_process() {
    Vector3 collision_origin = collision_object->get_global_transform().origin;
    collision_origin = Vector3(collision_origin.x, collision_origin.z, collision_origin.y);
    if (!collision_origin.is_equal_approx(get_global_position_3d())) { set_global_position_3d(collision_origin); }
}

void IsometricPositionable::on_exit_tree() {
    if (self.is_valid()) {
        ISOMETRIC_SERVER->isometric_element_detach_canvas_item(self);
        ISOMETRIC_SERVER->free_rid(self);
        self = RID();
    }
    if (world_owner) {
        world_owner = false;
        ISOMETRIC_SERVER->free_rid(world);
    }
    world = RID();
    collision_object = nullptr;
    remove_from_group(IsometricStringNames::get_singleton()->debug_group_name);

#ifdef TOOLS_ENABLED
    if (debug_mesh_instance_3d) {
        debug_mesh_instance_3d->queue_free();
        debug_mesh_instance_3d = nullptr;
    }
#endif
}

void IsometricPositionable::update_position() {
    if (!world.is_valid()) { return; }

    const data::IsometricParameters* params = ISOMETRIC_SERVER->space_get_configuration(world);
    Vector2 position_2d = utils::from_3D_to_screen(*params, local_position);

    Transform2D updated_transform = get_transform();
    updated_transform.set_origin(position_2d);
    set_transform(updated_transform);
}

Vector3 IsometricPositionable::get_local_position_3d() const {
    return local_position;
}

void IsometricPositionable::set_local_position_3d(const Vector3& p_local) {
    local_position = p_local;
    if (self.is_valid() && is_dynamic) {
        ISOMETRIC_SERVER->isometric_element_set_position(self, get_global_position_3d());
    }
    update_position();
    _rebind_collision_object_position();

#ifdef TOOLS_ENABLED
    _rebind_debug_mesh_instance();
#endif
}

Vector3 IsometricPositionable::get_global_position_3d() const {
    Vector3 global_position = local_position;
    if (auto* parent = Object::cast_to<IsometricPositionable>(get_parent())) {
        global_position += parent->get_global_position_3d();
    }
    return global_position;
}

void IsometricPositionable::set_global_position_3d(const Vector3& p_position) {
    Vector3 offset = p_position - get_global_position_3d();
    set_local_position_3d(local_position + offset);
}

Vector3 IsometricPositionable::get_size() const {
    return size;
}

void IsometricPositionable::set_size(const Vector3& p_size) {
    size = p_size;
    if (self.is_valid() && is_dynamic) {
        ISOMETRIC_SERVER->isometric_element_set_size(self, size);
#ifdef TOOLS_ENABLED
        editor::OutlineDrawer::draw_outline(this);
#endif
    }
    _rebind_collision_object_position();

#ifdef TOOLS_ENABLED
    _rebind_debug_mesh_instance();

    emit_signal(IsometricStringNames::get_singleton()->size_changed_signal);
#endif
}

int IsometricPositionable::get_depth() const {
    return depth;
}

void IsometricPositionable::set_depth(int p_depth) {
    depth = p_depth;
    if (self.is_valid() && is_dynamic) { ISOMETRIC_SERVER->isometric_element_set_depth(self, p_depth); }
}

void IsometricPositionable::_notification(int p_what) {
    switch (p_what) {
        case NOTIFICATION_ENTER_TREE:
            on_enter_tree();
            break;
        case NOTIFICATION_EXIT_TREE:
            on_exit_tree();
            break;
        case NOTIFICATION_READY:
            on_ready();
            break;
        case NOTIFICATION_PHYSICS_PROCESS:
            on_physics_process();
            break;
        default:
            break;
    }
}

RID IsometricPositionable::get_space_rid() const {
    return world;
}

IsometricPositionable::SlopeType IsometricPositionable::get_slope_type() const {
    return slope_type;
}

bool IsometricPositionable::get_is_dynamic() const {
    return is_dynamic;
}

void IsometricPositionable::set_is_dynamic(bool p_is_dynamic) {
    if (p_is_dynamic == is_dynamic) { return; }
    is_dynamic = p_is_dynamic;
    if (!self.is_valid()) { return; }

    ISOMETRIC_SERVER->free_rid(self);
    self = ISOMETRIC_SERVER->isometric_element_create(is_dynamic, AABB(get_global_position_3d(), size));
    ISOMETRIC_SERVER->isometric_element_attach_canvas_item(self, get_canvas_item(), depth);
    ISOMETRIC_SERVER->space_attach_isometric_element(world, self);
    update_position();
}

NodePath IsometricPositionable::get_collision_object_node_path() const {
    return collision_object_node_path;
}

void IsometricPositionable::set_collision_object_node_path(const NodePath& p_node_path) {
    collision_object_node_path = p_node_path;
}

void IsometricPositionable::_rebind_collision_object_position() const {
    if (!collision_object) { return; }
    Vector3 global_position = get_global_position_3d();
    collision_object->set_global_transform(
      Transform3D(Basis(), Vector3(global_position.x, global_position.z, global_position.y))
    );
}

RID IsometricPositionable::get_isometric_element_rid() const {
    return self;
}

#ifdef TOOLS_ENABLED

void IsometricPositionable::_rebind_debug_mesh_instance() const {
    if (!debug_mesh_instance_3d || !Engine::get_singleton()->is_editor_hint()) { return; }

    Vector3 global_position = get_global_position_3d();
    debug_mesh->set_size(size);
    debug_mesh_instance_3d->set_global_transform(
      Transform3D(Basis(), Vector3(global_position.x, global_position.z, global_position.y) + (size - Vector3(1, 1, 1)) * 0.5)
    );
}

editor::OutlineData& IsometricPositionable::get_outline_data() {
    return outline_data;
}

void IsometricPositionable::set_debug_view(bool p_debug) {
    debug_view = p_debug;
    if (is_container) { return; }
    for (int i = 0; i < get_child_count(); i++) {
        auto* item = Object::cast_to<CanvasItem>(get_child(i));
        if (item && !Object::cast_to<IsometricPositionable>(item)) { item->set_visible(!p_debug); }
    }
    outline_data.should_draw_polygons = p_debug;
    editor::OutlineDrawer::set_outline_visible(this, p_debug);
    if (p_debug) { editor::OutlineDrawer::draw_outline(this); }
    queue_redraw();
}

void IsometricPositionable::update_debug_mesh_color(const Color& p_color) const {
    if (debug_mesh_material.is_null()) { return; }
    debug_mesh_material->set_albedo(p_color);
}

void IsometricPositionable::set_editor_modulate(const Color& p_modulate) {
    if (!self.is_valid()) { return; }
    ISOMETRIC_SERVER->isometric_element_set_editor_modulate(self, p_modulate);
}

#endif

IsometricPositionable::~IsometricPositionable() {
#ifdef TOOLS_ENABLED
    RenderingServer::get_singleton()->free_rid(outline_data.rid);
#endif
}

void IsometricPositionable::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_local_position_3d", "local_position"), &IsometricPositionable::set_local_position_3d);
    ClassDB::bind_method(D_METHOD("get_local_position_3d"), &IsometricPositionable::get_local_position_3d);
    ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "local_position_3d"), "set_local_position_3d", "get_local_position_3d");

    ClassDB::bind_method(D_METHOD("get_global_position_3d"), &IsometricPositionable::get_global_position_3d);

    ClassDB::bind_method(D_METHOD("set_size", "size"), &IsometricPositionable::set_size);
    ClassDB::bind_method(D_METHOD("get_size"), &IsometricPositionable::get_size);
    ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "size"), "set_size", "get_size");

    ClassDB::bind_method(D_METHOD("set_depth", "depth"), &IsometricPositionable::set_depth);
    ClassDB::bind_method(D_METHOD("get_depth"), &IsometricPositionable::get_depth);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "depth"), "set_depth", "get_depth");

    ClassDB::bind_method(D_METHOD("set_is_dynamic", "is_dynamic"), &IsometricPositionable::set_is_dynamic);
    ClassDB::bind_method(D_METHOD("get_is_dynamic"), &IsometricPositionable::get_is_dynamic);
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "is_dynamic"), "set_is_dynamic", "get_is_dynamic");

    ClassDB::bind_method(D_METHOD("set_collision_object_node_path", "node_path"), &IsometricPositionable::set_collision_object_node_path);
    ClassDB::bind_method(D_METHOD("get_collision_object_node_path"), &IsometricPositionable::get_collision_object_node_path);
    ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "collision_object_node_path"), "set_collision_object_node_path", "get_collision_object_node_path");

    ClassDB::bind_method(D_METHOD("get_space_rid"), &IsometricPositionable::get_space_rid);
    ClassDB::bind_method(D_METHOD("get_isometric_element_rid"), &IsometricPositionable::get_isometric_element_rid);

#ifdef TOOLS_ENABLED
    ClassDB::bind_method(D_METHOD("set_debug_view", "debug"), &IsometricPositionable::set_debug_view);
    ADD_SIGNAL(MethodInfo("size_changed"));
#endif
}
