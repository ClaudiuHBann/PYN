#pragma once

#include "Base.hpp"

class TCPClient
{
public:
	enum class TCPClientSocketConfig
	{
		AF = AF_INET,
		TYPE = SOCK_STREAM,
		FAMILY = AF_INET
	};

	inline TCPClient(const ::std::string& IP, const ::std::uint16_t port) {
		Connect(IP, port);
	}

	inline const bool Connect(const ::std::string& IP, const ::std::uint16_t port) {
		return Initialize(IP, port) && mBase.Connect(mSocketInfo.socket, mSocketInfo.hint);
	}

	inline const auto Send(const ::std::string& data) {
		return mBase.Send(mSocketInfo.socket, data.c_str(), (::std::int32_t)data.size() + 1, 0);
	}

	inline const auto SendAll(const ::std::string& data) {
		return mBase.SendAll(mSocketInfo.socket, data.c_str(), (::std::int32_t)data.size() + 1, 0);
	}

	inline auto Receive(const ::std::uint32_t length = 8192) {
		::std::string data(length, 0);
		const auto bytesReceived = mBase.Receive(mSocketInfo.socket, &data[0], length, 0);
		data.resize(bytesReceived);
		
		return data;
	}

	inline auto ReceiveAll(const ::std::uint32_t length = 8192) {
		::std::string data(length, 0);
		mBase.ReceiveAll(mSocketInfo.socket, &data[0], length, 0);

		return data;
	}

	inline ~TCPClient() {
		Deinitialize();
	}

private:
	inline bool Initialize(const ::std::string& IP, const ::std::uint16_t port) {
		mSocketInfo.socket = mBase.Socket((int32_t)TCPClientSocketConfig::AF, (int32_t)TCPClientSocketConfig::TYPE, 0);
		if (mSocketInfo.socket == INVALID_SOCKET) {
			return false;
		}

		mSocketInfo.hint = mBase.Hint((uint16_t)TCPClientSocketConfig::FAMILY, port, IP.c_str());

		return !mBase.GetErrorCode();
	}

	inline void Deinitialize() {
		mBase.Shutdown(mSocketInfo.socket, SHUTDOWN_BOTH);
		mBase.Close(mSocketInfo.socket);
	}

	Base mBase{};
	SocketInfo mSocketInfo{};
};