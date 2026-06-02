build:	isa-top.c printer.c hashtable.c help.c arg_parse.c
	gcc -Wall isa-top.c printer.c hashtable.c help.c arg_parse.c -lpcap -lncurses -lpthread -o isa-top

clean:
	rm isa-top

pack: clean
	tar --exclude='.vscode' --exclude='.git' --exclude='.gitignore' -cf xmikul74.tar *