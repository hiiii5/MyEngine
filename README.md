# MyEngine

This is my sandbox game engine to teach myself engine development. It is a hobby work for educational purposes.

# Dependencies

Any dependencies not already on the host system at their respective versions
will automatically be fetched via cmake. All submodules for the engine are located in the `/MyEngine/vendor` folder.

Be sure to clone the git submodules before building from source. The shaderc library requires running `/MyEngine/vendor/shaderc/utils/git-sync-deps`.

- [glm](http://github.com/g-truc/glm.git)
- [SDL2](https://github.com/libsdl-org/SDL.git)
- [Vulkan](https://github.com/KhronosGroup/Vulkan-Hpp.git)

# Building From Source (CMAKE)

The build script is used to build the engine and testing sandbox.

When inside the MyEngine directory call:
```bash
./scripts/build.sh -c Debug
```

For multi-configuration generation call:
```bash
./scripts/build.sh -m
```

# Running

To run the testing sandbox call:
```bash
./scripts/run.sh
```
