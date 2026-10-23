#ifdef TOOLS_ENABLED

#include "move_editor_view_limiter_command_emitter.h"

#include "constants.h"
#include "editor/commands/composite_command.h"
#include "editor/commands/move_editor_plane_command.h"
#include "editor/commands/set_positionable_visibility_command.h"
#include "editor/isometric_editor_plugin.h"

#include <classes/input.hpp>

#ifdef MACOS_ENABLED
#include <classes/time.hpp>
#endif

using namespace godot;
using namespace editor::commands::emitters;

static constexpr Key LIMITER_AXIS_KEYS[] = {KEY_1, KEY_2, KEY_3};

Vector<Ref<editor::commands::Command>> MoveEditorViewLimiterCommandEmitter::from_gui_input_to_command(const Ref<ScrollInputEvent>& p_event) {
    Vector<Ref<Command>> commands;

    if (!_is_event_activated(p_event) || !p_event->is_command_or_control_pressed()) { return commands; }

    EventMotion motion = _get_event_motion(p_event);
    if (motion == EventMotion::NONE) { return commands; }

    int limiter_axis = -1;
    for (int axis = Vector3::AXIS_X; axis <= Vector3::AXIS_Z; ++axis) {
        if (Input::get_singleton()->is_key_pressed(LIMITER_AXIS_KEYS[axis])) {
            limiter_axis = axis;
            break;
        }
    }
    if (limiter_axis < 0) { return commands; }

    IsometricEditorPlugin* isometric_editor_plugin = IsometricEditorPlugin::get_instance();
    node::IsometricMap* map = isometric_editor_plugin->get_selected_map();
    Vector3 map_size = map->get_size();

    // Shift moves the min limiter, the max limiter otherwise; a max limiter pushed past the map end means "no limit".
    bool is_max_limiter = !p_event->is_shift_pressed();
    EditorPlane::PlaneType plane_type = is_max_limiter
                                        ? EditorPlane::get_max_view_limiter(static_cast<Vector3::Axis>(limiter_axis))
                                        : EditorPlane::get_min_view_limiter(static_cast<Vector3::Axis>(limiter_axis));
    auto plane_max_position = static_cast<int>(map_size[limiter_axis]);

    const EditorPlane& plane = isometric_editor_plugin->get_editor_plane_for_map(map, plane_type);
    auto editor_plane_position = static_cast<real_t>(plane.get_position());

    bool is_forward = motion == EventMotion::FORWARD;
    if ((editor_plane_position == 0 && !is_forward) || (editor_plane_position >= plane_max_position && is_forward)) {
        return commands;
    }
    editor_plane_position = MIN(editor_plane_position, static_cast<real_t>(plane_max_position));

    int min_limits[3];
    int max_limits[3];
    for (int axis = Vector3::AXIS_X; axis <= Vector3::AXIS_Z; ++axis) {
        auto map_size_on_axis = static_cast<int>(map_size[axis]);
        min_limits[axis] = CLAMP(
          isometric_editor_plugin
            ->get_editor_plane_for_map(map, EditorPlane::get_min_view_limiter(static_cast<Vector3::Axis>(axis)))
            .get_position(),
          0,
          map_size_on_axis
        );
        max_limits[axis] = CLAMP(
          isometric_editor_plugin
            ->get_editor_plane_for_map(map, EditorPlane::get_max_view_limiter(static_cast<Vector3::Axis>(axis)))
            .get_position(),
          0,
          map_size_on_axis
        );
    }

    // The slice the limiter crosses changes visibility.
    int plane_axis = plane.get_axis();
    int first_axis = (plane_axis + 1) % 3;
    int second_axis = (plane_axis + 2) % 3;
    Vector3 position;
    position[plane_axis] = is_forward ? editor_plane_position : editor_plane_position - 1;

    Ref<CompositeCommand> composite_command;
    composite_command.instantiate();
    for (int first = min_limits[first_axis]; first < max_limits[first_axis]; ++first) {
        for (int second = min_limits[second_axis]; second < max_limits[second_axis]; ++second) {
            position[first_axis] = static_cast<real_t>(first);
            position[second_axis] = static_cast<real_t>(second);
            if (!map->get_positionable_at(position)) { continue; }

            Ref<SetPositionableVisibilityCommand> visibility_command;
            visibility_command.instantiate();
            visibility_command->set_position(position);
            composite_command->append_command(visibility_command);
        }
    }
    commands.push_back(composite_command);

    int current_position = plane.get_position();
    int new_position = current_position;
    if (is_forward) {
        if (current_position < plane_max_position) { new_position += 1; }
        if (new_position >= plane_max_position && is_max_limiter) { new_position = Constants::int_max; }
    } else {
        new_position = CLAMP(current_position - 1, 0, plane_max_position - 1);
    }

    Ref<MoveEditorPlaneCommand> move_command;
    move_command.instantiate();
    move_command->set_plane_type(plane_type);
    move_command->set_old_position(current_position);
    move_command->set_new_position(new_position);
    commands.push_back(move_command);

    return commands;
}

bool MoveEditorViewLimiterCommandEmitter::_is_event_activated([[maybe_unused]] const Ref<ScrollInputEvent>& p_event) {
#ifdef MACOS_ENABLED
    return true;
#else
    return p_event->is_pressed();
#endif
}

MoveEditorViewLimiterCommandEmitter::EventMotion MoveEditorViewLimiterCommandEmitter::_get_event_motion(const Ref<ScrollInputEvent>& p_event) {
#ifdef MACOS_ENABLED
    // Pan gestures arrive as many small deltas: they are accumulated into whole steps, and forgotten after a pause.
    const real_t threshold = 1.0;
    cumulative_scroll_delta += p_event->get_delta().y;
    uint64_t now = Time::get_singleton()->get_ticks_msec() / 1000;
    if (now - cumulative_scroll_last_time > 2) { cumulative_scroll_delta = 0; }
    cumulative_scroll_last_time = now;
    if (cumulative_scroll_delta >= threshold) {
        cumulative_scroll_delta = 0;
        return EventMotion::BACKWARD;
    }
    if (cumulative_scroll_delta < -threshold) {
        cumulative_scroll_delta = 0;
        return EventMotion::FORWARD;
    }
    return EventMotion::NONE;
#else
    switch (p_event->get_button_index()) {
        case MOUSE_BUTTON_WHEEL_UP:
            return EventMotion::FORWARD;
        case MOUSE_BUTTON_WHEEL_DOWN:
            return EventMotion::BACKWARD;
        default:
            return EventMotion::NONE;
    }
#endif
}

#endif
