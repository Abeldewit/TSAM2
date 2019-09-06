IPS=130.208.243.61
IPN=74.207.244.221

output: main
	@echo "Files made"

run: main
	@echo "Running sniffer on $(IPS)"
	@sudo ./main $(IPS) 4000 4100


main: main.cpp
	g++ -Wall -std=c++14 main.cpp -o main