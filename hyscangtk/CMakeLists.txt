add_library (${HYSCAN_GTK_LIBRARY} SHARED
             hyscan-gtk-pane.c
             hyscan-gtk-automover.c
             hyscan-gtk-colorizer.c
             hyscan-gtk-leveller.c
             hyscan-gtk-mark-editor.c
             hyscan-gtk-project-viewer.c
             hyscan-gtk-scaler.c
             hyscan-gtk-recorder.c
             hyscan-gtk-tvg-control.c
             hyscan-gtk-gen-control.c
             hyscan-gtk-player.c
             hyscan-colormap-spec.c
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

install (FILES hyscan-gtk-area.h
               hyscan-gtk-pane.h
               hyscan-gtk-automover.h
               hyscan-gtk-colorizer.h
               hyscan-gtk-leveller.h
               hyscan-gtk-mark-editor.h
               hyscan-gtk-project-viewer.h
               hyscan-gtk-scaler.h
               hyscan-gtk-recorder.h
               hyscan-gtk-tvg-control.h
               hyscan-gtk-player.h
               hyscan-colormap-spec.h
         COMPONENT development
         DESTINATION "include/hyscan-${HYSCAN_MAJOR_VERSION}/hyscangtk"
         PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)
