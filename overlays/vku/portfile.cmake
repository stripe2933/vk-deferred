vcpkg_check_linkage(ONLY_STATIC_LIBRARY)

vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO stripe2933/vku
    REF v0.1.0-test22
    SHA512 e090d7a4995a4e740272352f24d92b86344a94d1fa65f0697a097f43911490bf4ef3d8416af5904a80a2a276ce4429635679c6db71945c4656538098e132de0a
    HEAD_REF module
    PATCHES vcpkg-deps.patch
)

# Module project doesn't use header files.
set(VCPKG_POLICY_EMPTY_INCLUDE_FOLDER enabled)

# Set CMake variables from the requested features.
vcpkg_check_features(OUT_FEATURE_OPTIONS FEATURE_OPTIONS
    FEATURES
        std-module VKU_USE_STD_MODULE
        shaderc VKU_USE_SHADERC
        dynamic-dispatcher VKU_DEFAULT_DYNAMIC_DISPATCHER
)

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
    OPTIONS ${FEATURE_OPTIONS}
)
vcpkg_cmake_install()

file(MAKE_DIRECTORY ${CURRENT_PACKAGES_DIR}/debug/share)
file(RENAME ${CURRENT_PACKAGES_DIR}/debug/cmake/vku ${CURRENT_PACKAGES_DIR}/debug/share/vku)
file(MAKE_DIRECTORY ${CURRENT_PACKAGES_DIR}/share)
file(RENAME ${CURRENT_PACKAGES_DIR}/cmake/vku ${CURRENT_PACKAGES_DIR}/share/vku)

vcpkg_cmake_config_fixup(PACKAGE_NAME "vku")

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/cmake" "${CURRENT_PACKAGES_DIR}/debug/cmake")

file(INSTALL "${SOURCE_PATH}/LICENSE.txt" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}" RENAME copyright)
configure_file("${CMAKE_CURRENT_LIST_DIR}/usage" "${CURRENT_PACKAGES_DIR}/share/${PORT}/usage" COPYONLY)
