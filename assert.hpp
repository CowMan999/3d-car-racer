#include <exception>
#include <string>

#ifdef NDEBUG
// there is a strange sense of elegance to this generic shitty macro
#define ASSERT(x) \
	if(!(x)) throw std::runtime_error("Assertion failed: " #x " in " __FILE__ " in " +std::string(__PRETTY_FUNCTION__)+ " at line " +std::to_string(__LINE__))
#else
// bad asm hack
#define ASSERT(x) \
	if(!(x)) __asm__ ("int $3")
#endif
	
#pragma once

#include <iostream>
