#include "positionable_set.h"

#include "logging.h"

#include <classes/file_access.hpp>
#include <classes/resource_loader.hpp>

using namespace godot;
using namespace resource;

void PositionableSet::preload_scenes() {
    identifier_to_loaded_scene.clear();

    for (const KeyValue<int, String>& entry : identifier_to_scene_path) {
        _load_positionable_scene(entry.key, entry.value);
    }

    is_loaded = true;
}

bool PositionableSet::is_set_loaded() const {
    return is_loaded;
}

Ref<PackedScene> PositionableSet::get_positionable_scene_for_id(int id) const {
    const Ref<PackedScene>* scene = identifier_to_loaded_scene.getptr(id);
    return scene ? *scene : Ref<PackedScene>();
}

#ifdef TOOLS_ENABLED

const PackedStringArray& PositionableSet::get_categories() const {
    return categories;
}

void PositionableSet::set_categories(const PackedStringArray& p_categories) {
    categories = p_categories;
}

HashMap<int, String> PositionableSet::get_scene_paths_for_category(const String& p_category) const {
    const Vector<int>* ids = categories_to_identifiers.getptr(p_category);
    if (!ids) { return {}; }

    HashMap<int, String> paths_for_category;
    for (int id : *ids) {
        if (const String* path = identifier_to_scene_path.getptr(id)) { paths_for_category[id] = *path; }
    }
    return paths_for_category;
}

Vector<PositionableSet::RemovedElement> PositionableSet::get_removed_elements() const {
    Vector<RemovedElement> removed_elements;

    for (const KeyValue<int, String>& entry : identifier_to_scene_path) {
        if (!FileAccess::file_exists(entry.value)) { removed_elements.push_back({entry.key, entry.value}); }
    }

    return removed_elements;
}

bool PositionableSet::add_category(const String& category) {
    if (categories.has(category)) { return false; }

    categories.append(category);
    categories_to_identifiers[category] = Vector<int>();
    return true;
}

void PositionableSet::remove_category(const String& category) {
    if (const Vector<int>* category_ids = categories_to_identifiers.getptr(category)) {
        for (int id : *category_ids) {
            identifier_to_scene_path.erase(id);
        }
    }

    categories_to_identifiers.erase(category);

    int64_t index = categories.find(category);
    if (index >= 0) { categories.remove_at(index); }
}

bool PositionableSet::add_or_update_positionable(int id, const String& path) {
    for (const KeyValue<int, String>& entry : identifier_to_scene_path) {
        if (entry.value == path) { return false; }
    }
    identifier_to_scene_path[id] = path;

    if (is_loaded) { _load_positionable_scene(id, path); }

    return true;
}

void PositionableSet::remove_positionable(int id) {
    for (KeyValue<StringName, Vector<int>>& entry : categories_to_identifiers) {
        int64_t id_index = entry.value.find(id);
        if (id_index >= 0) {
            entry.value.remove_at(id_index);
            break;
        }
    }

    identifier_to_scene_path.erase(id);
}

void PositionableSet::insert_positionable_if_not_present(const String& category, const String& resource_path) {
    for (const KeyValue<int, String>& entry : identifier_to_scene_path) {
        if (entry.value == resource_path) { return; }
    }

    StringName category_name(category);
    if (!categories_to_identifiers.has(category_name)) { categories_to_identifiers[category_name] = Vector<int>(); }

    _insert_positionable_for_category_and_id(category_name, resource_path, ++last_id);
}

void PositionableSet::_insert_positionable_for_category_and_id(const StringName& category, const String& resource_path, int id) {
    categories_to_identifiers[category].push_back(id);
    identifier_to_scene_path[id] = resource_path;

    if (is_loaded) { _load_positionable_scene(id, resource_path); }
}

Dictionary PositionableSet::_get_categories_to_identifiers() const {
    Dictionary converted;

    for (const KeyValue<StringName, Vector<int>>& entry : categories_to_identifiers) {
        Array identifiers;
        for (int id : entry.value) {
            identifiers.append(id);
        }
        converted[String(entry.key)] = identifiers;
    }

    return converted;
}

