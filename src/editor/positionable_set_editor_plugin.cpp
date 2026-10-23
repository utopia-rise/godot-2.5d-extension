#ifdef TOOLS_ENABLED

#include "positionable_set_editor_plugin.h"

using namespace godot;
using namespace editor;

static constexpr const char* POSITIONABLE_SET_EDITOR_BUTTON_TITLE = "Edit Positionable Set";

void PositionableSetEditorPlugin::_notification(int p_what) {
    if (p_what == NOTIFICATION_ENTER_TREE) {
        positionable_set_editor = memnew(inspector::PositionableSetEditor);
        positionable_set_editor_button = add_control_to_bottom_panel(positionable_set_editor, POSITIONABLE_SET_EDITOR_BUTTON_TITLE);
        positionable_set_editor_button->set_visible(false);
    } else if (p_what == NOTIFICATION_EXIT_TREE) {
        remove_control_from_bottom_panel(positionable_set_editor);
        memdelete(positionable_set_editor);
        positionable_set_editor = nullptr;
    }
}

bool PositionableSetEditorPlugin::_handles(Object* p_object) const {
    return Object::cast_to<resource::PositionableSet>(p_object) != nullptr;
}

void PositionableSetEditorPlugin::_make_visible(bool p_visible) {
    positionable_set_editor_button->set_visible(p_visible);
}

void PositionableSetEditorPlugin::_edit(Object* p_object) {
    if (auto* positionable_set = Object::cast_to<resource::PositionableSet>(p_object)) {
        positionable_set_editor->set_positionable_set(Ref<resource::PositionableSet>(positionable_set));
    }
}

#endif
