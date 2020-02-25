#############################################################################
# Makefile for building: EuNeurone
# Generated by qmake (1.04a) (Qt 3.1.1) on: Mon Aug 18 21:27:50 2003
# Project:  EuNeurone.pro
# Template: app
# Command: $(QMAKE) -o Makefile EuNeurone.pro
# Modified : 19/08/2003 ToS - Fixed: remove UI nonsense
#############################################################################

####### Compiler, tools and options

CPUOPTS = -march=nocona -mmmx -msse3 -mfpmath=sse -m64
## For 32 bit applications and present Matlab library:
## MATOPTS =  -DMAT_FILE -DMATLIB
QTDIR=/usr/lib/qt3
SOCOPTS= -DUSE_SOCKETS
## Otherwise use internal functions:
MATOPTS =  -DMAT_FILE
#CDFOPTS = -DCDF_FILE
HDFOPTS = -DHDF_FILE
HDF5OPTS = -DHDF5_FILE
## Data spinner implementation, otherwise use mutex
#SPINOPTS = -DSPINNER
CC       = gcc
CXX      = g++
LEX      = flex
YACC     = yacc
## MATPATH = -I/usr/local/matlab6p5/
## MATLIBPATH = -L/usr/local/matlab6p5/extern/lib/glnx86 -lmat -lut -lmx
## Qwt 4.2 vs 5
#QWT5OPT = -DQWT5 
#QWT5LIB = -lqwt5
QWT42LIB = -L/usr/lib/qt3/lib64/ -lqwt
#CDFPATH=-I/usr/local/cdf/include/
#CDFLIBPATH=-L/usr/local/cdf/lib/ -lcdf
HDFPATH=-I/usr/local/hdf4/include/
HDFLIBPATH=-L/usr/local/hdf4/lib/ -ldf -lmfhdf -ljpeg
HDF5LIBPATH=-lhdf5
CFLAGS   = -pipe -Wall -O2 $(CPUOPTS) -DGLX_GLXEXT_LEGACY -fno-use-cxa-atexit -fexceptions -D_REENTRANT   -DQT_DEBUG -DQT_THREAD_SUPPORT -DDO3D -DUSE_GSL $(MATOPTS) $(CDFOPTS) $(HDFOPTS) $(HDF5OPTS) $(SPINOPTS) $(QWT5OPT) $(SOCOPTS)
CXXFLAGS = -pipe -Wall -O2 $(CPUOPTS) -DGLX_GLXEXT_LEGACY -fno-use-cxa-atexit -fexceptions -D_REENTRANT   -DQT_DEBUG -DQT_THREAD_SUPPORT -DDO3D -DUSE_GSL $(MATOPTS) $(CDFOPTS) $(HDFOPTS) $(HDF5OPTS) $(SPINOPTS) $(QWT5OPT) $(SOCOPTS)
##CFLAGS   = -pipe -Wall -ggdb -pg $(CPUOPTS) -DGLX_GLXEXT_LEGACY -fno-use-cxa-atexit -fexceptions -D_REENTRANT   -DQT_DEBUG -DQT_THREAD_SUPPORT -D_DEBUG -DDO3D -DUSE_GSL $(MATOPTS) $(CDFOPTS) $(HDFOPTS) $(HDF5OPTS) $(SPINOPTS) $(QWT5OPT) $(SOCOPTS)
##CXXFLAGS = -pipe -Wall -ggdb -pg $(CPUOPTS) -DGLX_GLXEXT_LEGACY -fno-use-cxa-atexit -fexceptions -D_REENTRANT   -DQT_DEBUG -DQT_THREAD_SUPPORT -D_DEBUG -DDO3D -DUSE_GSL $(MATOPTS) $(CDFOPTS) $(HDFOPTS) $(HDF5OPTS) $(SPINOPTS) $(QWT5OPT) $(SOCOPTS)
LEXFLAGS = 
YACCFLAGS= -d
INCPATH  = $(CDFPATH) $(HDFPATH)  -I$(QTDIR)/mkspecs/default -I. -I/usr/local/include/ -I$(QTDIR)/include -I/usr/lib/qt3/include/qwt/ -I.moc/ $(MATPATH) 
LINK     = g++
LFLAGS   = -pg 
LIBS     = $(CDFLIBPATH)  $(HDFLIBPATH) $(HDF5LIBPATH) $(SUBLIBS) -L$(QTLIB)/lib/ -L/usr/X11R6/lib -L/usr/local/lib $(MATLIBPATH) $(QWT42LIB) $(QWT5LIB) -lstdc++ -lqt-mt -lXext -lX11 -lm -lGL -lGLU -lpthread -lgsl -lgslcblas
AR       = ar cqs
RANLIB   = 
MOC      = $(QTDIR)/bin/moc
UIC      = $(QTDIR)/bin/uic
QMAKE    = qmake
TAR      = tar -cf
GZIP     = gzip -9f
COPY     = cp -f
COPY_FILE= $(COPY) -p
COPY_DIR = $(COPY) -pR
DEL_FILE = rm -f
SYMLINK  = ln -sf
DEL_DIR  = rmdir
MOVE     = mv -f
CHK_DIR_EXISTS= test -d
MKDIR    = mkdir -p

