#include "core.h"
#include <exception>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <errno.h>
int Networking::write(int socket, const void *buffer, size_t length) throw(NetworkError, LocalError){
    int write_count = 0;
    while(write_count < length){
        int count = send(socket, (char *)buffer+write_count, length-write_count, 0);
        if(count < 0){
            switch (errno) {
            case ECONNRESET:
            case EHOSTUNREACH:
            case ENETDOWN:
            case ENETUNREACH:
            case EPIPE:
                throw NetworkError();
            case EINTR:
                return Networking::write(socket, buffer, length);
            default:
                throw LocalError();
            }
        }
        write_count += count;
    }
    return 0;
}


int Networking::read_msg(int socket, void *buffer) throw(NetworkError, LocalError){
    int read_count = recv(socket, buffer, 4095, 0);
    if(read_count < 0){
        switch(errno){
        case EINTR:
            return Networking::read_msg(socket, buffer);
        case ECONNRESET:
        case ETIMEDOUT:
            throw NetworkError();
        default:
            throw LocalError();
        }
    }
    return read_count;
}
