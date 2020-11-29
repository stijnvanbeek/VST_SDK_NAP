set(TARGET_NAME test2)
set(THIRDPARTY_DIR ${SDK_ROOT}/../../thirdparty)
set(NAP_DIRECTORY ${SDK_ROOT}/../../nap)
set(NAP_LIB_DIRECTORY ${NAP_DIRECTORY}/lib/AppleClang-x86_64-Debug)
set(NAP_ENABLE_PYTHON 1)
add_definitions(-DNAP_ENABLE_PYTHON)

list(APPEND CMAKE_MODULE_PATH "${NAP_DIRECTORY}/cmake")

include(${NAP_DIRECTORY}/cmake/configure.cmake)

find_package(moodycamel REQUIRED)
find_package(rapidjson REQUIRED)
find_package(glm REQUIRED)
find_package(libsndfile REQUIRED)
find_package(libmpg123 REQUIRED)
find_rttr()
find_python_in_thirdparty()
set(pybind11_DIR ${THIRDPARTY_DIR}/pybind11/install/share/cmake/pybind11)
find_package(pybind11 REQUIRED)

set(plug_sources
    include/plugcontroller.h
    include/plugids.h
    include/plugprocessor.h
    include/version.h
    include/packaginginfo.h
    source/plugfactory.cpp
    source/plugcontroller.cpp
    source/plugprocessor.cpp
    source/napinfo.cpp
)

set (nap_libs
        ${NAP_LIB_DIRECTORY}/naputility.a
        ${NAP_LIB_DIRECTORY}/naprtti.dylib
        ${NAP_LIB_DIRECTORY}/napcore.dylib
        ${NAP_LIB_DIRECTORY}/mod_napmath.dylib
        ${NAP_LIB_DIRECTORY}/mod_napscene.dylib
        ${NAP_LIB_DIRECTORY}/mod_nappython.dylib
        ${NAP_LIB_DIRECTORY}/mod_napparameter.dylib
        ${NAP_LIB_DIRECTORY}/mod_nappythonparameter.dylib
        ${NAP_LIB_DIRECTORY}/mod_napaudio.dylib
        ${NAP_LIB_DIRECTORY}/mod_napaudioadvanced.dylib
        ${NAP_LIB_DIRECTORY}/mod_napmidi.dylib
)

set(include_directories
        include/
        ${NAP_DIRECTORY}/utility/src/
        ${NAP_DIRECTORY}/rtti/src/
        ${NAP_DIRECTORY}/core/src/
        ${NAP_DIRECTORY}/modules/napmath/src/
        ${NAP_DIRECTORY}/modules/napscene/src/
        ${NAP_DIRECTORY}/modules/nappython/src/
        ${NAP_DIRECTORY}/modules/napcolor/src/
        ${NAP_DIRECTORY}/modules/napparameter/src/
        ${NAP_DIRECTORY}/modules/nappythonparameter/src/
        ${NAP_DIRECTORY}/modules/napaudio/src/
        ${NAP_DIRECTORY}/modules/napaudioadvanced/src/
        ${NAP_DIRECTORY}/modules/napmidi/src/
)


#--- HERE change the target Name for your plug-in (for ex. set(target myDelay))-------
set(target ${TARGET_NAME})

smtg_add_vst3plugin(${target} ${plug_sources})
set_target_properties(${target} PROPERTIES ${SDK_IDE_MYPLUGINS_FOLDER})
target_link_directories(${target} PUBLIC ${NAP_DIRECTORY}/lib/AppleClang-x86_64-Debug)
target_compile_definitions(${target} PRIVATE _USE_MATH_DEFINES)
if(APPLE)
    target_link_libraries(${target} PRIVATE base sdk RTTR::Core pybind11::embed libsndfile libmpg123 "-framework CoreServices"  "-framework AppKit" ${nap_libs})
elseif(win32)
    target_compile_definitions(${target} PUBLIC NOMINMAX)
    target_link_libraries(${target} PRIVATE base sdk RTTR::Core pybind11::embed libsndfile libmpg123 ${nap_libs})
endif()
target_include_directories(${target} PUBLIC ${pybind11_INCLUDE_DIRS} ${MOODYCAMEL_INCLUDE_DIRS} ${RAPIDJSON_INCLUDE_DIRS} ${GLM_INCLUDE_DIRS} ${LIBSNDFILE_INCLUDE_DIR} ${LIBMPG123_INCLUDE_DIR} ${NATIVEFILEDIALOG_INCLUDE_DIRS} ${include_directories})

if(SMTG_MAC)
    smtg_set_bundle(${target} INFOPLIST "${CMAKE_CURRENT_LIST_DIR}/resource/Info.plist" PREPROCESS)
elseif(SMTG_WIN)
    target_sources(${target} PRIVATE resource/plug.rc)
endif()
