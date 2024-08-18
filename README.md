# MyEngine

This is my sandbox game engine to teach myself engine development. It is a hobby work for educational purposes.

# Dependencies

Any dependencies not already on the host system at their respective versions
will automatically be fetched via cmake. One exclusion is for compiling shaders.
GlslangValidator is required for this and is bundled with the VulkanSDK.
If you are rely on fetch content for Vulkan, be sure to install glslangValidator
to your local path.

- [fmt](https://github.com/fmtlib/fmt)
- [glm](http://github.com/g-truc/glm.git)
- [SDL2](https://github.com/libsdl-org/SDL.git)
- [Vulkan](https://github.com/KhronosGroup/Vulkan-Hpp.git)
- [glslangValidator](https://github.com/KhronosGroup/glslang)

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
