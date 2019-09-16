IP=130.208.243.61

output: sniffer raw
	@echo "Files compiled"

sniffer: sniffer.cpp
	@g++ -Wall -std=c++11 -pthread sniffer.cpp -o sniffer

raw: raw.cpp
	@g++ -Wall -std=c++11 -pthread raw.cpp -o raw

srun:
	@./sniffer $(IP) 4000 4100

rrun:
	@sudo ./raw $(IP)