#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

#include "GameThread/GameThread.h"

int main()
{
    std::shared_ptr<sf::RenderWindow> window = std::make_shared<sf::RenderWindow>(sf::VideoMode(800,600), "Space Invader");
    GameEngine::GameThread gameThread(window);
    gameThread.GameWatcherThread();
    
    return 0;
}