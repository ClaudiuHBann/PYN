#pragma once

#define WIN32_LEAN_AND_MEAN

#include <string>

#include <WinSock2.h>
#include <WS2tcpip.h>
#pragma comment(lib, "WS2_32.lib")

class Base
{
public:
	inline Base() {
		Initialize();
	}

	inline Base(const Base&) {
		Initialize();
	}

	inline Base& operator=(const Base&) {
		Initialize();
	}

	inline Base(Base&&) noexcept {
		Initialize();
	}

	inline Base& operator=(Base&&) noexcept {
		Initialize();
	}

	inline ~Base() {
		Uninitialize();
	}

	inline const auto IsInitialized() const {
		return mIsInitialized;
	}

	inline const auto GetErrorCode() const {
		return mErrorCode;
	}

	inline const auto& GetErrorMessage() const {
		return mErrorMessage;
	}

	inline const auto& GetWSADATA() const {
		return mWSADATA;
	}

	inline ::std::string GetErrorMessageFromErrorCode(const ::std::int32_t errorCode) const
	{
		if (!errorCode) {
			return "";
		}

		char* errorMessage = nullptr;
		const auto size = ::FormatMessageA(
			FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			nullptr,
			errorCode,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(char*)&errorMessage,
			0,
			nullptr
		);

		::std::string errorMessageAsString(errorMessage, size);
		::LocalFree(errorMessage);
		return errorMessageAsString;
	}

	inline auto CreateHint(const ::std::uint16_t family, const ::std::uint16_t port, const char* address) {
		struct ::sockaddr_in hint;
		::memset(&hint, 0, sizeof(hint));

		hint.sin_family = family;
		hint.sin_port = ::htons(port);
		if (::inet_pton(family, address, &hint.sin_addr) == -1) {
			mErrorCode = ::WSAGetLastError();
			mErrorMessage = "Hint creation failed with error: " + GetErrorMessageFromErrorCode(mErrorCode);
		}

		return hint;
	}

	inline const auto CreateSocket(const ::std::int32_t af, const ::std::int32_t type, const ::std::int32_t protocol) {
		const auto socket = ::socket(af, type, protocol);
		if (socket == INVALID_SOCKET) {
			mErrorCode = ::WSAGetLastError();
			mErrorMessage = "Socket creation failed with error: " + GetErrorMessageFromErrorCode(mErrorCode);
		}

		return socket;
	}

	inline const bool Connect(const ::SOCKET socket, const struct ::sockaddr_in& hint) {
		mErrorCode = ::connect(socket, (struct ::sockaddr*) & hint, sizeof(hint));
		if (mErrorCode == SOCKET_ERROR) {
			mErrorCode = ::WSAGetLastError();
			mErrorMessage = "Connecting failed with error: " + GetErrorMessageFromErrorCode(mErrorCode);
		}

		return mErrorCode != SOCKET_ERROR;
	}

	inline const auto Send(const ::SOCKET socket, const char* buffer, const ::std::int32_t length, const ::std::int32_t flags) {
		const auto bytesSent = ::send(socket, buffer, length, flags);
		if (bytesSent == SOCKET_ERROR) {
			mErrorCode = ::WSAGetLastError();
			mErrorMessage = "Sending failed with error: " + GetErrorMessageFromErrorCode(mErrorCode);
		}

		return bytesSent;
	}

	inline const auto Receive(const ::SOCKET socket, char* buffer, const ::std::int32_t length, const ::std::int32_t flags) {
		const auto bytesReceived = ::recv(socket, buffer, length, flags);
		if (bytesReceived == SOCKET_ERROR) {
			mErrorCode = ::WSAGetLastError();
			mErrorMessage = "Receiving failed with error: " + GetErrorMessageFromErrorCode(mErrorCode);
		}

		return bytesReceived;
	}

