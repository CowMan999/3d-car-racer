#include "udp.hpp"
#include "tcp.hpp"

std::vector<Client> clients;
size_t identifier;
std::mutex clientmutex;

int main() {
	
	std::cout << sf::IpAddress::getLocalAddress().toString() << "\n";
	std::cout << sf::IpAddress::getPublicAddress(sf::seconds(5)) << "\n";

	std::thread* udpThread = new std::thread(udplisten);
	std::thread* tcpThread = new std::thread(tcplisten);
	
	srand(time(nullptr));
	
	while(true) {
		std::string input;
		std::cin >> input;
		if(input == "exit") {
			abort();
		}
	}

	udpThread->join();
	tcpThread->join();
}

