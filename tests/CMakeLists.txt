set (TEST_LIBRARIES ${GTK3_LIBRARIES}
                    ${LIBXML2_LIBRARIES}
                    ${HYSCAN_LIBRARIES}
                    ${HYSCAN_GTK_LIBRARY})

add_executable (gtk-param-test gtk-param-test.c)
add_executable (gtk-datetime-test gtk-datetime-test.c)
add_executable (gtk-spin-button-test gtk-spin-button-test.c)
add_executable (gtk-connector-test gtk-connector-test.c)
add_executable (gtk-paned-test gtk-paned-test.c)

target_link_libraries (gtk-param-test ${TEST_LIBRARIES})
target_link_libraries (gtk-spin-button-test ${TEST_LIBRARIES})
target_link_libraries (gtk-datetime-test ${TEST_LIBRARIES})
target_link_libraries (gtk-connector-test ${TEST_LIBRARIES})
target_link_libraries (gtk-paned-test ${TEST_LIBRARIES})

