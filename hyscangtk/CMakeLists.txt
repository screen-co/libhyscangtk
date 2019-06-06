add_library (${HYSCAN_GTK_LIBRARY} SHARED
             hyscan-gtk-profile.c
             hyscan-gtk-profile-db.c
             hyscan-gtk-profile-hw.c
             hyscan-gtk-profile-offset.c
             hyscan-gtk-connector.c
             ${CMAKE_BINARY_DIR}/resources/hyscan-gtk-templates.c)

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
