CC = gcc
CFLAGS = -Wall -fprofile-arcs -ftest-coverage
LDFLAGS = -lgcov

all: supplier warehouse

supplier: atom_supplier.o
	$(CC) $(CFLAGS) -o supplier atom_supplier.o $(LDFLAGS)

atom_supplier.o: atom_supplier.c
	$(CC) $(CFLAGS) -c atom_supplier.c

warehouse: atom_warehouse.o
	$(CC) $(CFLAGS) -o warehouse atom_warehouse.o $(LDFLAGS)

atom_warehouse.o: atom_warehouse.c
	$(CC) $(CFLAGS) -c atom_warehouse.c

coverage:
	make clean
	make all
	chmod +x coverage_test.sh
	./coverage_test.sh

clean:
	rm -f supplier warehouse *.o *.gcda *.gcno *.gcov
	
.PHONY: all coverage clean