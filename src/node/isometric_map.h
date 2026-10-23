#ifndef ISOMETRIC_MAPS_ISOMETRIC_MAP_H
#define ISOMETRIC_MAPS_ISOMETRIC_MAP_H

#include "containers/grid_3d.h"
#include "node/isometric_positionable.h"
#include "resource/positionable_set.h"

#include <templates/hash_map.hpp>
#include <variant/dictionary.hpp>

namespace node {
    using namespace godot;

    class IsometricMap : public IsometricPositionable {
        GDCLASS(IsometricMap, IsometricPositionable);

    public:
        static constexpr uint32_t DEFAULT_LAYER_ID = 0;
        static constexpr uint32_t NO_LAYER_ID = 0xffffffff;

#ifdef TOOLS_ENABLED
        static constexpr const char* LAST_EDITED_LAYER_META_NAME = "_LAST_EDITED_LAYER";
        static constexpr const char* LAYER_COLOR_META_NAME_FORMAT = "_LAYER_%s_COLOR";
#endif

    private:
        containers::Grid3D<int, resource::PositionableSet::NONE_POSITIONABLE_ID> grid_3d;
        containers::Grid3D<uint32_t, DEFAULT_LAYER_ID> layers_grid_3d;
        containers::Grid3D<IsometricPositionable*, nullptr> instances_grid_3d;
        Ref<resource::PositionableSet> positionable_set;
        bool child_positionable_initialized = false;
        Dictionary layers;
        HashMap<uint32_t, StringName> layers_groups;

#ifdef TOOLS_ENABLED
        uint32_t last_layer_id = 0;
#endif

        Array _get_grid_3d() const;
        void _set_grid_3d(const Array& array);
        Array _get_layers_grid_3d() const;
        void _set_layers_grid_3d(const Array& array);
        void add_positionable_as_child(int positionable_id, const Vector3& p_position, uint32_t layer_id);

    protected:
        void on_enter_tree() override;

        static void _bind_methods();

    public:
        IsometricMap();
        ~IsometricMap() override = default;

        Ref<resource::PositionableSet> get_positionable_set() const;
        void set_positionable_set(const Ref<resource::PositionableSet>& p_set);

        Dictionary get_layers() const;
        void set_layers(const Dictionary& p_layers);

        IsometricPositionable* get_positionable_at(const Vector3& p_position) const;
        int get_positionable_id_for_position(const Vector3& p_position) const;
        uint32_t get_layer_id_at(const Vector3& p_position) const;
        void set_size(const Vector3& p_size) override;
        bool is_aabb_in_map(const AABB& aabb) const;
        bool is_overlapping(const AABB& aabb) const;

#ifdef TOOLS_ENABLED
        void add_positionable_if_nothing_present(const AABB& aabb, int id, uint32_t layer_id);
        void remove_positionable(const AABB& aabb);
        Vector<IsometricPositionable*> get_positionables_in(const AABB& p_aabb) const;
        uint32_t add_layer(const String& p_layer, uint32_t p_layer_id = NO_LAYER_ID);
        void remove_layer(uint32_t p_layer_id);
        void remove_layer(const String& p_layer_name);
        uint32_t get_last_layer_id() const;
        void set_last_layer_id(uint32_t p_last_layer_id);
        void set_layer_visible(uint32_t p_layer_id, bool is_visible);
        void set_layer_color(uint32_t p_layer_id, const Color& p_color);
#endif
    };
}// namespace node

#endif// ISOMETRIC_MAPS_ISOMETRIC_MAP_H
