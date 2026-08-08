#include <iostream>
#include <string>
#include <cstring>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>

constexpr const char* SERVER_IP = "127.0.0.1";
constexpr uint16_t SERVER_PORT = 8080;
constexpr size_t BUFFER_SIZE = 1024;

int main()
{
    //----------------------------------------------------------
    // Create socket
    //----------------------------------------------------------

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    /*
        File Descriptor Table
        0 → stdin
        1 → stdout
        2 → stderr
        3 → socket creates successfully
    */

    if (sock == -1)
    {
        std::cerr << "socket() failed: "
                  << strerror(errno)
                  << '\n';
        return EXIT_FAILURE;
    }

    //----------------------------------------------------------
    // Configure server address
    //----------------------------------------------------------

    sockaddr_in server{};  // initialized server address with 0
    server.sin_family = AF_INET;  // for ipv4 
    server.sin_port = htons(SERVER_PORT); // conver serverport to network style(big Endian) 

    if (inet_pton(AF_INET, SERVER_IP, &server.sin_addr) != 1)
    {
        std::cerr << "Invalid IP address\n";
        close(sock);
        return EXIT_FAILURE;
    }

    //----------------------------------------------------------
    // Connect
    //----------------------------------------------------------

    if (connect(sock,
                reinterpret_cast<sockaddr*>(&server),
                sizeof(server)) == -1)
    {
        std::cerr << "connect() failed: "
                  << strerror(errno)
                  << '\n';

        close(sock);
        return EXIT_FAILURE;
    }
    /*
    3 ways handshaking
    Client       Server
    SYN ------------>
    <--------- SYN ACK
    ACK ------------>
    after the handshaking connection will get establish
    */
    std::cout << "Connected to server.\n";

    //----------------------------------------------------------
    // Message to send
    //----------------------------------------------------------

    const std::string message = "Hello Server";

    //----------------------------------------------------------
    // Send all bytes
    //----------------------------------------------------------

    size_t totalSent = 0;

    while (totalSent < message.size())
    {
        ssize_t bytesSent = send(
            sock,
            message.data() + totalSent,
            message.size() - totalSent,
            0);

        if (bytesSent == -1)
        {
            if (errno == EINTR)
                continue;

            std::cerr << "send() failed: "
                      << strerror(errno)
                      << '\n';

            close(sock);
            return EXIT_FAILURE;
        }

        totalSent += static_cast<size_t>(bytesSent);
    }

    std::cout << "Sent " << totalSent << " bytes.\n";

    //----------------------------------------------------------
    // Receive response
    //----------------------------------------------------------

    char buffer[BUFFER_SIZE];

    ssize_t bytesReceived;

    while (true)
    {
        bytesReceived = recv(
            sock,
            buffer,
            sizeof(buffer),
            0);

        if (bytesReceived == -1)
        {
            if (errno == EINTR)
                continue;

            std::cerr << "recv() failed: "
                      << strerror(errno)
                      << '\n';

            close(sock);
            return EXIT_FAILURE;
        }

        break;
    }

    if (bytesReceived == 0)
    {
        std::cout << "Server closed the connection.\n";
    }
    else
    {
        std::cout << "Server Response: "
                  << std::string(buffer, bytesReceived)
                  << '\n';
    }

    //----------------------------------------------------------
    // Cleanup
    //----------------------------------------------------------

    if (close(sock) == -1)
    {
        std::cerr << "close() failed: "
                  << strerror(errno)
                  << '\n';
    }

    return EXIT_SUCCESS;
}