#ifndef ISOMETRIC_MAPS_ISOMETRIC_CONFIGURATION_H
#define ISOMETRIC_MAPS_ISOMETRIC_CONFIGURATION_H

#include <classes/resource.hpp>

namespace resource {
    using namespace godot;

    class IsometricConfiguration : public Resource {
        GDCLASS(IsometricConfiguration, Resource)

        int tile_width;
        int angle;
        float topological_margin;

    public:
        IsometricConfiguration();
        ~IsometricConfiguration() override = default;

        int get_angle() const;
        void set_angle(int p_angle);

        int get_tile_width() const;
        void set_tile_width(int p_tile_width);

        float get_topological_margin() const;
        void set_topological_margin(float p_margin);

    protected:
        static void _bind_methods();
    };
}// namespace resource

#endif// ISOMETRIC_MAPS_ISOMETRIC_CONFIGURATION_H