####### Output directory

OBJECTS_DIR = .obj/

####### Files

HEADERS = about.h \
		dataform.h \
		gpl.h \
		graphform.h \
		poinisi.h \
		stats.h \
		analysis.h \
		editseriesform.h \
		graph.h \
		graphtype.h \
		random250.h \
		tglgraph.h \
		anasetdialog.h \
		equation.h \
		graph3dform.h \
		integrator.h \
		savestat.h \
		controlobject.h \
		formulc.h \
		graph3dtype.h \
		dataspinner.h \
		mainform.h
SOURCES = main.cpp \
		controlobject.cpp \
		equation.cpp \
		formulc.cpp \
		integrator.cpp \
		dataform.cpp \
		graph3dform.cpp \
		graph.cpp \
		graphtype.cpp \
		tglgraph.cpp \
		stats.cpp \
		anasetdialog.cpp \
		savestat.cpp \
		random250.cpp \
		about.cpp \
		analysis.cpp \
		dataspinner.cpp \
		poinisi.cpp
OBJECTS = .obj/main.o \
		.obj/controlobject.o \
		.obj/equation.o \
		.obj/formulc.o \
		.obj/integrator.o \
		.obj/graph.o \
		.obj/graphtype.o \
		.obj/mainform.o \
		.obj/dataform.o \
		.obj/graph3dform.o \
		.obj/tglgraph.o \
		.obj/stats.o \
		.obj/analysis.o \
		.obj/editseriesform.o \
		.obj/anasetdialog.o \
		.obj/savestat.o \
		.obj/poinisi.o \
		.obj/random250.o \
		.obj/about.o \
		.obj/dataspinner.o \
		.obj/qmake_image_collection.o 
SRCMOC   = .moc/moc_mainform.cpp \
		.moc/moc_dataform.cpp \
		.moc/moc_graph3dform.cpp \
		.moc/moc_editseriesform.cpp \
		.moc/moc_controlobject.cpp \
		.moc/moc_graph.cpp \
		.moc/moc_anasetdialog.cpp \
		.moc/moc_about.cpp \
		.moc/moc_savestat.cpp \
		.moc/moc_tglgraph.cpp \
		.moc/moc_poinisi.cpp
OBJMOC = .obj/moc_mainform.o \
		.obj/moc_dataform.o \
		.obj/moc_graph3dform.o \
		.obj/moc_editseriesform.o \
		.obj/moc_controlobject.o \
		.obj/moc_graph.o \
		.obj/moc_anasetdialog.o \
		.obj/moc_about.o \
		.obj/moc_savestat.o \
		.obj/moc_tglgraph.o \
		.obj/moc_poinisi.o
IMAGES = images/euneurone.png \
		images/filenew.png \
		images/fileopen.png \
		images/fileload.png \
		images/filesave.png \
		images/fileprint.png \
		images/undo.png \
		images/redo.png \
		images/editcut.png \
		images/editcopy.png \
		images/editpaste.png \
		images/search.png \
		images/newgraph.png \
		images/graph.png \
		images/new3dgraph.png \
		images/graph3d.png \
		images/filesaveas.png \
		images/exit.png \
		images/fileclose.png \
		images/newdata.png \
		images/filehistory.png \
		images/fileedit.png \
		images/anadata.png \
		images/dragdrop.png \
		images/undozoom.png \
		images/isi.png \
		images/dd.png \
		images/statq.png \
		images/statp.png \
		images/statok.png \
		images/update.png \
		images/stop.png \
		images/threadoff.png \
		images/threadon.png \
		images/threadwait.png

