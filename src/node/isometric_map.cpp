#include "isometric_map.h"

#include <classes/engine.hpp>
#include <classes/scene_tree.hpp>
#include <variant/typed_array.hpp>

using namespace godot;
using namespace node;

#ifdef TOOLS_ENABLED
static const char* DEFAULT_LAYER_NAME = "Default";
#endif

IsometricMap::IsometricMap() {
    is_container = true;
}

Ref<resource::PositionableSet> IsometricMap::get_positionable_set() const {
    return positionable_set;
}

void IsometricMap::set_positionable_set(const Ref<resource::PositionableSet>& p_set) {
    positionable_set = p_set;
    if (positionable_set.is_valid() && !positionable_set->is_set_loaded()) { positionable_set->preload_scenes(); }
    emit_signal("positional_set_changed", positionable_set);
}

Dictionary IsometricMap::get_layers() const {
    return layers;
}

void IsometricMap::set_layers(const Dictionary& p_layers) {
    layers = p_layers;
    layers_groups.clear();
    Array layer_ids = layers.keys();
    for (int i = 0; i < layer_ids.size(); ++i) {
        uint32_t layer_id = layer_ids[i];
        layers_groups[layer_id] = vformat("iso-map-layer-%s", layers[layer_ids[i]]);
    }
}

IsometricPositionable* IsometricMap::get_positionable_at(const Vector3& p_position) const {
    return instances_grid_3d.get_data(p_position);
}

int IsometricMap::get_positionable_id_for_position(const Vector3& p_position) const {
    return grid_3d.get_data(p_position);
}

uint32_t IsometricMap::get_layer_id_at(const Vector3& p_position) const {
    return layers_grid_3d.get_data(p_position);
}

void IsometricMap::set_size(const Vector3& p_size) {
    IsometricPositionable::set_size(p_size);
    grid_3d.update_array_size(p_size);
    instances_grid_3d.update_array_size(p_size);
    layers_grid_3d.update_array_size(p_size);
}

bool IsometricMap::is_aabb_in_map(const AABB& aabb) const {
    const Vector3& aabb_position = aabb.position;
    const Vector3& aabb_size = aabb.size;
    int pos_x = static_cast<int>(aabb_position.x);
    int pos_y = static_cast<int>(aabb_position.y);
    int pos_z = static_cast<int>(aabb_position.z);
    int max_x = pos_x + static_cast<int>(aabb_size.x) - 1;
    int max_y = pos_y + static_cast<int>(aabb_size.y) - 1;
    int max_z = pos_z + static_cast<int>(aabb_size.z) - 1;

    return pos_x >= 0 && pos_y >= 0 && pos_z >= 0 && max_x < grid_3d.get_width() && max_y < grid_3d.get_depth()
        && max_z < grid_3d.get_height();
}

bool IsometricMap::is_overlapping(const AABB& aabb) const {
    return instances_grid_3d.is_overlapping(aabb);
}

#ifdef TOOLS_ENABLED

void IsometricMap::add_positionable_if_nothing_present(const AABB& aabb, int id, uint32_t layer_id) {
    if (instances_grid_3d.is_overlapping(aabb)) { return; }

    const Vector3& aabb_position = aabb.position;
    grid_3d.set_data(aabb_position, id);
    layers_grid_3d.set_data(aabb_position, layer_id);
    add_positionable_as_child(id, aabb_position, layer_id);
}

void IsometricMap::remove_positionable(const AABB& aabb) {
    IsometricPositionable* element_to_remove = instances_grid_3d.get_data(aabb.position);
    grid_3d.set_data(aabb.position, resource::PositionableSet::NONE_POSITIONABLE_ID);
    instances_grid_3d.insert_box(aabb, nullptr, true);
    layers_grid_3d.set_data(aabb.position, DEFAULT_LAYER_ID);
    if (!element_to_remove) { return; }
    remove_child(element_to_remove);
    element_to_remove->queue_free();
}

