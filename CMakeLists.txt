cmake_minimum_required(VERSION 3.17)

# Set minimum macOS version before defining the project
set(CMAKE_OSX_DEPLOYMENT_TARGET "15.0" CACHE STRING "Minimum macOS version")

# Get environment variables
set(PROJECT_NAME $ENV{PROJECT_NAME})
set(PROJECT_BUNDLE_ID $ENV{PROJECT_BUNDLE_ID})
set(JUCE_REPO $ENV{JUCE_REPO})
set(JUCE_TAG $ENV{JUCE_TAG})
set(COMPANY_NAME $ENV{COMPANY_NAME})

# Fallback defaults if env vars not set
if(NOT PROJECT_NAME)
    set(PROJECT_NAME "griddy-v2")
endif()
if(NOT PROJECT_BUNDLE_ID)
    set(PROJECT_BUNDLE_ID "com.generouscorp.griddy-v2")
endif()
if(NOT JUCE_REPO)
    set(JUCE_REPO "https://github.com/juce-framework/JUCE.git")
endif()
if(NOT JUCE_TAG)
    set(JUCE_TAG "8.0.8")
endif()
if(NOT COMPANY_NAME)
    set(COMPANY_NAME "Generous Corp")
endif()

# Use semantic versioning (major.minor.patch)
# Read from environment or use defaults
set(VERSION_MAJOR $ENV{VERSION_MAJOR})
set(VERSION_MINOR $ENV{VERSION_MINOR})
set(VERSION_PATCH $ENV{VERSION_PATCH})

if(NOT VERSION_MAJOR)
    set(VERSION_MAJOR 1)
endif()
if(NOT VERSION_MINOR)
    set(VERSION_MINOR 0)
endif()
if(NOT VERSION_PATCH)
    set(VERSION_PATCH 0)
endif()

set(PLUGIN_VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")

project(${PROJECT_NAME} VERSION ${PLUGIN_VERSION})

# Fetch JUCE using FetchContent
include(FetchContent)

# Use shared JUCE cache to avoid re-downloading across projects
set(FETCHCONTENT_BASE_DIR "$ENV{HOME}/.juce_cache")

FetchContent_Declare(
    JUCE 
    GIT_REPOSITORY ${JUCE_REPO}
    GIT_TAG ${JUCE_TAG}
    GIT_SHALLOW ON
)
FetchContent_MakeAvailable(JUCE)

# ==============================================================================
# Visage GPU UI Framework (shared by macOS plugin and iOS app)
# ==============================================================================
set(VISAGE_BUILD_EXAMPLES OFF CACHE BOOL "Build Visage examples" FORCE)
set(VISAGE_BUILD_TESTS OFF CACHE BOOL "Build Visage tests" FORCE)
add_subdirectory(external/visage)

# ==============================================================================
# macOS Plugin Target (skip when cross-compiling for iOS)
# ==============================================================================
if(NOT CMAKE_SYSTEM_NAME STREQUAL "iOS")

# Get AU version from environment or calculate it
set(AU_VERSION_INT $ENV{AU_VERSION_INT})
if(NOT AU_VERSION_INT)
    # Calculate from version components: (major << 16) | (minor << 8) | patch
    math(EXPR AU_VERSION_INT "(${VERSION_MAJOR} << 16) | (${VERSION_MINOR} << 8) | ${VERSION_PATCH}")
endif()

# Create the plugin
juce_add_plugin(${PROJECT_NAME}
    COMPANY_NAME ${COMPANY_NAME}
    BUNDLE_ID ${PROJECT_BUNDLE_ID}
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT TRUE
    IS_MIDI_EFFECT TRUE
    NEEDS_EDITOR TRUE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
    COPY_PLUGIN_AFTER_BUILD TRUE
    PLUGIN_MANUFACTURER_CODE GECO
    PLUGIN_CODE GRDY
    FORMATS AU VST3 Standalone
    PRODUCT_NAME ${PROJECT_NAME}
    VERSION ${PLUGIN_VERSION}
    AU_MAIN_TYPE kAudioUnitType_MIDIProcessor
    AU_EXPORT_PREFIX "GriddyAU"
    AU_SANDBOX_SAFE TRUE
    VST3_CATEGORIES "Drum|Tools"
)

juce_generate_juce_header(${PROJECT_NAME})

target_sources(${PROJECT_NAME} PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginProcessor.h
    Source/PluginEditor.cpp
    Source/PluginEditor.h
    Source/Visage/JuceVisageBridge.cpp
    Source/Visage/JuceVisageBridge.h
    Source/Grids/GridsEngine.cpp
    Source/Grids/GridsEngine.h
    Source/Grids/GridsPatternData.h
    Source/Grids/EuclideanEngine.h
    Source/Grids/EuclideanTables.h
    Source/Settings/SettingsManager.h
    Source/UI/XYPadFrame.h
    Source/UI/LEDMatrixFrame.h
    Source/UI/DensitySliderFrame.h
    Source/UI/RotaryKnobFrame.h
    Source/UI/ResetButtonFrame.h
    Source/UI/SettingsButtonFrame.h
    Source/UI/SettingsPanelFrame.h
    Source/Modulation/LFO.h
    Source/Modulation/ModulationMatrix.h
    Source/PatternChain/PatternChain.h
)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/Source
    ${CMAKE_SOURCE_DIR}/external/visage
    ${CMAKE_SOURCE_DIR}/external/visage/visage_ui
    ${CMAKE_SOURCE_DIR}/external/visage/visage_graphics
    ${CMAKE_SOURCE_DIR}/external/visage/visage_windowing
    ${CMAKE_SOURCE_DIR}/external/visage/visage_app
    ${CMAKE_SOURCE_DIR}/external/visage/visage_widgets
    ${CMAKE_SOURCE_DIR}/external/visage/visage_utils
)

