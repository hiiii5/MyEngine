#ifdef _WIN32
#ifdef _WIN64
#define ME_PLATFORM_WINDOWS
#else
#error "x86 Builds are not supported!"
#endif
#elif defined(__APPLE__) || defined(__MACH__)
#error "Apple platform not supported!"
#elif defined(__linux__)
#define ME_PLATFORM_LINUX
#else
#define #error "Unknown platform!"
#endif
