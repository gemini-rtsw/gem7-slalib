include .applTop
-include $(APPLIC_TOP)/config/CONFIG

APPLIC_DIR_TYPE = src

-include $(APPLIC_TOP)/config/RULES.Dirs

# Define cp if we don't have it yet
ifndef CP
CP = cp
endif

.applTop : ../.applTop
	$(CP) $< $@

clean ::
	$(RM) .applTop

