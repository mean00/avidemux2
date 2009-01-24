#
# Dirty makefile to include addons
#

DISTFILES= Makefile ad2_copy_nuv \
	ffv1rec/History \
	ffv1rec/LICENSE \
	ffv1rec/Makefile \
	ffv1rec/ffv1.h \
	ffv1rec/ffv1audio.c \
	ffv1rec/ffv1codec.c \
	ffv1rec/ffv1color.c \
	ffv1rec/ffv1rec.c \
	ffv1rec/ffv1video.c \
	ffv1rec/ffv1write.c \
	ffv1rec/nuppelvideo.h
install: 
	@echo "nothing to install here"
clean: 
	@echo "nothing to clean here"
distclean: 
	@echo "nothing to distclean here"
all: $(DISTFILE)
	@echo "nothing to build here"
srcdir= .
distdir: $(DISTFILES)
	mkdir $(distdir)/ffv1rec
	@for file in $(DISTFILES); do \
	if test -f $$file; then d=.; else d=$(srcdir); fi; \
	dir=`echo "$$file" | sed -e 's,/[^/]*$$,,'`; \
	if test "$$dir" != "$$file" && test "$$dir" != "."; then \
	$(mkinstalldirs) "$(distdir)/$$dir"; \
	fi; \
	if test -d $$d/$$file; then \
	cp -pR $$d/$$file $(distdir) \
	|| exit 1; \
	else \
	test -f $(distdir)/$$file \
	|| cp -p $$d/$$file $(distdir)/$$file \
	|| exit 1; \
	fi; \
	done

