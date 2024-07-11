all:
	gcc ncurses.c -o pipeline -lpanel -lncurses
	./pipeline
