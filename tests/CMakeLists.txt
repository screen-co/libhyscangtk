set (TEST_LIBRARIES ${GTK3_LIBRARIES}
                    ${LIBXML2_LIBRARIES}
                    ${HYSCAN_LIBRARIES}
                    ${HYSCAN_GTK_LIBRARY})

add_executable (gtk-connector-test gtk-connector-test.c)
target_link_libraries (gtk-connector-test ${TEST_LIBRARIES})
