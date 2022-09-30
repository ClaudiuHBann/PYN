#include <iostream>
using namespace std;

#include "TCPClient.hpp"

int main() {
	TCPClient tcpClient("162.55.32.18", 32406);
	cout << tcpClient.Receive() << endl;
	tcpClient.SendAll("Hello BRAH!");

	return 0;
}

/*
	TO DO:
		- not thread safe
*/