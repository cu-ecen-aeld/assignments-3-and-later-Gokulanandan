#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <syslog.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#define PORT 9000
#define MAX_SIZE 1024

int server_fd = 0, new_fd = 0, fd = 0;
const char* filename = "/var/tmp/aesdsocketdata";
char *full_buf = NULL;

void signal_handler(int signal){
    if(signal == SIGINT || signal == SIGTERM) {
        syslog(LOG_INFO,"Caught signal, exiting");
        if (server_fd >= 0) {
            close(server_fd);
            server_fd = -1;
        }
        if (new_fd >= 0) {
            close(new_fd);
            new_fd = -1;
        }
        if (fd >= 0) {
            close(fd);
            fd = -1;
        }

        remove(filename);
        if(full_buf) {
            free(full_buf);
            full_buf = NULL;
        }
        exit(0);
    }
}

int main(int argc, char *argv[]){

    int deamon = 0;
    if(argc == 2 && !strcmp(argv[1],"-d")) deamon = 1;
    // Register for signals
    void (*handler) (int);
    handler = signal_handler;
    struct sockaddr_in addr, client_addr;
    signal(SIGINT, handler);
    signal(SIGTERM, handler);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        syslog(LOG_ERR,"socket Creation failed");
        return -1;
    }

    int opt = 1;
    if(setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        syslog(LOG_ERR,"setsockopt failed");
        close(server_fd);
        return -1;
    }
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) <0 ) {
        syslog(LOG_ERR,"bind failed");
        close(server_fd);
        return -1;
    }
            
    if(deamon) {
        int pid = fork();
        if(pid < 0) {
            syslog(LOG_ERR, "Fork call failed");
            return -1;
        }
        if(pid > 0) {
             // parent can exit
             return 0;
        }
        // the child has to run as deamon, so close all console access
        close(STDIN_FILENO);
        close(STDOUT_FILENO);
        close(STDERR_FILENO);
    }

    if (listen(server_fd, 5) < 0) {
        close(server_fd);
        syslog(LOG_ERR,"listen failed");
        return -1;
    }

    while(1) {
        socklen_t len = sizeof(client_addr);
        new_fd = accept(server_fd, (struct sockaddr *)&client_addr, &len);
        if(new_fd < 0) {
            syslog(LOG_ERR, "Accept failed");
            return -1;
        }

        char *client_ip = inet_ntoa(client_addr.sin_addr);
        if(!client_ip){
            syslog(LOG_ERR, "Malformed client ip");
            close(server_fd);
            close(new_fd);
            return -1;
        }
        syslog(LOG_INFO,"Accepted connection from %s",client_ip);
    
        char buf[MAX_SIZE] = {0};
        int full_buf_size = 0;

        while(1) {
            ssize_t bytes_read = recv(new_fd, buf, MAX_SIZE, 0);
            if (bytes_read <= 0) break;
            char *temp = realloc(full_buf, full_buf_size+bytes_read+1);
            if(!temp) {
                syslog(LOG_ERR, "Memory Allocation failed");
                close(server_fd);
                close(new_fd);
                free(full_buf);
                full_buf = NULL;
                return -1;
            }

            full_buf = temp;
            memcpy(full_buf+full_buf_size, buf, bytes_read);
            full_buf_size += bytes_read;
            full_buf[full_buf_size] = '\0';


            if(memchr(buf, '\n', bytes_read)) {
                break;
            }
        }
        if (full_buf_size > 0) {
            fd = open(filename, O_WRONLY | O_CREAT | O_APPEND, 0755);
            
            if (fd < 0) {
                syslog(LOG_ERR,"Unable to open file for appending");
                close(server_fd);
                close(new_fd);
                return -1;
            }
            if(write(fd, full_buf, full_buf_size) < 0) {
                syslog(LOG_ERR,"Write failed");
                close(server_fd);
                close(new_fd);
                close(fd);
                return -1;
            }
            close(fd);

            fd = open(filename, O_RDONLY, 0755);
            if (fd < 0) {
                syslog(LOG_ERR,"Unable to open file to read");
                close(server_fd);
                close(new_fd);
                return -1;
            }
            // Read the file content into buf
            memset(buf, 0, MAX_SIZE);
            ssize_t bytes;
            while ((bytes = read(fd, buf, MAX_SIZE)) > 0) {
                if(send(new_fd, buf, bytes, 0) < 0 ) {
                    syslog(LOG_ERR,"Send to client failed");
                    break;
                }
                memset(buf, 0, MAX_SIZE);
            }
            syslog(LOG_INFO,"Closed connection from %s",client_ip);
            close(fd);
        }
        else {
            syslog(LOG_ERR,"recv call failed");
            return -1;
        }
        free(full_buf);
        full_buf = NULL;
        close(new_fd);
    }
    close(server_fd);
    remove(filename);
    return 0;
}
