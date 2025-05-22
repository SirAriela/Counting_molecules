#include <poll.h>
#include <unistd.h>
#include <stdio.h>
#include <unistd.h>

#define EXIT_FAILURE 1

//struct that saves the number of atoms per element 
typedef struct wareHouse{
    unsigned long long carbon;
    unsigned long long hydrogen;
    unsigned long long oxygen;
}wareHouse;



void addAtom(int atom, int quantity, wareHouse *warehouse) {
    switch(atom) {
        case 1: // Carbon
            warehouse->carbon += quantity;
            break;
        case 2: // Hydrogen
            warehouse->hydrogen += quantity;
            break;
        case 3: // Oxygen
            warehouse->oxygen += quantity;
            break;
        default:
            printf("you dont know how to convert correctly my friend");
            break;
    }
}

int main(int argc, char *argv[]){
    wareHouse warehouse = {0};

    if(argc != 2){
        printf("incorrect number of arguments");
        return EXIT_FAILURE;
    }
    //number port
    int port = atoi(argv[1]);

    
}