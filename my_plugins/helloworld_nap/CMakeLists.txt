set(NAP_DIRECTORY ${SDK_ROOT}/../../4dsound_source)
set(THIRDPARTY_DIR ${SDK_ROOT}/../../thirdparty)

list(APPEND CMAKE_MODULE_PATH "${NAP_DIRECTORY}/cmake")

include(${NAP_DIRECTORY}/cmake/configure.cmake)

find_package(moodycamel REQUIRED)
find_package(rapidjson REQUIRED)
find_rttr()

set(plug_sources
    include/plugcontroller.h
    include/plugids.h
    include/plugprocessor.h
    include/version.h
    include/packaginginfo.h
    source/plugfactory.cpp
    source/plugcontroller.cpp
    source/plugprocessor.cpp
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

# audio sources
file(GLOB_RECURSE audio_sources ${NAP_DIRECTORY}/modules/audio/src/*.*)
list(REMOVE_ITEM audio_sources ${NAP_DIRECTORY}/modules/audio/src/mod_napaudio.cpp)
list(APPEND plug_sources ${audio_sources})

message(STATUS ${plug_sources})

set(include_directories
        include/
        ${NAP_DIRECTORY}/utility/src/
        ${NAP_DIRECTORY}/rtti/src/
        ${NAP_DIRECTORY}/core/src/
        ${NAP_DIRECTORY}/audio/src/
        )


#--- HERE change the target Name for your plug-in (for ex. set(target myDelay))-------
set(target helloworld_nap)

smtg_add_vst3plugin(${target} ${plug_sources})
set_target_properties(${target} PROPERTIES ${SDK_IDE_MYPLUGINS_FOLDER})
if(APPLE)
    target_link_libraries(${target} PRIVATE base sdk RTTR::Core "-framework CoreServices")
elseif(win32)
    target_link_libraries(${target} PRIVATE base sdk RTTR::Core)
endif()
target_include_directories(${target} PUBLIC ${MOODYCAMEL_INCLUDE_DIRS} ${RAPIDJSON_INCLUDE_DIRS} ${include_directories})

if(SMTG_MAC)
    smtg_set_bundle(${target} INFOPLIST "${CMAKE_CURRENT_LIST_DIR}/resource/Info.plist" PREPROCESS)
elseif(SMTG_WIN)
    target_sources(${target} PRIVATE resource/plug.rc)
endif()
