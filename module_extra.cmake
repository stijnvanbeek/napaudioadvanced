set(NAP_AUDIOFILE_SUPPORT ON)

# Add sources to target
if(NAP_BUILD_CONTEXT MATCHES "source")
    if (NAP_AUDIOFILE_SUPPORT)
        # Add compile definition to enable audio file support
        target_compile_definitions(${PROJECT_NAME} PRIVATE NAP_AUDIOFILE_SUPPORT)
    else()
        # Filter out sources for audio file functionality
        set(AUDIO_FILE_SUPPORT_FILTER ".*audiofile.*")
    endif()

    add_source_dir("core" "src/audio/core")
    add_source_dir("node" "src/audio/node" ${AUDIO_FILE_SUPPORT_FILTER})
    add_source_dir("object" "src/audio/object" ${AUDIO_FILE_SUPPORT_FILTER})
    add_source_dir("component" "src/audio/component")
    add_source_dir("service" "src/audio/service")
    add_source_dir("resource" "src/audio/resource" ${AUDIO_FILE_SUPPORT_FILTER})
    add_source_dir("utility" "src/audio/utility")
endif()
