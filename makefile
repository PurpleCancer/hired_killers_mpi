MPICC?=mpic++

all: main clean
 
main: main.cpp request company
	${MPICC} main.cpp -std=c++11 -o main.out CompanyRequest.o Company.o

request: CompanyRequest.cpp CompanyRequest.hpp
	g++ -c CompanyRequest.cpp

company: Company.cpp Company.hpp
	g++ -c Company.cpp

clean:
	rm -f CompanyRequest.o Company.o