QMAKE_TARGET = EuNeurone
DESTDIR  = 
TARGET   = EuNeurone

first: all
####### Implicit rules

.SUFFIXES: .c .cpp .cc .cxx .C

.cpp.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<

.cc.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<

.cxx.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<

.C.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<

.c.o:
	$(CC) -c $(CFLAGS) $(INCPATH) -o $@ $<

####### Build rules

all: Makefile $(TARGET)

$(TARGET): $(OBJECTS) $(OBJMOC) 
	$(LINK) $(LFLAGS) -o $(TARGET) $(OBJECTS) $(OBJMOC) $(LIBS)

mocables: $(SRCMOC)

$(MOC): 
	( cd $(QTDIR)/src/moc ; $(MAKE) )

dist: 
	@mkdir -p .obj/EuNeurone && $(COPY_FILE) --parents $(SOURCES) $(HEADERS) .obj/EuNeurone/ && $(COPY_FILE) --parents .obj/EuNeurone/ && ( cd `dirname .obj/EuNeurone` && $(TAR) EuNeurone.tar EuNeurone && $(GZIP) EuNeurone.tar ) && $(MOVE) `dirname .obj/EuNeurone`/EuNeurone.tar.gz . && $(DEL_FILE) -r .obj/EuNeurone

mocclean:
	-$(DEL_FILE) $(OBJMOC)
	-$(DEL_FILE) $(SRCMOC)

#uiclean:
#	-$(DEL_FILE) $(UICIMPLS) $(UICDECLS)

yaccclean:
lexclean:
clean: mocclean
	-$(DEL_FILE) $(OBJECTS) 
	-$(DEL_FILE) qmake_image_collection.cpp
	-$(DEL_FILE) *~ core *.core


####### Sub-libraries

distclean: clean
	-$(DEL_FILE) $(TARGET) $(TARGET)


FORCE:

####### Compile

.obj/main.o: main.cpp mainform.h \
		controlobject.h \
		equation.h \
		integrator.h \
		formulc.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o .obj/main.o main.cpp

.obj/controlobject.o: controlobject.cpp controlobject.h \
		.moc/moc_controlobject.cpp \
		equation.h \
		integrator.h \
		anasetdialog.h \
		formulc.h \
		graphtype.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o .obj/controlobject.o controlobject.cpp

.obj/equation.o: equation.cpp formulc.h \
		equation.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o .obj/equation.o equation.cpp

.obj/formulc.o: formulc.cpp formulc.h \
		random250.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o .obj/formulc.o formulc.cpp

.obj/integrator.o: integrator.cpp controlobject.h \
		integrator.h \
		equation.h \
		formulc.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o .obj/integrator.o integrator.cpp

.obj/dataspinner.o: dataspinner.cpp dataspinner.h \
		integrator.h \
		equation.h \
		formulc.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o .obj/dataspinner.o dataspinner.cpp

.obj/stats.o: stats.cpp controlobject.h \
		stats.h 
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o .obj/stats.o stats.cpp

.obj/analysis.o: analysis.cpp controlobject.h \
		analysis.h 
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o .obj/analysis.o analysis.cpp

.obj/graph.o: graph.cpp graph.h \
		 graphtype.h \
		.moc/moc_graph.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o .obj/graph.o graph.cpp

.obj/graphtype.o: graphtype.cpp graphtype.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o .obj/graphtype.o graphtype.cpp

.obj/tglgraph.o: tglgraph.cpp tglgraph.h \
		.moc/moc_tglgraph.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o .obj/tglgraph.o tglgraph.cpp

.obj/mainform.o: mainform.cpp \
		mainform.h \
		poinisi.h \
		graph.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o .obj/mainform.o mainform.cpp

.obj/dataform.o: dataform.cpp dataform.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o .obj/dataform.o dataform.cpp

.obj/poinisi.o: poinisi.cpp poinisi.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o .obj/poinisi.o poinisi.cpp

.obj/graph3dform.o: graph3dform.cpp graph3dform.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o .obj/graph3dform.o graph3dform.cpp

