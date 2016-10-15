CONFIG += qtopiaapp
CONFIG -= buildQuicklaunch
DESTDIR = $(PWD)

HEADERS = roadmaster.hh

SOURCES = roadmaster.cc

SOURCES += main.cc


INTERFACES = ../layouts/roadmaster_base.ui
LIBS += -lrt
TARGET = gui/roadmaster-gui
