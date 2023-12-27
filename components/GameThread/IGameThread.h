#pragma once

#include <memory>

#include "SFML/Window.hpp"
#include "SFML/Graphics.hpp"
#include "SFML/Audio.hpp"
#include "GameUtils/GameUtils.h"


namespace GameEngine 
{
    class IGameThread
    {
    public:
        ~IGameThread() = default;

        virtual std::shared_ptr<sf::RenderWindow> GetRenderWindow() = 0;
        virtual std::vector<GameUtils::Object>& GetObjects() = 0;
        virtual std::unordered_map<sf::Keyboard::Scancode, std::shared_ptr<GameUtils::Input>>& GetKeys() = 0;
        virtual void PlayAudioChannel(GameUtils::SoundName soundName) = 0;
        virtual void DoSpriteAnimation(GameUtils::Object& obj, sf::Vector2i newSpritePos) = 0;

        virtual void GameWatcherThread() = 0;
    private:
        virtual void InitializeState() = 0;
        virtual void DrawSprites() = 0;
        virtual void CaptureKeyInput() = 0;
        virtual void ExecuteLogic() = 0;
        virtual void ClearScreen() = 0;


    };

    using IGameThreadPtr = std::shared_ptr<IGameThread>;
}