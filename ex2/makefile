CC = gcc
CFLAGS=-Wall -fprofile-arcs -ftest-coverage
LDFLAGS=-lgcov

all: supplier warehouse molecule_requestor

supplier: supplier.o
	$(CC) $(CFLAGS) -o supplier atom_supplier.o
supplier.o: atom_supplier.c
	$(CC) $(CFLAGS) -c atom_supplier.c

warehouse: molecule_supplier.o
	$(CC) $(CFLAGS) -o warehouse molecule_supplier.o
warehouse.o: molecule_supplier.c
	$(CC) $(CFLAGS) -c molecule_supplier.c

molecule_requestor: molecule_requestor.o
	$(CC) $(CFLAGS) -o molecule_requestor molecule_requestor.o
molecule_requestor.o: molecule_requestor.c
	$(CC) $(CFLAGS) -c molecule_requestor.c

clean:
	rm -f supplier warehouse molecule_requestor *.o *.gcda *.gcno *.gcov

.PHONY: all clean test coverage

