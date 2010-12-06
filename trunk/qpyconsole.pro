######################################################################
# Automatically generated by qmake (1.05a) Mon Jul 18 19:50:31 2005
######################################################################

TEMPLATE = app
DEPENDPATH += include src ui
INCLUDEPATH += . include
CONFIG  += warn_on debug
UI_DIR = .ui
MOC_DIR = .moc
OBJECTS_DIR = .obj
LIBS += -Wl,-E -lpython$(PYTHON_VERSION)

unix {
  INCLUDEPATH += "$(PYTHON_PATH)/include/python$(PYTHON_VERSION)"
  LIBS += -L"$(PYTHON_PATH)/lib/python$(PYTHON_VERSION)/config" -lutil
}

win32 {
  INCLUDEPATH += "$(PYTHON_PATH)/include"
  LIBS += -L"$(PYTHON_PATH)/libs"
}

# Input
HEADERS += include/qpyconsole.h \
           include/qconsole.h
SOURCES += src/pymain.cpp \
           src/qpyconsole.cpp \
           src/qconsole.cpp
