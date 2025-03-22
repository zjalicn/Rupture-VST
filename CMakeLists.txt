cmake_minimum_required(VERSION 3.15)
project(Rupture VERSION 1.0.0)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

execute_process(
    COMMAND chmod +x ${CMAKE_CURRENT_SOURCE_DIR}/compile_scss.sh
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_subdirectory(JUCE)

add_custom_target(CompileSCSS
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/compile_scss.sh
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Compiling SCSS to CSS"
    BYPRODUCTS
        ${CMAKE_CURRENT_SOURCE_DIR}/src/resources/layout.css
)

juce_add_binary_data(RuptureResources 
    SOURCES
        # Resources
        src/resources/layout.html
        src/resources/layout.css

        # Fonts
        src/resources/fonts/old_english_hearts.ttf
)

add_dependencies(RuptureResources CompileSCSS)

juce_add_plugin(Rupture
    PRODUCT_NAME "Rupture"
    COMPANY_NAME "createdbyniko."
    FORMATS VST3
    PLUGIN_MANUFACTURER_CODE Niko  # Use your own 4-character code
    PLUGIN_CODE Rupt              # Use your own 4-character code
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
    COPY_PLUGIN_AFTER_BUILD TRUE
    PLUGIN_DESCRIPTION "Atmospheric Reverb"
    VST3_CATEGORIES "Fx,Reverb"
    JUCE_ENABLE_LIVE_CONSTANT_EDITOR=0
    JUCE_USE_WIN_WEBVIEW2=1  # For Windows
    JUCE_USE_WEBKIT=1        # For Mac/iOS
    JUCE_GENERATE_JUCE_HEADER ON
)

juce_generate_juce_header(Rupture)

target_sources(Rupture
    PRIVATE
        # Core
        src/core/PluginProcessor.cpp
        src/core/PluginProcessor.h
        src/core/PluginEditor.cpp
        src/core/PluginEditor.h

        # UI
        src/ui/LayoutView.cpp
        src/ui/LayoutView.h

        # DSP
        src/dsp/reverb/ReverbProcessor.cpp
        src/dsp/reverb/ReverbProcessor.h
)

target_include_directories(Rupture
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core
        ${CMAKE_CURRENT_SOURCE_DIR}/src/ui
        ${CMAKE_CURRENT_SOURCE_DIR}/src/dsp/reverb
)

target_compile_definitions(Rupture
    PRIVATE
        JUCE_WEB_BROWSER=1
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:Rupture,PRODUCT_NAME>"
        JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:Rupture,VERSION>"
)

target_link_libraries(Rupture
    PRIVATE
        juce::juce_audio_basics
        juce::juce_audio_devices
        juce::juce_audio_formats
        juce::juce_audio_processors
        juce::juce_audio_utils
        juce::juce_core
        juce::juce_data_structures
        juce::juce_events
        juce::juce_graphics
        juce::juce_gui_basics
        juce::juce_gui_extra
        RuptureResources
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)