OUTDIR = bin
OUTFILE = $(OUTDIR)/raytracer
CC = gcc 
EXT = c

CPPFLAGS :=
# CC+= -O3
# CPPFLAGS+= -DRUN_TEST
CFLAGS :=
LDFLAGS := -lm -lpthread -static
#DEBUG_BUILD = -g
#DEBUG_TEST = gdb --eval-command=run
#OPT= -Ofast

ifeq ($(OS),Windows_NT)
	CPPFLAGS += -DWIN32
endif

objs := $(patsubst %.$(EXT),$(OUTDIR)/%.o,$(wildcard *.$(EXT)))
deps := $(objs:.o=.dep)

.PHONY: all test
all: $(OUTFILE)

-include $(deps)

$(OUTDIR)/%.o: %.$(EXT)
	@mkdir -p $(@D)
	$(CC) $(OPT) $(DEBUG_BUILD) $(CPPFLAGS) $(CFLAGS) -c $< -o $@
	@$(CC) $(CPPFLAGS) $(CFLAGS) -MM $< | sed -e '1,1 s|[^:]*:|$@:|' > $(OUTDIR)/$*.dep

$(OUTFILE) : $(objs)
	$(CC) $(OPT) $(DEBUG_BUILD) $^ $(LDFLAGS) -o $@

test: $(OUTFILE)
	@$(DEBUG_TEST) $(OUTFILE)
	
test2: $(OUTFILE)
	@$(DEBUG_TEST) $(OUTFILE) -mg -w512 -h512

clean:
	@rm -f $(deps) $(objs) $(OUTFILE) $(OUTFILE).exe
