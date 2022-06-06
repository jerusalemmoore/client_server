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
int send_recv(SOCKET &ConnectSocket){
        int recvbuflen = DEFAULT_BUFLEN;
        const char *sendbuf = "a";
        char recvbuf[DEFAULT_BUFLEN];
        int iResult;
        //send initial buffer
        iResult = send(ConnectSocket, sendbuf, (int) strlen(sendbuf), 0);
        if(iResult == SOCKET_ERROR) {
                printf("send failed: %d\n", WSAGetLastError());
                closesocket(ConnectSocket);
                WSACleanup();
                return 1;
        }
        printf("Bytes Sent: %ld\n", iResult);
        //shutdown the connection for sending
        //the client can still use the same socket for receiving
        iResult = shutdown(ConnectSocket, SD_SEND);
        if(iResult == SOCKET_ERROR) {
                printf("shutdown failed: %d\n", WSAGetLastError());
                closesocket(ConnectSocket);
                WSACleanup();
                return 1;
        }
        //recv data from send in response to send request
        do{
                iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
                if(iResult > 0) {
                        printf("Bytes received: %d\n", iResult);
                }
                else if (iResult == 0) {
                        printf("Connection closed\n");
                }
                else{
                        printf("recv failed: %d\n", WSAGetLastError());
                }
        }while(iResult > 0);

        //after finished receiving data
        closesocket(ConnectSocket);
        WSACleanup();

        return 0;
}



int main(int argc, char *argv[]) {
        WSADATA wsaData;

        int iResult;
        //WSAStartup initiates use of WS2_32.dll
        iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
        //check if result is expected(return 0 on success)
        if(iResult != 0) {
                printf("WSAStartup failed with val: %d\n", iResult);
                return 1;
        }
        struct addrinfo *result = NULL,
                        *ptr = NULL,
                        hints;
        //zero out addrinfo struct hints
        //enter the following vaues for the host
        ZeroMemory( &hints, sizeof(hints) );
        hints.ai_family = AF_UNSPEC;//caller only acceps af_inet and af_inet6 address families. any protocol family
        hints.ai_socktype = SOCK_STREAM;//only handle tcp
        hints.ai_protocol = IPPROTO_TCP;


        // Resolve the server address and port
        //get results and store into result
        //result seems to be a linked list and will go to the next element if call fails
        //ip, port, my defined addrinfo, results
        iResult = getaddrinfo("127.0.0.1", DEFAULT_PORT, &hints, &result);
        if (iResult != 0) {
                printf("getaddrinfo failed: %d\n", iResult);
                WSACleanup();
                return 1;
        }
        //create socket
        SOCKET ConnectSocket = INVALID_SOCKET;
        //assign anonymous ptr to the result of getaddrinfo
        ptr = result;
        //create sockets from values received from getaddrinfo
        ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);

        if (ConnectSocket == INVALID_SOCKET) {
                printf("Error on socket(): %d\n", WSAGetLastError());
                freeaddrinfo(result);
                WSACleanup();
                return 1;
        }
        iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if(iResult == SOCKET_ERROR) {
                printf("socket error\n");
                closesocket(ConnectSocket);
                ConnectSocket =INVALID_SOCKET;
        }
        // Should really try the next address returned by getaddrinfo
        // if the connect call failed
        // But for this simple example we just free the resources
        // returned by getaddrinfo and print an error message
        freeaddrinfo(result);
        if(ConnectSocket == INVALID_SOCKET) {
                printf("Unable to connect to server\n");
                WSACleanup();
                return 1;
        }

        int sendrecv = send_recv(ConnectSocket);
        if(sendrecv != 0) {
                printf("Error in sendrecv function\n");
                return 1;
        }




        return 0;
}
