#include "isometric_configuration.h"

#include "data/isometric_parameters.h"

using namespace godot;
using namespace resource;

IsometricConfiguration::IsometricConfiguration() :
  tile_width(DEFAULT_WIDTH),
  angle(DEFAULT_ANGLE),
  topological_margin(DEFAULT_MARGIN) {}

int IsometricConfiguration::get_tile_width() const {
    return tile_width;
}

void IsometricConfiguration::set_tile_width(int p_tile_width) {
    tile_width = p_tile_width;
}

int IsometricConfiguration::get_angle() const {
    return angle;
}

void IsometricConfiguration::set_angle(int p_angle) {
    angle = p_angle;
}

float IsometricConfiguration::get_topological_margin() const {
    return topological_margin;
}

void IsometricConfiguration::set_topological_margin(float p_margin) {
    topological_margin = p_margin;
}

void IsometricConfiguration::_bind_methods() {
    ClassDB::bind_method(D_METHOD("get_angle"), &IsometricConfiguration::get_angle);
    ClassDB::bind_method(D_METHOD("set_angle", "angle"), &IsometricConfiguration::set_angle);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "angle"), "set_angle", "get_angle");

    ClassDB::bind_method(D_METHOD("get_tile_width"), &IsometricConfiguration::get_tile_width);
    ClassDB::bind_method(D_METHOD("set_tile_width", "tile_width"), &IsometricConfiguration::set_tile_width);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "diamond_width"), "set_tile_width", "get_tile_width");

    ClassDB::bind_method(D_METHOD("get_margin"), &IsometricConfiguration::get_topological_margin);
    ClassDB::bind_method(D_METHOD("set_margin", "margin"), &IsometricConfiguration::set_topological_margin);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "margin"), "set_margin", "get_margin");
}
