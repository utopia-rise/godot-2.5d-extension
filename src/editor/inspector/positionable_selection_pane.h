#ifndef ISOMETRIC_MAPS_POSITIONABLE_SELECTION_PANE_H
#define ISOMETRIC_MAPS_POSITIONABLE_SELECTION_PANE_H

#ifdef TOOLS_ENABLED

#include "resource/positionable_set.h"

#include <classes/button.hpp>
#include <classes/h_box_container.hpp>
#include <classes/item_list.hpp>
#include <classes/option_button.hpp>
#include <classes/v_split_container.hpp>

namespace editor::inspector {
    using namespace godot;

    // Bottom panel listing the positionables of the edited map's set, by category, to pick the one to paint.
    class PositionableSelectionPane : public VSplitContainer {
        GDCLASS(PositionableSelectionPane, VSplitContainer)

        HBoxContainer* top_container;
        OptionButton* category_selector;
        Button* refresh_button;
        ItemList* item_list;

        Ref<resource::PositionableSet> positionable_set;

        void _select_item_from_path_selector(int index);
        void _refresh_icons();

    protected:
        static void _bind_methods() {}

    public:
        void set_positionable_set(const Ref<resource::PositionableSet>& p_set);
        int get_selected_positionable_id() const;

        void refresh_path_selector();

        PositionableSelectionPane();
        ~PositionableSelectionPane() override;
    };
}// namespace editor::inspector

#endif
#endif// ISOMETRIC_MAPS_POSITIONABLE_SELECTION_PANE_H
