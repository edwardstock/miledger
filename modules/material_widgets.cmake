
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets Core StateMachine REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets Core StateMachine REQUIRED)

set(MATERIAL_LIB_PATH ${CMAKE_CURRENT_SOURCE_DIR}/libs/material/components)

set(MATERIAL_HEADERS
    ${MATERIAL_LIB_PATH}/qtmaterialavatar_p.h
    ${MATERIAL_LIB_PATH}/qtmaterialavatar.h
    ${MATERIAL_LIB_PATH}/lib/qtmaterialstyle_p.h
    ${MATERIAL_LIB_PATH}/lib/qtmaterialstyle.h
    ${MATERIAL_LIB_PATH}/lib/qtmaterialtheme_p.h
    ${MATERIAL_LIB_PATH}/lib/qtmaterialtheme.h
    ${MATERIAL_LIB_PATH}/qtmaterialbadge_p.h
    ${MATERIAL_LIB_PATH}/qtmaterialbadge.h
    ${MATERIAL_LIB_PATH}/lib/qtmaterialoverlaywidget.h
    ${MATERIAL_LIB_PATH}/qtmaterialcheckbox_p.h
    ${MATERIAL_LIB_PATH}/qtmaterialcheckbox.h
    ${MATERIAL_LIB_PATH}/lib/qtmaterialcheckable_internal.h
    ${MATERIAL_LIB_PATH}/lib/qtmaterialcheckable_p.h
    ${MATERIAL_LIB_PATH}/lib/qtmaterialripple.h
    ${MATERIAL_LIB_PATH}/lib/qtmaterialrippleoverlay.h
    ${MATERIAL_LIB_PATH}/lib/qtmaterialcheckable.h
    ${MATERIAL_LIB_PATH}/qtmaterialfab_p.h
    ${MATERIAL_LIB_PATH}/qtmaterialfab.h
    ${MATERIAL_LIB_PATH}/qtmaterialraisedbutton_p.h
    ${MATERIAL_LIB_PATH}/qtmaterialraisedbutton.h
    ${MATERIAL_LIB_PATH}/qtmaterialflatbutton_internal.h
    ${MATERIAL_LIB_PATH}/qtmaterialflatbutton_p.h
    ${MATERIAL_LIB_PATH}/qtmaterialflatbutton.h
    ${MATERIAL_LIB_PATH}/lib/qtmaterialstatetransition.h
    ${MATERIAL_LIB_PATH}/lib/qtmaterialstatetransitionevent.h
    ${MATERIAL_LIB_PATH}/qtmaterialiconbutton_p.h
    ${MATERIAL_LIB_PATH}/qtmaterialiconbutton.h
    ${MATERIAL_LIB_PATH}/qtmaterialprogress_internal.h
    ${MATERIAL_LIB_PATH}/qtmaterialprogress_p.h
    ${MATERIAL_LIB_PATH}/qtmaterialprogress.h
    ${MATERIAL_LIB_PATH}/qtmaterialcircularprogress_internal.h
    ${MATERIAL_LIB_PATH}/qtmaterialcircularprogress_p.h
    ${MATERIAL_LIB_PATH}/qtmaterialcircularprogress.h
    ${MATERIAL_LIB_PATH}/qtmaterialslider_internal.h
    ${MATERIAL_LIB_PATH}/qtmaterialslider_p.h
    ${MATERIAL_LIB_PATH}/qtmaterialslider.h
    ${MATERIAL_LIB_PATH}/qtmaterialsnackbar_internal.h
    ${MATERIAL_LIB_PATH}/qtmaterialsnackbar_p.h
    ${MATERIAL_LIB_PATH}/qtmaterialsnackbar.h
    ${MATERIAL_LIB_PATH}/qtmaterialradiobutton_p.h
    ${MATERIAL_LIB_PATH}/qtmaterialradiobutton.h
    ${MATERIAL_LIB_PATH}/qtmaterialtoggle_internal.h
    ${MATERIAL_LIB_PATH}/qtmaterialtoggle_p.h
    ${MATERIAL_LIB_PATH}/qtmaterialtoggle.h
    ${MATERIAL_LIB_PATH}/qtmaterialtextfield_internal.h
    ${MATERIAL_LIB_PATH}/qtmaterialtextfield_p.h
    ${MATERIAL_LIB_PATH}/qtmaterialtextfield.h
    ${MATERIAL_LIB_PATH}/qtmaterialtabs_internal.h
    ${MATERIAL_LIB_PATH}/qtmaterialtabs_p.h
    ${MATERIAL_LIB_PATH}/qtmaterialtabs.h
    ${MATERIAL_LIB_PATH}/qtmaterialscrollbar_internal.h
    ${MATERIAL_LIB_PATH}/qtmaterialscrollbar_p.h
    ${MATERIAL_LIB_PATH}/qtmaterialscrollbar.h
    ${MATERIAL_LIB_PATH}/qtmaterialdialog_internal.h
    ${MATERIAL_LIB_PATH}/qtmaterialdialog_p.h
    ${MATERIAL_LIB_PATH}/qtmaterialdialog.h
    ${MATERIAL_LIB_PATH}/qtmaterialdrawer_internal.h
    ${MATERIAL_LIB_PATH}/qtmaterialdrawer_p.h
    ${MATERIAL_LIB_PATH}/qtmaterialdrawer.h
    ${MATERIAL_LIB_PATH}/qtmaterialappbar.h
    ${MATERIAL_LIB_PATH}/qtmaterialappbar_p.h
    ${MATERIAL_LIB_PATH}/qtmaterialautocomplete.h
    ${MATERIAL_LIB_PATH}/qtmaterialautocomplete_p.h
    ${MATERIAL_LIB_PATH}/qtmaterialpaper.h
    ${MATERIAL_LIB_PATH}/qtmaterialpaper_p.h
    ${MATERIAL_LIB_PATH}/qtmaterialtable.h
    ${MATERIAL_LIB_PATH}/qtmaterialtable_p.h
    ${MATERIAL_LIB_PATH}/layouts/qtmaterialsnackbarlayout.h
    ${MATERIAL_LIB_PATH}/layouts/qtmaterialsnackbarlayout_p.h
    ${MATERIAL_LIB_PATH}/qtmaterialautocomplete_internal.h
    ${MATERIAL_LIB_PATH}/qtmaterialmenu.h
    ${MATERIAL_LIB_PATH}/qtmaterialmenu_p.h
    ${MATERIAL_LIB_PATH}/qtmaterialmenu_internal.h
    ${MATERIAL_LIB_PATH}/qtmateriallist.h
    ${MATERIAL_LIB_PATH}/qtmateriallist_p.h
    ${MATERIAL_LIB_PATH}/qtmateriallistitem.h
    ${MATERIAL_LIB_PATH}/qtmateriallistitem_p.h

    )
