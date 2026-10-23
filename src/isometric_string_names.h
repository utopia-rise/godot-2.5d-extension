#ifndef ISOMETRIC_MAPS_ISOMETRIC_STRING_NAMES_H
#define ISOMETRIC_MAPS_ISOMETRIC_STRING_NAMES_H

#include <variant/string_name.hpp>

// StringNames cannot be static globals in an extension: they must be created after and freed before the engine's
// StringName table, so they live with the extension's scene initialization level.
class IsometricStringNames {
    static IsometricStringNames* singleton;

public:
    static IsometricStringNames* get_singleton() { return singleton; }

    static void create();
    static void free();

    godot::StringName debug_group_name = "isometric_debug_view";
    godot::StringName size_changed_signal = "size_changed";
};

#endif// ISOMETRIC_MAPS_ISOMETRIC_STRING_NAMES_H
