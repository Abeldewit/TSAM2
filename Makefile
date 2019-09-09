IP=130.208.243.61

output: sniffer
	@echo "Files made"

run: sniffer
	@echo "Running sniffer on $(IP)"
	@sudo ./sniffer $(IP) 4000 4100


sniffer: main.cpp
	g++ -Wall -std=c++14 main.cpp -o sniffer