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
	\( -name bin -o -name config -o -name epics     \
	-o -name 'O.*' -o -name '*%' -o -name '*~'      \
	-o -name '*.Z' -o -name '*.gz' -o -name include \
	-o -name Distfile -o -name data -o -name CVS    \
	\) -prune -print > .xfile
	echo 'capfast/cad.rc' >> .xfile
	echo 'data/resource.def' >> .xfile
	echo 'dl/template.adl' >> .xfile
#
# Because some systems still don't use the default colors.adl
# Also because 'applSetup' does not copy the local colors.adl
# from templates if the 'dl' directory exists
#       echo 'dl/colors.adl' >> .xfile
#
	echo 'startup/local.vws' >> .xfile
	echo 'startup/resource.def' >> .xfile
	$(TAR) cvXf .xfile $(PROD).tar *
	$(COMPRESS) $(PROD).tar
	$(RM) .xfile
