#include <arpa/inet.h>
#include <netinet/in.h>
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <unistd.h>

#define BACKLOG 10
#define MAX_CLIENTS 12

extern int optopt;
extern char *optarg;

// Global variable to control server shutdown
volatile sig_atomic_t running = 1;

void handle_sigint(int sig)
{
  running = 0;
  printf("\nSIGINT received — shutting down server gracefully... bli neder\n");
}

//-------------------atom functions---------------------------------------------
typedef struct wareHouse
{
  unsigned long long carbon;
  unsigned long long hydrogen;
  unsigned long long oxygen;
} wareHouse;

void addAtom(int atom, int quantity, wareHouse *warehouse)
{
  switch (atom)
  {
  case 1:
    warehouse->carbon += quantity;
    break;
  case 2:
    warehouse->hydrogen += quantity;
    break;
  case 3:
    warehouse->oxygen += quantity;
    break;
  default:
    printf("Unknown atom type\n");
    break;
  }
}

void printAtoms(wareHouse *warehouse)
{
  printf("Carbon: %llu\n", warehouse->carbon);
  printf("Hydrogen: %llu\n", warehouse->hydrogen);
  printf("Oxygen: %llu\n", warehouse->oxygen);
}

//------------------------------------------------------------------------

// ---------------molecule deliver functions-----------------------------

void numberOfAtomsNeeded(const char *molecule, int *carbon, int *oxygen,
                         int *hydrogen, int numberOfMoleculs)
{
  if (numberOfMoleculs > 0)
  {
    if (strcmp(molecule, "WATER") == 0)
    {
      *hydrogen = 2 * numberOfMoleculs;
      *oxygen = 1 * numberOfMoleculs;
      *carbon = 0 * numberOfMoleculs;
    }

    else if (strcmp(molecule, "CARBON DIOXIDE") == 0)
    {
      *carbon = 1 * numberOfMoleculs;
      *oxygen = 2 * numberOfMoleculs;
      *hydrogen = 0 * numberOfMoleculs;
    }

    else if (strcmp(molecule, "GLUCOSE") == 0)
    {
      *carbon = 6 * numberOfMoleculs;
      *hydrogen = 12 * numberOfMoleculs;
      *oxygen = 6 * numberOfMoleculs;
    }

    else if (strcmp(molecule, "ALCOHOL") == 0)
    {
      *carbon = 2 * numberOfMoleculs;
      *hydrogen = 6 * numberOfMoleculs;
      *oxygen = 1 * numberOfMoleculs;
    }

    else
    {
      *carbon = 0;
      *hydrogen = 0;
      *oxygen = 0;
    }
  }
}

int deliverMolecules(wareHouse *wareHouse, const char *molecule,
                     int numOfMolecules)
{
  int carbon, oxygen, hydrogen;
  numberOfAtomsNeeded(molecule, &carbon, &oxygen, &hydrogen, numOfMolecules);

  if (carbon == 0 && oxygen == 0 && hydrogen == 0)
  {
    printf("you tried to deliver unexisting molecule");
    return (0);
  }

  if (wareHouse->carbon < carbon || wareHouse->hydrogen < hydrogen ||
      wareHouse->oxygen < oxygen)
  {
    printf("there is not enough atoms to deliver %s\n", molecule);
    return 0;
  }

  wareHouse->carbon -= carbon;
  wareHouse->hydrogen -= hydrogen;
  wareHouse->oxygen -= oxygen;
  return 1;
}

//--------------------------------------------------------------------------
// --------------------------gen drinks -------------------------------------
// --------------------------------------------------------------------------

