PLATFORM=cos
PLATFORM_DEFAULT_TESTER=qemu
OBJ_DIR=.

all: app-tools include/bmk/machine cosrun.o commonlibs

include ../../global.mk
-include config.mk
ifdef BUILDRUMP_TOOLFLAGS
include ${BUILDRUMP_TOOLFLAGS}
endif

include ../Makefile.inc

CPPFLAGS=	-Iinclude -Irump/include -I../../include -nostdinc
CFLAGS+=	${BUILDRUMP_TOOL_CFLAGS}
CFLAGS+=        -I../../../Composite/src/components/implementation/no_interface/rumpbooter/
CFLAGS+=        -I../../../Composite/src/components/include/
CFLAGS+=        -I../../../Composite/src/kernel/include/shared/
CFLAGS+=        -Iinclude -Irump/includle -I../../include

# Check if we're building for a supported target.
supported= false
HASPCI= y
# assume we're doing "make clean"
MACHINE?= i386
ifeq (${MACHINE},i386)
supported:= true
endif
ifeq (${MACHINE},amd64)
supported:= true
endif
ifeq (${MACHINE},arm)
ifdef IWANTARM
supported:= true
HASPCI:= n
endif
endif
ifneq (${supported},true)
$(error only supported target is x86, you have ${MACHINE})
endif

LDFLAGS_BAKE+=	-L$(abspath rump/lib)

include arch/${MACHINE}/Makefile.inc

OBJS_BMK = sched.o init.o memalloc.o pgalloc.o

OBJS= ${OBJS_APP} ${OBJS_BMK}

.PHONY:	clean cleandir test

APP_TOOLS_PLATFORM= cos
APP_TOOLS_OBJS_BAKE=	$(abspath cosrun.o)
APP_TOOLS_LDSCRIPT_BAKE:= $(abspath ${LDSCRIPT})
APP_TOOLS_DIR:= $(abspath ../../app-tools)

include ${APP_TOOLS_DIR}/Makefile.app-tools

include/bmk/machine:
	@ln -s ../arch/${MACHINE} include/bmk/machine

links: include/bmk/machine

cosrun.o: ${OBJS}
	${CC} -nostdlib ${CFLAGS} -o $@ cosrun.c -Wl,-r ${OBJS_BMK}

clean: app-tools_clean commonclean
	rm -f ${OBJS} include/bmk/machine buildtest

cleandir: clean

cleanrump: clean
	rm -rf rump rumpobj rumptools

distcleanrump: cleanrump
	rm -f config.mk
	${MAKE} -C ../../tests clean
