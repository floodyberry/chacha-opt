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
INCLUDE = $(addprefix -I$(BASEDIR)/,driver extensions include src $(addsuffix $(ARCH)/,driver/))
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

SRCDRIVER = $(call rwildcard, driver/, *.c)
SRCEXT = $(call rwildcard, extensions/, *.c)
SRCASM =
SRCMAIN = src/main.c
SRCUTIL = src/util.c
SRCSHARED = src/shared.c
SRCUTIL += $(call rwildcard, src/util/, *.c)

# do we have an assembler?
ifeq ($(HAVEAS),yes)

# grab all the assembler files
SRCASM = $(call rwildcard, extensions/, *.S)

# add asm for the appropriate arch
SRCASM += $(call rwildcard, $(addsuffix $(ARCH),driver/), *.S)

endif

##########################
# expand all source file paths in to object files in $(BUILDDIR)/$(BUILDDIRUTIL)
#
OBJDRIVER = $(patsubst %.c, $(BUILDDIR)/%.o, $(SRCDRIVER))
OBJEXT = $(patsubst %.c, $(BUILDDIR)/%.o, $(SRCEXT))
OBJASM = $(patsubst %.S, $(BUILDDIR)/%.o, $(SRCASM))
OBJMAIN = $(patsubst %.c, $(BUILDDIR)/%.o, $(SRCMAIN))
OBJUTIL = $(patsubst %.c, $(BUILDDIRUTIL)/%.o, $(SRCUTIL))
OBJEXTUTIL = $(patsubst %.c, $(BUILDDIRUTIL)/%.o, $(SRCEXT))
OBJSHARED = $(patsubst %.c, $(BUILDDIR)/%.o, $(SRCSHARED))

##########################
# non-file targets
#
.PHONY: all
.PHONY: default
.PHONY: exe
.PHONY: lib
.PHONY: shared
.PHONY: util

.PHONY: install-shared
.PHONY: install-generic
.PHONY: install-lib
.PHONY: uninstall

.PHONY: clean
.PHONY: distclean


all: default

default: lib

exe: $(PROJECTNAME)$(EXE)
	@echo built [$(PROJECTNAME)$(EXE)]

install-generic:
	$(INSTALL) -d $(includedir)/lib$(PROJECTNAME)
	$(INSTALL) -d $(libdir)
	$(INSTALL) -m 644 include/*.h $(includedir)/lib$(PROJECTNAME)

lib: $(PROJECTNAME)$(STATICLIB)
	@echo built [$(PROJECTNAME)$(STATICLIB)]

install-lib: lib install-generic
	$(INSTALL) -m 644 $(PROJECTNAME)$(STATICLIB) $(libdir)
	$(if $(RANLIB), $(RANLIB) $(libdir)/$(PROJECTNAME)$(STATICLIB))

util: $(PROJECTNAME)-util$(EXE)
	@echo built [$(PROJECTNAME)-util$(EXE)]

ifeq ($(HAVESHARED),yes)
shared: $(SONAME)
	@echo built [$(SONAME)]

install-shared: shared install-generic
ifneq ($(SOIMPORT),)
	$(INSTALL) -d $(bindir)
	$(INSTALL) -m 755 $(SONAME) $(bindir)
	$(INSTALL) -m 644 $(SOIMPORT) $(libdir)
else ifneq ($(SONAME),)
	ln -f -s $(SONAME) $(libdir)/lib$(PROJECTNAME).$(SOSUFFIX)
	$(INSTALL) -m 755 $(SONAME) $(libdir)
endif
else
shared:
	@echo project must be /configured with --pic

install-shared:
	@echo project must be /configured with --pic
endif # HAVESHARED

uninstall:
	rm -rf $(includedir)/lib$(PROJECTNAME)
ifneq ($(SOIMPORT),)
	rm -f $(bindir)/$(SONAME) $(libdir)/lib$(SOIMPORT)
else ifneq ($(SONAME),)
	rm -f $(libdir)/$(SONAME) $(libdir)/lib$(PROJECTNAME).$(SOSUFFIX)
endif

clean:
	@echo cleaning project [$(PROJECTNAME)]
	@rm -rf $(BUILDDIR)/*
	@rm -rf $(BUILDDIRUTIL)/*
	@rm -f $(PROJECTNAME)$(EXE)
	@rm -f $(PROJECTNAME)$(STATICLIB)
ifneq ($(SOIMPORT),)
	@rm -f $(SOIMPORT)
endif
	@rm -f $(SONAME)
	@rm -f $(PROJECTNAME)-util$(EXE)

distclean: clean
	@rm asmopt.mak
	@rm config.log

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
	$(AS) $(ASFLAGS) $(ASMINCLUDE) $(DEPMM) $(DEPMF) $(BASEOBJ).temp -D BUILDING_ASM -c -o $(BASEOBJ).o $<
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

-include $(OBJDRIVER:%.o=%.P)
-include $(OBJEXT:%.o=%.P)
-include $(OBJASM:%.o=%.P)
-include $(OBJMAIN:%.o=%.P)
-include $(OBJUTIL:%.o=%.P)
-include $(OBJEXTUTIL:%.o=%.P)
-include $(OBJSHARED:%.o=%.P)

##########################
# final build targets
#
$(PROJECTNAME)$(EXE): $(OBJDRIVER) $(OBJEXT) $(OBJASM) $(OBJMAIN)
	$(CC) $(CFLAGS) -o $@ $(OBJDRIVER) $(OBJEXT) $(OBJASM) $(OBJMAIN)

$(PROJECTNAME)$(STATICLIB): $(OBJDRIVER) $(OBJEXT) $(OBJASM)
	rm -f $(PROJECTNAME)$(STATICLIB)
	$(AR)$@ $(OBJDRIVER) $(OBJEXT) $(OBJASM)
	$(if $(RANLIB), $(RANLIB) $@)

$(PROJECTNAME)-util$(EXE): $(OBJDRIVER) $(OBJEXTUTIL) $(OBJASM) $(OBJUTIL)
	$(CC) $(CFLAGS) -o $@ $(OBJDRIVER) $(OBJEXTUTIL) $(OBJASM) $(OBJUTIL)

ifeq ($(HAVESHARED),yes)
$(SONAME): $(OBJDRIVER) $(OBJEXT) $(OBJASM) $(OBJSHARED)
	$(LD)$@ $(OBJDRIVER) $(OBJEXT) $(OBJASM) $(OBJSHARED) $(SOFLAGS) $(LDFLAGS)
endif
