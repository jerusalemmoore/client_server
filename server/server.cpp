#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
//default port present on server that client will connect to
#define DEFAULT_PORT "27015"
//GETADDRINFO DOESN'T WORK WITHOUT THIS
#define _WIN32_WINNT 0x501

#define DEFAULT_BUFLEN 512

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <stdio.h>
#include <iostream>

#pragma comment(lib, "Ws2_32.lib")
#define DEFAULT_PORT "27015"
int send_recv(SOCKET &ClientSocket){
        printf("starting server function\n");
        char recvbuf[DEFAULT_BUFLEN];
        int iResult, iSendResult;
        int recvbuflen = DEFAULT_BUFLEN;
        // Receive until the peer shuts down the connection
        do {

                iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
                if (iResult > 0) {
                        printf("Bytes received: %d\n", iResult);

                        // Echo the buffer back to the sender
                        iSendResult = send(ClientSocket, recvbuf, iResult, 0);
                        if (iSendResult == SOCKET_ERROR) {
                                printf("send failed: %d\n", WSAGetLastError());
                                closesocket(ClientSocket);
                                WSACleanup();
                                return 1;
                        }
                        printf("Bytes sent: %d\n", iSendResult);
                        if(!strcmp(recvbuf,"a")) {
                                printf("you've selected a\n");
                        }
                } else if (iResult == 0)
                        printf("Connection closing...\n");
                else {
                        printf("recv failed: %d\n", WSAGetLastError());
                        closesocket(ClientSocket);
                        WSACleanup();
                        return 1;
                }

        } while (iResult > 0);
        // shutdown the send half of the connection since no more data will be sent
        iResult = shutdown(ClientSocket, SD_SEND);

        if (iResult == SOCKET_ERROR) {
                printf("shutdown failed: %d\n", WSAGetLastError());
                closesocket(ClientSocket);
                WSACleanup();
                return 1;
        }
        // printf("message received: %s\n", recvbuf);

        closesocket(ClientSocket);
        WSACleanup();
        return 0;
}
int main(int argc, char* argv[]){
        WSADATA wsaData;

        int iResult;
        iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
        //check if result is expected(return 0 on success)
        if(iResult != 0) {
                printf("WSAStartup failed with val: %d\n", iResult);
                return 1;
        }
        struct addrinfo *result = NULL, *ptr = NULL, hints;

        ZeroMemory(&hints, sizeof (hints));
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;
        //we set this for server not client
        hints.ai_flags = AI_PASSIVE;

        // Resolve the local address and port to be used by the server
        iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
        if (iResult != 0) {
                printf("getaddrinfo failed: %d\n", iResult);
                WSACleanup();
                return 1;
        }
        SOCKET ListenSocket = INVALID_SOCKET;
        //create socket for server to listen
        ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
        if(ListenSocket == INVALID_SOCKET) {
                printf("Error at socket(): %d\n", WSAGetLastError());
                freeaddrinfo(result);
                WSACleanup();
                return 1;
        }
        //server specific: bind to network within system
        iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
        if(iResult == SOCKET_ERROR) {
                printf("bind failed with error: %d\n", WSAGetLastError());
                freeaddrinfo(result);
                closesocket(ListenSocket);
                WSACleanup();
                return 1;
        }
        //don't need addrinfo after bind, so we free
        freeaddrinfo(result);

        //listen on socket with SOMAXCONN backlog. this allows for maximum reasonable
        //pending connections in queue
        if ( listen( ListenSocket, SOMAXCONN ) == SOCKET_ERROR ) {
                printf( "Listen failed with error: %ld\n", WSAGetLastError() );
                closesocket(ListenSocket);
                WSACleanup();
                return 1;
        }
        //handle requests on listening socket
        //accept connections from clients with temperary socket
        SOCKET ClientSocket;
        ClientSocket = accept(ListenSocket, NULL, NULL);
        if(ClientSocket ==INVALID_SOCKET) {
                printf("Accept failed: %d\n", WSAGetLastError());
                closesocket(ListenSocket);
                WSACleanup();
                return 1;
        }
        closesocket(ListenSocket);
        int sendrecv = send_recv(ClientSocket);
        if(sendrecv != 0) {
                printf("error in send_recv function\n");
                return 1;

        }
        return 0;

}
