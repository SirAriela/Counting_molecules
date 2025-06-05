CC = gcc
CFLAGS=-Wall -fprofile-arcs -ftest-coverage
LDFLAGS=-lgcov

all: atom_supplier drinks_bar molecule_requestor

atom_supplier: atom_supplier.o
	$(CC) $(CFLAGS) -o atom_supplier atom_supplier.o
supplier.o: atom_supplier.c
	$(CC) $(CFLAGS) -c atom_supplier.c

drinks_bar: drinks_bar.o
	$(CC) $(CFLAGS) -o drinks_bar drinks_bar.o
drinks_bar.o: drinks_bar.c
	$(CC) $(CFLAGS) -c drinks_bar.c

molecule_requestor: molecule_requestor.o
	$(CC) $(CFLAGS) -o molecule_requestor molecule_requestor.o
molecule_requestor.o: molecule_requestor.c
	$(CC) $(CFLAGS) -c molecule_requestor.c

clean:
	rm -f atom_supplier drinks_bar molecule_requestor *.o *.gcda *.gcno *.gcov

.PHONY: all clean test coverage

