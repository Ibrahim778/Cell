cmake_minimum_required(VERSION 3.12)
set(CMAKE_TOOLCHAIN_FILE "$ENV{SCE_PSP2_SDK_DIR}/host_tools/build/cmake/psp2-snc-toolchain.cmake")
project(Cell)
include(VitaDevelopmentSuite)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 11)

enable_language(EMD)
enable_language(C ASM)
enable_language(CXX ASM)

add_compile_options(
    -D_SCE_TARGET_OS_PSP2=1 -Xdiag=0 -Xquit=2 -D_DEBUG
)
add_link_options(
  --prx-no-runtime-support
)

include_directories(
  ${VDSUITE_USER_INCLUDE_DIRECTORIES}
  ${VDSUITE_STANDARD_INCLUDE_DIRECTORIES}
  $ENV{SCE_PSP2_SDK_DIR}/target/include
  $ENV{SCE_PSP2_SDK_DIR}/target/include_common
)

link_directories(
  ${VDSUITE_LIBRARY_DIRECTORIES}
)

# Main Plugin

add_library(${PROJECT_NAME} MODULE
    src/main.cpp
    src/main.hpp
    src/utils.cpp
    src/utils.hpp
    Cell.emd
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