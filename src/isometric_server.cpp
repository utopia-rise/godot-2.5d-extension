#include "isometric_server.h"

#include "logging.h"
#include "utils/isometric_maths.h"

#include <classes/engine.hpp>
#include <classes/rendering_server.hpp>
#include <classes/scene_tree.hpp>
#include <classes/window.hpp>

using namespace godot;
using namespace data;

IsometricServer* IsometricServer::singleton = nullptr;

IsometricServer::IsometricServer() {
    spaces.reserve(8);
    stack.reserve(32);
    // Server singletons are registered after the extension's scene level, so the hooks wait for the main loop.
    callable_mp(this, &IsometricServer::hook_into_main_loop).call_deferred();
}

void IsometricServer::hook_into_main_loop() {
    RenderingServer::get_singleton()->connect("frame_pre_draw", callable_mp(this, &IsometricServer::fetch_data_and_request_ordering));

    // On exit, the engine waits for every WorkerThreadPool thread to go idle before deinitializing extensions, so the
    // server thread lives exactly as long as the scene tree. Until it starts, and without a scene tree, commands run
    // on the calling thread.
    auto* scene_tree = Object::cast_to<SceneTree>(Engine::get_singleton()->get_main_loop());
    if (!scene_tree) { return; }
    scene_tree->get_root()->connect("tree_exiting", callable_mp(this, &IsometricServer::stop_thread));
    command_queue.start_consumer();
    server_task_id = WorkerThreadPool::get_singleton()->add_native_task(&IsometricServer::thread_loop, this, true, "IsometricServer");
}

void IsometricServer::stop_thread() {
    if (server_task_id < 0) { return; }

    command_queue.push(this, &IsometricServer::command_stop_server);
    WorkerThreadPool::get_singleton()->wait_for_task_completion(server_task_id);
    server_task_id = -1;
    command_queue.stop_consumer();
}

/////////////// STATIC //////////////////

IsometricServer* IsometricServer::get_instance() {
    return singleton;
}

void IsometricServer::create_server() {
    if (!singleton) { singleton = memnew(IsometricServer); }
}

void IsometricServer::thread_loop(void* p_server) {
    auto* server = static_cast<IsometricServer*>(p_server);
    while (!server->exit_thread) {
        server->command_queue.wait_and_flush();
    }
}

void IsometricServer::terminate_server() {
    if (!singleton) { return; }

    // The RenderingServer is already gone at this level; freeing the server drops its frame_pre_draw connection.
    singleton->stop_thread();

    while (!singleton->spaces.is_empty()) {
        singleton->command_space_delete(singleton->spaces[0]);
    }
    memdelete(singleton);
    singleton = nullptr;
}

/////////////// PUBLIC API ///////////////

RID IsometricServer::space_create() {
    auto* space = memnew(IsometricSpace);
    RID rid = spaces_owner.make_rid(space);
    command_queue.push(this, &IsometricServer::command_space_create, space);
    return rid;
}

void IsometricServer::space_attach_isometric_element(const RID& space_rid, const RID& element_rid) {
    IsometricSpace* space = spaces_owner.get_or_null(space_rid);
    IsometricElement* element = elements_owner.get_or_null(element_rid);
    ISO_DEBUG_FAIL_COND_MSG(!space, vformat("Invalid space RID: %s", space_rid.get_id()));
    ISO_DEBUG_FAIL_COND_MSG(!element, vformat("Invalid isometric element RID: %s", element_rid.get_id()));
    command_queue.push(this, &IsometricServer::command_space_attach_isometric_element, space, element);
}

void IsometricServer::space_detach_isometric_element(const RID& element_rid) {
    IsometricElement* element = elements_owner.get_or_null(element_rid);
    ISO_DEBUG_FAIL_COND_MSG(!element, vformat("Invalid isometric element RID: %s", element_rid.get_id()));
    command_queue.push(this, &IsometricServer::command_space_detach_isometric_element, element);
}

