#pragma once

#define WIN32_LEAN_AND_MEAN

#include <string>

#include <WinSock2.h>
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

	inline auto& GetWSADATA() const {
		return mWSADATA;
	}

	inline ::std::string GetErrorMessageFromCode(const ::std::int32_t errorCode)
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
			mErrorMessage = "Windows Sockets API initialization failed with error: " + GetErrorMessageFromCode(mErrorCode);
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
			mErrorMessage = "Windows Sockets API uninitialization failed with error: " + GetErrorMessageFromCode(mErrorCode);
		}
	}

	static inline ::std::uint16_t mCount = 0;
	static inline bool mIsInitialized = false;
	static inline ::WSADATA mWSADATA{};
	static inline ::std::int32_t mErrorCode = 0;
	static inline ::std::string mErrorMessage = "No errors occured.";
};