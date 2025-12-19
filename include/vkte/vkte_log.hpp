#pragma once
#if VKTE_LOGGING
#include "spdlog/spdlog.h"
#endif

#define VKTE_CHECKING 1

#if VKTE_LOGGING
#define VKTE_THROW(...) \
{ \
	spdlog::error(__VA_ARGS__); \
	std::string s(__FILE__); \
	s.append(": "); \
	s.append(std::to_string(__LINE__)); \
	spdlog::throw_spdlog_ex(s); \
}
#else
#define VKTE_THROW(...) \
{ \
	throw std::runtime_error(__VA_ARGS__); \
}
#endif

#if VKTE_CHECKING
#define VKTE_ASSERT(X, ...) if (!(X)) VKTE_THROW(__VA_ARGS__);
#define VKTE_CHECK(X, M) vk::detail::resultCheck(X, M)
#else
#define VKTE_ASSERT(X, ...) X
#define VKTE_CHECK(X, M) void(X)
#endif

#if VKTE_LOGGING
#define VKTE_DEBUG(...) spdlog::debug(__VA_ARGS__);
#define VKTE_INFO(...) spdlog::info(__VA_ARGS__);
#define VKTE_WARN(...) spdlog::warn(__VA_ARGS__);
#define VKTE_ERROR(...) spdlog::error(__VA_ARGS__);
#else
#define VKTE_DEBUG(...) void();
#define VKTE_INFO(...) void();
#define VKTE_WARN(...) void();
#define VKTE_ERROR(...) void();
#endif