.obj/editseriesform.o: editseriesform.cpp editseriesform.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o .obj/editseriesform.o editseriesform.cpp

.obj/savestat.o: savestat.cpp savestat.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o .obj/savestat.o savestat.cpp

.obj/anasetdialog.o: anasetdialog.cpp anasetdialog.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o .obj/anasetdialog.o anasetdialog.cpp

.obj/random250.o: random250.cpp random250.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o .obj/random250.o random250.cpp

.obj/about.o: about.cpp about.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o .obj/about.o about.cpp

.obj/moc_mainform.o: .moc/moc_mainform.cpp mainform.h 
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o .obj/moc_mainform.o .moc/moc_mainform.cpp

.obj/moc_dataform.o: .moc/moc_dataform.cpp dataform.h 
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o .obj/moc_dataform.o .moc/moc_dataform.cpp

.obj/moc_poinisi.o: .moc/moc_poinisi.cpp poinisi.h 
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o .obj/moc_poinisi.o .moc/moc_poinisi.cpp

.obj/moc_anasetdialog.o: .moc/moc_anasetdialog.cpp anasetdialog.h 
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o .obj/moc_anasetdialog.o .moc/moc_anasetdialog.cpp

.obj/moc_graph3dform.o: .moc/moc_graph3dform.cpp graph3dform.h 
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o .obj/moc_graph3dform.o .moc/moc_graph3dform.cpp

.obj/moc_editseriesform.o: .moc/moc_editseriesform.cpp editseriesform.h 
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o .obj/moc_editseriesform.o .moc/moc_editseriesform.cpp

.obj/moc_controlobject.o: .moc/moc_controlobject.cpp controlobject.h 
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o .obj/moc_controlobject.o .moc/moc_controlobject.cpp

.obj/moc_about.o: .moc/moc_about.cpp about.h 
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o .obj/moc_about.o .moc/moc_about.cpp

.obj/moc_graph.o: .moc/moc_graph.cpp graph.h 
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o .obj/moc_graph.o .moc/moc_graph.cpp

.obj/moc_savestat.o: .moc/moc_savestat.cpp savestat.h 
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o .obj/moc_savestat.o .moc/moc_savestat.cpp

.obj/moc_tglgraph.o: .moc/moc_tglgraph.cpp tglgraph.h 
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o .obj/moc_tglgraph.o .moc/moc_tglgraph.cpp

.moc/moc_controlobject.cpp: $(MOC) controlobject.h
	$(MOC) controlobject.h -o .moc/moc_controlobject.cpp

.moc/moc_graph.cpp: $(MOC) graph.h
	$(MOC) graph.h -o .moc/moc_graph.cpp

.moc/moc_tglgraph.cpp: $(MOC) tglgraph.h
	$(MOC) tglgraph.h -o .moc/moc_tglgraph.cpp

.moc/moc_mainform.cpp: $(MOC) mainform.h
	$(MOC) mainform.h -o .moc/moc_mainform.cpp

.moc/moc_dataform.cpp: $(MOC) dataform.h
	$(MOC) dataform.h -o .moc/moc_dataform.cpp

.moc/moc_poinisi.cpp: $(MOC) poinisi.h
	$(MOC) poinisi.h -o .moc/moc_poinisi.cpp

.moc/moc_anasetdialog.cpp: $(MOC) anasetdialog.h
	$(MOC) anasetdialog.h -o .moc/moc_anasetdialog.cpp

.moc/moc_savestat.cpp: $(MOC) savestat.h
	$(MOC) savestat.h -o .moc/moc_savestat.cpp

.moc/moc_graph3dform.cpp: $(MOC) graph3dform.h
	$(MOC) graph3dform.h -o .moc/moc_graph3dform.cpp

.moc/moc_editseriesform.cpp: $(MOC) editseriesform.h
	$(MOC) editseriesform.h -o .moc/moc_editseriesform.cpp

.moc/moc_about.cpp: $(MOC) about.h
	$(MOC) about.h -o .moc/moc_about.cpp

.obj/qmake_image_collection.o: qmake_image_collection.cpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o .obj/qmake_image_collection.o qmake_image_collection.cpp

qmake_image_collection.cpp: $(IMAGES)
	$(UIC)  -embed EuNeurone $(IMAGES) -o qmake_image_collection.cpp

####### Install

install: all 

uninstall: 