	inline const bool Bind(const ::SOCKET socket, const struct ::sockaddr_in& hint) {
		mErrorCode = ::bind(socket, (struct ::sockaddr*) & hint, sizeof(hint));
		if (mErrorCode == SOCKET_ERROR) {
			mErrorCode = ::WSAGetLastError();
			mErrorMessage = "Binding failed with error: " + GetErrorMessageFromErrorCode(mErrorCode);
		}

		return mErrorCode != SOCKET_ERROR;
	}

	inline const auto Listen(const ::SOCKET socket, const ::std::int32_t backlog) {
		mErrorCode = ::listen(socket, backlog);
		if (mErrorCode == SOCKET_ERROR) {
			mErrorCode = ::WSAGetLastError();
			mErrorMessage = "Listening failed with error: " + GetErrorMessageFromErrorCode(mErrorCode);
		}

		return !mErrorCode;
	}

	inline const auto Accept(const ::SOCKET socket, struct ::sockaddr_in& hint, ::std::int32_t& length) {
		const auto client = ::accept(socket, (struct ::sockaddr*) & hint, &length);
		if (client == INVALID_SOCKET) {
			mErrorCode = ::WSAGetLastError();
			mErrorMessage = "Accepting failed with error: " + GetErrorMessageFromErrorCode(mErrorCode);
		}

		return client;
	}

	inline const bool Close(const ::SOCKET socket) {
		mErrorCode = ::closesocket(socket);
		if (mErrorCode == SOCKET_ERROR) {
			mErrorCode = ::WSAGetLastError();
			mErrorMessage = "Closing failed with error: " + GetErrorMessageFromErrorCode(mErrorCode);
		}

		return mErrorCode != SOCKET_ERROR;
	}

	inline const auto Shutdown(const ::SOCKET socket, const ::std::int32_t how) {
		mErrorCode = ::shutdown(socket, how);
		if (mErrorCode == SOCKET_ERROR) {
			mErrorCode = ::WSAGetLastError();
			mErrorMessage = "Shutting down failed with error: " + GetErrorMessageFromErrorCode(mErrorCode);
		}

		return mErrorCode != SOCKET_ERROR;
	}

	inline const auto GetSocketInfo(struct ::sockaddr_in& hint) {
		char host[NI_MAXHOST];
		::memset(host, 0, NI_MAXHOST);

		char service[NI_MAXSERV];
		::memset(service, 0, NI_MAXSERV);

		if (::getnameinfo((struct ::sockaddr*) & hint, sizeof(hint), host, NI_MAXHOST, service, NI_MAXSERV, 0)) {
			::inet_ntop(AF_INET, &hint.sin_addr, host, NI_MAXHOST);
			return ::std::tuple<::std::string, ::std::string>(host, ::std::to_string(::ntohs(hint.sin_port)));
		}

		return ::std::tuple<::std::string, ::std::string>(host, service);
	}

private:
	inline void Initialize() {
		mCount++;

		if (mIsInitialized) {
			return;
		}

		mErrorCode = ::WSAStartup(MAKEWORD(2, 2), &mWSADATA);
		if (!mErrorCode) {
			mIsInitialized = true;
		} else {
			mErrorMessage = "Windows Sockets API initialization failed with error: " + GetErrorMessageFromErrorCode(mErrorCode);
		}
	}

	inline void Uninitialize() {
		if (--mCount) {
			return;
		}

		if (!mIsInitialized) {
			return;
		}

		mErrorCode = ::WSACleanup();
		if (!mErrorCode) {
			mIsInitialized = false;
		} else {
			mErrorMessage = "Windows Sockets API uninitialization failed with error: " + GetErrorMessageFromErrorCode(mErrorCode);
		}
	}

	static inline ::std::uint16_t mCount = 0;
	static inline bool mIsInitialized = false;
	static inline ::WSADATA mWSADATA{};
	static inline ::std::int32_t mErrorCode = 0;
	static inline ::std::string mErrorMessage = "All Clear BRA!";
};