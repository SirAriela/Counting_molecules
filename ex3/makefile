CC = gcc
CFLAGS=-Wall -fprofile-arcs -ftest-coverage
LDFLAGS=-lgcov

all: supplier drinks_bar molecule_requestor

supplier: supplier.o
	$(CC) $(CFLAGS) -o supplier atom_supplier.o
supplier.o: atom_supplier.c
	$(CC) $(CFLAGS) -c atom_supplier.c

drinks_bar: molecule_supplier.o
	$(CC) $(CFLAGS) -o drinks_bar molecule_supplier.o
drinks_bar.o: molecule_supplier.c
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
	rm -f supplier drinks_bar molecule_requestor *.o *.gcda *.gcno *.gcov

.PHONY: all clean test coverage

