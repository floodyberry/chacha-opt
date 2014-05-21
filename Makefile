ifeq ($(wildcard config/config.mak),)
$(error Run ./configure first)
endif

include config/config.mak

##########################
# set up variables
#

BASEDIR = .
BUILDDIR = build
BUILDDIRFUZZ = build_fuzz
INCLUDE = $(addprefix -I$(BASEDIR)/,config src driver $(addsuffix $(ARCH)/,driver/))
CINCLUDE = $(INCLUDE) 
ASMINCLUDE = $(INCLUDE)
# yasm doesn't need includes passed to the assembler
ifneq ($(AS),yasm)
COMMA := ,
ASMINCLUDE += $(addprefix -Wa$(COMMA),$(INCLUDE))
endif

###########################
# define recursive wildcard: $(call rwildcard, basepath, globs)
#
rwildcard = $(foreach d, $(wildcard $(1)*), $(call rwildcard, $(d)/, $(2)) $(filter $(subst *, %, $(2)), $(d)))

# grab all the c files in driver/ and src/
SRCC += $(call rwildcard, driver/, *.c)
SRCC += $(call rwildcard, src/, *.c)

# do we have an assembler?
ifeq ($(HAVEAS),yes)

# grab all the assembler files
SRCASM = $(call rwildcard, src/, *.S)

# add asm for the appropriate arch
SRCASM += $(call rwildcard, $(addsuffix $(ARCH),driver/), *.S)

endif

##########################
# expand all source file paths in to object files in $(BUILDDIR)/$(BUILDDIRFUZZ)
#
OBJC =
OBJCFUZZ =
OBJASM =
OBJC += $(patsubst %.c, $(BUILDDIR)/%.o, $(SRCC))
OBJCFUZZ += $(patsubst %.c, $(BUILDDIRFUZZ)/%.o, $(SRCC))
OBJASM += $(patsubst %.S, $(BUILDDIR)/%.o, $(SRCASM))



##########################
# non-file targets
#
.PHONY: all
.PHONY: default
.PHONY: fuzz
.PHONY: clean

all: default
default: example$(EXE)
fuzz: example-fuzz$(EXE)
clean:
	@rm -rf $(BUILDDIR)/*
	@rm -rf $(BUILDDIRFUZZ)/*
	@rm -f example$(EXE)
	@rm -f example-fuzz$(EXE)


##########################
# build rules for files
#

# use $(BASEOBJ) in build rules to grab the base path/name of the object file, without an extension
BASEOBJ = $(BUILDDIR)/$*
BASEOBJFUZZ = $(BUILDDIRFUZZ)/$*

# building .S (assembler) files
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

# building .c (C) files
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

# building .c (C) files for fuzzing
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


##########################
# include all auto-generated dependencies
#
-include $(SRCC:%.c=$(BUILDDIR)/%.P)
-include $(SRCC:%.c=$(BUILDDIRFUZZ)/%.P)
-include $(SRCASM:%.S=$(BUILDDIR)/%.P)


##########################
# final build targets
#
example$(EXE): $(OBJC) $(OBJASM)
	$(CC) $(CFLAGS) -o $@ $(OBJC) $(OBJASM)

example-fuzz$(EXE): $(OBJCFUZZ) $(OBJASM)
	$(CC) $(CFLAGS) -o $@ $(OBJCFUZZ) $(OBJASM) -DFUZZ

