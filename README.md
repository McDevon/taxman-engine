# Taxman Engine
Taxman is a Game Engine designed originally for the [Playdate](https://play.date) console. However, it is not platform-dependent.

The engine is written in C for high portability and speed in small-scale devices. No optimisation has been done, as the target device is not yet available for purchase.

Some features of the engine are based on the idea of having 1-bit screen, but adding support for more colors is entirely possible.

## Features

Current Features include

- Custom object type with runtime type information to allow easier memory management (destructor-like behavior)
- Scenes and transforms between scenes
- Scene Graph, Game Objects, reusable Components
- Dynamic ArrayLists, HashMaps, and String Builders, which support the memory management scheme
- Dithering to draw grayscale images
- Bitmap fonts (currently monospace only)
- Sprite atlases (currently only using LibGDX format)
- Frame animations
- Transform animations
- Easings for transform animations, including cubic bezier curves
- Tilemaps with automatic tile selection
- Platformer physics with one-directional platforms and walls
- Collision detection using sweep-and-prune algorithm
- Camera
- Pseudorandom number generator

The engine uses fixed-point numbers for in-game calculations. This is due to the original game idea needing platform-indepent deterministic game replays using only player inputs. Option to use floats instead is on the works.

## Rough Backlog

- How-to-use guide
- Examples of scenes, objects, components, and game state
- Audio support
- 9-sprite
- Easier UI layout scheme using object's top-left corner as origin for child objects
- Support sprite rotation in sprite atlases
- Moving platforms and pushable objects
- Clean exit (clear all allocated memory and quit)
- Option to use floats instead of fixed-point numbers as the primary number type
- Tool to compute bezier curve values ahead-of-time