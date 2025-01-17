#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h> // getopt(), access(), read(), close()
#include <arpa/inet.h>
#include <time.h> // time()

#define MAX_IP 64
#define MAX_PATH 1024
#define MAX_REQUEST 4096

int opt_I, _I, // client id
    opt_A,
    opt_P, _P, // Input port no
    opt_O;

char _O[MAX_PATH]; // Path to query file
char _A[MAX_IP];   // IP address

/* -------------------MACROS--------------------*/

// Exitting macro with a message
#define errExit(msg)    \
  do                    \
  {                     \
    perror(msg);        \
    exit(EXIT_FAILURE); \
  } while (0)
// Logging macro to add timestamp before prints
#define print_log(f_, ...) printf("%s ", timestamp()), printf((f_), ##__VA_ARGS__), printf("\n")

/* -------------------PROTOTYPES--------------------*/

void make_request(int server_socket, int *src, int *dst);

// Misc functions
void print_usage(void);
char *timestamp(void);
void exit_on_invalid_input(void);

int main(int argc, char *argv[])
{
  char _A[MAX_IP];
  int server_socket, option;
  struct sockaddr_in addr_server = {0};

  /* -------------Parse command line input ---------------*/
  while ((option = getopt(argc, argv, "i:a:p:o:")) != -1)
  {
    switch (option)
    {
    case 'i':
      opt_I = 1;
      _I = atoi(optarg);
      break;
    case 'a':
      opt_A = 1;
      snprintf(_A, MAX_PATH, "%s", optarg);
      break;
    case 'p':
      opt_P = 1;
      _P = atoi(optarg);
      break;
    case 'o':
      opt_O = 1;
      snprintf(_O, MAX_PATH, "%s", optarg);
      break;
    default:
      print_usage();
      exit(EXIT_FAILURE);
      break;
    }
  }

  exit_on_invalid_input();

  // socket create and varification
  server_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (server_socket == -1)
    errExit("Socket failed");

  // assign IP, PORT
  addr_server.sin_family = AF_INET;
  addr_server.sin_addr.s_addr = inet_addr(_A);
  addr_server.sin_port = htons(_P);

  // connect the client socket to server socket
  print_log("Client-%d connecting to %s:%d", _I, _A, _P);
  if (connect(server_socket, (struct sockaddr *)&addr_server, sizeof(addr_server)) != 0)
    errExit("Connect failed");

  print_log("Client (%d) connected", _I);

  FILE *fp = fopen(_O, "r");
  char line[MAX_REQUEST];

  int total_queries = 0;
  setbuf(stdout, NULL);
  setbuf(stderr, NULL);

  while (fgets(line, MAX_REQUEST, fp))
  {
    int i;
    char buffer[MAX_REQUEST];
    for (int k = 0; k < MAX_REQUEST; k++)
      buffer[k] = '\0';

    sscanf(line, "%d", &i);
    line[strlen(line) - 1] = '\0';
    if (i == _I)
    {
      total_queries++;
      int n_records = 0;
      print_log("Client-%d sending query  '%s'", _I, line);

      clock_t t = clock();
      write(server_socket, line, strlen(line)+1 );
      bzero(line, MAX_REQUEST);
      printf("==================================================");
      while (read(server_socket, buffer, sizeof(buffer)))
      {
        n_records++;
        if (buffer[0] == '^')
        {
          printf("\n");
          t = clock() - t;
          double time_taken = ((double)t) / CLOCKS_PER_SEC;
          print_log("Server’s response to Client-%d is %d records, and arrived in %f seconds.", _I, n_records-2, time_taken);
          printf("==================================================\n");
          break;
        }

        printf("%s", buffer);
        bzero(buffer, MAX_REQUEST);

        char c = '?';
        write(server_socket, &c, sizeof(char));
        //printf("wrote c\n");
      }
    }
  }
  print_log("A total of %d queries were executed, client is terminating.", total_queries);
  shutdown(server_socket, SHUT_RDWR);
  close(server_socket);
  //make_request(server_socket, &_s, &_d);
  fclose(fp);
  exit(EXIT_SUCCESS);
}

void print_usage(void)
{
  printf("\n========================================\n");
  printf("Usage:\n"
         "./client [-i id] [-a IPv4 address] [-p PORT]  [-o path to query file]\n");
  printf("========================================\n");
}

char *timestamp()
{
  time_t now = time(NULL);
  char *time = asctime(gmtime(&now));
  time[strlen(time) - 1] = '\0'; // Remove \n
  return time;
}

void exit_on_invalid_input(void)
{
  if (_I < 1)
  {
    printf("Client ID must be larger then 0.\n");
    print_usage();
    exit(EXIT_FAILURE);
  }
  if (_P <= 1000)
  {
    printf("Port number should be larger then 1000.\n");
    print_usage();
    exit(EXIT_FAILURE);
  }
  if (!(opt_I && opt_A && opt_P && opt_O))
  {
    printf("Missing parameters\n");
    print_usage();
    exit(EXIT_FAILURE);
  }
}