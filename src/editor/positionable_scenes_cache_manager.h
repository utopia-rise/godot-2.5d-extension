#ifndef ISOMETRIC_MAPS_POSITIONABLE_SCENES_CACHE_MANAGER_H
#define ISOMETRIC_MAPS_POSITIONABLE_SCENES_CACHE_MANAGER_H

#ifdef TOOLS_ENABLED

#include <classes/control.hpp>
#include <classes/packed_scene.hpp>
#include <classes/sub_viewport.hpp>
#include <classes/texture2d.hpp>
#include <templates/hash_map.hpp>
#include <templates/vector.hpp>
#include <variant/callable.hpp>

namespace editor {
    using namespace godot;

    // Renders positionable scenes into icons for the item lists of the registered controls. A scene is drawn by a
    // one-shot SubViewport; its texture is copied once rendered, and the viewport is freed.
    class PositionableScenesCacheManager {
        struct CacheEntry {
            Ref<PackedScene> scene;
            Ref<Texture2D> icon;
        };

        static PositionableScenesCacheManager* singleton;

        HashMap<Control*, Vector<CacheEntry>> cache;
        HashMap<Control*, Vector<SubViewport*>> drawing_viewports;
        HashMap<Control*, bool> adding;
        HashMap<Control*, Callable> refresh_icons_callbacks;

        static SubViewport* _create_viewport_for_scene(const Ref<PackedScene>& scene);

    public:
        static PositionableScenesCacheManager* get_singleton();
        static void create();
        static void free();

        void add_scene(Control* p_control, int index, const Ref<PackedScene>& scene);
        void clear(Control* p_control);
        Ref<Texture2D> get_icon(Control* p_control, int index);

        void copy_current_viewports_textures();
        void clear_current_viewports();

        void register_control(Control* p_control, const Callable& p_refresh_icons);
        void unregister_control(Control* p_control);
        void start_adding(Control* p_control, int cache_size);
        void end_adding(Control* p_control);
        bool is_adding() const;
        void refresh_all_icons() const;
    };
}// namespace editor

#endif
#endif// ISOMETRIC_MAPS_POSITIONABLE_SCENES_CACHE_MANAGER_H
