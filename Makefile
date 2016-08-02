CFLAGES = -Wall -ggdb -W -O
objects = main.o ClientHttp.o ClientSocket.o ParseParams.o

mywebbench: $(objects)
	g++ -o mywebbench  $(objects) $(CFLAGES)

main.o: ClientHttp.hpp
ClientHttp.o: ClientHttp.hpp ParseParams.hpp ClientSocket.hpp
ParseParams.o: ParseParams.hpp
ClientSocket.o: ClientSocket.hpp

.PHONY: clean

clean:
	rm mywebbench $(objects)
