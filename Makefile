CC ?= cc
AR ?= ar
RANLIB ?= ranlib
SILENT ?= @

CFLAGS ?=
LDFLAGS ?=

CFLAGS_ALL = -MD -MP -Iinclude -include config.h $(CPPFLAGS) $(CFLAGS)
LDFLAGS_ALL = $(LDFLAGS)

LIB_SRCS = $(shell find ./lib -name "*.c" -type f)
LIB_OBJS = $(LIB_SRCS:.c=.o)
LIB_STATIC = lib/lib.a

all:

RUNIT = runit/runit runit/runit-init runit/runsvchdir runit/runsvdir runit/sv
RUNIT+= runit/svlogd
$(RUNIT) : % : %.o $(LIB_STATIC)

SOCKLOG = socklog/socklog socklog/tryto socklog/uncat
$(SOCKLOG) : % : %.o $(LIB_STATIC)

PROGS = $(RUNIT) $(SOCKLOG) $(SOCKIT)

$(LIB_STATIC): $(LIB_OBJS)
	@rm -f $@
	@printf "[AR]	%s\n" "$@"
	${SILENT}$(AR) rc $@ $^
	${SILENT}$(RANLIB) $@

$(PROGS):
	@printf "[CCLD]	%s\n" "$@"
	${SILENT}$(CC) $(CFLAGS_ALL) $(LDFLAGS_ALL) -o $@ $^

%.o: %.c
	@printf "[CC]	%s\n" "$@"
	${SILENT}$(CC) $(CFLAGS_ALL) -o $@ -c $<

config.h: config.def.h
	cp $< $@

all: config.h $(LIB_STATIC) $(PROGS)
runit: config.h $(RUNIT)
socklog: config.h $(SOCKLOG)
sockit: config.h $(SOCKIT)

clean:
	@rm -f $(LIB_OBJS) $(LIB_OBJS:.o=.d) $(LIB) runit/*.o

-include config.mk
-include $(shell find . -name "*.d" -type f)

.PHONY: all runit sockit clean
