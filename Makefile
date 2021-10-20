#! /usr/bin/make -f
#---
#
#  gintrace project Makefile
#
# Build architecture:
# build/
#  |-- libraries/
#  |    |-- static/
#  |    |    |-- fx/
#  |    |    |    |-- gui_display.o
#  |    |    |    |-- gui_menu.o
#  |    |    |    ...
#  |    |    |    `-- ubc_handler.o
#  |    |    `-- cg/
#  |    |         |-- gui_display.o
#  |    |         |-- gui_menu.o
#  |    |         ...
#  |    |         `-- ubc_handler.o
#  |    `-- dynamic/
#  |         |-- fx/
#  |         |    |-- gui_display.o
#  |         |    |-- gui_menu.o
#  |         |    ...
#  |         |    `-- ubc_handler.o
#  |         `-- cg/
#  |              |-- gui_display.o
#  |              |-- gui_menu.o
#  |              ...
#  |              `-- ubc_handler.o
#  `-- demo/
#       |-- fx/
#       |    |-- main.o
#       |    `-- map (ELF link map informations)
#       `-- cg/
#            |-- main.o
#            `-- map
#
#---
MAJOR	:= 0
MINOR	:= 9
PATCH	:= 0
EXTRAVERSION	:=




#---
# Build rules
#---
# Make selects the first rule set when you type "make" but, in our case, we are
# going to generate most of the rules. So, we set a placeholder to force the
# "all" rule to be the "first" rule
first: all

# display the library version
version:
	@echo "$(MAJOR).$(MINOR).$(PATCH)$(EXTRAVERSION)"

# Display helper
help:
	@ echo 'Rules listing:'
	@ echo '... all         the default, if no target is provided'
	@ echo '... clean       remove build object'
	@ echo '... fclean      remove all generated object'
	@ echo '... re          same as `make fclean all`'
	@ echo '... version     display version'
	@ echo '... install     install the library'
	@ echo '... uninstall   uninstall the library'

.PHONY: help version first




#---
#  Build configuration
#---
# Require configuration file
CONFIG := gintrace.cfg
ifeq "$(wildcard $(CONFIG))" ""
$(error "config file $(CONFIG) does not exist (you should use `./configure`")
endif
include $(CONFIG)

