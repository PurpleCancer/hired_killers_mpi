MPICC?=mpic++

all: main clean
 
main: main.cpp request company rank
	${MPICC} main.cpp -lpthread -std=c++11 -o main.out CompanyRequest.o Company.o CompanyRank.o

request: CompanyRequest.cpp CompanyRequest.hpp
	g++ -c CompanyRequest.cpp

company: Company.cpp Company.hpp
	g++ -c Company.cpp

rank: CompanyRank.cpp CompanyRank.hpp
	g++ -c CompanyRank.cpp

clean:
	rm -f CompanyRequest.o Company.o CompanyRank.o