target_compile_definitions(${PROJECT_NAME} PUBLIC
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_USE_CURL=0
    JUCE_WEB_BROWSER=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
    ENABLE_VELOCITY_SYSTEM=1
    ENABLE_EUCLIDEAN_MODE=1
    ENABLE_PATTERN_CHAIN=1
    ENABLE_MODULATION_MATRIX=1
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    juce::juce_audio_basics
    juce::juce_audio_devices
    juce::juce_audio_formats
    juce::juce_audio_processors
    juce::juce_audio_utils
    juce::juce_core
    juce::juce_data_structures
    juce::juce_dsp
    juce::juce_events
    juce::juce_graphics
    juce::juce_gui_basics
    juce::juce_gui_extra
    visage
)

# ==============================================================================
# Standard macOS Paths (no permission prompts)
# ==============================================================================
# This plugin uses Apple-recommended Application Support paths:
#
# ~/Library/Application Support/${PROJECT_NAME}/
#   ├── Samples/    - Sample files
#   ├── Presets/    - User presets
#   └── UserData/   - Other user data
#
# ~/Library/Logs/${PROJECT_NAME}/
#   └── *.log       - Log files
#
# ~/Library/Caches/${PROJECT_BUNDLE_ID}/
#   └── *           - Temporary cache files
#
# These paths follow Apple's Human Interface Guidelines and do not trigger
# permission dialogs during installation. Helper functions are provided in
# PluginProcessor.cpp for accessing these paths.
# ==============================================================================

add_custom_command(TARGET ${PROJECT_NAME}_AU
    POST_BUILD
    COMMAND "${CMAKE_SOURCE_DIR}/scripts/post_build.sh" "$<TARGET_BUNDLE_DIR:${PROJECT_NAME}_AU>"
    COMMENT "Updating Info.plist version for ${PROJECT_NAME}_AU"
    VERBATIM
)

# Add post-build for VST3 if it's being built
if(TARGET ${PROJECT_NAME}_VST3)
    add_custom_command(TARGET ${PROJECT_NAME}_VST3
        POST_BUILD
        COMMAND "${CMAKE_SOURCE_DIR}/scripts/post_build.sh" "$<TARGET_BUNDLE_DIR:${PROJECT_NAME}_VST3>"
        COMMENT "Updating Info.plist version for ${PROJECT_NAME}_VST3"
        VERBATIM
    )
endif()

endif() # NOT iOS

# ==============================================================================
# iOS GUI App Target
# ==============================================================================
# App sample files (bundled as binary data for iOS app)
set(GRIDDY_APP_SAMPLES
    ${CMAKE_SOURCE_DIR}/Resources/Samples/kick.wav
    ${CMAKE_SOURCE_DIR}/Resources/Samples/snare.wav
    ${CMAKE_SOURCE_DIR}/Resources/Samples/hihat.wav
    ${CMAKE_SOURCE_DIR}/Resources/Images/app_icon_1024.png
)

