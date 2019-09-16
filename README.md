# TSAM2
## Ports

**Build this project**
To build this project all you have to do is get into the directory and run the ``make command``. This will compile two seperate files for you, a sniffer and a raw executable. 

**Run this project**
To run this project you will first have to run the sniffer with the command

``./sniffer <IP> <port> <port>`` (_or_ ``make srun`` for the default settings)

Where (IP) is filled with the IP you want to sniff, and (port) is filled with the first and last port in the range you want to scan.
When the open ports are found the command line will show the results and the responding messages from the open UDP ports. 

To then send the raw made packets to the open ports you will have to run the command

``sudo ./raw <IP>`` (_or_ ``make rrun`` for the default settings)

With again the IP for your destination IP address

The program will then ask you for the port numbers discovered with the sniffer and when these are entered it will send packets accordingly*

_*The part above I was not able to solve properly, prohibiting me from completing the rest of the puzzle. I got very excited with the project when it was presented but the lack of help for someone who has no clue what ``&`` and ``*``symbols are doing in C++, and the lack of documentation online made me feel both irritated and disappointed in the ending of this project. I have put a substantial amount of effort and time in this project and it hurts to leave the puzzle unsolved._

_I hope the correct code will be released so I can see what I did wrong and also learn from my mistakes instead of just being left behind with a code that still doesn't reveal it's faults to me_

<https://github.com/Abeldewit/TSAM2>
