#ifdef TOOLS_ENABLED

#include "positionable_scenes_cache_manager.h"

#include "data/isometric_parameters.h"
#include "isometric_editor_plugin.h"
#include "node/isometric_positionable.h"
#include "utils/isometric_maths.h"

#include <classes/camera2d.hpp>
#include <classes/editor_interface.hpp>
#include <classes/image.hpp>
#include <classes/image_texture.hpp>
#include <classes/viewport_texture.hpp>

using namespace godot;
using namespace editor;

PositionableScenesCacheManager* PositionableScenesCacheManager::singleton = nullptr;

PositionableScenesCacheManager* PositionableScenesCacheManager::get_singleton() {
    return singleton;
}

void PositionableScenesCacheManager::create() {
    singleton = memnew(PositionableScenesCacheManager);
}

void PositionableScenesCacheManager::free() {
    memdelete(singleton);
    singleton = nullptr;
}

void PositionableScenesCacheManager::add_scene(Control* p_control, int index, const Ref<PackedScene>& scene) {
    SubViewport* scene_viewport = _create_viewport_for_scene(scene);
    if (!scene_viewport) { return; }

    EditorInterface::get_singleton()->get_base_control()->add_child(scene_viewport);
    Vector<SubViewport*>* viewports = drawing_viewports.getptr(p_control);
    if (!viewports) { return; }
    viewports->set(index, scene_viewport);
    cache[p_control].set(index, {scene, scene_viewport->get_texture()});
}

void PositionableScenesCacheManager::clear(Control* p_control) {
    if (Vector<CacheEntry>* entries = cache.getptr(p_control)) { entries->clear(); }
}

Ref<Texture2D> PositionableScenesCacheManager::get_icon(Control* p_control, int index) {
    const Vector<CacheEntry>* entries = cache.getptr(p_control);
    if (!entries || index < 0 || index >= entries->size()) { return {}; }
    return (*entries)[index].icon;
}

void PositionableScenesCacheManager::copy_current_viewports_textures() {
    for (const KeyValue<Control*, Vector<SubViewport*>>& entry : drawing_viewports) {
        Vector<CacheEntry>& entries = cache[entry.key];
        for (int64_t i = 0; i < entry.value.size() && i < entries.size(); ++i) {
            SubViewport* viewport = entry.value[i];
            if (!viewport) { continue; }
            Ref<Image> texture_image = viewport->get_texture()->get_image();
            if (texture_image.is_null()) { continue; }
            texture_image->flip_y();
            CacheEntry copy = entries[i];
            copy.icon = ImageTexture::create_from_image(texture_image);
            entries.set(i, copy);
        }
    }
}

void PositionableScenesCacheManager::clear_current_viewports() {
    for (KeyValue<Control*, Vector<SubViewport*>>& entry : drawing_viewports) {
        for (SubViewport* viewport : entry.value) {
            if (viewport) { viewport->queue_free(); }
        }
        entry.value.clear();
    }
}

void PositionableScenesCacheManager::register_control(Control* p_control, const Callable& p_refresh_icons) {
    cache[p_control] = Vector<CacheEntry>();
    drawing_viewports[p_control] = Vector<SubViewport*>();
    adding[p_control] = false;
    refresh_icons_callbacks[p_control] = p_refresh_icons;
}

void PositionableScenesCacheManager::unregister_control(Control* p_control) {
    // Pending viewports are children of the editor base control, which frees them.
    cache.erase(p_control);
    drawing_viewports.erase(p_control);
    adding.erase(p_control);
    refresh_icons_callbacks.erase(p_control);
}

void PositionableScenesCacheManager::start_adding(Control* p_control, int cache_size) {
    adding[p_control] = true;
    Vector<SubViewport*>& viewports = drawing_viewports[p_control];
    viewports.resize(cache_size);
    viewports.fill(nullptr);
    cache[p_control].resize(cache_size);
}

void PositionableScenesCacheManager::end_adding(Control* p_control) {
    adding[p_control] = false;
    if (IsometricEditorPlugin* plugin = IsometricEditorPlugin::get_instance()) {
        plugin->set_should_clear_buffer_on_next_frame(is_adding());
    }
}

bool PositionableScenesCacheManager::is_adding() const {
    for (const KeyValue<Control*, bool>& entry : adding) {
        if (entry.value) { return true; }
    }
    return false;
}

void PositionableScenesCacheManager::refresh_all_icons() const {
    for (const KeyValue<Control*, Callable>& entry : refresh_icons_callbacks) {
        entry.value.call();
    }
}

SubViewport* PositionableScenesCacheManager::_create_viewport_for_scene(const Ref<PackedScene>& scene) {
    Node* instance = scene->instantiate();
    auto* positionable = Object::cast_to<node::IsometricPositionable>(instance);
    if (!positionable) {
        if (instance) { memdelete(instance); }
        return nullptr;
    }

    // The scene is drawn upside down, then the copied image is flipped back.
    positionable->set_scale(positionable->get_scale() * Vector2(1, -1));

    auto* viewport = memnew(SubViewport);
    auto* camera = memnew(Camera2D);
    viewport->add_child(camera);
    camera->add_child(positionable);
    viewport->set_update_mode(SubViewport::UPDATE_ONCE);

    utils::Hexagone hexagone_coordinates = utils::get_hexagone_points(
      data::IsometricParameters::get_default_configuration(),
      AABB(positionable->get_local_position_3d(), positionable->get_size())
    );
    Vector2 scene_size(
      hexagone_coordinates.max_x - hexagone_coordinates.min_x,
      hexagone_coordinates.max_y - hexagone_coordinates.min_y
    );
    positionable->set_scale(positionable->get_scale() / scene_size);
    Vector2 viewport_size(128, 128);
    camera->set_position(viewport_size / 2);
    viewport->set_size(Vector2i(viewport_size));
    return viewport;
}

#endif