void IsometricServer::space_update_configuration(const RID& space_rid, const Ref<resource::IsometricConfiguration>& p_configuration) {
    IsometricSpace* space = spaces_owner.get_or_null(space_rid);
    ISO_DEBUG_FAIL_COND_MSG(!space, vformat("Invalid space RID: %s", space_rid.get_id()));
    ERR_FAIL_COND(p_configuration.is_null());
    space->configuration =
      IsometricParameters(p_configuration->get_tile_width(), p_configuration->get_angle(), p_configuration->get_topological_margin());
    command_queue.push(this, &IsometricServer::command_space_set_configuration, space, space->configuration);
}

const IsometricParameters* IsometricServer::space_get_configuration(const RID& space_rid) {
    IsometricSpace* space = spaces_owner.get_or_null(space_rid);
    ISO_DEBUG_FAIL_COND_V_MSG(!space, nullptr, vformat("Invalid space RID: %s", space_rid.get_id()));
    return &space->configuration;
}

int IsometricServer::space_get_diamond_width(const RID& space_rid) {
    const IsometricParameters* configuration = space_get_configuration(space_rid);
    ISO_DEBUG_FAIL_COND_V_MSG(!configuration, 0, "Cannot read diamond width.");
    return configuration->diamond_width;
}

int IsometricServer::space_get_diamond_height(const RID& space_rid) {
    const IsometricParameters* configuration = space_get_configuration(space_rid);
    ISO_DEBUG_FAIL_COND_V_MSG(!configuration, 0, "Cannot read diamond height.");
    return configuration->diamond_height;
}

float IsometricServer::space_get_z_length(const RID& space_rid) {
    const IsometricParameters* configuration = space_get_configuration(space_rid);
    ISO_DEBUG_FAIL_COND_V_MSG(!configuration, 0, "Cannot read z length.");
    return configuration->z_length;
}

RID IsometricServer::isometric_element_create(bool is_dynamic, const AABB& aabb) {
    auto* element = memnew(IsometricElement);
    element->is_dynamic = is_dynamic;
    element->aabb = aabb;
    element->self = elements_owner.make_rid(element);
    return element->self;
}

void IsometricServer::isometric_element_attach_canvas_item(const RID& element_rid, const RID& canvas_rid, int depth) {
    IsometricElement* element = elements_owner.get_or_null(element_rid);
    ISO_DEBUG_FAIL_COND_MSG(!element, vformat("Invalid isometric element RID: %s", element_rid.get_id()));
    element->canvas_item = canvas_rid;
    command_queue.push(this, &IsometricServer::command_isometric_element_set_depth, element, depth);
}

void IsometricServer::isometric_element_detach_canvas_item(const RID& element_rid) {
    IsometricElement* element = elements_owner.get_or_null(element_rid);
    ISO_DEBUG_FAIL_COND_MSG(!element, vformat("Invalid isometric element RID: %s", element_rid.get_id()));
    element->canvas_item = RID();
}

void IsometricServer::isometric_element_set_position(const RID& element_rid, const Vector3& global_position) {
    IsometricElement* element = elements_owner.get_or_null(element_rid);
    ISO_DEBUG_FAIL_COND_MSG(!element, vformat("Invalid isometric element RID: %s", element_rid.get_id()));
    command_queue.push(this, &IsometricServer::command_isometric_element_set_position, element, global_position);
}

void IsometricServer::isometric_element_set_size(const RID& element_rid, const Vector3& size) {
    IsometricElement* element = elements_owner.get_or_null(element_rid);
    ISO_DEBUG_FAIL_COND_MSG(!element, vformat("Invalid isometric element RID: %s", element_rid.get_id()));
    command_queue.push(this, &IsometricServer::command_isometric_element_set_size, element, size);
}

void IsometricServer::isometric_element_set_depth(const RID& element_rid, int depth) {
    IsometricElement* element = elements_owner.get_or_null(element_rid);
    ISO_DEBUG_FAIL_COND_MSG(!element, vformat("Invalid isometric element RID: %s", element_rid.get_id()));
    command_queue.push(this, &IsometricServer::command_isometric_element_set_depth, element, depth);
}

int IsometricServer::isometric_element_get_z_order(const RID& element_rid) {
    IsometricElement* element = elements_owner.get_or_null(element_rid);
    ISO_DEBUG_FAIL_COND_V_MSG(!element, 0, vformat("Invalid isometric element RID: %s", element_rid.get_id()));
    return element->applied_z_order;
}

