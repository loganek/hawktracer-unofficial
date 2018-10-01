find_package(Qt5 COMPONENTS Widgets Charts REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

add_executable(hawktracer-viewer WIN32
    viewer_main.cpp
    graph_dialog.cpp
    graph_manager.cpp
    event_db.cpp
    chart_view.cpp
    main_window.cpp
    controller.cpp)

target_link_libraries(hawktracer-viewer Qt5::Widgets Qt5::Charts hawktracer_parser hawktracer_client_utils)
