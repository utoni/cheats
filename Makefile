SUBDIRS := aoe2hd csgo_radar csgo_wh libghack h1z1

all: $(SUBDIRS)
$(SUBDIRS):
	$(MAKE) -C $@

.PHONY: all $(SUBDIRS)