if(IOS OR CMAKE_SYSTEM_NAME STREQUAL "iOS")
    juce_add_gui_app(GriddyApp
        COMPANY_NAME ${COMPANY_NAME}
        BUNDLE_ID ${PROJECT_BUNDLE_ID}.app
        PRODUCT_NAME "Griddy"
        VERSION ${PLUGIN_VERSION}
        ICON_BIG Resources/Images/app_icon_1024.png
        BACKGROUND_AUDIO_ENABLED TRUE
        STATUS_BAR_HIDDEN TRUE
        REQUIRES_FULL_SCREEN TRUE
        IPHONE_SCREEN_ORIENTATIONS "UIInterfaceOrientationPortrait"
        IPAD_SCREEN_ORIENTATIONS "UIInterfaceOrientationPortrait"
    )

    juce_generate_juce_header(GriddyApp)

    target_sources(GriddyApp PRIVATE
        App/GriddyApp.cpp
        App/GriddyAppMainComponent.cpp
        App/GriddyAppMainComponent.h
        App/GriddyAppEngine.cpp
        App/GriddyAppEngine.h
        App/GriddyMidiOut.cpp
        App/GriddyMidiOut.h
        App/GriddySampleLibrary.cpp
        App/GriddySampleLibrary.h
        App/XYPad.cpp
        App/XYPad.h
        Source/Settings/SettingsManager.h
        Source/Grids/GridsEngine.cpp
        Source/Grids/GridsEngine.h
        Source/Grids/GridsPatternData.h
    )

    target_include_directories(GriddyApp PRIVATE
        ${CMAKE_SOURCE_DIR}/Source
        ${CMAKE_SOURCE_DIR}/Source/Grids
        ${CMAKE_SOURCE_DIR}/App
        ${CMAKE_SOURCE_DIR}/external/visage
        ${CMAKE_SOURCE_DIR}/external/visage/visage_ui
        ${CMAKE_SOURCE_DIR}/external/visage/visage_graphics
        ${CMAKE_SOURCE_DIR}/external/visage/visage_windowing
        ${CMAKE_SOURCE_DIR}/external/visage/visage_app
        ${CMAKE_SOURCE_DIR}/external/visage/visage_widgets
        ${CMAKE_SOURCE_DIR}/external/visage/visage_utils
    )

    target_compile_features(GriddyApp PRIVATE cxx_std_17)

    target_compile_definitions(GriddyApp PRIVATE
        JUCE_USE_CURL=0
        JUCE_WEB_BROWSER=0
        JUCE_DISPLAY_SPLASH_SCREEN=0
    )

    juce_add_binary_data(GriddyApp_Resources SOURCES ${GRIDDY_APP_SAMPLES})

    target_link_libraries(GriddyApp PRIVATE
        juce::juce_audio_basics
        juce::juce_audio_devices
        juce::juce_audio_formats
        juce::juce_audio_processors
        juce::juce_audio_utils
        juce::juce_core
        juce::juce_data_structures
        juce::juce_dsp
        juce::juce_events
        juce::juce_graphics
        juce::juce_gui_basics
        juce::juce_gui_extra
        GriddyApp_Resources
        visage
    )

    # Allow simulator builds without signing
    set_target_properties(GriddyApp PROPERTIES
        XCODE_ATTRIBUTE_CODE_SIGNING_ALLOWED[sdk=iphonesimulator*] "NO"
        XCODE_ATTRIBUTE_CODE_SIGNING_REQUIRED[sdk=iphonesimulator*] "NO"
        XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY[sdk=iphonesimulator*] ""
        XCODE_ATTRIBUTE_DEBUG_INFORMATION_FORMAT "dwarf-with-dsym"
        XCODE_ATTRIBUTE_GCC_GENERATE_DEBUGGING_SYMBOLS "YES"
        XCODE_ATTRIBUTE_STRIP_INSTALLED_PRODUCT "NO"
        XCODE_ATTRIBUTE_COPY_PHASE_STRIP "NO"
    )

    # Optional: set a development team for device builds via -DDEVELOPMENT_TEAM=XXXX
    if(DEFINED DEVELOPMENT_TEAM AND NOT DEVELOPMENT_TEAM STREQUAL "")
        set_target_properties(GriddyApp PROPERTIES
            XCODE_ATTRIBUTE_DEVELOPMENT_TEAM "${DEVELOPMENT_TEAM}"
        )
    endif()
endif()

# Set global C++ standard for all targets
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Fix code signing and C++ standard for VST3 helper tool (macOS only)
if(NOT CMAKE_SYSTEM_NAME STREQUAL "iOS")
    if(TARGET juce_vst3_helper)
        set_target_properties(juce_vst3_helper PROPERTIES
            XCODE_ATTRIBUTE_CODE_SIGNING_REQUIRED "NO"
            XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "-"
            XCODE_ATTRIBUTE_CODE_SIGNING_ALLOWED "NO"
            CXX_STANDARD 17
            CXX_STANDARD_REQUIRED YES
            XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD "c++17"
            XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libc++"
        )
        target_compile_features(juce_vst3_helper PRIVATE cxx_std_17)
        # Explicitly set compile options
        target_compile_options(juce_vst3_helper PRIVATE -std=c++17)
    endif()
endif()
