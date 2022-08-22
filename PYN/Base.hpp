#pragma once

// Standard Headers
#include <string>
#include <typeinfo>

// Helpful defines
#define GET_LOCATION "file \"" << __FILE__ << "\" in function '" << __func__ << "' at line '" << __LINE__ << '\''
#define PARAMETER_FROM_IS_NULL(p) ::std::string(::std::string(typeid(p).name()) + " '" + #p + "' parameter from method '" + __func__ + "' call is null!")

#define CHECK_STORE_AND_RETURN(pred, param) \
if(pred) { \
    SetErrorMessage(PARAMETER_FROM_IS_NULL(param)); \
	return; \
}
#define CHECK_STORE_AND_RETURN_X(pred, param, x) \
if(pred) { \
    SetErrorMessage(PARAMETER_FROM_IS_NULL(param)); \
	return x; \
}

// OS Specific Headers
#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN

#include <WinSock2.h>
#include <WS2tcpip.h>

#pragma comment(lib, "WS2_32.lib")

#elif defined(__linux__)

#include <string.h>
#include <tuple>

#include <arpa/inet.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>

#endif // OS

// Cross-Platform Defines
#ifdef _WIN32

#define ERROR_CODE ::h_errno
#define CLOSE_SOCKET(socket) ::closesocket(socket)

#define SHUTDOWN_RECEIVE SD_RECEIVE
#define SHUTDOWN_SEND SD_SEND
#define SHUTDOWN_BOTH SD_BOTH

#elif defined(__linux__)

#define SOCKET std::uint64_t

#define INVALID_SOCKET 0
#define SOCKET_ERROR -1

#define ERROR_CODE errno
#define CLOSE_SOCKET(socket) ::close(socket)

#define SHUTDOWN_RECEIVE SHUT_RD
#define SHUTDOWN_SEND SHUT_WR
#define SHUTDOWN_BOTH SHUT_RDWR

#endif // OS

typedef struct SocketInfo
{
	::SOCKET socket;
	struct ::sockaddr_in hint;
} SocketInfo;

class Base
{
public:
	inline Base() {
		Initialize();
	}

	inline Base(const Base&) {
		Initialize();
	}

	inline auto& operator=(const Base&) {
		Initialize();
		return *this;
	}

	inline Base(Base&&) noexcept {
		Initialize();
	}

	inline auto& operator=(Base&&) noexcept {
		Initialize();
		return *this;
	}

	inline ~Base() {
		Deinitialize();
	}

	inline const auto GetErrorCode() const {
		return mErrorCode;
	}

	inline const auto& GetErrorMessage() const {
		return mErrorMessage;
	}

#ifdef _WIN32
	inline const auto IsInitialized() const {
		return mIsInitialized;
	}

	inline const auto& GetWSADATA() const {
		return mWSADATA;
	}
#endif // _WIN32

	inline ::std::string GetErrorMessageFromErrorCode(const ::std::int32_t errorCode) const
	{
		if (!errorCode) {
			return "";
		}

		::std::string errorMessageAsString;

#ifdef _WIN32
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

		errorMessageAsString.assign(errorMessage, size);
		::LocalFree(errorMessage);

#elif defined(__linux__)

		errorMessageAsString.assign(strerror(ERROR_CODE));

#endif // OS

		return errorMessageAsString;
	}

	inline auto Hint(const ::std::uint16_t family, const ::std::uint16_t port, const char* address) {
		CHECK_STORE_AND_RETURN_X(!address, address, ::sockaddr_in());

		struct ::sockaddr_in hint;
		::memset(&hint, 0, sizeof(hint));

		hint.sin_family = family;
		hint.sin_port = ::htons(port);
		CheckAndStoreError(::inet_pton(family, address, &hint.sin_addr) == -1, "inet_pton");

		return hint;
	}

	inline const auto Socket(const ::std::int32_t af, const ::std::int32_t type, const ::std::int32_t protocol) {
		const auto socket = ::socket(af, type, protocol);
		CheckAndStoreError(socket == INVALID_SOCKET, "socket");
		return socket;
	}

	inline const bool Connect(const ::SOCKET socket, const struct ::sockaddr_in& hint) {
		CHECK_STORE_AND_RETURN_X(!&hint, hint, false);

		const auto result = ::connect(socket, (struct ::sockaddr*) & hint, sizeof(hint));
		CheckAndStoreError(result == SOCKET_ERROR, "connect");
		return !result;
	}

	inline const auto Send(const ::SOCKET socket, const char* buffer, const ::std::int32_t length, const ::std::int32_t flags) {
		CHECK_STORE_AND_RETURN_X(!buffer, buffer, -1);

		const auto bytesSent = ::send(socket, buffer, length, flags);
		CheckAndStoreError(bytesSent == SOCKET_ERROR, "send");
		return bytesSent;
	}

	const auto SendAll(const ::SOCKET socket, const char* buffer, const ::std::int32_t length, const ::std::int32_t flags) {
		CHECK_STORE_AND_RETURN_X(!buffer, buffer, false);

		auto bytesSentTotal = 0;
		while (bytesSentTotal != length) {
			const auto bytesSentCurrent = ::send(socket, buffer, length, flags);
			if (bytesSentCurrent == SOCKET_ERROR) {
				CheckAndStoreError(true, "send");
				return false;
			}

			bytesSentTotal += bytesSentCurrent;
		}

		return true;
	}

