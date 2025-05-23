CC = gcc
CFLAGS = -Wall -g

all: supplier warehouse

supplier: supplier.o
	$(CC) $(CFLAGS) -o supplier atom_supplier.o
supplier.o: atom_supplier.c
	$(CC) $(CFLAGS) -c atom_supplier.c
warehouse: atom_warehouse.o
	$(CC) $(CFLAGS) -o warehouse atom_warehouse.o
warehouse.o: atom_warehouse.c
	$(CC) $(CFLAGS) -c atom_warehouse.c
clean:
	rm -f supplier warehouse *.o
