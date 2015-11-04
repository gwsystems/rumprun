LIB=		rumprun_base
LIBISPRIVATE=	# defined

SRCS=		rumprun.c
SRCS+=		parseargs.c rumpconfig.c
SRCS+=		malloc.c netbsd_initfini.c signals.c
SRCS+=		syscall_mman.c syscall_misc.c
SRCS+=		__errno.c _lwp.c libc_stubs.c
SRCS+=		sysproxy.c

CPPFLAGS+=	-I${.CURDIR}/../../include

# RG edits:
# We need to be able to use rumpcalls.h inorder to call cos_thdid, this was apart of the fix for
# __thread (TLS problem)
CFLAGS+=        -I${.CURDIR}/../../../Composite/src/components/implementation/no_interface/rumpbooter

.include <bsd.lib.mk>
