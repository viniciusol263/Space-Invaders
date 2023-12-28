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
        virtual void CreateObject(std::string id = "UNKNOWN", GameUtils::ObjectType objType = GameUtils::ObjectType::PLAYER, 
            std::string texturePath = "", std::string soundPath = "",
            std::function<void(GameUtils::Object&)> startupHandler = [](GameUtils::Object&){}, std::function<void(GameUtils::Object&)> logicHandler = [](GameUtils::Object&){}, 
            std::chrono::milliseconds animationFrametime = 166ms) = 0;
        virtual std::unordered_map<sf::Keyboard::Scancode, std::shared_ptr<GameUtils::Input>>& GetKeys() = 0;
        virtual int& GetScore() = 0;
        virtual void SetScore(int score) = 0;
        virtual void PlayAudioChannel(GameUtils::SoundName soundName) = 0;
        virtual void DoAnimatedAction(GameUtils::Object& obj, bool isLoop = true, std::function<void()> actionFunc = [](){}) = 0;

        virtual void GameWatcherThread() = 0;
    private:
        virtual void InitializeState() = 0;
        virtual void CaptureKeyInput() = 0;
        virtual void PauseLogic() = 0;
        virtual void ExecuteLogic() = 0;
        virtual void DrawSprites() = 0;
        virtual void ClearScreen() = 0;
        virtual void RespawnGame() = 0;
        virtual void CleanupPointers() = 0;


    };

    using IGameThreadPtr = std::shared_ptr<IGameThread>;
}