#ifdef TOOLS_ENABLED

#include "delete_command_emitter.h"

#include "editor/commands/add_positionable_command.h"
#include "editor/commands/composite_command.h"
#include "editor/commands/revert_command.h"
#include "editor/commands/select_positionable_command.h"
#include "editor/isometric_editor_plugin.h"
#include "editor/positionable_selector_manager.h"

using namespace godot;
using namespace editor::commands::emitters;

Vector<Ref<editor::commands::Command>> DeleteCommandEmitter::from_gui_input_to_command(const Ref<InputEventKey>& p_event) {
    Vector<Ref<Command>> commands;

    if (!p_event->is_pressed() || p_event->get_keycode() != KEY_BACKSPACE) { return commands; }

    node::IsometricMap* map = IsometricEditorPlugin::get_instance()->get_selected_map();

    const Vector<Vector3>& selected_positions = PositionableSelectorManager::get_singleton()->get_selected_for_map(map);

    for (const Vector3& position : selected_positions) {
        node::IsometricPositionable* current = map->get_positionable_at(position);
        if (!current) { continue; }

        Vector3 local_position = current->get_local_position_3d();

        Ref<SelectPositionableCommand> select_command;
        select_command.instantiate();
        select_command->set_should_deselect_first(false);
        select_command->set_position(position);

        Ref<RevertCommand> deselect_command;
        deselect_command.instantiate();
        deselect_command->set_reverse_command(select_command);

        Ref<AddPositionableCommand> add_command;
        add_command.instantiate();
        add_command->set_layer_id(map->get_layer_id_at(position));
        add_command->set_aabb(AABB(local_position, current->get_size()));
        add_command->set_positionable_id(map->get_positionable_id_for_position(local_position));

        Ref<RevertCommand> delete_command;
        delete_command.instantiate();
        delete_command->set_reverse_command(add_command);

        Ref<CompositeCommand> composite_command;
        composite_command.instantiate();
        composite_command->append_command(deselect_command);
        composite_command->append_command(delete_command);

        commands.push_back(composite_command);
    }

    return commands;
}

#endif
