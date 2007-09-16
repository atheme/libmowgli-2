SUBDIRS = src
DISTCLEAN = extra.mk

include buildsys.mk

install-extra: ${LIB} ${PROG}
	i="libmowgli.pc"; \
	${INSTALL_STATUS}; \
	if ${INSTALL} -D -m 644 $$i ${libdir}/pkgconfig/$$i; then \
		${INSTALL_OK}; \
	else \
		${INSTALL_FAILED}; \
	fi
