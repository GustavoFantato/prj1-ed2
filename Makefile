all:
	gcc src/programaTrab.c modules/functions.c modules/utils.c -o programaTrab.exe -lmd

run:
	./programaTrab.exe