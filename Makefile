SUBDIRS = core gui cli

all:: $(SUBDIRS)

upload: CMD=upload
upload: $(SUBDIRS)
	./upload.sh

clean: CMD=clean
clean: $(SUBDIRS)

$(SUBDIRS)::
	make -C $@ $(CMD)
