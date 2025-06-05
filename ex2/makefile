CC = gcc
CFLAGS=-Wall -fprofile-arcs -ftest-coverage
LDFLAGS=-lgcov

all: supplier warehouse molecule_requestor

supplier: supplier.o
	$(CC) $(CFLAGS) -o atom_supplier atom_supplier.o
supplier.o: atom_supplier.c
	$(CC) $(CFLAGS) -c atom_supplier.c

warehouse: molecule_supplier.o
	$(CC) $(CFLAGS) -o molecule_supplier molecule_supplier.o
warehouse.o: molecule_supplier.c
	$(CC) $(CFLAGS) -c molecule_supplier.c

molecule_requestor: molecule_requestor.o
	$(CC) $(CFLAGS) -o molecule_requestor molecule_requestor.o
molecule_requestor.o: molecule_requestor.c
	$(CC) $(CFLAGS) -c molecule_requestor.c

coverage:
	make clean
	make all
	chmod +x coverage_test.sh
	./coverage_test.sh


clean:
	rm -f atom_supplier molecule_supplier molecule_requestor *.o *.gcda *.gcno *.gcov

.PHONY: all clean test coverage

