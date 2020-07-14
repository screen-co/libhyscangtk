find_program (MARSHAL_COMPILER glib-genmarshal)
if (${MARSHAL_COMPILER} STREQUAL MARSHAL_COMPILER-NOTFOUND)
  message (FATAL_ERROR "GLib marshallers compiler not found.")
endif ()

file (MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/marshallers")
include_directories ("${CMAKE_BINARY_DIR}/marshallers")

add_custom_command (OUTPUT "${CMAKE_BINARY_DIR}/marshallers/hyscan-gtk-marshallers.c"
                    COMMAND ${MARSHAL_COMPILER} --header
                            --prefix hyscan_gtk_marshal
                            hyscan-gtk-marshallers.list >
                            "${CMAKE_BINARY_DIR}/marshallers/hyscan-gtk-marshallers.h"
                    COMMAND ${MARSHAL_COMPILER} --body
                            --prefix hyscan_gtk_marshal
                            hyscan-gtk-marshallers.list >
                            "${CMAKE_BINARY_DIR}/marshallers/hyscan-gtk-marshallers.c"
                    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                    DEPENDS hyscan-gtk-marshallers.list
                    VERBATIM)

add_library (${HYSCAN_GTK_LIBRARY} SHARED
             hyscan-cell-renderer-pixbuf.c
             hyscan-gtk-spin-button.c
             hyscan-gtk-datetime.c
             hyscan-gtk-param-key.c
             hyscan-gtk-param.c
             hyscan-gtk-param-list.c
             hyscan-gtk-param-cc.c
             hyscan-gtk-param-tree.c
             hyscan-gtk-param-merge.c
             hyscan-gtk-profile.c
             hyscan-gtk-profile-editor.c
             hyscan-gtk-profile-editor-db.c
             hyscan-gtk-profile-db.c
             hyscan-gtk-device-auto.c
             hyscan-gtk-device-manual.c
             hyscan-gtk-profile-editor-hw.c
             hyscan-gtk-profile-hw.c
             hyscan-gtk-profile-editor-offset.c
             hyscan-gtk-profile-offset.c
             hyscan-gtk-connector.c
             hyscan-gtk-paned.c
             ${CMAKE_BINARY_DIR}/resources/hyscan-gtk-templates.c
             ${CMAKE_BINARY_DIR}/marshallers/hyscan-gtk-marshallers.c)

target_link_libraries (${HYSCAN_GTK_LIBRARY} ${GTK3_LIBRARIES} ${HYSCAN_LIBRARIES})

set_target_properties (${HYSCAN_GTK_LIBRARY} PROPERTIES DEFINE_SYMBOL "HYSCAN_API_EXPORTS")
set_target_properties (${HYSCAN_GTK_LIBRARY} PROPERTIES SOVERSION ${HYSCAN_GTK_VERSION})

configure_file (hyscangtk.pc.in
                "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/hyscangtk-${HYSCAN_MAJOR_VERSION}.pc"
                @ONLY)

install (TARGETS ${HYSCAN_GTK_LIBRARY}
         COMPONENT runtime
         RUNTIME DESTINATION bin
         LIBRARY DESTINATION lib
         ARCHIVE DESTINATION lib
         PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)

install (FILES "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/hyscangtk-${HYSCAN_MAJOR_VERSION}.pc"
         COMPONENT development
         DESTINATION "lib/pkgconfig"
         PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)

install (FILES hyscan-gtk-profile.h
         COMPONENT development
         DESTINATION "include/hyscan-${HYSCAN_MAJOR_VERSION}/hyscangtk"
         PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)
