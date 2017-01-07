project(Screen_Capturelib)

if(WIN32)
    set(SCREEN_CAPTURE_PLATFORM_SRC     
        ${Screen_Capture_SOURCE_DIR}/src/windows/GetMonitors.cpp
        ${Screen_Capture_SOURCE_DIR}/include/windows/DXFrameProcessor.h
        ${Screen_Capture_SOURCE_DIR}/src/windows/DXFrameProcessor.cpp
        ${Screen_Capture_SOURCE_DIR}/include/windows/GDIFrameProcessor.h
        ${Screen_Capture_SOURCE_DIR}/src/windows/GDIFrameProcessor.cpp
        ${Screen_Capture_SOURCE_DIR}/src/windows/ThreadRunner.cpp
    )

    set(SCREEN_CAPTURE_PLATFORM_INC     
        ${Screen_Capture_SOURCE_DIR}/include/windows
        ${Screen_Capture_SOURCE_DIR}/src/windows
    )

elseif(APPLE)
    set(SCREEN_CAPTURE_PLATFORM_SRC
        ${Screen_Capture_SOURCE_DIR}/include/ios/CGFrameProcessor.h
        ${Screen_Capture_SOURCE_DIR}/src/ios/CGFrameProcessor.cpp
        ${Screen_Capture_SOURCE_DIR}/src/ios/GetMonitors.cpp
        ${Screen_Capture_SOURCE_DIR}/src/ios/ThreadRunner.cpp
    )
    set(SCREEN_CAPTURE_PLATFORM_INC
        ${Screen_Capture_SOURCE_DIR}/include/ios
        ${Screen_Capture_SOURCE_DIR}/src/ios
    )
elseif(UNIX)

endif()

set(SCREEN_CAPTURE_COMMON_SRC 
    ${Screen_Capture_SOURCE_DIR}/include/ScreenCapture.h
    ${Screen_Capture_SOURCE_DIR}/src/ScreenCapture.cpp
    ${Screen_Capture_SOURCE_DIR}/include/Internal/SCCommon.h
    ${Screen_Capture_SOURCE_DIR}/src/Internal/SCCommon.cpp
    ${Screen_Capture_SOURCE_DIR}/include/Internal/ThreadManager.h
    ${Screen_Capture_SOURCE_DIR}/src/Internal/ThreadManager.cpp
)
set(SCREEN_CAPTURE_COMMON_INC
    ${Screen_Capture_SOURCE_DIR}/include
    ${Screen_Capture_SOURCE_DIR}/src
    ${Screen_Capture_SOURCE_DIR}/include/Internal
    ${Screen_Capture_SOURCE_DIR}/src/Internal
)

include_directories(${SCREEN_CAPTURE_COMMON_INC} ${SCREEN_CAPTURE_PLATFORM_INC})

add_library(${PROJECT_NAME} ${SCREEN_CAPTURE_COMMON_SRC} ${SCREEN_CAPTURE_PLATFORM_SRC})


install(TARGETS ${PROJECT_NAME} 
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib/static)

