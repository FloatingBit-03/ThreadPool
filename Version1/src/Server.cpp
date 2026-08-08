#include <iostream>
#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <atomic>
#include <string>

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>


class ThreadPool {

private:

    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;

    std::mutex queueMutex;
    std::condition_variable condition;

    bool stop = false;

    size_t maxQueueSize = 1000;


    std::atomic<int> requestCounter{0};


public:


    ThreadPool(size_t threadCount)
    {

        for(size_t i = 0; i < threadCount; i++)
        {

            workers.emplace_back([this, i]
            {

                std::string workerName =
                    "Worker-" + std::to_string(i + 1);


                while(true)
                {

                    std::function<void()> task;


                    {
                        std::unique_lock<std::mutex> lock(queueMutex);


                        condition.wait(lock,[this]
                        {
                            return stop || !tasks.empty();
                        });


                        if(stop && tasks.empty())
                            return;


                        task = std::move(tasks.front());

                        tasks.pop();
                    }


                    std::cout 
                    << workerName
                    << " picked a request\n";


                    task();

                }

            });

        }

    }



    bool enqueue(std::function<void()> task)
    {

        {

            std::lock_guard<std::mutex> lock(queueMutex);


            if(tasks.size() >= maxQueueSize)
            {
                return false;
            }


            tasks.push(std::move(task));

        }


        condition.notify_one();

        return true;

    }



    int getRequestNumber()
    {
        return ++requestCounter;
    }



    ~ThreadPool()
    {

        {
            std::lock_guard<std::mutex> lock(queueMutex);

            stop=true;
        }


        condition.notify_all();


        for(auto &worker : workers)
        {
            worker.join();
        }

    }

};




// Process client connection

void processClient(int clientSocket, ThreadPool &pool)
{

    char buffer[1024];


    while(true)
    {

        int bytes =
            recv(
                clientSocket,
                buffer,
                sizeof(buffer)-1,
                0
            );


        if(bytes <= 0)
        {
            std::cout 
            << "Client disconnected\n";

            break;
        }


        buffer[bytes]='\0';


        int requestId =
            pool.getRequestNumber();



        std::cout
        << "Request #"
        << requestId
        << " received: "
        << buffer
        << std::endl;



        std::string response =
            "Request #" +
            std::to_string(requestId) +
            " processed successfully";


        send(
            clientSocket,
            response.c_str(),
            response.size(),
            0
        );

    }


    close(clientSocket);

}




int main()
{

    int serverSocket =
        socket(
            AF_INET,
            SOCK_STREAM,
            0
        );


    sockaddr_in serverAddr{};


    serverAddr.sin_family = AF_INET;

    serverAddr.sin_port =
        htons(8080);

    serverAddr.sin_addr.s_addr =
        INADDR_ANY;



    bind(
        serverSocket,
        (sockaddr*)&serverAddr,
        sizeof(serverAddr)
    );


    listen(
        serverSocket,
        10
    );



    std::cout
    << "Server started on port 8080\n";



    ThreadPool pool(4);



    while(true)
    {

        int clientSocket =
            accept(
                serverSocket,
                nullptr,
                nullptr
            );


        std::cout
        << "Client connected\n";



        bool accepted =
            pool.enqueue(
                [&pool, clientSocket]
                {
                    processClient(
                        clientSocket,
                        pool
                    );
                }
            );



        if(!accepted)
        {

            std::string msg =
                "Server busy. Try later";


            send(
                clientSocket,
                msg.c_str(),
                msg.size(),
                0
            );


            close(clientSocket);

        }

    }


    close(serverSocket);

}