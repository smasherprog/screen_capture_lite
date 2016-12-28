project(Screen_Capturelib)

if(WIN32)
    set(SCREEN_CAPTURE_PLATFORM_SRC     
        ${Screen_Capture_SOURCE_DIR}/include/windows/ScreenCaptureDX.h
        ${Screen_Capture_SOURCE_DIR}/src/windows/ScreenCaptureDX.cpp
        ${Screen_Capture_SOURCE_DIR}/include/windows/DXDuplicationManager.h
        ${Screen_Capture_SOURCE_DIR}/src/windows/DXDuplicationManager.cpp
        ${Screen_Capture_SOURCE_DIR}/include/windows/DXFrameProcessor.h
        ${Screen_Capture_SOURCE_DIR}/src/windows/DXFrameProcessor.cpp
        ${Screen_Capture_SOURCE_DIR}/include/windows/DXThreadManager.h
        ${Screen_Capture_SOURCE_DIR}/src/windows/DXThreadManager.cpp
        ${Screen_Capture_SOURCE_DIR}/src/windows/ScreenCaptureGDI.cpp
        ${Screen_Capture_SOURCE_DIR}/include/windows/CommonTypes.h
		${Screen_Capture_SOURCE_DIR}/src/windows/CommonTypes.cpp
		${Screen_Capture_SOURCE_DIR}/src/windows/PixelShader.hlsl
		${Screen_Capture_SOURCE_DIR}/src/windows/VertexShader.hlsl
    )

	set_source_files_properties(${Screen_Capture_SOURCE_DIR}/src/windows/PixelShader.hlsl PROPERTIES VS_SHADER_TYPE Pixel VS_SHADER_MODEL 4.0 VS_SHADER_ENTRYPOINT PS VS_SHADER_FLAGS "/DFLAGS_ADDED /Fh \"../../src/windows/%(Filename).h\"")
	set_source_files_properties(${Screen_Capture_SOURCE_DIR}/src/windows/VertexShader.hlsl PROPERTIES VS_SHADER_TYPE Vertex VS_SHADER_MODEL 4.0 VS_SHADER_ENTRYPOINT VS VS_SHADER_FLAGS "/DFLAGS_ADDED /Fh \"../../src/windows/%(Filename).h\"")


    set(SCREEN_CAPTURE_PLATFORM_INC     
        ${Screen_Capture_SOURCE_DIR}/include/windows
        ${Screen_Capture_SOURCE_DIR}/src/windows
    )
elseif(APPLE)
    set(TARGET_BUILD_PLATFORM apple)
elseif(UNIX)
    set(TARGET_BUILD_PLATFORM unix)
endif()

set(SCREEN_CAPTURE_COMMON_SRC 
    ${Screen_Capture_SOURCE_DIR}/include/ScreenTypes.h
    ${Screen_Capture_SOURCE_DIR}/include/ScreenCaptureManager.h
    ${Screen_Capture_SOURCE_DIR}/src/ScreenCaptureManager.cpp
)
set(SCREEN_CAPTURE_COMMON_INC
    ${Screen_Capture_SOURCE_DIR}/include
    ${Screen_Capture_SOURCE_DIR}/src
)

include_directories(${SCREEN_CAPTURE_COMMON_INC} ${SCREEN_CAPTURE_PLATFORM_INC})

add_library(${PROJECT_NAME} ${SCREEN_CAPTURE_COMMON_SRC} ${SCREEN_CAPTURE_PLATFORM_SRC})

install(TARGETS ${PROJECT_NAME} 
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib/static)

