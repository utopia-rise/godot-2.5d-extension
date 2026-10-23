#include "isometric_parameters.h"

#include <core/math.hpp>

using namespace godot;
using namespace data;

static int calculate_tile_height(int tile_width, int angle) {
    return static_cast<int>(Math::round(tile_width * Math::sin(Math::deg_to_rad(static_cast<double>(angle)))));
}

static float calculate_ez(int tile_width, int angle) {
    return static_cast<float>((tile_width / Math::sqrt(2.0)) * Math::cos(Math::deg_to_rad(static_cast<double>(angle))));
}

IsometricParameters::IsometricParameters(int p_tile_width, int p_angle, float p_topological_margin) :
  diamond_width(p_tile_width),
  diamond_height(calculate_tile_height(p_tile_width, p_angle)),
  z_length(calculate_ez(p_tile_width, p_angle)),
  z_ratio(z_length / static_cast<float>(diamond_height)),
  topological_margin(p_topological_margin) {}

IsometricParameters IsometricParameters::get_default_configuration() {
    static const IsometricParameters default_configuration(DEFAULT_WIDTH, DEFAULT_ANGLE, DEFAULT_MARGIN);
    return default_configuration;
}
