SUBDIRS = src
DISTCLEAN = extra.mk

include buildsys.mk

install-extra:
	i="libmowgli.pc"; \
	${INSTALL_STATUS}; \
	if ${MKDIR_P} ${DESTDIR}${datadir}/pkgconfig && ${INSTALL} -m 644 $$i ${DESTDIR}${datadir}/pkgconfig/$$i; then \
		${INSTALL_OK}; \
	else \
		${INSTALL_FAILED}; \
	fi

uninstall-extra:
	i="libmowgli.pc"; \
	if [ -f ${DESTDIR}${datadir}/pkgconfig/$$i ]; then \
		if rm -f ${DESTDIR}${datadir}/pkgconfig/$$i; then \
			${DELETE_OK}; \
		else \
			${DELETE_FAILED}; \
		fi \
	fi
