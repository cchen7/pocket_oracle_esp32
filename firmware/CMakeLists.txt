cmake_minimum_required(VERSION 3.16)

set(PROJECT_VER "0.1.0")

# GCC 14 in ESP-IDF 5.4 flags designated-initializer style as a warning. Vendor
# components (M5Unified, M5GFX) use it heavily; project-wide downgrade is safe.
add_compile_options(-Wno-missing-field-initializers)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

project(pocket_oracle)

# Vendor components also trip GCC 14's may-be-uninitialized analyzer; relax
# -Werror just for those targets so we still catch the warning in our own code.
foreach(vendor_component IN ITEMS M5Unified M5GFX)
    if(TARGET __idf_${vendor_component})
        target_compile_options(__idf_${vendor_component}
            PRIVATE -Wno-error=maybe-uninitialized)
    endif()
endforeach()
