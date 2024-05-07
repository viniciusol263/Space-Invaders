#pragma once

#include <memory>

#include "SFML/Window.hpp"
#include "SFML/Graphics.hpp"
#include "SFML/Audio.hpp"

#include "Object/Object.h"
#include "Input/Input.h"
#include "Sound/Sound.h"
#include "GameUtils/GameUtils.h"


namespace GameEngine 
{
    class IGameThread
    {
    public:
        ~IGameThread() = default;

        virtual std::shared_ptr<sf::RenderWindow> GetRenderWindow() = 0;
        virtual std::vector<GameUtils::Object>& GetObjects() = 0;
        virtual GameUtils::Object& CreateObject(const std::string& id = "UNKNOWN", const GameUtils::ObjectType& objType = GameUtils::ObjectType::PLAYER, 
            const std::string& texturePath = "", const std::string& soundPath = "",
            const std::function<void(GameUtils::Object&)>& startupHandler = [](GameUtils::Object&){}, const std::function<void(GameUtils::Object&)>& logicHandler = [](GameUtils::Object&){}, 
            const std::chrono::milliseconds& animationFrametime = 166ms, const int& hitPoints = 1, const int& scorePoint = 1) = 0;
        virtual GameUtils::Object& CreateObjectAnimated(const std::string& id = "UNKNOWN", const GameUtils::ObjectType& objType = GameUtils::ObjectType::PLAYER, 
            const std::string& texturePath = "", const std::string& soundPath = "",
            const std::function<void(GameUtils::Object&)>& startupHandler = [](GameUtils::Object&){}, const std::function<void(GameUtils::Object&)>& logicHandler = [](GameUtils::Object&){}, 
            const std::chrono::milliseconds& animationFrametime = 166ms, const int& hitPoints = 1, const int& scorePoint = 1, const int& textureRow = 0, const bool& isLoop = true) = 0;
        virtual void DestroyObject(const GameUtils::Object& obj) = 0;
        virtual void DestroyObjectAnimated(const GameUtils::Object& obj, const int& textureRow) = 0;
        virtual std::unordered_map<sf::Keyboard::Scancode, std::shared_ptr<GameUtils::Input>>& GetKeys() = 0;
        virtual int& GetScore() = 0;
        virtual void SetScore(const int& score) = 0;
        virtual void PlayAudioChannel(const GameUtils::SoundName& soundName) = 0;

        virtual void GameWatcherThread() = 0;
    private:
        virtual void InitializeState() = 0;
        virtual void RenderStage() = 0;
        virtual void MenuScreen() = 0;
        virtual void CaptureKeyInput() = 0;
        virtual void PauseLogic() = 0;
        virtual void ExecuteLogic() = 0;
        virtual void DrawSprites() = 0;
        virtual void ClearScreen() = 0;
        virtual void ProgressionCheck() = 0;
        virtual void CleanupGame() = 0;
        virtual void RenderText() = 0;


    };

    using IGameThreadPtr = std::shared_ptr<IGameThread>;
}