include ../../../mk/rules.mk
include ../../../mk/init.mk
include ../../../mk/objective.mk

OBJECTIVE_BINS = dicttest

LDADD = -L../../libmowgli -lmowgli

CFLAGS += -I../../libmowgli

SOURCES = dicttest.c

OBJECTS = ${SOURCES:.c=.o}

dicttest: $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) $(LDADD) -o $@
	@printf "%10s     %-20s\n" LINK $@