	inline const auto Receive(const ::SOCKET socket, char* buffer, const ::std::int32_t length, const ::std::int32_t flags) {
		CHECK_STORE_AND_RETURN_X(!buffer, buffer, -1);

		const auto bytesReceived = ::recv(socket, buffer, length, flags);
		CheckAndStoreError(bytesReceived == SOCKET_ERROR, "recv");
		return bytesReceived;
	}

	const auto ReceiveAll(const ::SOCKET socket, char* buffer, const ::std::int32_t length, const ::std::int32_t flags) {
		CHECK_STORE_AND_RETURN_X(!buffer, buffer, false);

		auto bytesReceivedTotal = 0;
		while (bytesReceivedTotal != length) {
			const auto bytesReceivedCurrent = ::recv(socket, buffer, length, flags);
			if (bytesReceivedCurrent == SOCKET_ERROR) {
				CheckAndStoreError(true, "recv");
				return false;
			}

			bytesReceivedTotal += bytesReceivedCurrent;
		}

		return true;
	}

	inline const bool Bind(const ::SOCKET socket, const struct ::sockaddr_in& hint) {
		CHECK_STORE_AND_RETURN_X(!&hint, hint, false);

		const auto result = ::bind(socket, (struct ::sockaddr*) & hint, sizeof(hint));
		CheckAndStoreError(result == SOCKET_ERROR, "bind");
		return !result;
	}

	inline const bool Listen(const ::SOCKET socket, const ::std::int32_t backlog) {
		const auto result = ::listen(socket, backlog);
		CheckAndStoreError(result == SOCKET_ERROR, "listen");
		return !result;
	}

	inline const auto Accept(const ::SOCKET socket, struct ::sockaddr_in& hint, ::std::int32_t& length) {
		CHECK_STORE_AND_RETURN_X(!&hint, hint, INVALID_SOCKET);
		CHECK_STORE_AND_RETURN_X(!&length, length, INVALID_SOCKET);


		const auto client = ::accept(socket, (struct ::sockaddr*) & hint,
#ifdef __linux__
		(::std::uint32_t*)
#endif
			& length);
		CheckAndStoreError(client == INVALID_SOCKET, "accept");
		return client;
	}

	inline const bool Close(const ::SOCKET socket) {
		const auto result = CLOSE_SOCKET(socket);
		CheckAndStoreError(result == SOCKET_ERROR, "close");
		return !result;
	}

	inline const auto Shutdown(const ::SOCKET socket, const ::std::int32_t how) {
		const auto result = ::shutdown(socket, how);
		CheckAndStoreError(result == SOCKET_ERROR, "shutdown");
		return !result;
	}

	inline const ::std::tuple<::std::string, ::std::string> GetHostAndService(struct ::sockaddr_in& hint) {
		CHECK_STORE_AND_RETURN_X(!&hint, hint, {});

		char host[NI_MAXHOST];
		::memset(host, 0, NI_MAXHOST);

		char service[NI_MAXSERV];
		::memset(service, 0, NI_MAXSERV);

		const auto result1 = ::getnameinfo((struct ::sockaddr*) & hint, sizeof(hint), host, NI_MAXHOST, service, NI_MAXSERV, 0);
		if (!result1) {
			return { host, service };
		} else {
			CheckAndStoreError(true, "getnameinfo");
		}

		const auto result2 = ::inet_ntop(AF_INET, &hint.sin_addr, host, NI_MAXHOST);
		if (result2) {
			return { host, ::std::to_string(::ntohs(hint.sin_port)) };
		} else {
			CheckAndStoreError(true, "inet_ntop");
		}

		return {};
	}

private:
	inline void Initialize() {
#ifdef _WIN32
		mCount++;

		if (mIsInitialized) {
			return;
		}

		const auto result = ::WSAStartup(MAKEWORD(2, 2), &mWSADATA);
		if (result) {
			CheckAndStoreError(true, "WSAStartup");
		} else {
			mIsInitialized = true;
		}
#endif // _WIN32
	}

	inline void Deinitialize() {
#ifdef _WIN32
		if (--mCount) {
			return;
		}

		if (!mIsInitialized) {
			return;
		}

		const auto result = ::WSACleanup();
		if (result == SOCKET_ERROR) {
			CheckAndStoreError(true, "WSACleanup");
		} else {
			mIsInitialized = false;
		}
#endif // _WIN32
	}

	inline void CheckAndStoreError(const bool fail, const ::std::string& what) {
		if (fail) {
			mErrorCode = ERROR_CODE;
			mErrorMessage = what + " failed with error: " + GetErrorMessageFromErrorCode(mErrorCode);
		}
	}

	inline void SetErrorMessage(const ::std::string& errorMessage) {
		mErrorMessage = errorMessage;
	}

#ifdef _WIN32
	static inline ::WSADATA mWSADATA{};
	static inline ::std::uint16_t mCount = 0;
	static inline bool mIsInitialized = false;
#endif // _WIN32

	static inline ::std::int32_t mErrorCode = 0;
	static inline ::std::string mErrorMessage = "All Clear BRAH!";
};