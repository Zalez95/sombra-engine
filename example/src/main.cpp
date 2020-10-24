#include "game/Game.h"

#if 1
#include <iostream>
#include <cstdlib>

bool PRINT = false;


void* operator new(std::size_t count)
{
	if (PRINT) std::cout << "new(std::size_t " << count << ")" << std::endl;
	return std::malloc(count);
}


void operator delete(void* ptr) noexcept
{
	if (PRINT) std::cout << "delete(void* " << ptr << ")" << std::endl;
	free(ptr);
}


void operator delete(void* ptr, std::size_t size) noexcept
{
	if (PRINT) std::cout << "delete(void* " << ptr << ", size_t " << size << ")" << std::endl;
	free(ptr);
}
#endif


/** Creates and starts the game */
int main()
{
	game::Game miGame;
	miGame.start();
}