#ifdef DEBUG_ENABLED
void IsometricServer::isometric_element_set_debug_modulate(const RID& element_rid, const Color& color) {
    IsometricElement* element = elements_owner.get_or_null(element_rid);
    ISO_DEBUG_FAIL_COND_MSG(!element, vformat("Invalid isometric element RID: %s", element_rid.get_id()));
    element->debug_modulate = color;
    element->has_debug_modulate = true;
    command_queue.push(this, &IsometricServer::command_isometric_element_mark_dirty, element);
}
#endif

void IsometricServer::set_synchronous_mode(bool sync) {
    synchronous = sync;
}

bool IsometricServer::is_synchronous_mode() const {
    return synchronous;
}

void IsometricServer::fetch_data_and_request_ordering() {
    if (synchronous) {
        command_queue.push_and_sync(this, &IsometricServer::command_isometric_sort);
        apply_sort_results();
        return;
    }

    // An asynchronous sort spans frames: until it lands, the previous z-indices stay on screen.
    if (sort_in_flight.load(std::memory_order_acquire)) { return; }
    apply_sort_results();
    sort_in_flight.store(true, std::memory_order_relaxed);
    command_queue.push(this, &IsometricServer::command_isometric_sort);
}

void IsometricServer::apply_sort_results() {
    {
        std::lock_guard<std::mutex> lock(results_mutex);
        if (available_results.is_empty()) { return; }
        applied_results = std::move(available_results);
        available_results = LocalVector<SortResult>();
    }

    RenderingServer* rendering_server = RenderingServer::get_singleton();
    for (const SortResult& result : applied_results) {
        // Elements freed since the sort no longer resolve: their RID is released on the main thread first.
        IsometricElement* element = elements_owner.get_or_null(result.element);
        if (!element) { continue; }
        element->applied_z_order = result.z_order;
        if (!element->canvas_item.is_valid()) { continue; }

        rendering_server->canvas_item_set_z_index(element->canvas_item, result.z_order);
#ifdef DEBUG_ENABLED
        // Untinted canvas items keep the modulate their node gave them.
        bool is_cycle_tinted = is_debug && result.is_invalid;
        if (is_cycle_tinted || element->is_cycle_tinted || element->has_debug_modulate) {
            rendering_server->canvas_item_set_modulate(
              element->canvas_item,
              is_cycle_tinted ? Color(1, 0.5, 0.5) : element->debug_modulate
            );
            element->is_cycle_tinted = is_cycle_tinted;
        }
#endif
    }
    applied_results.clear();
}

void IsometricServer::free_rid(const RID& rid) {
    if (IsometricSpace* space = spaces_owner.get_or_null(rid)) {
        spaces_owner.free(rid);
        command_queue.push(this, &IsometricServer::command_space_delete, space);
        return;
    }
    if (IsometricElement* element = elements_owner.get_or_null(rid)) {
        elements_owner.free(rid);
        command_queue.push(this, &IsometricServer::command_isometric_element_delete, element);
        return;
    }
    LOG_WARNING(vformat("This is not a valid space RID nor a valid isometric element RID: %s", rid.get_id()));
}

void IsometricServer::set_debug(bool p_debug) {
    is_debug = p_debug;
    command_queue.push(this, &IsometricServer::command_set_debug, p_debug);
}

/////////////// SERVER THREAD COMMANDS ///////////////

void IsometricServer::command_space_create(IsometricSpace* space) {
    spaces.push_back(space);
}

void IsometricServer::command_space_attach_isometric_element(IsometricSpace* space, IsometricElement* element) {
    if (element->space) { detach_from_space(element); }
    Chunk* chunk = get_or_create_chunk(space, element->aabb.position);
    element->space = space;
    element->chunk = chunk;
    space->dirty = true;

    if (element->is_dynamic) {
        space->dynamic_elements.push_back(element);
        chunk->dynamic_elements.push_back(element);
        return;
    }

    // Statics never move, so their edges with other statics are computed once, here.
    for (Chunk* neighbor : chunk->all_neighbors) {
        for (IsometricElement* static_element : neighbor->static_elements) {
            if (!utils::are_boxes_overlapping(space->sort_configuration, static_element->aabb, element->aabb)) {
                continue;
            }
            if (utils::is_box_in_front(space->sort_configuration, static_element->aabb, element->aabb)) {
                static_element->behind_statics.push_back(element);
            } else {
                element->behind_statics.push_back(static_element);
            }
        }
    }
    space->static_elements.push_back(element);
    chunk->static_elements.push_back(element);
}

