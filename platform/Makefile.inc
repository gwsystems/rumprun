ifeq (${PLATFORM},)
$(error need to specify $$PLATFORM!)
endif

COREDIR:=       $(shell pwd)/../../lib/libbmk_core
RUMPUSERDIR:=   $(shell pwd)/../../lib/libbmk_rumpuser
BASEDIR:=       $(shell pwd)/../../lib/librumprun_base

COMMONDIR:=	$(abspath ../)

# For cos platform, we don't want libbmk_core
ifeq (${PLATFORM}, cos)
LDFLAGS_BAKE+= -L${BASEDIR}/${PLATFORM} -L${RUMPUSERDIR}/${PLATFORM}
else
LDFLAGS_BAKE+=	-L${COREDIR}/${PLATFORM} -L${BASEDIR}/${PLATFORM} -L${RUMPUSERDIR}/${PLATFORM}
endif

.PHONY: ${BASEDIR}/${PLATFORM}/librumprun_base.a
${BASEDIR}/${PLATFORM}/librumprun_base.a:
	( cd ${BASEDIR} \
	    && ${RUMPMAKE} MAKEOBJDIR=${PLATFORM} obj \
	    && ${RUMPMAKE} MAKEOBJDIR=${PLATFORM} dependall )

${COMMONDIR}/pseudolinkstubs.c: ${BASEDIR}/${PLATFORM}/librumprun_base.a
	sh ../makepseudolinkstubs.sh ${NM} ${RUMPSRC} $< $@

.PHONY: ${COREDIR}/${PLATFORM}/libbmk_core.a
${COREDIR}/${PLATFORM}/libbmk_core.a:
	( cd ${COREDIR} \
	    && ${RUMPMAKE} MAKEOBJDIR=${PLATFORM} obj \
	    && ${RUMPMAKE} MAKEOBJDIR=${PLATFORM} dependall )

.PHONY: ${RUMPUSERDIR}/${PLATFORM}/libbmk_rumpuser.a
${RUMPUSERDIR}/${PLATFORM}/libbmk_rumpuser.a:
	( cd ${RUMPUSERDIR} \
	    && ${RUMPMAKE} MAKEOBJDIR=${PLATFORM} obj \
	    && ${RUMPMAKE} MAKEOBJDIR=${PLATFORM} dependall )



ifeq (${PLATFORM}, cos)
COMMONLIBS := ${BASEDIR}/${PLATFORM}/librumprun_base.a ${RUMPUSERDIR}/${PLATFORM}/libbmk_rumpuser.a ${COMMONDIR}/pseudolinkstubs.o
else
COMMONLIBS := ${BASEDIR}/${PLATFORM}/librumprun_base.a ${COREDIR}/${PLATFORM}/libbmk_core.a ${RUMPUSERDIR}/${PLATFORM}/libbmk_rumpuser.a ${COMMONDIR}/pseudolinkstubs.o
endif

commonlibs: ${COMMONLIBS}

.PHONY: buildtest
buildtest: ../../tests/hello/hello.c ${OBJ_DIR}/rumprun.o commonlibs app-tools
	${APP_TOOLS_CC} -g -o $@ $< -lrumprun_tester
	@echo Testing baking ...
	@export RUMPRUN_WARNING_STFU=please ; for board in \
	    $(shell RUMPRUN_WARNING_STFU=please ${APP_TOOLS_DIR}/rumpbake list\
		| grep ^${PLATFORM}); do \
			echo $${board} ; \
			${APP_TOOLS_DIR}/rumpbake $${board} $@.$${board} $@ \
			    || exit 1;\
	done
	@echo done

.PHONY: commonclean
commonclean:
	( cd ${BASEDIR} && ${RUMPMAKE} MAKEOBJDIR=${PLATFORM} cleandir )
	( cd ${COREDIR} && ${RUMPMAKE} MAKEOBJDIR=${PLATFORM} cleandir )
	( cd ${RUMPUSERDIR} && ${RUMPMAKE} MAKEOBJDIR=${PLATFORM} cleandir )
	rm -f ${COMMONDIR}/pseudolinkstubs.c ${COMMONDIR}/pseudolinkstubs.o

.PHONY: tests
tests: ${OBJ_DIR}/rumprun.o commonlibs app-tools
	../../tests/buildtests.sh ${PLATFORM}
	../../tests/runtests.sh ${PLATFORM_DEFAULT_TESTER}
