
set (TEST_LIBRARIES ${GTK3_LIBRARIES}
                    ${LIBXML2_LIBRARIES}
                    ${HYSCAN_LIBRARIES}
                    ${HYSCAN_GTK_LIBRARY})

add_executable (gtk-pane-test gtk-pane-test.c)

target_link_libraries (gtk-pane-test ${TEST_LIBRARIES})

install (TARGETS gtk-pane-test
         COMPONENT test
         RUNTIME DESTINATION bin
         LIBRARY DESTINATION lib
         ARCHIVE DESTINATION lib
         PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)
