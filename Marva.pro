######################################################################
# Automatically generated by qmake (3.0) ?? ??? 24 12:21:03 2015
######################################################################

TEMPLATE = app
TARGET = Marva
QT += widgets core gui
INCLUDEPATH += .

# Input
HEADERS += edge.h graphwidget.h \
    blast_record.h \
    blast_data.h \
    main_window.h \
    tax_map.h \
    map_loader_thread.h \
    tree_loader_thread.h \
    graph_node.h \
    loader_thread.h \
    base_tax_node.h \
    ui_components/statuslistpanel.h \
    threadsafelist.h \
    ui_components/taxlistwidget.h \
    taxnodesignalsender.h
FORMS += \
    main_window.ui \
    ui_components/taxlistwidget.ui
SOURCES += edge.cpp graphwidget.cpp \
    blast_record.cpp \
    blast_data.cpp \
    main_window.cpp \
    tax_map.cpp \
    map_loader_thread.cpp \
    tree_loader_thread.cpp \
    graph_node.cpp \
    loader_thread.cpp \
    base_tax_node.cpp \
    ui_components/statuslistpanel.cpp \
    main.cpp \
    ui_components/taxlistwidget.cpp \
    taxnodesignalsender.cpp
