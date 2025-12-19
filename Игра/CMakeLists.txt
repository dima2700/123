cmake_minimum_required(VERSION 3.10)
project(GeographyGame)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Поиск SFML (поддержка версий 2.5 и 3.0)
# В SFML 3.0 компоненты называются по-другому
find_package(SFML 3.0 COMPONENTS Window Graphics)
if(NOT SFML_FOUND)
    # Попытка найти SFML 2.5
    find_package(SFML 2.5 COMPONENTS system window graphics REQUIRED)
    set(SFML_VERSION_2_5 TRUE)
else()
    set(SFML_VERSION_3_0 TRUE)
endif()

# Добавление исполняемого файла
add_executable(${PROJECT_NAME} main.cpp)

# Связывание с SFML
if(SFML_VERSION_3_0)
    target_link_libraries(${PROJECT_NAME} 
        SFML::Window
        SFML::Graphics
    )
else()
    target_link_libraries(${PROJECT_NAME} 
        SFML::System
        SFML::Window
        SFML::Graphics
    )
endif()

# Настройки компиляции
if(APPLE)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_GUI_IDENTIFIER com.geographygame.app
    )
endif()

