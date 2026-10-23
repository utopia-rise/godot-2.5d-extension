#ifndef ISOMETRIC_MAPS_ISOMETRIC_SPACE_H
#define ISOMETRIC_MAPS_ISOMETRIC_SPACE_H

#include "isometric_element.h"
#include "isometric_parameters.h"

#include <templates/local_vector.hpp>

namespace data {
    struct IsometricSpace {
        //////// Main thread ////////
        IsometricParameters configuration = IsometricParameters::get_default_configuration();

        //////// Server thread ////////
        IsometricParameters sort_configuration = IsometricParameters::get_default_configuration();
        godot::LocalVector<IsometricElement*> static_elements;
        godot::LocalVector<IsometricElement*> dynamic_elements;
        bool dirty = false;

        IsometricSpace() {
            static_elements.reserve(512);
            dynamic_elements.reserve(32);
        }
    };
}// namespace data

#endif// ISOMETRIC_MAPS_ISOMETRIC_SPACE_H
