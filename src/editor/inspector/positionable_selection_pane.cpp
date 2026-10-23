#ifdef TOOLS_ENABLED

#include "positionable_selection_pane.h"

#include "editor/editor_utils.h"
#include "editor/positionable_scenes_cache_manager.h"

#include <classes/resource_saver.hpp>

using namespace godot;
using namespace editor::inspector;

void PositionableSelectionPane::set_positionable_set(const Ref<resource::PositionableSet>& p_set) {
    positionable_set = p_set;
    refresh_path_selector();
}

int PositionableSelectionPane::get_selected_positionable_id() const {
    PackedInt32Array selected_items = item_list->get_selected_items();
    if (selected_items.is_empty()) { return resource::PositionableSet::NONE_POSITIONABLE_ID; }
    if (auto* metadata = Object::cast_to<PositionableItemListMetadata>(item_list->get_item_metadata(selected_items[0]))) {
        return metadata->positionable_id;
    }
    return resource::PositionableSet::NONE_POSITIONABLE_ID;
}

void PositionableSelectionPane::refresh_path_selector() {
    category_selector->clear();

    if (positionable_set.is_null()) {
        item_list->clear();
        return;
    }

    // Built-in resources are saved with their scene, only standalone set files are written here.
    String set_path = positionable_set->get_path();
    if (set_path.is_absolute_path() && !set_path.contains("::")) {
        ResourceSaver::get_singleton()->save(positionable_set);
    }
    for (const String& category : positionable_set->get_categories()) {
        category_selector->add_item(category);
    }
    _select_item_from_path_selector(category_selector->get_selected());
}

void PositionableSelectionPane::_select_item_from_path_selector(int index) {
    PositionableScenesCacheManager::get_singleton()->clear(this);
    item_list->clear();

    if (index < 0) { return; }

    EditorUtils::refresh_item_list_containing_tiles_for_category(category_selector->get_item_text(index), item_list, positionable_set, this);
}

void PositionableSelectionPane::_refresh_icons() {
    EditorUtils::refresh_positionable_icons_for_item_list(item_list, this);
}

PositionableSelectionPane::PositionableSelectionPane() :
  top_container(memnew(HBoxContainer)),
  category_selector(memnew(OptionButton)),
  refresh_button(memnew(Button)),
  item_list(memnew(ItemList)) {
    set_name("Isometric positionables");
    refresh_button->set_text("Refresh");
    add_child(top_container);
    top_container->add_child(category_selector);
    top_container->add_child(refresh_button);
    category_selector->set_h_size_flags(SIZE_EXPAND_FILL);
    add_child(item_list);

    category_selector->connect("item_selected", callable_mp(this, &PositionableSelectionPane::_select_item_from_path_selector));
    refresh_button->connect("pressed", callable_mp(this, &PositionableSelectionPane::refresh_path_selector));

    PositionableScenesCacheManager::get_singleton()->register_control(this, callable_mp(this, &PositionableSelectionPane::_refresh_icons));
}

PositionableSelectionPane::~PositionableSelectionPane() {
    if (PositionableScenesCacheManager* cache_manager = PositionableScenesCacheManager::get_singleton()) {
        cache_manager->unregister_control(this);
    }
}

#endif
