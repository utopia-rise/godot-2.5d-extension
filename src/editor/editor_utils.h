#ifndef ISOMETRIC_MAPS_EDITOR_UTILS_H
#define ISOMETRIC_MAPS_EDITOR_UTILS_H

#ifdef TOOLS_ENABLED

#include "resource/positionable_set.h"

#include <classes/control.hpp>
#include <classes/item_list.hpp>
#include <classes/ref_counted.hpp>
#include <variant/packed_string_array.hpp>

namespace editor {
    using namespace godot;

    // Metadata of an ItemList entry listing a positionable of a set; is_valid is false when its scene file is missing.
    class PositionableItemListMetadata : public RefCounted {
        GDCLASS(PositionableItemListMetadata, RefCounted)

    protected:
        static void _bind_methods() {}

    public:
        int positionable_id = resource::PositionableSet::NONE_POSITIONABLE_ID;
        bool is_valid = false;
    };

    class EditorUtils {
    public:
        static PackedStringArray find_all_positionables_in_path(const String& path);
        static bool get_positionable_size(const Ref<resource::PositionableSet>& p_set, int p_id, Vector3& r_size);
        static void refresh_item_list_containing_tiles_for_category(
          const String& category,
          ItemList* item_list,
          const Ref<resource::PositionableSet>& positionable_set,
          Control* calling_control
        );
        static bool item_from_list_has_valid_tile_metadata(ItemList* item_list, int index);
        static void refresh_positionable_icons_for_item_list(ItemList* item_list, Control* registered_in_manager);
    };
}// namespace editor

#endif
#endif// ISOMETRIC_MAPS_EDITOR_UTILS_H
