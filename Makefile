# $Id$
#

include .applTop/config/CONFIG

APPLIC_DIR_TYPE = sys

include .applTop/config/RULES.Dirs

PROD       = $(notdir $(shell pwd))
TAR        = /usr/bin/tar
COMPRESS   = gzip
LS         = /bin/ls
FIXED_PATH = /gemini

release:
	$(RM) $(PROD).tar $(PROD).tar.Z $(PROD).tar.gz .xfile
	find * \
	    \( -name bin -o -name config -o -name db -o -name epics \
		-o -name 'O.*' -o -name '*.dctsdr' -o -name '*.sdrSum' \
		-o -name '*%' -o -name '*~' -o -name '*.Z' -o -name '*.gz' \
		-o -name include -o -name Distfile -o -name data \
		-o -name CVS \
	    \) -prune -print > .xfile
	echo 'capfast/cad.rc' >> .xfile
	echo 'data/resource.def' >> .xfile
	echo 'dl/template.adl' >> .xfile
#
# Because some systems still don't use the default colors.adl
# Also because 'applSetup' does not copy the local colors.adl
# from templates if the 'dl' directory exists
#	echo 'dl/colors.adl' >> .xfile
#
	echo 'include/rec' >> .xfile
	echo 'startup/local.vws' >> .xfile
	echo 'startup/resource.def' >> .xfile
	$(TAR) cvXf .xfile $(PROD).tar $(APPLIC_SUBDIR_FILE) *
	$(COMPRESS) $(PROD).tar
	$(RM) .xfile

gemini:
	@if [ "$(SYS)" = "" -a "$(LIB)" = "" ]; \
	then echo Useage: "gmake gemini SYS=<name> or LIB=<name>"; \
	else \
	  if [ "$(SYS)" != "" ]; then \
	    echo "Installing $(SYS)/bin..."; \
	    cp -r -p $(APPLIC_INSTALL)/bin   $(FIXED_PATH)/$(SYS); \
	    chmod g+s $(FIXED_PATH)/$(SYS)/bin; \
	    echo "Installing $(SYS)/data..."; \
	    cp -r -p $(APPLIC_INSTALL)/data  $(FIXED_PATH)/$(SYS); \
	    chmod g+s $(FIXED_PATH)/$(SYS)/data; \
          else \
	    dir=`$(LS) $(APPLIC_INSTALL)/bin`; \
	    for DIR in $${dir}; do \
	      echo "Installing $(LIB)/lib/$$DIR..."; \
	      if [ ! -d $(FIXED_PATH)/$(LIB)/lib ]; then \
	        mkdir $(FIXED_PATH)/$(LIB)/lib; \
	      fi; \
	      cp -r -p $(APPLIC_INSTALL)/bin/$$DIR  $(FIXED_PATH)/$(LIB)/lib; \
	      chmod g+s $(FIXED_PATH)/$(LIB)/lib/$$DIR; \
	    done; \
          fi; \
	fi
