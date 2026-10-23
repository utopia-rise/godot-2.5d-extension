#ifndef ISOMETRIC_MAPS_GRID_3D_H
#define ISOMETRIC_MAPS_GRID_3D_H

#include <templates/vector.hpp>
#include <variant/aabb.hpp>
#include <variant/array.hpp>
#include <variant/vector3.hpp>
#include <variant/vector3i.hpp>

namespace containers {
    using namespace godot;

    template<class T, T default_value>
    class Grid3D {
    private:
        int width = 1;
        int depth = 1;
        int height = 1;

        Vector<T> internal_array;

        bool is_in_range(int x, int y, int z) const;

        int get_index_from_position(const Vector3& position) const;

        Vector3 plane_square_and_jumps_from(const Vector3& size) const;

        int compute_array_size() const { return width * depth * height; }

        static int index_increment_from(const Vector3& plane_square_and_jumps, const Vector3& size, int iteration);

    public:
        Grid3D();

        void update_array_size(const Vector3& size, bool reset = false);

        Vector3i get_dimensions() const;

        void reset();

        T get_data(const Vector3& position) const;

        void set_data(const Vector3& position, T data);

        Vector3 get_position_3d_from_index(int id) const;

        bool insert_box(const AABB& aabb, T data, bool remove = false);

        bool is_overlapping(const AABB& aabb) const;

        bool has(T object) const;

        int get_width() const;

        int get_depth() const;

        int get_height() const;

        const Vector<T>& get_internal_array() const;

        void set_internal_array(const Vector<T>& array);

        static T get_default_value();

        Array to_array() const;
        void from_array(const Array& array);
    };

    template<class T, T default_value>
    Grid3D<T, default_value>::Grid3D() {
        update_array_size(Vector3(1, 1, 1));
    }

    template<class T, T default_value>
    void Grid3D<T, default_value>::update_array_size(const Vector3& size, bool reset) {
        int64_t old_size = internal_array.size();
        Vector<T> old_array = internal_array;
        int old_width = width;
        int old_depth = depth;

        width = static_cast<int>(size.x);
        depth = static_cast<int>(size.y);
        height = static_cast<int>(size.z);
        internal_array = Vector<T>();
        internal_array.resize(compute_array_size());
        internal_array.fill(default_value);

        if (reset) { return; }

        for (int i = 0; i < old_size; i++) {
            int x = i % old_width;
            int y = (i / old_width) % old_depth;
            int z = i / (old_width * old_depth);

            if (!is_in_range(x, y, z)) { continue; }

            T value = old_array[i];
            if (value != default_value) {
                set_data({static_cast<real_t>(x), static_cast<real_t>(y), static_cast<real_t>(z)}, value);
            }
        }
    }

    template<class T, T default_value>
    Vector3i Grid3D<T, default_value>::get_dimensions() const {
        return {width, depth, height};
    }

    template<class T, T default_value>
    void Grid3D<T, default_value>::reset() {
        update_array_size({static_cast<real_t>(width), static_cast<real_t>(depth), static_cast<real_t>(height)}, true);
    }

    template<class T, T default_value>
    T Grid3D<T, default_value>::get_data(const Vector3& position) const {
        int pos_x = static_cast<int>(position.x);
        int pos_y = static_cast<int>(position.y);
        int pos_z = static_cast<int>(position.z);
        return pos_x >= 0 && pos_x < width && pos_y >= 0 && pos_y < depth && pos_z >= 0 && pos_z < height
               ? internal_array[pos_x + width * pos_y + width * depth * pos_z]
               : default_value;
    }

    template<class T, T default_value>
    void Grid3D<T, default_value>::set_data(const Vector3& position, T data) {
        internal_array.set(get_index_from_position(position), data);
    }