void PositionableSet::_set_categories_to_identifiers(const Dictionary& p_categories_to_identifiers) {
    categories_to_identifiers.clear();

    Array keys = p_categories_to_identifiers.keys();
    for (int i = 0; i < keys.size(); ++i) {
        Array identifiers = p_categories_to_identifiers[keys[i]];
        Vector<int> entry;
        for (int j = 0; j < identifiers.size(); ++j) {
            entry.push_back(identifiers[j]);
        }
        categories_to_identifiers[StringName(keys[i])] = entry;
    }
}

int PositionableSet::_get_last_id() const {
    return last_id;
}

void PositionableSet::_set_last_id(int p_last_id) {
    last_id = p_last_id;
}

#endif

void PositionableSet::_load_positionable_scene(int id, const String& scene_path) {
    Ref<PackedScene> loaded = ResourceLoader::get_singleton()->load(scene_path);
    if (loaded.is_null()) { LOG_ERROR(vformat("Element %s from positionable set is not valid.", scene_path)); }
    identifier_to_loaded_scene[id] = loaded;
}

Dictionary PositionableSet::_get_identifier_to_scene_path() const {
    Dictionary converted;

    for (const KeyValue<int, String>& entry : identifier_to_scene_path) {
        converted[entry.key] = entry.value;
    }

    return converted;
}

void PositionableSet::_set_identifier_to_scene_path(const Dictionary& p_identifier_to_scene_path) {
    identifier_to_scene_path.clear();

    Array keys = p_identifier_to_scene_path.keys();
    for (int i = 0; i < keys.size(); ++i) {
        int id = keys[i];
        identifier_to_scene_path[id] = p_identifier_to_scene_path[keys[i]];
    }
}

void PositionableSet::_bind_methods() {
    ClassDB::bind_method(D_METHOD("preload_scenes"), &PositionableSet::preload_scenes);
    ClassDB::bind_method(D_METHOD("is_set_loaded"), &PositionableSet::is_set_loaded);
    ClassDB::bind_method(D_METHOD("get_positionable_scene_for_id", "id"), &PositionableSet::get_positionable_scene_for_id);

#ifdef TOOLS_ENABLED
    ClassDB::bind_method(D_METHOD("set_categories", "categories"), &PositionableSet::set_categories);
    ClassDB::bind_method(D_METHOD("get_categories"), &PositionableSet::get_categories);
    ADD_PROPERTY(
      PropertyInfo(Variant::PACKED_STRING_ARRAY, "categories", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NO_EDITOR | PROPERTY_USAGE_INTERNAL),
      "set_categories",
      "get_categories"
    );

    ClassDB::bind_method(D_METHOD("_set_categories_to_identifiers", "categories_to_identifiers"), &PositionableSet::_set_categories_to_identifiers);
    ClassDB::bind_method(D_METHOD("_get_categories_to_identifiers"), &PositionableSet::_get_categories_to_identifiers);
    ADD_PROPERTY(
      PropertyInfo(Variant::DICTIONARY, "categories_to_identifiers", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NO_EDITOR | PROPERTY_USAGE_INTERNAL),
      "_set_categories_to_identifiers",
      "_get_categories_to_identifiers"
    );

    ClassDB::bind_method(D_METHOD("_set_last_id", "last_id"), &PositionableSet::_set_last_id);
    ClassDB::bind_method(D_METHOD("_get_last_id"), &PositionableSet::_get_last_id);
    ADD_PROPERTY(
      PropertyInfo(Variant::INT, "last_id", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NO_EDITOR | PROPERTY_USAGE_INTERNAL),
      "_set_last_id",
      "_get_last_id"
    );
#endif

    ClassDB::bind_method(D_METHOD("_set_identifier_to_scene_path", "identifier_to_scene_path"), &PositionableSet::_set_identifier_to_scene_path);
    ClassDB::bind_method(D_METHOD("_get_identifier_to_scene_path"), &PositionableSet::_get_identifier_to_scene_path);
    ADD_PROPERTY(
      PropertyInfo(Variant::DICTIONARY, "identifier_to_scene_path", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NO_EDITOR | PROPERTY_USAGE_INTERNAL),
      "_set_identifier_to_scene_path",
      "_get_identifier_to_scene_path"
    );
}
