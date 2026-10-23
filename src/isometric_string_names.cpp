#include "isometric_string_names.h"

#include <core/memory.hpp>

using namespace godot;

IsometricStringNames* IsometricStringNames::singleton = nullptr;

void IsometricStringNames::create() {
    singleton = memnew(IsometricStringNames);
}

void IsometricStringNames::free() {
    memdelete(singleton);
    singleton = nullptr;
}
