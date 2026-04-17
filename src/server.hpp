#pragma once
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <openssl/ssl.h>

class Server {
     private:
         sockaddr_in Server_addr {};
     public:
         bool Start(const int &port);
         void Info_Sender(SSL *ssl, const int &CL_sock, const char *buff);
};                     
