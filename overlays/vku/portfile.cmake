vcpkg_check_linkage(ONLY_STATIC_LIBRARY)

vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO stripe2933/vku
    REF v0.1.0-test31
    HEAD_REF module
    SHA512 4e9e554d146592c6a15fcca91d950239c8f364b11b7a3f577fdd1e44320557fd07040102d649ef39fc6d0955002c14262cf5f67dcd6f075b1215da0d0e69ebb1
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
