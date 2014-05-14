ifeq ($(wildcard config/config.mak),)
$(error Run ./configure first)
endif

include config/config.mak

.PHONY: all
.PHONY: default
.PHONY: fuzz
.PHONY: clean

all: default
default: example$(EXE)
fuzz: example-fuzz$(EXE)

# recursive wildcard: $(call rwildcard, basepath, globs)
rwildcard = $(foreach d, $(wildcard $(1)*), $(call rwildcard, $(d)/, $(2)) $(filter $(subst *, %, $(2)), $(d)))

BASEDIR = .
BUILDDIR = build
BUILDDIRFUZZ = build_fuzz
INCLUDE = $(addprefix -I$(BASEDIR)/,config src driver)
CINCLUDE = $(INCLUDE)
ASMINCLUDE = $(INCLUDE)
# yasm doesn't need includes passed to the assembler
ifneq ($(AS),yasm)
COMMA := ,
ASMINCLUDE += $(addprefix -Wa$(COMMA),$(INCLUDE))
endif

# grab all the c files
SRCC += $(call rwildcard, driver/, *.c)
SRCC += $(call rwildcard, src/, *.c)

# do we have an assembler?
ifeq ($(HAVEAS),yes)

# grab all the assembler files
SRCASM = $(call rwildcard, src/, *.S)

# add cpuid for our arch
ifeq ($(ARCH),x86)
SRCASM += driver/x86/cpuid_x86.S
endif

endif

# expand all source file paths in to $(BUILDDIR)
OBJC =
OBJCFUZZ =
OBJASM =
OBJC += $(patsubst %.c, $(BUILDDIR)/%.o, $(SRCC))
OBJCFUZZ += $(patsubst %.c, $(BUILDDIRFUZZ)/%.o, $(SRCC))
OBJASM += $(patsubst %.S, $(BUILDDIR)/%.o, $(SRCASM))

# use $(BASEOBJ) in build rules to grab the base path/name of the object file, without an extension
BASEOBJ = $(BUILDDIR)/$*
BASEOBJFUZZ = $(BUILDDIRFUZZ)/$*

# rule to build assembler files
$(BUILDDIR)/%.o: %.S
	@mkdir -p $(dir $@)
# yasm needs one pass to compile, and one to generate dependencies
ifeq ($(AS),yasm)
	$(AS) $(ASFLAGS) $(ASMINCLUDE) -o $@ $<
	@$(AS) $(ASFLAGS) $(ASMINCLUDE) -o $@ -M $< >$(BASEOBJ).temp
else
	$(AS) $(ASFLAGS) $(ASMINCLUDE) $(DEPMM) $(DEPMF) $(BASEOBJ).temp -c -o $(BASEOBJ).o $<
endif
	@cp $(BASEOBJ).temp $(BASEOBJ).P
	@sed \
	-e 's/^[^:]*: *//' \
	-e 's/ *\\$$//' \
	-e '/^$$/ d' \
	-e 's/$$/ :/' \
	< $(BASEOBJ).temp >> $(BASEOBJ).P
	@rm -f $(BASEOBJ).temp

# rule to build C files
$(BUILDDIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(CINCLUDE) $(DEPMM) $(DEPMF) $(BASEOBJ).temp -c -o $(BASEOBJ).o $<
	@cp $(BASEOBJ).temp $(BASEOBJ).P
	@sed \
	-e 's/#.*//' \
	-e 's/^[^:]*: *//' \
	-e 's/ *\\$$//' \
	-e '/^$$/ d' \
	-e 's/$$/ :/' \
	< $(BASEOBJ).temp >> $(BASEOBJ).P
	@rm -f $(BASEOBJ).temp

# rule to build C files, fuzzing
$(BUILDDIRFUZZ)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(CINCLUDE) $(DEPMM) $(DEPMF) $(BASEOBJFUZZ).temp -DFUZZ -c -o $(BASEOBJFUZZ).o $<
	@cp $(BASEOBJFUZZ).temp $(BASEOBJFUZZ).P
	@sed \
	-e 's/#.*//' \
	-e 's/^[^:]*: *//' \
	-e 's/ *\\$$//' \
	-e '/^$$/ d' \
	-e 's/$$/ :/' \
	< $(BASEOBJFUZZ).temp >> $(BASEOBJFUZZ).P
	@rm -f $(BASEOBJFUZZ).temp


# include generated dependencies
-include $(SRCC:%.c=$(BUILDDIR)/%.P)
-include $(SRCC:%.c=$(BUILDDIRFUZZ)/%.P)
-include $(SRCASM:%.S=$(BUILDDIR)/%.P)

example$(EXE): $(OBJC) $(OBJASM)
	$(CC) $(CFLAGS) -o $@ $(OBJC) $(OBJASM)

example-fuzz$(EXE): $(OBJCFUZZ) $(OBJASM)
	$(CC) $(CFLAGS) -o $@ $(OBJCFUZZ) $(OBJASM) -DFUZZ

clean:
	@rm -rf $(BUILDDIR)/*
	@rm -rf $(BUILDDIRFUZZ)/*
	@rm -f example$(EXE)
	@rm -f example-fuzz$(EXE)

