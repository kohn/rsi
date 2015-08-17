#ifndef CORE_H
#define CORE_H
#include <sys/types.h>
class NetworkError{};
class LocalError{};

class Networking{
public:
    // make sure to write *length* bytes to *socket* from *buffer*
    // throw WriteError if there is fatal error
    static int write(int socket, const void *buffer, size_t length) throw(NetworkError, LocalError);
    // read a complete message from *socket* to *buffer*
    static int read_msg(int socket, void *buffer) throw(NetworkError, LocalError);
    Networking(){}
    ~Networking(){}
};
#endif
