#ifdef TOOLS_ENABLED

#include "editor_utils.h"

#include "logging.h"
#include "node/isometric_positionable.h"
#include "positionable_scenes_cache_manager.h"

#include <classes/dir_access.hpp>
#include <classes/editor_interface.hpp>
#include <classes/file_access.hpp>
#include <classes/resource_loader.hpp>

using namespace godot;
using namespace editor;

static bool is_positionable_scene(const String& path) {
    Ref<PackedScene> packed_scene = ResourceLoader::get_singleton()->load(path);
    if (packed_scene.is_null()) { return false; }
    Node* instance = packed_scene->instantiate();
    bool is_positionable = Object::cast_to<node::IsometricPositionable>(instance) != nullptr;
    if (instance) { memdelete(instance); }
    return is_positionable;
}

static void collect_positionables_in_path(const String& path, PackedStringArray& r_paths) {
    if (DirAccess::dir_exists_absolute(path)) {
        for (const String& file : DirAccess::get_files_at(path)) {
            collect_positionables_in_path(path.path_join(file), r_paths);
        }
        for (const String& directory : DirAccess::get_directories_at(path)) {
            collect_positionables_in_path(path.path_join(directory), r_paths);
        }
        return;
    }

    if (!FileAccess::file_exists(path)) {
        LOG_WARNING(vformat("%s cannot be opened", path));
        return;
    }
    if (path.ends_with(".tscn") && is_positionable_scene(path)) { r_paths.push_back(path); }
}

PackedStringArray EditorUtils::find_all_positionables_in_path(const String& path) {
    PackedStringArray paths;
    collect_positionables_in_path(path, paths);
    return paths;
}

bool EditorUtils::get_positionable_size(const Ref<resource::PositionableSet>& p_set, int p_id, Vector3& r_size) {
    if (p_set.is_null()) { return false; }
    Ref<PackedScene> scene = p_set->get_positionable_scene_for_id(p_id);
    if (scene.is_null()) { return false; }

    Node* instance = scene->instantiate();
    auto* positionable = Object::cast_to<node::IsometricPositionable>(instance);
    if (positionable) { r_size = positionable->get_size(); }
    if (instance) { memdelete(instance); }
    return positionable != nullptr;
}

void EditorUtils::refresh_item_list_containing_tiles_for_category(
  const String& category,
  ItemList* item_list,
  const Ref<resource::PositionableSet>& positionable_set,
  Control* calling_control
) {
    Vector<Ref<PackedScene>> scenes_to_add;
    Vector<Ref<PositionableItemListMetadata>> scenes_to_add_metadata;
    for (const KeyValue<int, String>& entry : positionable_set->get_scene_paths_for_category(category)) {
        if (!FileAccess::file_exists(entry.value)) { continue; }
        Ref<PackedScene> scene = ResourceLoader::get_singleton()->load(entry.value);
        if (scene.is_null()) { continue; }
        scenes_to_add.push_back(scene);
        Ref<PositionableItemListMetadata> metadata;
        metadata.instantiate();
        metadata->positionable_id = entry.key;
        metadata->is_valid = true;
        scenes_to_add_metadata.push_back(metadata);
    }

    PositionableScenesCacheManager* cache_manager = PositionableScenesCacheManager::get_singleton();
    auto scenes_to_add_size = static_cast<int>(scenes_to_add.size());
    cache_manager->start_adding(calling_control, scenes_to_add_size);

    for (int i = 0; i < scenes_to_add_size; ++i) {
        const Ref<PackedScene>& positionable_scene = scenes_to_add[i];
        int item_index = item_list->get_item_count();
        cache_manager->add_scene(calling_control, item_index, positionable_scene);
        item_list->add_item(positionable_scene->get_path(), cache_manager->get_icon(calling_control, item_index));
        item_list->set_item_metadata(item_index, scenes_to_add_metadata[i]);
    }

    cache_manager->end_adding(calling_control);

    Ref<Texture2D> removed_icon = EditorInterface::get_singleton()->get_base_control()->get_theme_icon("Remove", "EditorIcons");
    for (const resource::PositionableSet::RemovedElement& element : positionable_set->get_removed_elements()) {
        int item_index = item_list->add_item(element.element_path, removed_icon);
        Ref<PositionableItemListMetadata> metadata;
        metadata.instantiate();
        metadata->positionable_id = element.id;
        metadata->is_valid = false;
        item_list->set_item_metadata(item_index, metadata);
        item_list->set_item_custom_bg_color(item_index, Color(0.65, 0.14, 0));
    }
}

bool EditorUtils::item_from_list_has_valid_tile_metadata(ItemList* item_list, int index) {
    auto* metadata = Object::cast_to<PositionableItemListMetadata>(item_list->get_item_metadata(index));
    return metadata && metadata->is_valid;
}

void EditorUtils::refresh_positionable_icons_for_item_list(ItemList* item_list, Control* registered_in_manager) {
    for (int i = 0; i < item_list->get_item_count(); ++i) {
        if (!item_from_list_has_valid_tile_metadata(item_list, i)) { continue; }
        item_list->set_item_icon(i, PositionableScenesCacheManager::get_singleton()->get_icon(registered_in_manager, i));
    }
}

#endif
