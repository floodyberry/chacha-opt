ifeq ($(wildcard asmopt.mak),)
$(error Run ./configure first)
endif

include asmopt.mak

##########################
# set up variables
#

BASEDIR = .
BUILDDIR = build
BUILDDIRUTIL = build_util
INCLUDE = $(addprefix -I$(BASEDIR)/,include src driver $(addsuffix $(ARCH)/,driver/))
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
# expand all source file paths in to object files in $(BUILDDIR)/$(BUILDDIRUTIL)
#
OBJC =
OBJCUTIL =
OBJASM =
OBJC += $(patsubst %.c, $(BUILDDIR)/%.o, $(SRCC))
OBJCUTIL += $(patsubst %.c, $(BUILDDIRUTIL)/%.o, $(SRCC))
OBJASM += $(patsubst %.S, $(BUILDDIR)/%.o, $(SRCASM))



##########################
# non-file targets
#
.PHONY: all
.PHONY: default
.PHONY: util
.PHONY: clean

all: default
default: example$(EXE)
util: example-util$(EXE)
clean:
	@rm -rf $(BUILDDIR)/*
	@rm -rf $(BUILDDIRUTIL)/*
	@rm -f example$(EXE)
	@rm -f example-util$(EXE)


##########################
# build rules for files
#

# use $(BASEOBJ) in build rules to grab the base path/name of the object file, without an extension
BASEOBJ = $(BUILDDIR)/$*
BASEOBJUTIL = $(BUILDDIRUTIL)/$*

# building .S (assembler) files
$(BUILDDIR)/%.o: %.S
	@mkdir -p $(dir $@)
# yasm needs one pass to compile, and one to generate dependencies
ifeq ($(AS),yasm)
	$(AS) $(ASFLAGS) $(ASMINCLUDE) -o $@ $<
	@$(AS) $(ASFLAGS) $(ASMINCLUDE) -o $@ -M $< >$(BASEOBJ).temp
else
	$(AS) $(ASFLAGS) $(ASMINCLUDE) $(DEPMM) $(DEPMF) $(BASEOBJ).temp -D ASM_PASS -c -o $(BASEOBJ).o $<
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

# building .c (C) files for fuzzing/benching
$(BUILDDIRUTIL)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(CINCLUDE) $(DEPMM) $(DEPMF) $(BASEOBJUTIL).temp -DUTILITIES -c -o $(BASEOBJUTIL).o $<
	@cp $(BASEOBJUTIL).temp $(BASEOBJUTIL).P
	@sed \
	-e 's/#.*//' \
	-e 's/^[^:]*: *//' \
	-e 's/ *\\$$//' \
	-e '/^$$/ d' \
	-e 's/$$/ :/' \
	< $(BASEOBJUTIL).temp >> $(BASEOBJUTIL).P
	@rm -f $(BASEOBJUTIL).temp


##########################
# include all auto-generated dependencies
#
-include $(SRCC:%.c=$(BUILDDIR)/%.P)
-include $(SRCC:%.c=$(BUILDDIRUTIL)/%.P)
-include $(SRCASM:%.S=$(BUILDDIR)/%.P)


##########################
# final build targets
#
example$(EXE): $(OBJC) $(OBJASM)
	$(CC) $(CFLAGS) -o $@ $(OBJC) $(OBJASM)

example-util$(EXE): $(OBJCUTIL) $(OBJASM)
	$(CC) $(CFLAGS) -o $@ $(OBJCUTIL) $(OBJASM) -DUTILITIES

