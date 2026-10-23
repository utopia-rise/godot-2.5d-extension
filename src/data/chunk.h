#ifndef ISOMETRIC_MAPS_CHUNK_H
#define ISOMETRIC_MAPS_CHUNK_H

#include "isometric_element.h"

#include <templates/local_vector.hpp>

namespace data {
    // A square of a space, on x and y, used to only compare elements with the ones nearby.
    struct Chunk {
        godot::LocalVector<IsometricElement*> static_elements;
        godot::LocalVector<IsometricElement*> dynamic_elements;

        // This chunk and its existing neighbors (up to 9).
        godot::LocalVector<Chunk*> all_neighbors;
        // This chunk and the neighbors it compares dynamic elements with: each pair of chunks is listed only once.
        godot::LocalVector<Chunk*> dynamic_sorting_neighbors;

        Chunk() {
            all_neighbors.reserve(9);
            dynamic_sorting_neighbors.reserve(5);
            static_elements.reserve(256);
            dynamic_elements.reserve(16);
        }
    };
}// namespace data

#endif// ISOMETRIC_MAPS_CHUNK_H
