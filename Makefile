SUBDIRS := aoe2hd csgo_radar csgo_wh h1z1

all: $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@

clean:
	for subdir in $(SUBDIRS); do $(MAKE) -C $$subdir clean; done

.PHONY: all $(SUBDIRS)
