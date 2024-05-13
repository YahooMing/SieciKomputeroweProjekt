#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>

#define MAX_CLIENTS 10
#define BUFFER_SIZE 4096
#define MAX_DATA 100000

struct CALCDATA{
  uint32_t data[MAX_DATA];
};


int create_data(int idx, struct CALCDATA *cdata){
    if(cdata !=NULL){
      uint32_t i;
      uint32_t *value;
      uint32_t v;
      value = &cdata->data[0];

      for(i=0;i<MAX_DATA;i++){
        v = (uint32_t) rand() ^ (uint32_t) rand();

        printf("Creating value #%d v=%u addr=%lu \r",i,v,(unsigned long)value);
        *value = v & 0x0000FFFF;
        value++;
      }
      return(1);
    }
    return(0);
}

int create_package(char* package, char* name, char option){
  int nlen=strlen(name);
  if (nlen>8){
    return -1;
  }       
  package[0]='@';
  package[nlen+1]='0';
  package[nlen+2]='!';
  package[nlen+3]=option;
  package[nlen+4]=':';
  package[nlen+5]='#';
  switch(option){
    case 'N':
      for(int i=0;i<nlen;i++){
        package[i+1]=name[i];
      };
      break;
  }
}


//https://www.geeksforgeeks.org/socket-programming-cc/
//https://www.youtube.com/watch?v=Y6pFtgRdUts --obejrzeć jeszcze raz