int genDrinks(wareHouse *wareHouse, const char *drinkToMake)
{
  int total_carbon = 0, total_oxygen = 0, total_hydrogen = 0;
  int carbon, oxygen, hydrogen;

  if (strcmp(drinkToMake, "VODKA") == 0)
  {
    numberOfAtomsNeeded("WATER", &carbon, &oxygen, &hydrogen, 1);
    total_carbon += carbon;
    total_hydrogen += hydrogen;
    total_oxygen += oxygen;
    numberOfAtomsNeeded("ALCOHOL", &carbon, &oxygen, &hydrogen, 1);
    total_carbon += carbon;
    total_hydrogen += hydrogen;
    total_oxygen += oxygen;
    numberOfAtomsNeeded("GLUCOSE", &carbon, &oxygen, &hydrogen, 1);
    total_carbon += carbon;
    total_hydrogen += hydrogen;
    total_oxygen += oxygen;
  }

  if (strcmp(drinkToMake, "CHAMPAGNE") == 0)
  {
    numberOfAtomsNeeded("WATER", &carbon, &oxygen, &hydrogen, 1);
    total_carbon += carbon;
    total_hydrogen += hydrogen;
    total_oxygen += oxygen;
    numberOfAtomsNeeded("ALCOHOL", &carbon, &oxygen, &hydrogen, 1);
    total_carbon += carbon;
    total_hydrogen += hydrogen;
    total_oxygen += oxygen;
    numberOfAtomsNeeded("CARBON DIODXIDE", &carbon, &oxygen, &hydrogen, 1);
    total_carbon += carbon;
    total_hydrogen += hydrogen;
    total_oxygen += oxygen;
  }

  if (strcmp(drinkToMake, "SOFT DRINK") == 0)
  {
    numberOfAtomsNeeded("WATER", &carbon, &oxygen, &hydrogen, 1);
    total_carbon += carbon;
    total_hydrogen += hydrogen;
    total_oxygen += oxygen;
    numberOfAtomsNeeded("GLUCOSE", &carbon, &oxygen, &hydrogen, 1);
    total_carbon += carbon;
    total_hydrogen += hydrogen;
    total_oxygen += oxygen;
    numberOfAtomsNeeded("CARBON DIODXIDE", &carbon, &oxygen, &hydrogen, 1);
    total_carbon += carbon;
    total_hydrogen += hydrogen;
    total_oxygen += oxygen;
  }

  if (wareHouse->carbon < total_carbon ||
      wareHouse->hydrogen < total_hydrogen ||
      wareHouse->oxygen < total_oxygen)
  {
    printf("there is not enough atoms to deliver %s\n", drinkToMake);
    return 0;
  }

  wareHouse->carbon -= total_carbon;
  wareHouse->hydrogen -= total_hydrogen;
  wareHouse->oxygen -= total_oxygen;
  return 1;
}

int min(int a, int b, int c)
{
  if (a <= b && a <= c)
    return a;
  if (b <= a && b <= c)
    return b;
  return c;
}

void howManyDrinks(wareHouse *wareHouse, const char *drinkToMake)
{
  int total_carbon = 0, total_oxygen = 0, total_hydrogen = 0;
  int carbon, oxygen, hydrogen;
  unsigned long long CounerDrinksCarbon, CounerDrinksOxygen, CounerDrinksHydrogen;

  if (strcmp(drinkToMake, "VODKA") == 0)
  {
    numberOfAtomsNeeded("WATER", &carbon, &oxygen, &hydrogen, 1);
    total_carbon += carbon;
    total_hydrogen += hydrogen;
    total_oxygen += oxygen;
    numberOfAtomsNeeded("ALCOHOL", &carbon, &oxygen, &hydrogen, 1);
    total_carbon += carbon;
    total_hydrogen += hydrogen;
    total_oxygen += oxygen;
    numberOfAtomsNeeded("GLUCOSE", &carbon, &oxygen, &hydrogen, 1);
    total_carbon += carbon;
    total_hydrogen += hydrogen;
    total_oxygen += oxygen;
  }

  if (strcmp(drinkToMake, "CHAMPAGNE") == 0)
  {
    numberOfAtomsNeeded("WATER", &carbon, &oxygen, &hydrogen, 1);
    total_carbon += carbon;
    total_hydrogen += hydrogen;
    total_oxygen += oxygen;
    numberOfAtomsNeeded("ALCOHOL", &carbon, &oxygen, &hydrogen, 1);
    total_carbon += carbon;
    total_hydrogen += hydrogen;
    total_oxygen += oxygen;
    numberOfAtomsNeeded("CARBON DIODXIDE", &carbon, &oxygen, &hydrogen, 1);
    total_carbon += carbon;
    total_hydrogen += hydrogen;
    total_oxygen += oxygen;
  }

  if (strcmp(drinkToMake, "SOFT DRINK") == 0)
  {
    numberOfAtomsNeeded("WATER", &carbon, &oxygen, &hydrogen, 1);
    total_carbon += carbon;
    total_hydrogen += hydrogen;
    total_oxygen += oxygen;
    numberOfAtomsNeeded("GLUCOSE", &carbon, &oxygen, &hydrogen, 1);
    total_carbon += carbon;
    total_hydrogen += hydrogen;
    total_oxygen += oxygen;
    numberOfAtomsNeeded("CARBON DIODXIDE", &carbon, &oxygen, &hydrogen, 1);
    total_carbon += carbon;
    total_hydrogen += hydrogen;
    total_oxygen += oxygen;
  }

  CounerDrinksCarbon = wareHouse->carbon / total_carbon;
  CounerDrinksOxygen = wareHouse->oxygen / total_oxygen;
  CounerDrinksHydrogen = wareHouse->hydrogen / total_hydrogen;
  int minimum = min(CounerDrinksCarbon, CounerDrinksHydrogen, CounerDrinksOxygen);

  printf("number of %s drinks can make %d\n",drinkToMake,minimum);
}

