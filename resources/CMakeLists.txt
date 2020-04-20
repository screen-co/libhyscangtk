# Поиск GLib компилятора ресурсов.
find_program (RESOURCE_COMPILER glib-compile-resources)
if (${RESOURCE_COMPILER} STREQUAL RESOURCE_COMPILER-NOTFOUND)
  message (FATAL_ERROR "GLib resource compiler not found.")
endif ()


# Генерация ресурсного C-файла.
file (MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/resources)
add_custom_command (OUTPUT ${CMAKE_BINARY_DIR}/resources/hyscan-gtk-templates.c
        COMMAND ${RESOURCE_COMPILER}
        --generate-source
        --target ${CMAKE_BINARY_DIR}/resources/hyscan-gtk-templates.c hyscan-gtk-templates.xml
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../resources"
        VERBATIM)
