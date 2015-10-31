PROFILE = -O2 -s
CFLAGS = $(PROFILE)
LDFLAGS = -lsqlite3 -lconfig

all: main

# build it
main:
	$(CC) $(CFLAGS) load2sqlite.c $(LDFLAGS) -o load2sqlite

# clean it
clean:
	rm load2sqlite
