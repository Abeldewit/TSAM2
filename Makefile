IP=130.208.243.61

output: sniffer
	@echo "Files made"

run: sniffer
	@echo "Running sniffer on $(IP)"
	@sudo ./sniffer $(IP) 4000 4100


sniffer: main.cpp
	g++ -Wall -std=c++11 -pthread main.cpp -o sniffer

run2: test.cpp
	g++ -Wall -std=c++11 test.cpp -o sniffer2
	@sudo ./sniffer2 $(IP) 4023