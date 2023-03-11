FILE(GLOB_RECURSE cpp_sources ${CMAKE_SOURCE_DIR}/main/*.cpp)
FILE(GLOB_RECURSE c_sources ${CMAKE_SOURCE_DIR}/main/*.c)

idf_component_register(
#    SRCS "printer.cpp" "main.cpp" "lcdUI.cpp" "tftLCD.cpp" "utility.cpp" "widgets.cpp" "Menu/blac_w.cpp" "Menu/fileBrowser_Scr.cpp" "Menu/gcodePreview_Scr.cpp" "Menu/info_w.cpp"
    SRCS ${cpp_sources} ${c_sources}
    INCLUDE_DIRS ""
)

target_compile_options(${COMPONENT_LIB} PRIVATE -std=gnu++2a)
#spiffs_create_partition_image(storage ../data FLASH_IN_PROJECT)
