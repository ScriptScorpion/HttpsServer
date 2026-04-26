#include "server.hpp"
#include <cstdlib>
#include <cstring>
#include <string>
#include <iostream>
#include <fstream>

#define MAXPKGSIZE 8192

bool Server::Start(const int &port) {
    char buff[MAXPKGSIZE] {};
    memset(buff, '\0', sizeof(buff));
    int SV_sock = socket(AF_INET, SOCK_STREAM, 0);
    int CL_sock;
    const int opt = 1;

    if (SV_sock < 0) {
        return false;
    }
    memset(&Server_addr, 0, sizeof(Server_addr));
    Server_addr.sin_family = AF_INET;
    Server_addr.sin_addr.s_addr = INADDR_ANY;
    Server_addr.sin_port = htons(port);
    if (setsockopt(SV_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) { // setting an option that allows system to bind to an already used IP address
        close(SV_sock);
        return false;
    }
    if (bind(SV_sock, (sockaddr*)&Server_addr, sizeof(Server_addr)) < 0) {
        close(SV_sock);
        return false;
    }
    if (listen(SV_sock, 1) < 0) {
        close(SV_sock);
        return false;
    }

    // SSL magic   
    
    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();
    
    SSL_CTX *ctx = SSL_CTX_new(TLS_server_method());
    if (!ctx) {
        std::cerr << "Failed to create CTX config" << std::endl;
        close(SV_sock);
        return false;
    }


    SSL_CTX_set_min_proto_version(ctx, TLS1_2_VERSION);
    SSL_CTX_set_options(ctx, SSL_OP_NO_SSLv3 | SSL_OP_NO_TLSv1 | SSL_OP_NO_TLSv1_1);

    if (SSL_CTX_use_certificate_file(ctx, "src/cert.pem", SSL_FILETYPE_PEM) <= 0) {
        std::cerr << "Certificate file doesn't exists or expired" << std::endl;
        SSL_CTX_free(ctx);
        close(SV_sock);
        return false;
    }

    if (SSL_CTX_use_PrivateKey_file(ctx, "src/key.pem", SSL_FILETYPE_PEM) <= 0) {
        std::cerr << "Private key file doesn't exists or expired or incorrect private key inputted" << std::endl;
        SSL_CTX_free(ctx);
        close(SV_sock);
        return false;
    }

    if (!SSL_CTX_check_private_key(ctx)) {
        std::cerr << "Certificate and private key don't match" << std::endl;
        SSL_CTX_free(ctx);
        close(SV_sock);
        return false;
    }
    std::cout << "\nStarted listening on this address https://127.0.0.1:" << port << "\n\n";

    while (true) {
        CL_sock = accept(SV_sock, nullptr, nullptr);
        if (CL_sock < 0) {
            continue;
        }
        SSL *ssl = SSL_new(ctx);
        SSL_set_fd(ssl, CL_sock);
        int ssl_accept_ret = SSL_accept(ssl);
        if (ssl_accept_ret <= 0) {
            SSL_free(ssl);
            close(CL_sock);
            continue;
        }
        
        int received_bytes = SSL_read(ssl, buff, sizeof(buff) - 1);
        if (received_bytes > 0 && received_bytes <= sizeof(buff)) {
            buff[received_bytes] = '\0';
            std::cout << "\n\n" << buff << "\n\n";
            Parse(ssl, CL_sock, buff);
            memset(buff, 0, sizeof(buff));
            SSL_free(ssl);
            SSL_shutdown(ssl);
            close(CL_sock);
        }

    }
    SSL_CTX_free(ctx);
    EVP_cleanup();
    close(SV_sock);
    return true;
}
void Server::Parse(SSL *ssl, const int &CL_sock, const char *buff) {
    std::string Method = "";
    std::string Path = "";
    std::string Version = "";
    
    {
        char *buff_copy = const_cast<char*>(buff);
        buff_copy[strcspn(buff_copy, "\r\n")] = '\0';

        char *token = strtok(buff_copy, " ");
        size_t iteration = 0;
        while (token != NULL) {
            if (iteration == 0) {
                Method = token;
            }
            else if (iteration == 1) {
                Path = token;
            }
            else if (iteration == 2) {
                Version = token;
            }
            else {
                break;
            }
            token = strtok(NULL, " ");
            ++iteration;
        }
    }

    // std::cout << '|' << Method << "| |" << Path << "| |" << Version << "|\n";

    std::ifstream File("/path/to/file.html", std::ios::ate);
    if (!File) {
        std::cerr << "File you provided don't exists" << std::endl;
        return;
    }
    std::string Response;
    Response += "HTTP/1.1 200 OK\r\n";
    Response += "Content-Type: text/html\r\n";
    Response += "Content-Length: " + std::to_string(File.tellg()) + "\r\n\r\n";
    File.seekg(0);
    char ch;
    while (File.get(ch)) {
        Response += ch;
    }
    File.close();
    SSL_write(ssl, Response.c_str(), Response.size());
}
