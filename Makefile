VERSION = 0
PATCHLEVEL = 0
SUBLEVEL = 0
NAME = Fearless Coyote

all:	poem

poem: poem.o
	gcc poem.o -o poem

poem.o : poem.c poem.h
	gcc -g -O0 -c poem.c -o poem.o

config:
	scripts/kconfig/conf --oldaskconfig $(silent) Kconfig

menuconfig:
	scripts/kconfig/mconf $(silent) Kconfig

test: poem
	cp poem t/
	@echo 'Running testcases...'
	@echo 'test finished.'

help:
	@echo 'poem		- Update the poem executable'
	@echo 'config		- Update current config via a line-oriented program'
	@echo 'menuconfig	- Update current config via a menu based program'
	@echo 'test		- Run the testcases for poem'
	@echo 'help		- Provide help info as you can see'
	@echo ''

-include .config
-include config-option.table

profile:
	@echo $(obj-y) > .poem.pro

.PNONY: clean
clean:
	-rm -f poem *.o
