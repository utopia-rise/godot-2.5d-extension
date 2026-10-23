#ifndef ISOMETRIC_MAPS_ISOMETRIC_SERVER_H
#define ISOMETRIC_MAPS_ISOMETRIC_SERVER_H

#include "containers/command_queue.h"
#include "data/isometric_element.h"
#include "data/isometric_space.h"
#include "resource/isometric_configuration.h"

#include <atomic>
#include <classes/object.hpp>
#include <classes/worker_thread_pool.hpp>
#include <core/class_db.hpp>
#include <mutex>
#include <templates/local_vector.hpp>
#include <templates/rid_owner.hpp>
#include <variant/aabb.hpp>
#include <variant/color.hpp>
#include <variant/rid.hpp>

#define ISOMETRIC_SERVER IsometricServer::get_instance()

// Sorts isometric elements by depth. Like the engine servers (rendering, physics), it owns a dedicated thread: a
// high-priority WorkerThreadPool task consuming a command queue. The main thread only allocates RIDs and pushes
// commands; every sort runs on the server thread, and the main thread applies the resulting z-indices to the
// RenderingServer right before a frame is drawn.
class IsometricServer : public godot::Object {
    GDCLASS(IsometricServer, godot::Object);

    struct SortResult {
        godot::RID element;
        int z_order;
        bool is_invalid;
    };

    static IsometricServer* singleton;

    containers::CommandQueue command_queue;
    godot::WorkerThreadPool::TaskID server_task_id = -1;
    bool exit_thread = false;

    bool synchronous = false;
    std::atomic<bool> sort_in_flight = false;
    bool is_debug = false;

    godot::RID_PtrOwner<data::IsometricSpace> spaces_owner;
    godot::RID_PtrOwner<data::IsometricElement> elements_owner;

    // Server thread.
    godot::LocalVector<data::IsometricSpace*> spaces;
    godot::LocalVector<data::IsometricElement*> stack;
    godot::LocalVector<SortResult> produced_results;

    // Handed from the server thread to the main thread.
    std::mutex results_mutex;
    godot::LocalVector<SortResult> available_results;

    // Main thread.
    godot::LocalVector<SortResult> applied_results;

    static void thread_loop(void* p_server);

    void hook_into_main_loop();
    void stop_thread();

    void apply_sort_results();

public:
    IsometricServer();
    ~IsometricServer() override = default;

    IsometricServer(const IsometricServer&) = delete;

    static void create_server();
    static IsometricServer* get_instance();
    static void terminate_server();

    ///////////////////////// PUBLIC API /////////////////////////

    // SPACE
    godot::RID space_create();
    void space_attach_isometric_element(const godot::RID& space_rid, const godot::RID& element_rid);
    void space_detach_isometric_element(const godot::RID& element_rid);
    void space_update_configuration(const godot::RID& space_rid, const godot::Ref<resource::IsometricConfiguration>& p_configuration);
    const data::IsometricParameters* space_get_configuration(const godot::RID& space_rid);
    int space_get_diamond_width(const godot::RID& space_rid);
    int space_get_diamond_height(const godot::RID& space_rid);
    float space_get_z_length(const godot::RID& space_rid);

    // ISOMETRIC ELEMENT
    godot::RID isometric_element_create(bool is_dynamic, const godot::AABB& aabb = godot::AABB());
    void isometric_element_attach_canvas_item(const godot::RID& element_rid, const godot::RID& canvas_rid, int depth = 1);
    void isometric_element_detach_canvas_item(const godot::RID& element_rid);
    void isometric_element_set_position(const godot::RID& element_rid, const godot::Vector3& global_position);
    void isometric_element_set_size(const godot::RID& element_rid, const godot::Vector3& size);
    void isometric_element_set_depth(const godot::RID& element_rid, int depth);
    int isometric_element_get_z_order(const godot::RID& element_rid);

#ifdef TOOLS_ENABLED
    void isometric_element_set_editor_modulate(const godot::RID& element_rid, const godot::Color& color);
#endif

    // ORDERING
    void fetch_data_and_request_ordering();
    void set_synchronous_mode(bool sync);
    bool is_synchronous_mode() const;

    // DELETE
    void free_rid(const godot::RID& rid);

    // UTILITIES
    void set_debug(bool p_debug);

private:
    ///////////////////////// SERVER THREAD COMMANDS /////////////////////////
    void command_space_create(data::IsometricSpace* space);
    void command_space_attach_isometric_element(data::IsometricSpace* space, data::IsometricElement* element);
    void command_space_detach_isometric_element(data::IsometricElement* element);
    void command_space_set_configuration(data::IsometricSpace* space, data::IsometricParameters configuration);
    void command_space_delete(data::IsometricSpace* space);

    void command_isometric_element_set_position(data::IsometricElement* element, godot::Vector3 global_position);
    void command_isometric_element_set_size(data::IsometricElement* element, godot::Vector3 size);
    void command_isometric_element_set_depth(data::IsometricElement* element, int depth);
    void command_isometric_element_mark_dirty(data::IsometricElement* element);
    void command_isometric_element_delete(data::IsometricElement* element);

    void command_isometric_sort();
    void generate_topological_sorting_graph(data::IsometricSpace* p_space);
    void sort_isometric_element(data::IsometricElement* p_element);
    int calculate_z_order(data::IsometricElement* element_behind, int current_z_order);
    void produce_results(data::IsometricSpace* p_space);

    void command_set_debug(bool p_debug);
    void command_stop_server();

    static void detach_from_space(data::IsometricElement* element);

protected:
    static void _bind_methods();
};

#endif// ISOMETRIC_MAPS_ISOMETRIC_SERVER_H
