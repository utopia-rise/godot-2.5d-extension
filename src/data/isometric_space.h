#ifndef ISOMETRIC_MAPS_ISOMETRIC_SPACE_H
#define ISOMETRIC_MAPS_ISOMETRIC_SPACE_H

#include "chunk.h"
#include "isometric_element.h"
#include "isometric_parameters.h"

#include <templates/hash_map.hpp>
#include <templates/local_vector.hpp>
#include <variant/vector2i.hpp>

namespace data {
    struct IsometricSpace {
        // Elements are only compared with the ones of the neighboring chunks, so an element larger than a chunk on x
        // or y can miss some of the elements it overlaps.
        static constexpr int CHUNK_SIZE = 10;

        //////// Main thread ////////
        IsometricParameters configuration = IsometricParameters::get_default_configuration();

        //////// Server thread ////////
        IsometricParameters sort_configuration = IsometricParameters::get_default_configuration();
        godot::LocalVector<IsometricElement*> static_elements;
        godot::LocalVector<IsometricElement*> dynamic_elements;
        // HashMap allocates its elements one by one, so chunk pointers stay valid as chunks are added.
        godot::HashMap<godot::Vector2i, Chunk> chunks;
        bool dirty = false;

        IsometricSpace() {
            static_elements.reserve(512);
            dynamic_elements.reserve(32);
        }
    };
}// namespace data

#endif// ISOMETRIC_MAPS_ISOMETRIC_SPACE_H
