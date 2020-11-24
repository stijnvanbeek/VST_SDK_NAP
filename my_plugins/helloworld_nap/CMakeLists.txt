set(NAP_DIRECTORY ${SDK_ROOT}/../../nap)
set(THIRDPARTY_DIR ${SDK_ROOT}/../../thirdparty)
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

# Let find_python find our prepackaged Python in thirdparty
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

# utility sources
file(GLOB_RECURSE utility_sources ${NAP_DIRECTORY}/utility/src/*.*)
list(APPEND plug_sources ${utility_sources})

# rtti sources
file(GLOB_RECURSE rtti_sources ${NAP_DIRECTORY}/rtti/src/*.*)
list(REMOVE_ITEM rtti_sources ${NAP_DIRECTORY}/rtti/src/rtti/naprtti.cpp)
list(APPEND plug_sources ${rtti_sources})

# core sources
if(APPLE)
    file(GLOB core_sources ${NAP_DIRECTORY}/core/src/nap/*.cpp ${NAP_DIRECTORY}/core/src/nap/*.h ${NAP_DIRECTORY}/core/src/nap/native/*.cpp ${NAP_DIRECTORY}/core/src/nap/osx/*.cpp)
elseif(MSVC)
    file(GLOB core_sources ${NAP_DIRECTORY}/core/src/nap/*.cpp ${NAP_DIRECTORY}/win32/src/nap/*.h ${NAP_DIRECTORY}/core/src/nap/native/*.cpp)
endif()
list(REMOVE_ITEM core_sources ${NAP_DIRECTORY}/core/src/core/napcore.cpp)
list(APPEND plug_sources ${core_sources})

# math sources
file(GLOB_RECURSE math_sources ${NAP_DIRECTORY}/modules/napmath/src/*.*)
list(REMOVE_ITEM math_sources ${NAP_DIRECTORY}/modules/napmath/src/mod_napmath.cpp)
list(APPEND plug_sources ${math_sources})

# scene sources
file(GLOB_RECURSE scene_sources ${NAP_DIRECTORY}/modules/napscene/src/*.*)
list(REMOVE_ITEM scene_sources ${NAP_DIRECTORY}/modules/napscene/src/mod_napscene.cpp)
list(APPEND plug_sources ${scene_sources})

# python sources
file(GLOB_RECURSE python_sources ${NAP_DIRECTORY}/modules/nappython/src/*.*)
list(REMOVE_ITEM python_sources ${NAP_DIRECTORY}/modules/nappython/src/mod_nappython.cpp)
list(APPEND plug_sources ${python_sources})

# parameter sources
file(GLOB_RECURSE color_sources ${NAP_DIRECTORY}/modules/napcolor/src/*.*)
list(REMOVE_ITEM color_sources ${NAP_DIRECTORY}/modules/napcolor/src/mod_napcolor.cpp)
list(APPEND plug_sources ${color_sources})
file(GLOB_RECURSE parameter_sources ${NAP_DIRECTORY}/modules/napparameter/src/*.*)
list(REMOVE_ITEM parameter_sources ${NAP_DIRECTORY}/modules/napparameter/src/mod_napparameter.cpp)
list(APPEND plug_sources ${parameter_sources})

# python parameter sources
file(GLOB_RECURSE pythonparameter_sources ${NAP_DIRECTORY}/modules/nappythonparameter/src/*.*)
list(REMOVE_ITEM pythonparameter_sources ${NAP_DIRECTORY}/modules/nappythonparameter/src/mod_nappythonparameter.cpp)
list(APPEND plug_sources ${pythonparameter_sources})

# audio sources
file(GLOB_RECURSE audio_sources ${NAP_DIRECTORY}/modules/napaudio/src/*.*)
list(REMOVE_ITEM audio_sources ${NAP_DIRECTORY}/modules/napaudio/src/mod_napaudio.cpp)
list(APPEND plug_sources ${audio_sources})

# audio advanced sources
file(GLOB_RECURSE audioadvanced_sources ${NAP_DIRECTORY}/modules/napaudioadvanced/src/*.*)
list(REMOVE_ITEM audioadvanced_sources ${NAP_DIRECTORY}/modules/napaudioadvanced/src/mod_napaudioadvanced.cpp)
list(APPEND plug_sources ${audioadvanced_sources})

# midi sources
file(GLOB_RECURSE midi_sources ${NAP_DIRECTORY}/modules/napmidi/src/*.*)
list(REMOVE_ITEM midi_sources ${NAP_DIRECTORY}/modules/napmidi/src/mod_napmidi.cpp)
list(APPEND plug_sources ${midi_sources})

# native file dialog
#file(GLOB_RECURSE nativefiledialog_sources ${NAP_DIRECTORY}/modules/napnativefiledialog/src/*.*)
#list(REMOVE_ITEM nativefiledialog_sources ${NAP_DIRECTORY}/modules/napnativefiledialog/src/mod_napnativefiledialog.cpp)
#list(APPEND plug_sources ${nativefiledialog_sources})

message(STATUS ${plug_sources})


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
set(target helloworld_nap)

smtg_add_vst3plugin(${target} ${plug_sources})
set_target_properties(${target} PROPERTIES ${SDK_IDE_MYPLUGINS_FOLDER})

target_compile_definitions(${target} PRIVATE _USE_MATH_DEFINES)
if(APPLE)
    target_link_libraries(${target} PRIVATE base sdk RTTR::Core pybind11::embed libsndfile libmpg123 "-framework CoreServices"  "-framework AppKit")
elseif(win32)
    target_compile_definitions(${target} PUBLIC NOMINMAX)
    target_link_libraries(${target} PRIVATE base sdk RTTR::Core pybind11::embed libsndfile libmpg123)
endif()
target_include_directories(${target} PUBLIC ${pybind11_INCLUDE_DIRS} ${MOODYCAMEL_INCLUDE_DIRS} ${RAPIDJSON_INCLUDE_DIRS} ${GLM_INCLUDE_DIRS} ${LIBSNDFILE_INCLUDE_DIR} ${LIBMPG123_INCLUDE_DIR} ${NATIVEFILEDIALOG_INCLUDE_DIRS} ${include_directories})

if(SMTG_MAC)
    smtg_set_bundle(${target} INFOPLIST "${CMAKE_CURRENT_LIST_DIR}/resource/Info.plist" PREPROCESS)
elseif(SMTG_WIN)
    target_sources(${target} PRIVATE resource/plug.rc)
endif()
