#include <iostream>
#include <string>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>


int main()
{

    int sock =
        socket(AF_INET,SOCK_STREAM,0);



    sockaddr_in server{};

    server.sin_family = AF_INET;
    server.sin_port = htons(8080);


    inet_pton(
        AF_INET,
        "127.0.0.1",
        &server.sin_addr
    );



    connect(
        sock,
        (sockaddr*)&server,
        sizeof(server)
    );



    std::string message ="hello to server";


    send(
        sock,
        message.c_str(),
        message.size(),
        0
    );



    char buffer[1024];


    int bytes =
        read(
            sock,
            buffer,
            sizeof(buffer)
        );


    std::cout
    << "Response: "
    << std::string(buffer,bytes)
    << std::endl;



    close(sock);

}