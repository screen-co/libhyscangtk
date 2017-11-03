
set (TEST_LIBRARIES ${GTK3_LIBRARIES}
                    ${LIBXML2_LIBRARIES}
                    ${HYSCAN_LIBRARIES}
                    ${HYSCAN_GTK_LIBRARY})

add_executable (hyscan-gtk-views-test hyscan-gtk-views-test.c)

target_link_libraries (hyscan-gtk-views-test ${TEST_LIBRARIES})

install (TARGETS hyscan-gtk-views-test
         COMPONENT test
         RUNTIME DESTINATION bin
         LIBRARY DESTINATION lib
         ARCHIVE DESTINATION lib
         PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)