set(MATERIAL_SOURCES
    ${MATERIAL_HEADERS}
    ${MATERIAL_LIB_PATH}/qtmaterialavatar.cpp
    ${MATERIAL_LIB_PATH}/lib/qtmaterialstyle.cpp
    ${MATERIAL_LIB_PATH}/lib/qtmaterialtheme.cpp
    ${MATERIAL_LIB_PATH}/qtmaterialbadge.cpp
    ${MATERIAL_LIB_PATH}/lib/qtmaterialoverlaywidget.cpp
    ${MATERIAL_LIB_PATH}/qtmaterialcheckbox.cpp
    ${MATERIAL_LIB_PATH}/lib/qtmaterialcheckable_internal.cpp
    ${MATERIAL_LIB_PATH}/lib/qtmaterialcheckable.cpp
    ${MATERIAL_LIB_PATH}/lib/qtmaterialripple.cpp
    ${MATERIAL_LIB_PATH}/lib/qtmaterialrippleoverlay.cpp
    ${MATERIAL_LIB_PATH}/qtmaterialfab.cpp
    ${MATERIAL_LIB_PATH}/qtmaterialraisedbutton.cpp
    ${MATERIAL_LIB_PATH}/qtmaterialflatbutton_internal.cpp
    ${MATERIAL_LIB_PATH}/qtmaterialflatbutton.cpp
    ${MATERIAL_LIB_PATH}/lib/qtmaterialstatetransition.cpp
    ${MATERIAL_LIB_PATH}/qtmaterialiconbutton.cpp
    ${MATERIAL_LIB_PATH}/qtmaterialprogress_internal.cpp
    ${MATERIAL_LIB_PATH}/qtmaterialprogress.cpp
    ${MATERIAL_LIB_PATH}/qtmaterialcircularprogress_internal.cpp
    ${MATERIAL_LIB_PATH}/qtmaterialcircularprogress.cpp
    ${MATERIAL_LIB_PATH}/qtmaterialslider_internal.cpp
    ${MATERIAL_LIB_PATH}/qtmaterialslider.cpp
    ${MATERIAL_LIB_PATH}/qtmaterialsnackbar_internal.cpp
    ${MATERIAL_LIB_PATH}/qtmaterialsnackbar.cpp
    ${MATERIAL_LIB_PATH}/qtmaterialradiobutton.cpp
    ${MATERIAL_LIB_PATH}/qtmaterialtoggle_internal.cpp
    ${MATERIAL_LIB_PATH}/qtmaterialtoggle.cpp
    ${MATERIAL_LIB_PATH}/qtmaterialtextfield_internal.cpp
    ${MATERIAL_LIB_PATH}/qtmaterialtextfield.cpp
    ${MATERIAL_LIB_PATH}/qtmaterialtabs_internal.cpp
    ${MATERIAL_LIB_PATH}/qtmaterialtabs.cpp
    ${MATERIAL_LIB_PATH}/qtmaterialscrollbar_internal.cpp
    ${MATERIAL_LIB_PATH}/qtmaterialscrollbar.cpp
    ${MATERIAL_LIB_PATH}/qtmaterialdialog_internal.cpp
    ${MATERIAL_LIB_PATH}/qtmaterialdialog.cpp
    ${MATERIAL_LIB_PATH}/qtmaterialdrawer_internal.cpp
    ${MATERIAL_LIB_PATH}/qtmaterialdrawer.cpp
    ${MATERIAL_LIB_PATH}/qtmaterialappbar.cpp
    ${MATERIAL_LIB_PATH}/qtmaterialautocomplete.cpp
    ${MATERIAL_LIB_PATH}/qtmaterialpaper.cpp
    ${MATERIAL_LIB_PATH}/qtmaterialtable.cpp
    ${MATERIAL_LIB_PATH}/layouts/qtmaterialsnackbarlayout.cpp
    ${MATERIAL_LIB_PATH}/qtmaterialautocomplete_internal.cpp
    ${MATERIAL_LIB_PATH}/qtmaterialmenu.cpp
    ${MATERIAL_LIB_PATH}/qtmaterialmenu_internal.cpp
    ${MATERIAL_LIB_PATH}/qtmateriallist.cpp
    ${MATERIAL_LIB_PATH}/qtmateriallistitem.cpp
    ${MATERIAL_LIB_PATH}/resources.qrc
    )

add_library(qt_material STATIC ${MATERIAL_SOURCES})
target_include_directories(qt_material PUBLIC ${MATERIAL_LIB_PATH})
set_target_properties(qt_material PROPERTIES PUBLIC_HEADER "${MATERIAL_HEADERS}")
target_compile_definitions(qt_material PUBLIC QT_DEPRECATED_WARNINGS)
target_link_libraries(qt_material PUBLIC Qt${QT_VERSION_MAJOR}::Core)
target_link_libraries(qt_material PUBLIC Qt${QT_VERSION_MAJOR}::Widgets)
target_link_libraries(qt_material PUBLIC Qt${QT_VERSION_MAJOR}::StateMachine)