//
//  main.cpp
//  chatserver
//
//  Created by yamatonao on 2016/10/25.
//  Copyright © 2016年 yamatonao. All rights reserved.
//

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <netdb.h>
#include <pthread.h>

#define PORT 12345

void kill_zombie_process(int sig);
void close_process(int unused);
char *show_ip(char *ip_address);

void* readthreadfunction(void *p);

int main(void){
    
    struct hostent *server_host;
    char host_name[256];
    
    memset(host_name, 0, sizeof(host_name));
    gethostname(host_name, 256);
    server_host = gethostbyname(host_name);
    
    printf(  " --------- server information -------- \n");
    printf("host name : %s \n", host_name);
    printf("ip        : %s \n", "127.0.0.1");
    printf(  " ------------------------------------- \n");
    printf("\n");
    fflush(stdout);
    
    struct sockaddr_in server;
    struct sockaddr_in client;
    socklen_t size = sizeof(client);
    int socklisten = socket(AF_INET,SOCK_STREAM,0);
    memset((char *)&server, 0, sizeof(server));
    
    server.sin_family       = AF_INET;
    server.sin_addr.s_addr  = htonl(INADDR_ANY);
    server.sin_port         = htons(PORT);
    bind(socklisten, (struct sockaddr *)&server, sizeof(server));
    listen(socklisten,5);
    signal(SIGINT   , close_process);
    signal(SIGCHLD  , kill_zombie_process);
    
    printf(  " --------- Waiting connects ! -------- \n");
    
    while(1){
        int sockchatting;
        int pid;
        sockchatting = accept(socklisten, (struct sockaddr *)&client, &size);
        pid = fork();
        if(pid != 0){
            //parent process
            close(sockchatting);
            continue; // acceptに戻る。
        }
        //child process
        close(socklisten);
        printf("%s is connected \n",show_ip((char *)&client.sin_addr));
        
        pthread_t readthread;
        pthread_create(&readthread, NULL, &readthreadfunction , &sockchatting);
        
        while(1){
            char buf[80];
            memset(buf, '\0', sizeof(buf));
            recv(sockchatting, buf, 80, 0);
            printf("%s > %s",show_ip((char *)&client.sin_addr),buf);
            if(strncmp(buf, "exit", 4) == 0){
                printf("%s is disconnected \n",show_ip((char *)&client.sin_addr));
                close(sockchatting);
                exit(0);
            }
        }
    }
    return 0;
}

void kill_zombie_process(int unused){
    while(waitpid(-1, nullptr, WNOHANG) > 0);
    signal(SIGCHLD, kill_zombie_process);
}

void close_process(int unused){
    exit(0);
}

char *show_ip(char *ip_address){
    static char ip[7];
    char ipnum[4];
    
    bcopy(ip_address, ipnum, 4);
    sprintf(ip, "%u.%u.%u.%u",(unsigned char)ipnum[0],(unsigned char)ipnum[1],
            (unsigned char)ipnum[2],(unsigned char)ipnum[3]);
    
    return ip;
}

void* readthreadfunction(void *p){
    int sock = *((int *)p);
    while(1){
        char message[80];
        fgets(message,80,stdin);
        send(sock,message,strlen(message),0);
    }
    //never reach
    //pthread_exit(NULL);
}