Vector<IsometricPositionable*> IsometricMap::get_positionables_in(const AABB& p_aabb) const {
    Vector<IsometricPositionable*> ret;

    const Vector3& requested_size = p_aabb.size;
    const Vector3& map_size = get_size();
    int size_x = static_cast<int>(MIN(requested_size.x, map_size.x));
    int size_y = static_cast<int>(MIN(requested_size.y, map_size.y));
    int size_z = static_cast<int>(MIN(requested_size.z, map_size.z));
    for (int x = 0; x < size_x; ++x) {
        for (int y = 0; y < size_y; ++y) {
            for (int z = 0; z < size_z; ++z) {
                ret.push_back(instances_grid_3d.get_data(p_aabb.position + Vector3(x, y, z)));
            }
        }
    }

    return ret;
}

uint32_t IsometricMap::add_layer(const String& p_layer, uint32_t p_layer_id) {
    if (layers.values().has(p_layer)) { return NO_LAYER_ID; }

    uint32_t layer_id = p_layer_id == NO_LAYER_ID ? ++last_layer_id : p_layer_id;
    layers[layer_id] = p_layer;
    layers_groups[layer_id] = vformat("iso-map-layer-%s", p_layer);
    return layer_id;
}

void IsometricMap::remove_layer(uint32_t p_layer_id) {
    if (p_layer_id == DEFAULT_LAYER_ID) {
        return;// The default layer is never removed.
    }

    layers.erase(p_layer_id);
    layers_groups.erase(p_layer_id);
}

void IsometricMap::remove_layer(const String& p_layer_name) {
    Variant layer_id = layers.find_key(p_layer_name);
    if (layer_id.get_type() == Variant::NIL) { return; }
    remove_layer(static_cast<uint32_t>(layer_id));
}

uint32_t IsometricMap::get_last_layer_id() const {
    return last_layer_id;
}

void IsometricMap::set_last_layer_id(uint32_t p_last_layer_id) {
    last_layer_id = p_last_layer_id;
}

void IsometricMap::set_layer_visible(uint32_t p_layer_id, bool is_visible) {
    const StringName* group = layers_groups.getptr(p_layer_id);
    if (!group) { return; }

    TypedArray<Node> nodes_in_layer = get_tree()->get_nodes_in_group(*group);
    for (int i = 0; i < nodes_in_layer.size(); ++i) {
        if (auto* positionable = Object::cast_to<IsometricPositionable>(nodes_in_layer[i])) {
            positionable->set_visible(is_visible);
        }
    }
}

void IsometricMap::set_layer_color(uint32_t p_layer_id, const Color& p_color) {
    set_meta(vformat(LAYER_COLOR_META_NAME_FORMAT, p_layer_id), p_color);

    if (!Engine::get_singleton()->is_editor_hint()) { return; }

    const Vector<IsometricPositionable*>& instances = instances_grid_3d.get_internal_array();
    const Vector<uint32_t>& layer_assignations = layers_grid_3d.get_internal_array();
    for (int i = 0; i < instances.size(); ++i) {
        IsometricPositionable* instance = instances[i];
        if (instance && layer_assignations[i] == p_layer_id) { instance->update_debug_mesh_color(p_color); }
    }
}

#endif

void IsometricMap::on_enter_tree() {
    IsometricPositionable::on_enter_tree();

#ifdef TOOLS_ENABLED
    if (last_layer_id == DEFAULT_LAYER_ID && layers.is_empty()) {
        Dictionary default_layers;
        default_layers[DEFAULT_LAYER_ID] = DEFAULT_LAYER_NAME;
        set_layers(default_layers);
    }
#endif

    if (child_positionable_initialized) { return; }
    const Vector<int>& id_vector = grid_3d.get_internal_array();
    const Vector<uint32_t>& layers_vector = layers_grid_3d.get_internal_array();
    for (int i = 0; i < id_vector.size(); ++i) {
        add_positionable_as_child(id_vector[i], grid_3d.get_position_3d_from_index(i), layers_vector[i]);
    }
    child_positionable_initialized = true;
}

Array IsometricMap::_get_grid_3d() const {
    return grid_3d.to_array();
}

void IsometricMap::_set_grid_3d(const Array& array) {
    grid_3d.from_array(array);
}

Array IsometricMap::_get_layers_grid_3d() const {
    return layers_grid_3d.to_array();
}

