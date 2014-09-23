all: rawhttpget.o HTTP.o Cookies.o ManageCookie.o RegularExp.o RawSocket.o TCP.o IP.o Sock.o ARPReq.o
	g++ -o rawhttpget rawhttpget.o HTTP.o Cookies.o ManageCookie.o RegularExp.o RawSocket.o TCP.o IP.o Sock.o ARPReq.o -g -lpthread

ARPReq.o: ARPReq.cpp
	g++ -c ARPReq.cpp -g

Sock.o: Sock.cpp
	g++ -c Sock.cpp -g

rawhttpget.o: rawhttpget.cpp
	g++ -c rawhttpget.cpp -g

HTTP.o: HTTP.cpp
	g++ -c HTTP.cpp -g

Cookies.o: Cookies.cpp
	g++ -c Cookies.cpp -g

ManageCookie.o: ManageCookie.cpp
	g++ -c ManageCookie.cpp -g

RegularExp.o: RegularExp.cpp
	g++ -c RegularExp.cpp -g

RawSocket.o: RawSocket.cpp
	g++ -c RawSocket.cpp -g

TCP.o: TCP.cpp
	g++ -c TCP.cpp -g

IP.o: IP.cpp
	g++ -c IP.cpp -g

clean:
	rm *.o rawhttpget
