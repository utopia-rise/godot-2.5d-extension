#ifndef ISOMETRIC_MAPS_ISOMETRIC_ELEMENT_H
#define ISOMETRIC_MAPS_ISOMETRIC_ELEMENT_H

#include <templates/local_vector.hpp>
#include <variant/aabb.hpp>
#include <variant/color.hpp>
#include <variant/rid.hpp>

namespace data {
    struct Chunk;
    struct IsometricSpace;

    // Each field is owned by one thread: the main thread through the IsometricServer API, or the server thread through
    // commands. Ownership never changes, which is what lets the sort run without locking elements.
    struct IsometricElement {
        //////// Immutable after creation ////////
        godot::RID self;

        //////// Main thread ////////
        godot::RID canvas_item;
        bool is_dynamic = false;
        int applied_z_order = 0;

#ifdef DEBUG_ENABLED
        // Debug tint, such as tiles overlapping in a map. Applied only once set, so the canvas item keeps its own
        // modulate otherwise.
        godot::Color debug_modulate = godot::Color(1, 1, 1);
        bool has_debug_modulate = false;
        bool is_cycle_tinted = false;
#endif

        //////// Server thread ////////
        IsometricSpace* space = nullptr;
        Chunk* chunk = nullptr;
        godot::AABB aabb = godot::AABB(godot::Vector3(0, 0, 0), godot::Vector3(1, 1, 1));
        int depth = 1;
        int z_order = 0;
        bool dirty = true;
        bool in_stack = false;
        bool is_invalid = false;
        // Last result handed to the main thread: unchanged results are not sent again. needs_result forces the next one.
        int produced_z_order = 0;
        bool produced_is_invalid = false;
        bool needs_result = true;

        godot::LocalVector<IsometricElement*> behind_statics;
        godot::LocalVector<IsometricElement*> behind_dynamics;
    };
}// namespace data

#endif// ISOMETRIC_MAPS_ISOMETRIC_ELEMENT_H
