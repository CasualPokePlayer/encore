
set(CURRENT_MODULE_DIR ${CMAKE_CURRENT_LIST_DIR})

function(download_moltenvk)
    if (IOS)
        set(MOLTENVK_PLATFORM "iOS")
    else()
        set(MOLTENVK_PLATFORM "macOS")
    endif()

    set(MOLTENVK_DIR "${CMAKE_BINARY_DIR}/externals/MoltenVK")
    set(MOLTENVK_TAR "${CMAKE_BINARY_DIR}/externals/MoltenVK.tar")
    if (NOT EXISTS ${MOLTENVK_DIR})
        if (NOT EXISTS ${MOLTENVK_TAR})
            file(DOWNLOAD https://github.com/KhronosGroup/MoltenVK/releases/download/v1.2.7-rc2/MoltenVK-all.tar
                ${MOLTENVK_TAR} SHOW_PROGRESS)
        endif()

        execute_process(COMMAND ${CMAKE_COMMAND} -E tar xf "${MOLTENVK_TAR}"
            WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/externals")
    endif()

    # Add the MoltenVK library path to the prefix so find_library can locate it.
    list(APPEND CMAKE_PREFIX_PATH "${MOLTENVK_DIR}/MoltenVK/dylib/${MOLTENVK_PLATFORM}")
    set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} PARENT_SCOPE)
endfunction()
