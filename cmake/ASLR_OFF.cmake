
####################################################################################################
# disable ASLR
####################################################################################################

if(EG_ASLR_OFF)
    if(MSVC)
        # MSVC / clang-cl
        add_link_options(/DYNAMICBASE:NO)

    elseif(MINGW)
        # MinGW-w64 GNU linker
        add_link_options(-Wl,--disable-dynamicbase)

    elseif(UNIX AND NOT APPLE)
        # Linux: ASLR is not normally controlled at link time.
        # Disable PIE (position independent executable)
        add_link_options(-no-pie)
        add_compile_options(-fno-pie)

    endif()
endif()
