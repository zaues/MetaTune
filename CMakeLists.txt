cmake_minimum_required(VERSION 3.22)
project(MetaTune VERSION 0.3.14 LANGUAGES C CXX)

# Product naming lives here only; DSP code never refers to it.
set(PRODUCT_NAME "MetaTune" CACHE STRING "User-visible plugin name")

# GitHub "owner/repo" whose Releases the plugin checks for updates. Empty = no update check at all.
set(METATUNE_UPDATE_REPO "zaues/MetaTune" CACHE STRING "GitHub owner/repo that publishes MetaTune releases")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Static release C runtime (/MT) in all configs: no runtime DLLs needed on the target, and xwin ships no debug runtime.
set(CMAKE_MSVC_RUNTIME_LIBRARY MultiThreaded)

add_subdirectory(third_party/JUCE)

juce_add_plugin(MetaTune
    COMPANY_NAME "Digital"
    BUNDLE_ID com.zaues.metatune
    PLUGIN_MANUFACTURER_CODE Lyra
    PLUGIN_CODE LyTn
    PRODUCT_NAME "${PRODUCT_NAME}"
    FORMATS VST3
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    VST3_CATEGORIES Fx Pitch Shift
    VST3_AUTO_MANIFEST FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE)

juce_add_binary_data(MetaTuneAssets SOURCES
    Resources/fonts/Inter-Regular.ttf
    Resources/fonts/Inter-SemiBold.ttf)

target_sources(MetaTune PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/UI/Theme.cpp
    Source/UI/MetaTuneLookAndFeel.cpp
    Source/UI/Knob.cpp
    Source/UI/Orb.cpp
    Source/UI/KeyboardPanel.cpp
    Source/UI/Widgets.cpp
    Source/DSP/Pitch/TargetSelector.cpp
    Source/DSP/Pitch/YinDetector.cpp
    Source/DSP/Pitch/PitchShifter.cpp
    Source/DSP/Pitch/CorrectionEngine.cpp
    Source/DSP/TunerChain.cpp
    Source/Update/Update.cpp)

target_compile_definitions(MetaTune PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    METATUNE_UPDATE_REPO="${METATUNE_UPDATE_REPO}")

target_link_libraries(MetaTune
    PRIVATE MetaTuneAssets juce::juce_audio_processors
    PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_lto_flags juce::juce_recommended_warning_flags)

# Updater app: installed next to the licenses, started by the plugin's Update button.
juce_add_gui_app(MetaTuneUpdater
    PRODUCT_NAME "${PRODUCT_NAME} Updater"
    COMPANY_NAME "Digital"
    VERSION ${PROJECT_VERSION})
target_sources(MetaTuneUpdater PRIVATE
    Source/Updater/UpdaterMain.cpp
    Source/Updater/HostProcess.cpp
    Source/UI/Theme.cpp)
target_compile_definitions(MetaTuneUpdater PRIVATE JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:MetaTuneUpdater,JUCE_PRODUCT_NAME>"
    JUCE_APPLICATION_VERSION_STRING="${PROJECT_VERSION}")
target_link_libraries(MetaTuneUpdater
    PRIVATE MetaTuneAssets juce::juce_gui_basics juce::juce_cryptography shell32
    PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_lto_flags juce::juce_recommended_warning_flags)

# Dev tool: render the editor to PNG under Wine (not installed, not part of the plugin).
add_executable(ui_snapshot Tools/ui_snapshot.cpp)
target_include_directories(ui_snapshot PRIVATE $<TARGET_PROPERTY:MetaTune,INCLUDE_DIRECTORIES>)
target_compile_definitions(ui_snapshot PRIVATE $<TARGET_PROPERTY:MetaTune,COMPILE_DEFINITIONS>)
target_link_libraries(ui_snapshot PRIVATE MetaTune juce::juce_recommended_config_flags)
set_target_properties(ui_snapshot PROPERTIES WIN32_EXECUTABLE FALSE)
