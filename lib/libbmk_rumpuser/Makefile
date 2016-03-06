LIB=		bmk_rumpuser
LIBISPRIVATE=	# defined

# kernel-level source code
CFLAGS+=        -fno-stack-protector
CFLAGS+=        -I$(.CURDIR)/../../../Composite/src/components/implementation/no_interface/rumpbooter/
CFLAGS+=        -I$(.CURDIR)/../../../Composite/src/kernel/include/shared/


SRCS=		rumpuser_base.c
SRCS+=		rumpuser_clock.c
SRCS+=		rumpuser_cons.c
SRCS+=		rumpuser_mem.c
SRCS+=		rumpuser_synch.c

# RG: These bottom two are used by the PAWS application
SRCS+=    	rumpuser_fs.c
# SRCS+=    	backing.o

SRCS+=		rumpuser_stubs.c

CPPFLAGS+=	-I${.CURDIR}/../../include

.include <bsd.lib.mk>
