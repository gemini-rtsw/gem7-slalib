include .applTop
-include $(APPLIC_TOP)/config/CONFIG

APPLIC_DIR_TYPE = sys

-include $(APPLIC_TOP)/config/RULES.Dirs

PROD =  $(notdir $(shell pwd))
TAR =   /usr/bin/tar
COMPRESS = gzip

release:
	$(RM) $(PROD).tar $(PROD).tar.Z $(PROD).tar.gz .xfile
	find * \
	\( -name .applTop     -o \
	   -name .cvsignore   -o \
           -name CVS          -o \
	   -name bin          -o \
	   -name lib          -o \
           -name build.log    -o \
           -name config       -o \
           -name data         -o \
           -name db           -o \
           -name dbd          -o \
           -name include      -o \
           -name javalib      -o \
	   -name cad.rc       -o \
	   -name Distfile     -o \
           -name local.vws    -o \
           -name resource.def -o \
           -name UAE.dist     -o \
	   -name 'O.*'        -o \
           -name '*%'         -o \
           -name '*~'         -o \
	   -name '*.Z'        -o \
           -name '*.gz'          \
	\) -prune -print > .xfile
	$(TAR) cvXf .xfile $(PROD).tar *
	$(COMPRESS) $(PROD).tar
	$(RM) .xfile
