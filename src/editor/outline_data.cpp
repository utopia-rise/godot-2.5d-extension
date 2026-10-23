#ifdef TOOLS_ENABLED

#include "outline_data.h"

#include <classes/rendering_server.hpp>

using namespace godot;
using namespace editor;

OutlineData::OutlineData() : rid(RenderingServer::get_singleton()->canvas_item_create()) {}

#endif
