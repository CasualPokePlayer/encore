option(USE_SYSTEM_LIBS "Use system libraries over bundled ones" OFF)

# System library options
CMAKE_DEPENDENT_OPTION(USE_SYSTEM_MOLTENVK "Use the system MoltenVK lib (instead of the bundled one)" OFF "APPLE" OFF)
