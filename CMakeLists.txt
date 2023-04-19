cmake_minimum_required(VERSION 3.12)
set(CMAKE_TOOLCHAIN_FILE "$ENV{SCE_PSP2_SDK_DIR}/host_tools/build/cmake/psp2-snc-toolchain.cmake")

project(Cell)
include(VitaDevelopmentSuite)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 11)
enable_language(EMD)

add_compile_options(
    -D_SCE_TARGET_OS_PSP2=1 
)
add_link_options(
  --prx-no-runtime-support
)



# Main Plugin

add_library(${PROJECT_NAME} MODULE
    src/main.cpp
    src/main.hpp
    src/utils.cpp
    src/utils.hpp
    Cell.emd
)

target_link_directories(${PROJECT_NAME} PRIVATE
  ${VDSUITE_LIBRARY_DIRECTORIES}
)

target_include_directories(${PROJECT_NAME} PRIVATE
  ${CMAKE_SOURCE_DIR}
  ${VDSUITE_USER_INCLUDE_DIRECTORIES}
  ${VDSUITE_STANDARD_INCLUDE_DIRECTORIES}
  ${PSP2_SNC_STANDARD_INCLUDE_DIRECTORIES}
)

target_link_libraries(${PROJECT_NAME}
    SceLibKernel_stub
    SceThreadmgr_stub
    SceAppMgrUser_stub
    SceRegMgr_stub
    SceDisplay_stub
    taihenUnsafe_stub
	ScePafStdc_stub_weak
    ScePafThread_stub_weak
    ScePafTopLevel_stub_weak
    ScePafWidget_stub_weak
    ScePafCommon_stub_weak
    ScePafGraphics_stub_weak
    ScePafMisc_stub_weak
    ScePafResource_stub_weak
    SceCtrl_stub
    taihen_stub
    SceModulemgr_stub
)

VDSuiteSignElf(${PROJECT_NAME}.suprx ${PROJECT_NAME})

# Generate RCO
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND python $ENV{SCE_PSP2_SDK_DIR}/host_tools/build/cxml/appinfo/appinfo_compiler.py -o ${CMAKE_SOURCE_DIR}/resource/cell_plugin.rco ${CMAKE_SOURCE_DIR}/resource/src/cell_plugin.xml
)