# color definition, for swagg :D
red	:= \033[1;31m
green	:= \033[1;32m
blue	:= \033[1;34m
white	:= \033[1;37m
nocolor	:= \033[1;0m

# common information for FX platform
FX_INCLUDE	:= -I include -I .
FX_CFLAGS	:= -Wall -Wextra -Os

# common information for CG platform
CG_INCLUDE	:= -I include -I .
CG_CFLAGS	:= -Wall -Wextra -Os

# Determine the compiler install and include path
GCC_BASE_FX := $(shell sh-elf-gcc --print-search-dirs | grep install | sed 's/install: //')
GCC_BASE_CG := $(shell sh-elf-gcc --print-search-dirs | grep install | sed 's/install: //')
GCC_INCLUDE_FX := $(GCC_BASE_FX)/include
GCC_INCLUDE_CG := $(GCC_BASE_CG)/include


#---
# Generate building rules
#---
# This function will generate compilation rule for each sources.
# @params:
# *1 - source file pathname
# *2 - build directory path (output)
# *3 - C flags
# *4 - compiler name
# *5 - variable name (which store generated output filename)
# *6 - workaround to remove unwanted pathname information (src or demo)
# *7 - workaround to avoid undefined behaviour with $(eval $(call ...))
define generate-compilation-rule
# generate the rule name
# @note:
#   This is also the object filename, so to avoid multiple object filename
#   generation, we save it into one variable that will be added to the given
#   variable (arg $5), which list all object name.
object-$7-filename	:= $(patsubst $6_%,$2/%.o,$(subst /,_,$(basename $1)))

# generate the rules
$$(object-$7-filename): $1
ifeq ($(CONFIG.VERBOSE),true)
	@ mkdir -p $$(dir $$@)
	@ echo "$6 - $2 - $1"
	$4 $3 -o $$@ -c $$<
else
	@ mkdir -p $$(dir $$@)
	@ printf "$(green)>$(nocolor) $(white)$$@$(nocolor)\n"
	@ $4 $3 -o $$@ -c $$<
endif

# update the object fileame list, used by the main rule
$5	+= $$(object-$7-filename)
endef

# Function that will generate all rules for building each library.
# @params:
# *1 - format (dynamic/static)
# *2 - platform (fx/cg)
# *3 - source files list
# *4 - library name (without extra information ("gintrace" not "gintrace.a"))
# *5 - variable name (target list)
define generate-target-library
# generate common information
tname				:= $4-$2
target-$(tname)-build		:= build/library/$1/$2
target-$(tname)-build-src	:= build/library/$1/$2

# generate platform specific flags
ifeq ($2,fx)
	target-$(tname)-cflags	:= -D FX9860G -m3
	target-$(tname)-cflags	+= $(FX_INCLUDE) $(FX_CFLAGS)
endif
ifeq ($2,cg)
	target-$(tname)-cflags	:= -D FXCG50 -m4-nofpu
	target-$(tname)-cflags	+= $(CG_INCLUDE) $(CG_CFLAGS)
endif

# generate format-specific flags
ifeq ($1,static)
	target-$(tname)-ldflags	:=
	target-$(tname)-cflags	+= -mb -ffreestanding -nostdlib
	target-$(tname)-cflags	+= -fstrict-volatile-bitfields
	target-$(tname)-exec	:= lib$4-$2.a
	target-$(tname)-gcc	:= $(CONFIG.TOOLCHAIN)-gcc
	target-$(tname)-ar	:= $(CONFIG.TOOLCHAIN)-ar
endif
ifeq ($1,dynamic)
	target-$(tname)-ldflags	:= -shared -T dynlib.ld
	target-$(tname)-ldflags	+= -Wl,-Map=$$(target-$(tname)-build)/map
	target-$(tname)-ldflags	+= -Wl,-soname=$4-$2
	target-$(tname)-cflags	+= -mb -ffreestanding -nostdlib
	target-$(tname)-cflags	+= -fstrict-volatile-bitfields -fPIC
	target-$(tname)-exec	:= lib$4-$2-$(MAJOR).$(MINOR).$(PATCH).so
	target-$(tname)-gcc	:= $(CONFIG.TOOLCHAIN)-gcc
	target-$(tname)-ar	:= $(CONFIG.TOOLCHAIN)-ar
endif

# generate compilation rules and generate all object filename into the
# object list variable, this will be used by the `main` rule
target-$(tname)-obj	:=
$$(foreach source,$3,$$(eval \
	$$(call generate-compilation-rule,$$(source),\
		$$(target-$(tname)-build-src),$$(target-$(tname)-cflags),\
		$$(target-$(tname)-gcc),target-$(tname)-obj,src,$(tname))\
))

# Register the library building rule name
# @note:
# This rule list is used by the main compiling rule like a dependency. And it's
# this dependency that will involve all generated rules for building each
# libraries.
$5 += $$(target-$(tname)-exec)

# Generate the "linking" rule
$$(target-$(tname)-exec): $$(target-$(tname)-obj)
	@ mkdir -p $$(dir $$@)
	@ printf "$(blue)Create the library $(red)$$@$(nocolor)\n"
ifeq ($1,dynamic)
	$$(target-$(tname)-gcc) $$(target-$(tname)-ldflags) \
						-o $$@ $$^ -nostdlib -lgcc
else
	$$(target-$(tname)-ar) crs $$@ $$^
endif
endef

# Function that will generate all rules for building each demo addin
# @params:
# *1 - platform (fx/cg)
# *2 - source files list
# *3 - library name (without extra information ("gintrace" not "gintrace.a"))
# *4 - variable name (target list)
define generate-target-demo
# generate path information
tname				:= $3-$1
target-$(tname)-build		:= build/demo/$1
target-$(tname)-build-src	:= build/demo/$1

# generate common information
target-$(tname)-elf	:= $$(target-$(tname)-build)/$3-$1.elf
target-$(tname)-bin	:= $$(target-$(tname)-build)/$3-$1.bin
target-$(tname)-cflags	:= -mb -ffreestanding -nostdlib
target-$(tname)-cflags	+= -fstrict-volatile-bitfields
target-$(tname)-ldflags	:= -Wl,-Map=$$(target-$(tname)-build)/map
target-$(tname)-gcc	:= $(CONFIG.TOOLCHAIN)-gcc
target-$(tname)-objcopy	:= $(CONFIG.TOOLCHAIN)-objcopy

# generate platform specific flags
ifeq ($1,fx)
	target-$(tname)-cflags	+= -D FX9860G -m3
	target-$(tname)-cflags	+= $(FX_INCLUDE) $(FX_CFLAGS)
	target-$(tname)-ldflags	+= -T fx9860g.ld
	target-$(tname)-libs	:= -L. -L $(GCC_INCLUDE_FX)
	target-$(tname)-libs	+=  -lgint-fx -lgintrace-fx -lgint-fx -lc -lgcc
	target-$(tname)-exec	:= $3.g1a
endif
ifeq ($1,cg)
	target-$(tname)-cflags	+= -D FXCG50 -m4-nofpu
	target-$(tname)-cflags	+= $(CG_INCLUDE) $(CG_CFLAGS)
	target-$(tname)-ldflags	+= -T fxcg50.ld
	target-$(tname)-libs	:= -L. -L $(GCC_INCLUDE_CG)
	target-$(tname)-libs	+= -lgint-cg -lgintrace-cg -lgint-cg -lc -lgcc
	target-$(tname)-exec	:= $3.g3a
endif

# generate compilation rules and generate all object filename
target-$(tname)-obj	:=
$$(foreach source,$2,$$(eval \
	$$(call generate-compilation-rule,$$(source),\
		$$(target-$(tname)-build-src),$$(target-$(tname)-cflags),\
		$$(target-$(tname)-gcc),target-$(tname)-obj,demo,$(tname))\
))

# Register the demo building rule name
$4 += $$(target-$(tname)-exec)

# Generate the addin main rule
$$(target-$(tname)-exec): $$(target-$(tname)-obj)
	@ mkdir -p $$(dir $$@)
	@ printf "$(blue)Create the demo addin $(red)$$@$(nocolor)\n"
	$$(target-$(tname)-gcc) $$(target-$(tname)-cflags) \
		$$(target-$(tname)-ldflags) -o $$(target-$(tname)-elf) \
		$$(target-$(tname)-obj) $$(target-$(tname)-libs)
	$$(target-$(tname)-objcopy) -O binary -R .bss -R .gint_bss \
		$$(target-$(tname)-elf) $$(target-$(tname)-bin)
ifeq ($1,fx)
	fxg1a $$(target-$(tname)-bin) -o $$@ -i "assets/fx/icon-fx.png"
else
	mkg3a -n basic:"$3" -i uns:"assets/cg/icon-cg-uns.png" \
		-i sel:"assets/cg/icon-cg-sel.png" $$(target-$(tname)-bin) $$@
endif
endef




#---
# Generate all building rules for the "library" part
#---
# find sources files
target-lib-directory	:= $(shell find src -not -path "*/\.*" -type d)
target-lib-src 		:= $(foreach path,$(target-lib-directory), \
				$(wildcard $(path)/*.c) \
				$(wildcard $(path)/*.S) \
				$(wildcard $(path)/*.s))

# generate all library rules
target-lib-list		:=
$(foreach format,$(CONFIG.FORMAT),\
	$(foreach platform,$(CONFIG.PLATFORM),$(eval \
		$(call generate-target-library,$(format),$(platform),\
			$(target-lib-src),gintrace,target-lib-list) \
	))\
)

#---
# Generate all building rules for the "demo" part, if requested
#---
target-demo_list	:=
ifeq ($(CONFIG.DEMO),true)
# find source files
target-demo-directory	:= $(shell find demo -not -path "*/\.*" -type d)
target-demo-src 	:= $(foreach path,$(target-demo-directory), \
				$(wildcard $(path)/*.c) \
				$(wildcard $(path)/*.S) \
				$(wildcard $(path)/*.s))

# generate all demo rules
target-demo-list	:=
$(foreach platform,$(CONFIG.PLATFORM),$(eval \
	$(call generate-target-demo,$(platform),\
			$(target-demo-src),gintrace,target-demo-list) \
))
endif




#---
# Build rules
#---
all: $(target-lib-list) $(target-demo-list)

.PHONY: all




#---
# Generate installation rules (library only)
#---
# Common rules generated for the installation of each libraries.
# Basically, it will generate <libname>-install and <libname>-uninstall rules
# @note:
# *1 - library pathname
# *2 - variable name (installation rules list)
# *3 - variable name (uninstallation rules list)
define generate-install-rule
# Generate the installation rule
$(basename $(notdir $1))-install:
	install -d $(CONFIG.PREFIX)
	install $1 -m 644 $(CONFIG.PREFIX)

# Generate the uninstallation rule
$(basename $(notdir $1))-uninstall:
	rm -f $(CONFIG.PREFIX)$(notdir $1)

# Register generated rules into their appropriate list
$2	+= $(basename $(notdir $1))-install
$3	+= $(basename $(notdir $1))-uninstall
endef

# Generate all installation/uninstallation rules
target-install-rules	:=
target-uninstall-rules	:=
$(foreach target,$(target-lib-list),$(eval \
	$(call generate-install-rule,$(target),\
				target-install-rules,target-uninstall-rules) \
))

# Generate the path where include directory will be installed.
target-install-header-dir	:= $(CONFIG.PREFIX)include/
ifeq ($(wildcard $(target-install-header-dir)gintrace/.*),)
	target-install-header-dir	:= $(target-install-header-dir)gintrace
endif




#---
# Installation rules
#---
install: $(target-list) $(target-install-rules)
	cp -r include/gintrace/ $(target-install-header-dir)

uninstall: $(target-uninstall-rules)
	rm -rf $(CONFIG.PREFIX)include/gintrace




#---
# (internal) debug rule
#---
#target-list :=
#DEBUG=$(call generate-target-library,static,fx,src/gui/menu.c,gintrace,target-list)
##DEBUG=$(call generate-install-rule,/output/static/fxlibc.a)
##DEBUG=$(call generate-target-demo,fx,demo/main.c,gintrace,target-list)
##DEBUG=$(call generate-compilation-rule,demo/main.c,build/demo/fx,-Wall,sh-elf-gcc,target-list,demo)
#export DEBUG
#debug:
#	@ echo "$$DEBUG"
#	@ echo "target-lib: $(target-libs)"
#	@ echo "generated lib: $(lib-generation-rules)"
#	@ echo "target format: $(target-formats)"
#	@ echo "install-rules: $(lib-installation-rules)"
#	@ echo "uninstall-rules: $(lib-uninstallation-rules)"




#---
# cleaning rules
#---
clean:
	rm -rf build
fclean: clean
	rm -rf $(target-lib-list) $(target-demo-list)
re: fclean all

.PHONY: install uninstall clean fclean re all