void IsometricMap::_set_layers_grid_3d(const Array& array) {
    layers_grid_3d.from_array(array);
}

void IsometricMap::add_positionable_as_child(int positionable_id, const Vector3& p_position, uint32_t layer_id) {
    if (positionable_id == resource::PositionableSet::NONE_POSITIONABLE_ID || positionable_set.is_null()) { return; }

    Ref<PackedScene> scene = positionable_set->get_positionable_scene_for_id(positionable_id);
    if (scene.is_null()) { return; }

    Node* instance = scene->instantiate();
    auto* positionable = Object::cast_to<IsometricPositionable>(instance);
    if (!positionable) {
        if (instance) { memdelete(instance); }
        return;
    }

    positionable->set_local_position_3d(p_position);
    if (const StringName* group = layers_groups.getptr(layer_id)) { positionable->add_to_group(*group); }
    add_child(positionable);

    instances_grid_3d.insert_box(AABB(p_position, positionable->get_size()), positionable);

#ifdef TOOLS_ENABLED
    if (!Engine::get_singleton()->is_editor_hint()) { return; }

    positionable->update_debug_mesh_color(get_meta(vformat(LAYER_COLOR_META_NAME_FORMAT, layer_id), Color()));
#endif
}

void IsometricMap::_bind_methods() {
    ClassDB::bind_method(D_METHOD("get_positionable_set"), &IsometricMap::get_positionable_set);
    ClassDB::bind_method(D_METHOD("set_positionable_set", "positionable_set"), &IsometricMap::set_positionable_set);
    ClassDB::bind_method(D_METHOD("_get_grid_3d"), &IsometricMap::_get_grid_3d);
    ClassDB::bind_method(D_METHOD("_set_grid_3d", "grid"), &IsometricMap::_set_grid_3d);
    ClassDB::bind_method(D_METHOD("_get_layers_grid_3d"), &IsometricMap::_get_layers_grid_3d);
    ClassDB::bind_method(D_METHOD("_set_layers_grid_3d", "grid"), &IsometricMap::_set_layers_grid_3d);
    ClassDB::bind_method(D_METHOD("get_layers"), &IsometricMap::get_layers);
    ClassDB::bind_method(D_METHOD("set_layers", "layers"), &IsometricMap::set_layers);
    ClassDB::bind_method(D_METHOD("get_positionable_id_for_position", "position"), &IsometricMap::get_positionable_id_for_position);
    ClassDB::bind_method(D_METHOD("get_layer_id_at", "position"), &IsometricMap::get_layer_id_at);

    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "positionable_set", PROPERTY_HINT_RESOURCE_TYPE, "PositionableSet"), "set_positionable_set", "get_positionable_set");
    ADD_PROPERTY(
      PropertyInfo(Variant::ARRAY, "grid_3d", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NO_EDITOR | PROPERTY_USAGE_INTERNAL),
      "_set_grid_3d",
      "_get_grid_3d"
    );
    ADD_PROPERTY(
      PropertyInfo(Variant::ARRAY, "layers_grid_3d", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NO_EDITOR | PROPERTY_USAGE_INTERNAL),
      "_set_layers_grid_3d",
      "_get_layers_grid_3d"
    );
    ADD_PROPERTY(
      PropertyInfo(Variant::DICTIONARY, "layers", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NO_EDITOR | PROPERTY_USAGE_INTERNAL),
      "set_layers",
      "get_layers"
    );

    ADD_SIGNAL(MethodInfo("positional_set_changed", PropertyInfo(Variant::OBJECT, "set", PROPERTY_HINT_RESOURCE_TYPE, "PositionableSet")));

#ifdef TOOLS_ENABLED
    ClassDB::bind_method(D_METHOD("get_last_layer_id"), &IsometricMap::get_last_layer_id);
    ClassDB::bind_method(D_METHOD("set_last_layer_id", "last_layer_id"), &IsometricMap::set_last_layer_id);
    ADD_PROPERTY(
      PropertyInfo(Variant::INT, "last_layer_id", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NO_EDITOR | PROPERTY_USAGE_INTERNAL),
      "set_last_layer_id",
      "get_last_layer_id"
    );
#endif
}
