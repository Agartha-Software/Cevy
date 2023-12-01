##
## Project : Cevy
## File : Makefile
##

all:	build

build:
	cmake -S . -B ./build
	make -j --no-print-directory -C build

test:
	cmake -DTESTS=on -S . -B ./build
	make -j --no-print-directory -C build tests-run-cevy

.PHONY: all build test
