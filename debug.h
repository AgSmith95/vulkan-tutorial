#pragma once

#ifdef DEBUG
#include <iostream>

template<typename ...Args>
void dlog(Args && ...args)
{
	(std::cout << ... << args);
	std::cout << '\n';
}

#else

#define dlog(...) 

#endif // DEBUG

