#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <sys/socket.h>
#include <sys/unistd.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <iostream>
#include <string>

using std::cout;
using std::cin;
using std::string;
using std::endl;

#define BUFFSIZE 2048
#define SERVER_IP "127.0.0.1"    // 指定服务端的IP
#define SERVER_PORT 2021            // 指定服务端的port

int maain()
{
    cout<<"client start"<<endl;
    int sock;
    FILE* fp;

    char buf[BUFFSIZE];
    int read_cnt;
    struct sockaddr_in serv_adr;


    fp = fopen("receivee.cpp", "wb");
    sock = socket(PF_INET, SOCK_STREAM, 0);
    cout<<sock<<endl;
    bzero(&serv_adr, sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = inet_addr(SERVER_IP);
    serv_adr.sin_port = htons(SERVER_PORT);

    connect(sock, (struct sockaddr*) &serv_adr, sizeof(serv_adr));
    sleep(5);
    while((read_cnt = read(sock, buf, BUFFSIZE)) != 0) 
        fwrite((void*) buf, 1, read_cnt, fp);

    cout << "receiced file data" << endl;
    write(sock, "thank u" , 10);
    fclose(fp);
    close(sock);
    return 0;


}
