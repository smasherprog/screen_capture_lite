project(screen_capture_lite_example_csharp)
    
set(OUTPUT ${PROJECT_NAME}.dll)
set(CSPROJ ${PROJECT_NAME}.csproj)

execute_process(COMMAND dotnet --version
                  RESULT_VARIABLE result
                  OUTPUT_QUIET
                  ERROR_QUIET)
if(result) 
    message(STATUS "dotnet executable not found but by this build, if you want to build CSharp Bindings, you must install dotnet")
else()
    set(PLATFORM x64)
    if(NOT DEFINED CMAKE_GENERATOR_PLATFORM)
      set(PLATFORM x64)
    elseif("${CMAKE_GENERATOR_PLATFORM}" STREQUAL "Win32")
       set(PLATFORM x86) 
    ENDIF()
    message(STATUS "Found dotnet executable CSHARP bindings will be generated!")
    add_custom_command(OUTPUT ${OUTPUT} 
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:screen_capture_lite_${TARGET_SUFFIX}> ${CMAKE_CURRENT_LIST_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:screen_capture_lite_${TARGET_SUFFIX}> ${CMAKE_BINARY_DIR}/Example_CSharp
    COMMAND ${CMAKE_COMMAND} -E copy
      ${CMAKE_CURRENT_LIST_DIR}/${CSPROJ}
      ${CMAKE_BINARY_DIR}/Example_CSharp
    COMMAND ${CMAKE_COMMAND} -E copy
      ${CMAKE_CURRENT_LIST_DIR}/Program.cs
      ${CMAKE_BINARY_DIR}/Example_CSharp
    COMMAND dotnet build --configuration ${CMAKE_BUILD_TYPE} /p:Platform=${PLATFORM} ${CMAKE_BINARY_DIR}/Example_CSharp/${CSPROJ} -o ${CMAKE_BINARY_DIR}
    COMMENT "Building ${PROJECT_NAME} dotnet build --configuration ${CMAKE_BUILD_TYPE} /p:Platform=${PLATFORM} ${CMAKE_BINARY_DIR}/Example_CSharp/${CSPROJ} -o ${CMAKE_BINARY_DIR}"
  )
    add_custom_target(${PROJECT_NAME} ALL DEPENDS ${OUTPUT})

install (FILES ${CMAKE_BINARY_DIR}/${OUTPUT}
	DESTINATION bin
)
install (FILES ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.pdb
	DESTINATION bin
    OPTIONAL
)
 
install (TARGETS screen_capture_lite_${TARGET_SUFFIX}
	RUNTIME DESTINATION Examples 
) 

endif()


 