void IsometricServer::command_space_detach_isometric_element(IsometricElement* element) {
    detach_from_space(element);
}

void IsometricServer::detach_from_space(IsometricElement* element) {
    IsometricSpace* space = element->space;
    if (!space) { return; }

    Chunk* chunk = element->chunk;
    space->dirty = true;
    if (element->is_dynamic) {
        space->dynamic_elements.erase(element);
        chunk->dynamic_elements.erase(element);
    } else {
        space->static_elements.erase(element);
        chunk->static_elements.erase(element);
        for (Chunk* neighbor : chunk->all_neighbors) {
            for (IsometricElement* static_element : neighbor->static_elements) {
                static_element->behind_statics.erase(element);
            }
        }
    }
    element->behind_statics.clear();
    element->behind_dynamics.clear();
    element->space = nullptr;
    element->chunk = nullptr;
}

Chunk* IsometricServer::get_or_create_chunk(IsometricSpace* space, const Vector3& position) {
    Vector2i chunk_position(
      static_cast<int>(Math::floor(position.x / IsometricSpace::CHUNK_SIZE)),
      static_cast<int>(Math::floor(position.y / IsometricSpace::CHUNK_SIZE))
    );
    if (Chunk* chunk = space->chunks.getptr(chunk_position)) { return chunk; }

    Chunk* chunk = &space->chunks.insert(chunk_position, Chunk())->value;
    chunk->all_neighbors.push_back(chunk);
    chunk->dynamic_sorting_neighbors.push_back(chunk);
    for (int x = -1; x <= 1; ++x) {
        for (int y = -1; y <= 1; ++y) {
            if (x == 0 && y == 0) { continue; }
            Chunk* neighbor = space->chunks.getptr(chunk_position + Vector2i(x, y));
            if (!neighbor) { continue; }

            chunk->all_neighbors.push_back(neighbor);
            neighbor->all_neighbors.push_back(chunk);
            // Each pair of neighboring chunks compares its dynamic elements once, from the chunk listing the other.
            if (x == -1 || (x == 0 && y == -1)) {
                neighbor->dynamic_sorting_neighbors.push_back(chunk);
            } else {
                chunk->dynamic_sorting_neighbors.push_back(neighbor);
            }
        }
    }
    return chunk;
}

void IsometricServer::command_space_set_configuration(IsometricSpace* space, IsometricParameters configuration) {
    space->sort_configuration = configuration;
    space->dirty = true;
}

void IsometricServer::command_space_delete(IsometricSpace* space) {
    for (IsometricElement* static_element : space->static_elements) {
        static_element->space = nullptr;
        static_element->chunk = nullptr;
        static_element->behind_statics.clear();
        static_element->behind_dynamics.clear();
    }
    for (IsometricElement* dynamic_element : space->dynamic_elements) {
        dynamic_element->space = nullptr;
        dynamic_element->chunk = nullptr;
        dynamic_element->behind_statics.clear();
        dynamic_element->behind_dynamics.clear();
    }
    spaces.erase(space);
    memdelete(space);
}

void IsometricServer::command_isometric_element_set_position(IsometricElement* element, Vector3 global_position) {
    ISO_DEBUG_FAIL_COND_MSG(element->space && !element->is_dynamic, "A static element position can't be changed after being attached to a space.");
    element->aabb.position = global_position;
    IsometricSpace* space = element->space;
    if (!space) { return; }

    space->dirty = true;
    Chunk* new_chunk = get_or_create_chunk(space, global_position);
    if (new_chunk == element->chunk) { return; }
    element->chunk->dynamic_elements.erase(element);
    new_chunk->dynamic_elements.push_back(element);
    element->chunk = new_chunk;
}

