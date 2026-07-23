/* 
 * Small functions
**/

#pragma once


#define __LOOP_CONCAT_INTERNAL(a, b) a ## b
#define __LOOP_CONCAT(a, b) __LOOP_CONCAT_INTERNAL(a, b)
#define LOOP(n) for (int __LOOP_CONCAT(_loop_nvar_, __LINE__) = 0; __LOOP_CONCAT(_loop_nvar_, __LINE__) < n; ++__LOOP_CONCAT(_loop_nvar_, __LINE__))


#include <string>
#include <atomic>


inline std::atomic<bool> RUNNING(true);

void signalHandler(int signal);

// yes i need this
std::string concat(const std::string& str1, const char* str2);

// inclusive
template <typename T>
inline constexpr void clamp(T& num, const T& lower, const T& upper) {
		 if (num < lower) num = lower;
	else if (num > upper) num = upper;
}

template <typename T>
inline constexpr T max(const T& num1, const T& num2) {
	return num1 > num2 ? num1 : num2;
}
template <typename T>
inline constexpr T min(const T& num1, const T& num2) {
	return num1 < num2 ? num1 : num2;
}