# libsm64 - Super Mario 64 as a library

The purpose of this project is to provide a clean interface to the movement and rendering
code which was reversed from SM64 by the [SM64 decompilation project](https://github.com/n64decomp/sm64),
so that Mario can be dropped in to existing game engines or other systems with minimal effort.
This project produces a shared library file containing mostly code from the decompilation project,
and loads an official SM64 ROM at runtime to get Mario's texture and animation data, so any project
which makes use of this library must ask the user to provide a ROM for asset extraction.

## Building

Currently only linux is supported. Windows support coming soon. Requires python3 to build the library,
and SDL2 + GLEW for the test program.

- `make lib`: Build the `dist` directory, containing the shared object and public-facing header.
- `make test`: (Default) Builds the library `dist` directory as well as the test program.
- `make run`: Build and run the SDL+OpenGL test program.

To run the test program you'll need a SM64 US ROM in the root of the repository with the name `baserom.us.z64`.
