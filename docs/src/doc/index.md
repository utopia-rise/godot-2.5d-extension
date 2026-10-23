[![GitHub](https://img.shields.io/github/license/utopia-rise/godot-2.5D-isometric-map-editor?style=flat-square)](LICENSE)

A Godot 4.7 GDExtension to create complex 2.5D isometric maps.  

Classic tiled map does not permit to create complex isometric map with 3D logic. This tool is designed so. Here
every basic node is an `IsometricPositionable`, which means it has 3D coordinated and size (an `AABB`). Then a
topological graph is generated so that tiles are well ordered, according to their 3D position.  
An `IsometricPositionable` can be sumbitted to 3D physics if it has a godot physics child node.  

`IsometricPositionable` is a godot node. This means you can quickly derive from one, use a tile or a map in several maps, and
change an element used in many maps without taking care of editing those maps. In other words, this tool is designed for
re-usability of assets and easy fix.

**Disclaimer:** This project is still work in progress and not ready for production.
