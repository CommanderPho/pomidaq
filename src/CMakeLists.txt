# CMakeLists for PoMiDAQ GUI executable

if(WIN32)
  set(GUI_TYPE WIN32)
elseif(APPLE)
  set(GUI_TYPE MACOSX_BUNDLE)
endif()

set(POMIDAQ_SRC
    main.cpp
    mainwindow.h
    mainwindow.cpp
    mscontrolwidget.h
    mscontrolwidget.cpp
    imageviewwidget.h
    imageviewwidget.cpp
    elidedlabel.h
    elidedlabel.cpp
)

set(POMIDAQ_UI
    mainwindow.ui
)

qt5_wrap_ui(POMIDAQ_UI_SRC ${POMIDAQ_UI})

qt5_add_resources(POMIDAQ_RES_SRC ../data/resources.qrc)

add_executable(pomidaq
    ${GUI_TYPE}
    ${POMIDAQ_SRC}
    ${POMIDAQ_UI_SRC}
    ${POMIDAQ_RES_SRC}
)

target_link_libraries(pomidaq
    ${CMAKE_DL_LIBS}
    Qt5::Widgets
    ${KF5_ConfigWidgets}
    ${OpenCV_LIBS}
    ${OPENGL_LIBRARIES}
    miniscope
)

include_directories(SYSTEM
    ${OpenCV_INCLUDE_DIRS}

)
include_directories(
    ../libminiscope/
)

install(TARGETS pomidaq DESTINATION ${CMAKE_INSTALL_BINDIR})

# Windows stuff
if ("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
   add_definitions("-D_UNICODE" "-DUNICODE")
   set_target_properties(pomidaq PROPERTIES LINK_FLAGS_DEBUG "/SUBSYSTEM:CONSOLE")
   set_target_properties(pomidaq PROPERTIES COMPILE_DEFINITIONS_DEBUG "_CONSOLE")
   set_target_properties(pomidaq PROPERTIES LINK_FLAGS_RELWITHDEBINFO "/SUBSYSTEM:windows")
   set_target_properties(pomidaq PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:windows")
   set_target_properties(pomidaq PROPERTIES LINK_FLAGS_MINSIZEREL "/SUBSYSTEM:windows")
endif()


# MacOS Stuff
if ("${CMAKE_SYSTEM_NAME}" STREQUAL "Darwin")
    set(ICON_NAME "pomidaq.icns")
    set(ICON_PATH ${PROJECT_SOURCE_DIR}/data/MacOS/${ICON_NAME})
    set(pomidaq_ICON ${ICON_PATH})
    set_source_files_properties(${pomidaq_ICON} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
    file(COPY ${ICON_PATH} DESTINATION "pomidaq.app/Contents/Resources")

    set_target_properties(pomidaq PROPERTIES
        BUNDLE True
        MACOSX_BUNDLE_GUI_IDENTIFIER com.github.repo.bothlab.pomidaq
        MACOSX_BUNDLE_BUNDLE_NAME pomidaq
        MACOSX_BUNDLE_BUNDLE_VERSION "0.1"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "0.1"
        MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/data/MacOS/MacOSXBundleInfoTemplate.plist.in
    )
endif()