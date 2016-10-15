include config.mk

QPEDIR		= $(QDIR)/qtopia
QTDIR		= $(QDIR)/qt2
DQTDIR		= $(QDIR)/dqt
PATH		:= $(QPEDIR)/bin:$(QTDIR)/bin:$(DQTDIR)/bin:$(PATH)
LD_LIBRARY_PATH := $(QPEDIR)/lib:$(QTDIR)/lib:$(DQTDIR)/lib:$(LD_LIBRARY_PATH)

export QPEDIR
export QTDIR
export DQTDIR
export PATH
export LD_LIBRARY_PATH

SPEC		= $(QPEDIR)/mkspecs/qws/$(SPEC_TARGET)

QMAKE		= qmake
DESIGNER	= designer

LAYOUT		= layouts/roadmaster_base.ui
LAYOUT_F16	= layouts/roadmaster_base_f16.ui
LAYOUT_M3W	= layouts/roadmasterm3w.ui
EXEC		= $(PWD)/roadmaster-gui

PRO		= src/roadmaster.pro
SRC		= $(PRO)		\
		  $(wildcard src/*.cc)	\
		  $(wildcard src/*.hh)

all:: $(EXEC)

run:: $(EXEC)
	$(QTDIR)/bin/qvfb -width 280 -height 450 -depth 16 &
	sleep 2
	./$< -qws

$(EXEC): $(SRC)
	$(QMAKE) -spec $(SPEC) -o src/Makefile $(PRO)
	$(MAKE) -C src

design::
	$(DESIGNER) $(LAYOUT)

design_f16::
	$(DESIGNER) $(LAYOUT_F16)

design_m3w::
	$(DESIGNER) $(LAYOUT_M3W)

upload:: $(EXEC)
	./upload.sh

clean::
	$(MAKE) -C src clean
	rm -rf $(EXEC)

distclean:: clean
	$(RM) config.mk