void IsometricServer::command_isometric_element_set_size(IsometricElement* element, Vector3 size) {
    ISO_DEBUG_FAIL_COND_MSG(element->space && !element->is_dynamic, "A static element size can't be changed after being attached to a space.");
#ifdef DEBUG_ENABLED
    if (size.x > IsometricSpace::CHUNK_SIZE || size.y > IsometricSpace::CHUNK_SIZE) {
        LOG_WARNING(vformat("Isometric elements larger than %d on x or y may be sorted wrongly.", IsometricSpace::CHUNK_SIZE));
    }
#endif
    element->aabb.size = size;
    if (element->space) { element->space->dirty = true; }
}

void IsometricServer::command_isometric_element_set_depth(IsometricElement* element, int depth) {
    element->depth = depth;
    if (element->space) { element->space->dirty = true; }
}

void IsometricServer::command_isometric_element_mark_dirty(IsometricElement* element) {
    if (element->space) { element->space->dirty = true; }
}

void IsometricServer::command_isometric_element_delete(IsometricElement* element) {
    detach_from_space(element);
    memdelete(element);
}

void IsometricServer::command_isometric_sort() {
    for (IsometricSpace* space : spaces) {
        if (!space->dirty) { continue; }

        for (IsometricElement* static_element : space->static_elements) {
            static_element->behind_dynamics.clear();
            static_element->z_order = 0;
            static_element->dirty = true;
            static_element->is_invalid = false;
        }
        for (IsometricElement* dynamic_element : space->dynamic_elements) {
            dynamic_element->behind_dynamics.clear();
            dynamic_element->behind_statics.clear();
            dynamic_element->z_order = 0;
            dynamic_element->dirty = true;
            dynamic_element->is_invalid = false;
        }
        generate_topological_sorting_graph(space);
        produce_results(space);
        space->dirty = false;
    }

    {
        std::lock_guard<std::mutex> lock(results_mutex);
        for (const SortResult& result : produced_results) {
            available_results.push_back(result);
        }
    }
    produced_results.clear();
    sort_in_flight.store(false, std::memory_order_release);
}

// Records which of two overlapping elements is behind the other; the first one is dynamic.
static void link_if_overlapping(const IsometricParameters& configuration, IsometricElement* dynamic_element, IsometricElement* other) {
    if (!utils::are_boxes_overlapping(configuration, dynamic_element->aabb, other->aabb)) { return; }
    if (utils::is_box_in_front(configuration, dynamic_element->aabb, other->aabb)) {
        (other->is_dynamic ? dynamic_element->behind_dynamics : dynamic_element->behind_statics).push_back(other);
    } else {
        other->behind_dynamics.push_back(dynamic_element);
    }
}

void IsometricServer::generate_topological_sorting_graph(IsometricSpace* p_space) {
    const IsometricParameters& configuration = p_space->sort_configuration;

    for (KeyValue<Vector2i, Chunk>& entry : p_space->chunks) {
        Chunk& chunk = entry.value;
        for (Chunk* neighbor : chunk.dynamic_sorting_neighbors) {
            for (uint32_t i = 0; i < chunk.dynamic_elements.size(); ++i) {
                // Within a chunk, each pair of dynamic elements is compared once.
                uint32_t first_other = neighbor == &chunk ? i + 1 : 0;
                for (uint32_t j = first_other; j < neighbor->dynamic_elements.size(); ++j) {
                    link_if_overlapping(configuration, chunk.dynamic_elements[i], neighbor->dynamic_elements[j]);
                }
            }
        }

        for (IsometricElement* dynamic_element : chunk.dynamic_elements) {
            for (Chunk* neighbor : chunk.all_neighbors) {
                for (IsometricElement* static_element : neighbor->static_elements) {
                    link_if_overlapping(configuration, dynamic_element, static_element);
                }
            }
        }
    }

    for (IsometricElement* static_element : p_space->static_elements) {
        if (static_element->dirty) { sort_isometric_element(static_element); }
    }
    for (IsometricElement* dynamic_element : p_space->dynamic_elements) {
        if (dynamic_element->dirty) { sort_isometric_element(dynamic_element); }
    }
}

void IsometricServer::sort_isometric_element(IsometricElement* p_element) {
    p_element->dirty = false;
    p_element->in_stack = true;
    stack.push_back(p_element);

    int max_z = 0;
    for (IsometricElement* behind : p_element->behind_statics) {
        max_z = calculate_z_order(behind, max_z);
    }
    for (IsometricElement* behind : p_element->behind_dynamics) {
        max_z = calculate_z_order(behind, max_z);
    }

    p_element->in_stack = false;
    stack.remove_at(stack.size() - 1);
    p_element->z_order = max_z;
}

