#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[])
{
    int my_sock, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    //char buff[256];
	char buff[1024];
    printf("TCP DEMO CLIENT\n");
	
    if (argc < 3) {
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       exit(0);
    }
    // извлечение порта
	portno = atoi(argv[2]);
    
	// Шаг 1 - создание сокета
	my_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (my_sock < 0) 
        error("ERROR opening socket");
    // извлечение хоста
	server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    // заполенние структуры serv_addr
memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
   memcpy(&serv_addr.sin_addr.s_addr,
       server->h_addr_list[0],
       server->h_length);
    // установка порта
	serv_addr.sin_port = htons(portno);
    
	// Шаг 2 - установка соединения	
	if (connect(my_sock,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
        error("ERROR connecting");
    
	// Шаг 3 - чтение и передача сообщений
	while ((n = recv(my_sock, &buff[0], sizeof(buff) - 1, 0)) > 0)
     {
         // ставим завершающий ноль в конце строки
         buff[n] = 0;

         // выводим на экран
         printf("S=>C:%s", buff);

         // читаем пользовательский ввод с клавиатуры
         
        printf("SERVER: %s\n", buff);

		 fgets(&buff[0], sizeof(buff) - 1, stdin);

         // проверка на "quit"
         if (!strcmp(&buff[0], "quit\n"))
         {
             // Корректный выход
             printf("Exit...");
             close(my_sock);
              return 0;
         }

         // передаем строку клиента серверу
         send(my_sock, &buff[0], strlen(&buff[0]), 0);
     }
     printf("Recv error \n");
     close(my_sock);
     return -1;
	
	/*
	printf("Please enter the message: ");
    bzero(buff,256);
    fgets(buff,255,stdin);
    n = write(sockfd,buff,strlen(buff));
    if (n < 0) 
         error("ERROR writing to socket");
    
	bzero(buff,256);
    n = read(sockfd,buff,255);
    if (n < 0) 
         error("ERROR reading from socket");
    printf("%s\n",buff);
    
	printf("Please enter the message: ");
    bzero(buff,256);
    fgets(buff,255,stdin);
    n = write(sockfd,buff,strlen(buff));
    if (n < 0) 
         error("ERROR writing to socket");
    
	bzero(buff,256);
    n = read(sockfd,buff,255);
    if (n < 0) 
         error("ERROR reading from socket");
    printf("%s\n",buff);
	close(sockfd);
    return 0;*/
}