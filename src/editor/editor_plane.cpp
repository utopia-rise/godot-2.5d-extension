#ifdef TOOLS_ENABLED

#include "editor_plane.h"

#include <classes/rendering_server.hpp>

using namespace godot;
using namespace editor;

EditorPlane::EditorPlane(int p_position, Vector3::Axis p_axis, const Vector2& p_size) :
  position(p_position),
  axis(p_axis),
  size(p_size),
  self(RenderingServer::get_singleton()->canvas_item_create()) {}

int EditorPlane::get_position() const {
    return position;
}

void EditorPlane::set_position(int p_position) {
    position = p_position;
}

Vector3::Axis EditorPlane::get_axis() const {
    return axis;
}

void EditorPlane::set_axis(Vector3::Axis p_axis) {
    axis = p_axis;
}

const Vector2& EditorPlane::get_size() const {
    return size;
}

RID EditorPlane::get_rid() const {
    return self;
}

#endif
