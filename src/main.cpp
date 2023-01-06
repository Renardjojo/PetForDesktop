#include "Game/Game.hpp"

// Disable usage of external GPU
#ifdef __cplusplus
extern "C"
{
#endif
    __declspec(dllexport) int NvOptimusEnablement                = 0;
    __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 0;

#ifdef __cplusplus
}
#endif

// Disable console
#ifndef _DEBUG
#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
#endif

int main(int argc, char** argv)
{
    Game game;
    game.run();

    return 0;
}