int main(int argc, char *argv[]) {
  //Jeśli liczba argumentów wejściowych różni się od 2 to wywala błąd
  if (argc != 2) {
    printf("Usage: %s <port>\n", argv[0]);
    exit(EXIT_FAILURE);
  }





  //atoi konwertuje string na inta , w tym przypadku drugi argument który wpisujemy na wejściu
  int server_port = atoi(argv[1]);




  /*    struct sockaddr_in server_addr = {...}; - Ta linia deklaruje zmienną server_addr o typie struct sockaddr_in, która jest używana do przechowywania informacji o adresie IPv4 i numerze portu. Inicjalizacja zagnieżdżoną listą inicjalizacyjną umożliwia jednoczesne ustawienie wszystkich pól struktury.

    .sin_family = AF_INET - Pole sin_family określa rodzinę adresów, w tym przypadku AF_INET, co oznacza, że używany jest adres IPv4.

    .sin_port = htons(server_port) - Pole sin_port przechowuje numer portu, na którym serwer będzie nasłuchiwał na połączenia przychodzące. Funkcja htons() konwertuje wartość portu server_port na odpowiednią formę używaną w sieci, uwzględniając kolejność bajtów (host byte order vs. network byte order).

    .sin_addr.s_addr = INADDR_ANY - Pole sin_addr.s_addr przechowuje adres IP serwera. Wartość INADDR_ANY oznacza, że serwer będzie nasłuchiwał na wszystkich interfejsach sieciowych urządzenia, a nie tylko na jednym konkretnym adresie IP. Innymi słowy, serwer będzie akceptować połączenia przychodzące na dowolny adres IP tego urządzenia.

Podsumowując, ten kod inicjalizuje strukturę sockaddr_in z danymi potrzebnymi do nasłuchiwania na połączenia przychodzące na określonym porcie (określonym przez server_port) na wszystkich interfejsach sieciowych urządzenia. Jest to typowa procedura używana w programowaniu sieciowym w języku C, szczególnie przy tworzeniu serwerów sieciowych.*/
  struct sockaddr_in server_addr = {.sin_family = AF_INET,
                                    .sin_port = htons(server_port),
                                    .sin_addr.s_addr = INADDR_ANY};




  /*int sockfd = socket(domain, type, protocol)

    sockfd: socket descriptor, an integer (like a file handle)
    domain: integer, specifies communication domain. We use AF_ LOCAL as defined in the POSIX standard for communication between processes on the same host. For communicating between processes on different hosts connected by IPV4, we use AF_INET and AF_I NET 6 for processes connected by IPV6.
    type: communication type
    SOCK_STREAM: TCP(reliable, connection-oriented)
    SOCK_DGRAM: UDP(unreliable, connectionless)
    protocol: Protocol value for Internet Protocol(IP), which is 0. This is the same number that appears on the protocol field in the IP header of a packet.(man protocols for more details)
*/
  int sd = socket(AF_INET, SOCK_STREAM, 0);
  if (sd < 0) {
    perror("socket() error");
    exit(EXIT_FAILURE);
  }


  /*This helps in manipulating options for the socket referred by the file descriptor sockfd. This is completely optional, but it helps in reuse of address and port. Prevents error such as: “address already in use”.
  int setsockopt(int sockfd, int level, int optname,  const void *optval, socklen_t optlen);*/
  int value1 = 1;
  setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &value1, sizeof(value1));


  /*int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
  After the creation of the socket, the bind function binds the socket to the address and port number specified in addr(custom data structure). In the example code, we bind the server to the localhost, hence we use INADDR_ANY to specify the IP address.*/
  if (bind(sd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
    perror("bind() error");
    exit(EXIT_FAILURE);
  }

  /*int listen(int sockfd, int backlog);
  It puts the server socket in a passive mode, where it waits for the client to approach the server to make a connection. The backlog, defines the maximum length to which the queue of pending connections for sockfd may grow. If a connection request arrives when the queue is full, the client may receive an error with an indication of ECONNREFUSED.*/
  if (listen(sd, MAX_CLIENTS) < 0) {
    perror("listen() error");
    exit(EXIT_FAILURE);
  }

  //Powiadomienie dla użytkownika że jesteśmy w trakcie nasłuchiwania
  printf("Server listening on port %d\n", server_port);


  /*Ten fragment kodu również jest związany z programowaniem sieciowym w języku C. Przedstawia deklarację zmiennych i struktur, które są często używane do zarządzania połączeniami sieciowymi w systemach opartych na gniazdach. Przeanalizujmy każdą linię:

    fd_set readfds; - Ta linia deklaruje zmienną readfds o typie fd_set, która jest często używana w kontekście monitorowania gniazd (socketów) pod kątem czytania. Struktura fd_set jest używana wraz z funkcjami takimi jak select() lub poll() do wykrywania dostępności danych do odczytu na różnych gniazdach.

    int max_sd, activity, new_socket; - Tutaj deklarowane są trzy zmienne typu int.
        max_sd - Zazwyczaj jest używana do przechowywania największego deskryptora (numeru gniazda), który jest monitorowany przez select().
        activity - Ta zmienna często jest używana do przechowywania wyniku wywołania funkcji monitorującej aktywność na gniazdach, np. select().
        new_socket - Jest to zmienna, która będzie przechowywać deskryptor (numer) nowego gniazda, które zostanie zaakceptowane przez serwer.

    struct sockaddr_in client_addr; - Ta linia deklaruje strukturę sockaddr_in o nazwie client_addr, która będzie przechowywać informacje o adresie klienta, który nawiąże połączenie z serwerem.

    socklen_t client_len; - To zmienna typu socklen_t będzie przechowywać długość struktury adresowej klienta. Zazwyczaj jest to potrzebne do funkcji, takich jak accept(), aby wiedzieć, jak dużo miejsca przygotować na strukturę adresową klienta.

    int client_sockets[MAX_CLIENTS] = {0}; - To deklaruje tablicę client_sockets o rozmiarze MAX_CLIENTS, w której będą przechowywane deskryptory gniazd klientów. Wartości domyślne są ustawione na 0. Ta tablica często jest używana w serwerach do śledzenia połączonych klientów.

  Podsumowując, ten fragment kodu przygotowuje zmienne i struktury potrzebne do zarządzania połączeniami sieciowymi, w tym monitorowanie gniazd pod kątem aktywności oraz przechowywanie informacji o klientach i ich gniazdach. Jest to typowa praktyka stosowana w programowaniu serwerów sieciowych w języku C.
  */
  fd_set readfds;
  int max_sd, activity, new_socket;
  struct sockaddr_in client_addr;
  socklen_t client_len;
  int client_sockets[MAX_CLIENTS] = {0};



  /*Ten fragment kodu przedstawia typowy szkielet serwera TCP w języku C, który używa funkcji select() do wielokrotnego zarządzania połączeniami sieciowymi. Pozwala to serwerowi na obsługę wielu klientów jednocześnie, używając tylko jednego wątku.

Oto dokładne wyjaśnienie każdej części kodu:

    Pętla Nieskończona: Pętla while (1) jest pętlą nieskończoną, co oznacza, że serwer będzie działał w pętli i obsługiwał połączenia klientów, dopóki nie zostanie ręcznie zatrzymany.

    Czyszczenie Zestawu fd_set: FD_ZERO(&readfds) czyszczenie zestawu readfds, który będzie używany przez funkcję select() do monitorowania połączeń.

    Dodawanie Głównego Gniazda do Zestawu: FD_SET(sd, &readfds) dodaje główne gniazdo (sd), na którym serwer nasłuchuje nowych połączeń, do zestawu readfds.

    Ustawianie Maksymalnego Deskryptora: max_sd = sd inicjalizuje max_sd na wartość sd, czyli deskryptor głównego gniazda.

    Pętla po Połączeniach Klientów: Pętla for przechodzi przez tablicę client_sockets i dodaje każde aktywne gniazdo klienta do zestawu readfds. Ponadto aktualizuje wartość max_sd, jeśli deskryptor klienta jest większy niż obecnie ustawione max_sd.

    Wywołanie Funkcji select(): activity = select(max_sd + 1, &readfds, NULL, NULL, NULL) oczekuje na aktywność na dowolnym z gniazd w zestawie readfds. Funkcja ta blokuje wykonanie programu, dopóki nie pojawi się aktywność lub nie zostanie przerwana sygnałem.

    Obsługa Nowego Połączenia: Jeśli jest aktywność na głównym gnieździe (sd), oznacza to nowe połączenie klienta. Serwer zaakceptuje to połączenie za pomocą funkcji accept() i zaktualizuje tablicę client_sockets oraz wyświetli komunikat o nowym połączeniu.

    Obsługa Połączeń Klientów: Następnie, jeśli jest aktywność na którymkolwiek z gniazd klienta w zestawie readfds, serwer odbierze dane od klienta za pomocą funkcji recv(). Jeśli dane zostaną odebrane poprawnie, serwer wyświetli je i wyśle odpowiedź do klienta.

    Zakończenie Połączenia: Jeśli recv() zwróci wartość mniejszą lub równą 0, oznacza to zakończenie połączenia przez klienta. W takim przypadku serwer zamyka gniazdo klienta za pomocą funkcji close() i usuwa go z tablicy client_sockets.*/
  while (1) {
    FD_ZERO(&readfds);
    FD_SET(sd, &readfds);
    max_sd = sd;

    for (int i = 0; i < MAX_CLIENTS; ++i) {
      int client_socket = client_sockets[i];
      if (client_socket > 0) {
        FD_SET(client_socket, &readfds);
        if (client_socket > max_sd) {
          max_sd = client_socket;
        }
      }
    }

    activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);
    if (activity < 0) {
      perror("select() error");
      exit(EXIT_FAILURE);
    }

    if (FD_ISSET(sd, &readfds)) {
      client_len = sizeof(client_addr);
      new_socket = accept(sd, (struct sockaddr *)&client_addr, &client_len);
      if (new_socket < 0) {
        perror("accept() error");
        exit(EXIT_FAILURE);
      }

      printf("New connection, socket fd is %d\n", new_socket);

      for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (client_sockets[i] == 0) {
          client_sockets[i] = new_socket;
          break;
        }
      }
    }

    for (int i = 0; i < MAX_CLIENTS; ++i) {
      int client_socket = client_sockets[i];
      if (FD_ISSET(client_socket, &readfds)) {
        char buffer[BUFFER_SIZE];
        ssize_t bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
        if (bytes_received <= 0) {
          if (bytes_received == 0) {
            printf("Client %d disconnected\n", client_socket);
          } else {
            perror("recv() error");
          }
          close(client_socket);
          client_sockets[i] = 0;
        } else {
          buffer[bytes_received] = '\0';
          printf("Message from client %d: %s\n", client_socket, buffer);
          char response[] = "Dane zostaly otrzymane";
          send(client_socket, response, strlen(response), 0);
        }
      }
    }
  }

  return 0;
}