int IsometricServer::calculate_z_order(IsometricElement* element_behind, int current_z_order) {
    if (element_behind->dirty) {
        sort_isometric_element(element_behind);
    } else if (element_behind->in_stack) {
        // A cycle: every element between the top of the stack and element_behind cannot be ordered consistently.
        int64_t index = static_cast<int64_t>(stack.size()) - 1;
        IsometricElement* stack_element = stack[index];
        while (stack_element != element_behind) {
            stack_element->is_invalid = true;
            stack_element = stack[--index];
        }
        stack_element->is_invalid = true;
    }

    int new_z_order = element_behind->depth + element_behind->z_order;
    return new_z_order >= current_z_order ? new_z_order : current_z_order;
}

void IsometricServer::produce_results(IsometricSpace* p_space) {
    for (IsometricElement* static_element : p_space->static_elements) {
        produced_results.push_back({static_element->self, static_element->z_order, static_element->is_invalid});
    }
    for (IsometricElement* dynamic_element : p_space->dynamic_elements) {
        produced_results.push_back({dynamic_element->self, dynamic_element->z_order, dynamic_element->is_invalid});
    }
}

void IsometricServer::command_set_debug(bool p_debug) {
    for (IsometricSpace* space : spaces) {
        space->dirty = true;
    }
}

void IsometricServer::command_stop_server() {
    exit_thread = true;
}

/////////////// BINDING ///////////////

void IsometricServer::_bind_methods() {
    ClassDB::bind_method(D_METHOD("space_create"), &IsometricServer::space_create);
    ClassDB::bind_method(D_METHOD("space_attach_isometric_element", "space", "element"), &IsometricServer::space_attach_isometric_element);
    ClassDB::bind_method(D_METHOD("space_detach_isometric_element", "element"), &IsometricServer::space_detach_isometric_element);
    ClassDB::bind_method(D_METHOD("space_update_configuration", "space", "configuration"), &IsometricServer::space_update_configuration);
    ClassDB::bind_method(D_METHOD("space_get_diamond_width", "space"), &IsometricServer::space_get_diamond_width);
    ClassDB::bind_method(D_METHOD("space_get_diamond_height", "space"), &IsometricServer::space_get_diamond_height);
    ClassDB::bind_method(D_METHOD("space_get_z_length", "space"), &IsometricServer::space_get_z_length);

    ClassDB::bind_method(D_METHOD("isometric_element_create", "is_dynamic", "aabb"), &IsometricServer::isometric_element_create, DEFVAL(AABB()));
    ClassDB::bind_method(
      D_METHOD("isometric_element_attach_canvas_item", "element", "canvas_item", "depth"),
      &IsometricServer::isometric_element_attach_canvas_item,
      DEFVAL(1)
    );
    ClassDB::bind_method(D_METHOD("isometric_element_detach_canvas_item", "element"), &IsometricServer::isometric_element_detach_canvas_item);
    ClassDB::bind_method(D_METHOD("isometric_element_set_position", "element", "global_position"), &IsometricServer::isometric_element_set_position);
    ClassDB::bind_method(D_METHOD("isometric_element_set_size", "element", "size"), &IsometricServer::isometric_element_set_size);
    ClassDB::bind_method(D_METHOD("isometric_element_set_depth", "element", "depth"), &IsometricServer::isometric_element_set_depth);
    ClassDB::bind_method(D_METHOD("isometric_element_get_z_order", "element"), &IsometricServer::isometric_element_get_z_order);

    ClassDB::bind_method(D_METHOD("fetch_data_and_request_ordering"), &IsometricServer::fetch_data_and_request_ordering);
    ClassDB::bind_method(D_METHOD("set_synchronous_mode", "sync"), &IsometricServer::set_synchronous_mode);
    ClassDB::bind_method(D_METHOD("is_synchronous_mode"), &IsometricServer::is_synchronous_mode);

    ClassDB::bind_method(D_METHOD("free_rid", "rid"), &IsometricServer::free_rid);

    ClassDB::bind_method(D_METHOD("set_debug", "debug"), &IsometricServer::set_debug);
}
