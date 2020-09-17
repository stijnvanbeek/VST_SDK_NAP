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
    source/plugfactory.cpp
    source/plugcontroller.cpp
    source/plugprocessor.cpp
)

file(GLOB_RECURSE nap_sources
        ${NAP_DIRECTORY}/utility/src/*.*
        ${NAP_DIRECTORY}/rtti/src/*.*
        ${NAP_DIRECTORY}/modules/core/src/*.*
        )

set(include_directories
        ${NAP_DIRECTORY}/utility/src/
        ${NAP_DIRECTORY}/rtti/src/
        ${NAP_DIRECTORY}/core/src/
        )

list(APPEND plug_sources ${nap_sources})

message(STATUS ${MOODYCAMEL_INCLUDE_DIRS})

#--- HERE change the target Name for your plug-in (for ex. set(target myDelay))-------
set(target helloworld_nap)

smtg_add_vst3plugin(${target} ${plug_sources})
set_target_properties(${target} PROPERTIES ${SDK_IDE_MYPLUGINS_FOLDER})
target_link_libraries(${target} PRIVATE base sdk RTTR::Core)
target_include_directories(${target} PUBLIC ${MOODYCAMEL_INCLUDE_DIRS} ${RAPIDJSON_INCLUDE_DIRS} ${include_directories})

if(SMTG_MAC)
    smtg_set_bundle(${target} INFOPLIST "${CMAKE_CURRENT_LIST_DIR}/resource/Info.plist" PREPROCESS)
elseif(SMTG_WIN)
    target_sources(${target} PRIVATE resource/plug.rc)
endif()
