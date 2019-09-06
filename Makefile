IP=130.208.243.61

output: main
	@echo "Files made"

run: main
	@echo "Running sniffer on $(IP)"
	@sudo ./main $(IP) 4000 4100


main: main.cpp
	g++ -Wall -std=c++14 main.cpp -o main