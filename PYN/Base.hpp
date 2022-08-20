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

	Base(const Base&) {
		Initialize();
	}

	Base& operator=(const Base&) {
		Initialize();
	}

	Base(Base&&) noexcept {
		Initialize();
	}

	Base& operator=(Base&&) noexcept {
		Initialize();
	}

	inline ~Base() {
		Uninitialize();
	}

	inline auto IsInitialized() const {
		return mIsInitialized;
	}

	inline auto GetErrorCode() const {
		return mErrorCode;
	}

	inline auto& GetErrorMessage() const {
		return mErrorMessage;
	}

	inline auto& GetWSADATA() const {
		return mWSADATA;
	}

	inline ::std::string GetErrorMessageFromErrorCode(const ::std::int32_t errorCode)
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

	inline auto CreateSocket(const ::std::int32_t af, const ::std::int32_t type, const ::std::int32_t protocol) {
		auto socket = ::socket(af, type, protocol);
		if (socket == INVALID_SOCKET) {
			mErrorCode = ::WSAGetLastError();
			mErrorMessage = "Socket creation failed with error: " + GetErrorMessageFromErrorCode(mErrorCode);
		}

		return socket;
	}

	inline void Connect(const ::SOCKET socket, const struct ::sockaddr_in& hint) {
		mErrorCode = ::connect(socket, (struct ::sockaddr*) & hint, sizeof(hint));
		if (mErrorCode == SOCKET_ERROR) {
			mErrorCode = ::WSAGetLastError();
			mErrorMessage = "Socket connection failed with error: " + GetErrorMessageFromErrorCode(mErrorCode);
		}
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