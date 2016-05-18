OUTDIR = bin
OUTPUT=$(OUTDIR)/raytracer
CC=emcc
EXT = c

srcs:=$(wildcard *.$(EXT))
hdrs:=$(wildcard *.h)

all: $(srcs) $(OUTPUT).html

.PHONY: all test

$(OUTPUT).html: $(srcs) $(hdrs)
	@mkdir -p $(@D)
	$(CC) $(srcs) --emrun -o $(OUTPUT).html --preload-file data --exclude-file *.blend -s TOTAL_MEMORY=134217728 --js-library library.js

test:
	emrun $(OUTPUT).html

clean:
	rm $(OUTPUT).html $(OUTPUT).js $(OUTPUT).data