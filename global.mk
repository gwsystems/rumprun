DBG?=	 -O2
#DBG?=	 -O2 -g
CFLAGS+= -std=gnu99 ${DBG}
CFLAGS+= -fno-stack-protector -ffreestanding
CXXFLAGS+= -fno-stack-protector -ffreestanding

CFLAGS+= -Wall -Wimplicit -Wmissing-prototypes -Wstrict-prototypes -Wl,-r -Wl,-melf_i386 -m32
ifndef NOGCCERROR
CFLAGS+= -Werror
endif
