BIN := veka_server

CFLAGS := -g -Wall -std=c++11 -lnuma -lvsf
CC := g++ 

SRCFILE :=  get_config.cpp sysinfo.cpp veka_server.cpp main.cpp
OBJFILE :=  get_config.o sysinfo.o veka_server.o main.o

$(BIN): $(OBJFILE) 
	$(CC) $(CFLAGS) -o $(BIN) $(OBJFILE)

client: veka_client.cpp
	$(CC) $(CFLAGS) -o veka_client veka_client.cpp

%.o:%.cpp
	$(CC) $(CFLAGS)  -c $< -o $@

clean :  
	rm -rf $(OBJFILE) ${BIN}
