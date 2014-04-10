ifeq ($(wildcard config/config.mak),)
$(error Run ./configure first)
endif

include config/config.mak

BASEDIR = .
BUILDDIR = build
SRCINCLUDE = -I$(BASEDIR)/config -I$(BASEDIR)/src
SRCC = $(wildcard src/*.c)
SRCASM =
OBJC =
OBJASM =

all: default
default: example$(EXE)

# add assembler
ifeq ($(ARCH),x86)
SRCASM += src/cpuid_x86.S
endif

# expand source file paths in to $(BUILDDIR)
OBJC += $(patsubst %.c, $(BUILDDIR)/%.o, $(SRCC))
OBJASM += $(patsubst %.S, $(BUILDDIR)/%.o, $(SRCASM))

# base object name (without extension) placeholder
BASEOBJ = $(BUILDDIR)/$*

# compile assembler
$(BUILDDIR)/%.o: %.S
	@mkdir -p $(dir $@)
ifeq ($(AS),yasm)
	$(AS) $(ASFLAGS) $(SRCINCLUDE) -o $@ $<
	@$(AS) $(ASFLAGS) $(SRCINCLUDE) -o $@ -M $< >$(BASEOBJ).temp
else
	$(AS) $(ASFLAGS) $(SRCINCLUDE) -MMD -MF $(BASEOBJ).temp -c -o $(BASEOBJ).o $<
endif
	@cp $(BASEOBJ).temp $(BASEOBJ).P
	@sed \
	-e 's/^[^:]*: *//' \
	-e 's/ *\\$$//' \
	-e '/^$$/ d' \
	-e 's/$$/ :/' \
	< $(BASEOBJ).temp >> $(BASEOBJ).P
	@rm -f $(BASEOBJ).temp

# compiling C
$(BUILDDIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(SRCINCLUDE) -MMD -MF $(BASEOBJ).temp -c -o $(BASEOBJ).o $<
	@cp $(BASEOBJ).temp $(BASEOBJ).P
	@sed \
	-e 's/#.*//' \
	-e 's/^[^:]*: *//' \
	-e 's/ *\\$$//' \
	-e '/^$$/ d' \
	-e 's/$$/ :/' \
	< $(BASEOBJ).temp >> $(BASEOBJ).P
	@rm -f $(BASEOBJ).temp


# include generated dependencies
-include $(SRCC:%.c=$(BUILDDIR)/%.P)
-include $(SRCASM:%.S=$(BUILDDIR)/%.P)

# main program
example$(EXE): $(OBJC) $(OBJASM)
	$(CC) $(CFLAGS) -o $@ $(OBJC) $(OBJASM)

clean:
	@rm -rf $(BUILDDIR)/*
	@rm -f example$(EXE)

