CFLAGS=-Wall -Wextra
LIBS=

multiballs: main.c
	$(CC) $(CFLAGS) -o lamb main.c $(LIBS)
