#pragma once

#include "Base.hpp"

enum class TCPClientSocketConfig
{
	FAMILY = AF_INET,
	AF = AF_INET,
	TYPE = SOCK_STREAM,
};

class TCPClient
{
public:
	TCPClient() {}

	~TCPClient() {}

private:

};