//----------------------------------------------------------------------------------------

int main(int argc, char *argv[])
{

  if (argc != 3)
  {
    fprintf(stderr, "Usage: %s  <tcp_port> <udp_port>\n", argv[0]);
    return 1;
  }

  signal(SIGINT, handle_sigint);

  wareHouse warehouse = {0};
  const char *atoms[] = {"CARBON", "HYDROGEN", "OXYGEN"};
  int tcp_port = atoi(argv[1]);
  int udp_port = atoi(argv[2]);

  if (tcp_port <= 0 || tcp_port > 65535 || udp_port <= 0 || udp_port > 65535)
  {
    fprintf(stderr, "Invalid port number: %d or %d\n", tcp_port, udp_port);
    return 1;
  }
  //--------------------tcp socket setup-------------------------------

  // Create a listening socket
  int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (listen_fd < 0)
  {
    perror("socket");
    return 1;
  }

  // Set socket options to allow reuse of the address
  int opt = 1;
  setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

  struct sockaddr_in serv_addr = {.sin_family = AF_INET,
                                  .sin_port = htons(tcp_port),
                                  .sin_addr.s_addr = INADDR_ANY};

  if (bind(listen_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
  {
    perror("bind");
    close(listen_fd);
    return 1;
  }

  if (listen(listen_fd, BACKLOG) < 0)
  {
    perror("listen");
    close(listen_fd);
    return 1;
  }

  //---------------------------------------------------------------------------------------

  // -------------------udp socket setup-------------------------------
  int udp_fd = socket(AF_INET, SOCK_DGRAM, 0);

  if (udp_fd < 0)
  {
    perror("socket");
    close(udp_fd);
    return 1;
  }
  struct sockaddr_in udp_addr = {.sin_family = AF_INET,
                                 .sin_port = htons(udp_port),
                                 .sin_addr.s_addr = INADDR_ANY};

  if (bind(udp_fd, (struct sockaddr *)&udp_addr, sizeof(udp_addr)) < 0)
  {
    perror("UDP bind");
    close(listen_fd);
    close(udp_fd);
    return 1;
  }

  struct sockaddr_in client_addr;
  socklen_t addr_len = sizeof(client_addr);

  // ---------------- fds setup for poll ------------------------
  struct pollfd fds[MAX_CLIENTS];
  int nfds = 3;
  fds[0].fd = listen_fd;
  fds[0].events = POLLIN;
  fds[1].fd = udp_fd;
  fds[1].events = POLLIN;
  fds[2].fd = STDIN_FILENO;
  fds[2].events = POLLIN;

  printf("Server running on ports %d ,%d...\n", tcp_port, udp_port);

  while (running)
  {
    int ready = poll(fds, nfds, 1000);
    if (ready < 0)
    {
      if (!running)
        break;
      perror("poll");
      break;
    }

    if (ready == 0)
      continue; // timeout, no events

    // tcp
    //  Handle new connections first
    if (fds[0].revents & POLLIN)
    {
      int client_fd = accept(listen_fd, NULL, NULL);
      if (client_fd >= 0 && nfds < MAX_CLIENTS)
      {
        fds[nfds].fd = client_fd;
        fds[nfds].events = POLLIN;
        fds[nfds].revents = 0; // Clear revents
        nfds++;
        printf("New client connected: fd=%d\n", client_fd);
      }
      else if (client_fd >= 0)
      {
        printf("Max clients reached, rejecting connection\n");
        close(client_fd);
      }
    }

    if (fds[1].revents & POLLIN)
    {
      char buffer[256];
      ssize_t len = recvfrom(udp_fd, buffer, sizeof(buffer) - 1, 0,
                             (struct sockaddr *)&client_addr, &addr_len);
      if (len > 0)
      {
        buffer[len] = '\0';
        // Remove newline if present
        char *newline = strchr(buffer, '\n');
        if (newline)
          *newline = '\0';

        char molecule[32];
        char word1[16], word2[16];
        int quantity = 0;
        char response[256];

        if (sscanf(buffer, "DELIVER %15s %d", molecule, &quantity) == 2 &&
            quantity > 0)
        {
          int status = deliverMolecules(&warehouse, molecule, quantity);

          if (status)
          {
            printf("Delivered molecule %s\n", molecule);
            printf("currently in ware house there: \n");
            printAtoms(&warehouse);
            snprintf(response, sizeof(response), "OK: Delivered %s", molecule);
          }
          else
          {
            printAtoms(&warehouse);
            snprintf(response, sizeof(response), "did not deliver %s, sorry.",
                     molecule);
          }
          // sendto(udp_fd, response, strlen(response), 0, (struct sockaddr
          // *)&client_addr, addr_len);
        }
        else if (sscanf(buffer, "DELIVER %15s %15s %d", word1, word2,
                        &quantity) == 3 &&
                 quantity > 0)
        {
          snprintf(molecule, sizeof(molecule), "%s %s", word1, word2);
          int status = deliverMolecules(&warehouse, molecule, quantity);

          if (status)
          {
            printf("Delivered molecule %s\n", molecule);
            printf("currently in ware house there: \n");
            printAtoms(&warehouse);
            snprintf(response, sizeof(response), "OK: Delivered %s", molecule);
          }
          else
          {
            printAtoms(&warehouse);
            snprintf(response, sizeof(response), "did not deliver %s, sorry.",
                     molecule);
          }
        }
        sendto(udp_fd, response, strlen(response), 0,
               (struct sockaddr *)&client_addr, addr_len);
      }
    }
    // Handle client data - process from end to beginning to avoid index issues
    for (int i = nfds - 1; i >= 3; i--)
    {
      if (fds[i].revents & POLLIN)
      {
        char buffer[256];
        ssize_t len = read(fds[i].fd, buffer, sizeof(buffer) - 1);

        if (len <= 0)
        {
          printf("Client disconnected: fd=%d\n", fds[i].fd);
          close(fds[i].fd);
          // Move last element to current position
          if (i < nfds - 1)
          {
            fds[i] = fds[nfds - 1];
          }
          nfds--;
        }
        else
        {
          buffer[len] = '\0';
          // Remove newline if present
          char *newline = strchr(buffer, '\n');
          if (newline)
            *newline = '\0';

          char atom[16];
          int quantity = 0;
          if (sscanf(buffer, "ADD %15s %d", atom, &quantity) == 2 &&
              quantity > 0)
          {
            int index_atom = -1;
            for (int j = 0; j < 3; j++)
            {
              if (strcmp(atom, atoms[j]) == 0)
              {
                index_atom = j + 1;
                break;
              }
            }
            if (index_atom > 0)
            {
              addAtom(index_atom, quantity, &warehouse);
              printf("Added %d %s\n", quantity, atom);
              printAtoms(&warehouse);
            }
            else
            {
              printf("Error: Unknown atom type '%s'\n", atom);
            }
          }
        }
      }
    }

    if (fds[2].revents & POLLIN)
    {
      char buffer[256];
      char drink[64];
      int status;

      if (fgets(buffer, sizeof(buffer), stdin) != NULL)
      {
        char *newline = strchr(buffer, '\n');
        if (newline)
          *newline = '\0';

        if (strncmp(buffer, "GEN ", 4) == 0)
        {

          strncpy(drink, buffer + 4, sizeof(drink) - 1);
          drink[sizeof(drink) - 1] = '\0';

          howManyDrinks(&warehouse,drink);
          printf("---------------------------------------\n");
          status = genDrinks(&warehouse, drink);
          if (status)
          {
            printf("Generated drink %s\n", drink);
            printf("------------------------------\n");
            printAtoms(&warehouse);
            
          }
          else
          {
            printf("Sorry man, couldn't generate %s\n", drink);
          }
        }
        else
        {
          printf("Invalid command. Use: GEN <drink_name>\n");
          printf("Available drinks: VODKA, CHAMPAGNE, SOFT DRINK\n");
        }
      }
    }

    // Clear all revents for next iteration
    for (int i = 0; i < nfds; i++)
    {
      fds[i].revents = 0;
    }
  }
  // here only if running is false - signal CTRL C
  printf("Shutting down server...\n");
  for (int i = 1; i < nfds; i++)
  {
    close(fds[i].fd);
  }

  close(listen_fd);
  printf("Server terminated.\n");
  return 0;
}