    template<class T, T default_value>
    bool Grid3D<T, default_value>::insert_box(const AABB& aabb, T data, bool remove) {
        const Vector3& position = aabb.position;
        const Vector3& size = aabb.size;
        int index = get_index_from_position(position);
        int size_x = static_cast<int>(size.x);
        int size_y = static_cast<int>(size.y);
        int size_z = static_cast<int>(size.z);
        int end_x = static_cast<int>(position.x) + size_x;
        int end_y = static_cast<int>(position.y) + size_y;
        int end_z = static_cast<int>(position.z) + size_z;

        if (end_x > width || end_y > depth || end_z > height) { return false; }

        T value = remove ? default_value : data;
        Vector3 plane_square_and_jumps = plane_square_and_jumps_from(size);
        internal_array.set(index, value);
        for (int i = 1; i < size_x * size_y * size_z; i++) {
            index += index_increment_from(plane_square_and_jumps, size, i);
            internal_array.set(index, value);
        }
        return true;
    }

    template<class T, T default_value>
    bool Grid3D<T, default_value>::is_overlapping(const AABB& aabb) const {
        int index = get_index_from_position(aabb.position);
        const Vector3& size = aabb.size;

        if (index < 0 || index >= internal_array.size()) { return false; }
        if (internal_array[index] != default_value) { return true; }

        Vector3 plane_square_and_jumps = plane_square_and_jumps_from(size);
        int cell_count = static_cast<int>(size.x) * static_cast<int>(size.y) * static_cast<int>(size.z);
        for (int i = 1; i < cell_count; ++i) {
            index += index_increment_from(plane_square_and_jumps, size, i);
            if (index >= 0 && index < internal_array.size() && internal_array[index] != default_value) { return true; }
        }
        return false;
    }

    template<class T, T default_value>
    bool Grid3D<T, default_value>::has(T object) const {
        return internal_array.has(object);
    }

    template<class T, T default_value>
    int Grid3D<T, default_value>::get_width() const {
        return width;
    }

    template<class T, T default_value>
    int Grid3D<T, default_value>::get_depth() const {
        return depth;
    }

    template<class T, T default_value>
    int Grid3D<T, default_value>::get_height() const {
        return height;
    }

    template<class T, T default_value>
    const Vector<T>& Grid3D<T, default_value>::get_internal_array() const {
        return internal_array;
    }

    template<class T, T default_value>
    void Grid3D<T, default_value>::set_internal_array(const Vector<T>& array) {
        internal_array = array;
    }

    template<class T, T default_value>
    bool Grid3D<T, default_value>::is_in_range(int x, int y, int z) const {
        return x < width && y < depth && z < height;
    }

    template<class T, T default_value>
    int Grid3D<T, default_value>::get_index_from_position(const Vector3& position) const {
        return static_cast<int>(position.x) + width * static_cast<int>(position.y)
             + width * depth * static_cast<int>(position.z);
    }

    template<class T, T default_value>
    Vector3 Grid3D<T, default_value>::get_position_3d_from_index(int id) const {
        return {static_cast<real_t>(id % width), static_cast<real_t>((id / width) % depth), static_cast<real_t>(id / (width * depth))};
    }

    template<class T, T default_value>
    Vector3 Grid3D<T, default_value>::plane_square_and_jumps_from(const Vector3& size) const {
        return {size.x * size.y, static_cast<real_t>(width) - size.x, width * (depth - size.y)};
    }

    template<class T, T default_value>
    int Grid3D<T, default_value>::index_increment_from(const Vector3& plane_square_and_jumps, const Vector3& size, int iteration) {
        int increment = 1;
        if (iteration % static_cast<int>(size.x) == 0) { increment += static_cast<int>(plane_square_and_jumps.y); }
        if (iteration % static_cast<int>(plane_square_and_jumps.x) == 0) {
            increment += static_cast<int>(plane_square_and_jumps.z);
        }
        return increment;
    }

    template<class T, T default_value>
    T Grid3D<T, default_value>::get_default_value() {
        return default_value;
    }

    template<class T, T default_value>
    Array Grid3D<T, default_value>::to_array() const {
        Array ret;
        ret.resize(internal_array.size());
        for (int i = 0; i < internal_array.size(); ++i) {
            ret[i] = internal_array[i];
        }
        return ret;
    }

    template<class T, T default_value>
    void Grid3D<T, default_value>::from_array(const Array& array) {
        Vector<T> new_internal_array;
        new_internal_array.resize(array.size());
        for (int i = 0; i < array.size(); ++i) {
            new_internal_array.set(i, array[i]);
        }
        set_internal_array(new_internal_array);
    }

}// namespace containers

#endif// ISOMETRIC_MAPS_GRID_3D_H
