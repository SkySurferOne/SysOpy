CC=gcc
CFLAGS=-I.
DEPS=addressbooklib.h
OBJ=test.o addressbooklib.o

test: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

# all: $(OBJ)
# 	$(CC) $(OBJ) -o test
#
# test.o: test.c $(DEPS)
# 	$(CC) test.c -c -o test.o
#
# addressbooklib.o: addressbooklib.c $(DEPS)
# 	$(CC) addressbooklib.c -c -o addressbooklib.o

clean:
	rm -f *.o *.a *.so test
