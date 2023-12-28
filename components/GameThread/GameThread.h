#pragma once

#include <unordered_map>
#include <memory>
#include <chrono>
#include <vector>
#include <utility>
#include <mutex>
#include <thread>

#include "IGameThread.h"
#include "LogicFunctions/LogicFunctions.h"

namespace GameEngine
{
    class GameThread : public IGameThread
    {
    public:
        ~GameThread() = default;

        explicit GameThread(std::shared_ptr<sf::RenderWindow> window);

        std::shared_ptr<sf::RenderWindow> GetRenderWindow() override;
        std::vector<GameUtils::Object>& GetObjects() override;
        void CreateObject(std::string id = "UNKNOWN", GameUtils::ObjectType objType = GameUtils::ObjectType::PLAYER, 
            std::string texturePath = "", std::string soundPath = "",
            std::function<void(GameUtils::Object&)> startupHandler = [](GameUtils::Object&){}, std::function<void(GameUtils::Object&)> logicHandler = [](GameUtils::Object&){}, 
            std::chrono::milliseconds animationFrametime = 166ms) override;
        std::unordered_map<sf::Keyboard::Scancode, std::shared_ptr<GameUtils::Input>>& GetKeys() override;
        int& GetScore() override;
        void SetScore(int score) override;
        void PlayAudioChannel(GameUtils::SoundName soundName) override;
        void DoAnimatedAction(GameUtils::Object& obj, int textureRow, bool isLoop = true, std::function<void()> actionFunc = [](){}) override;

        void GameWatcherThread() override;
    private: 
        GameThread() = default;

        std::mutex m_mutex;
        std::shared_ptr<sf::RenderWindow> m_window;
        std::shared_ptr<sf::Texture> backgroundTexture;
        std::vector<std::shared_ptr<std::thread>> m_auxThreads;
        std::unordered_map<std::string,std::pair<std::shared_ptr<sf::SoundBuffer>, sf::Sound>> m_generalSoundChannels;
        sf::Sprite backgroundSprite;
        sf::Font m_font;
        std::map<GameUtils::TextType, sf::Text> m_textSprites;
        std::vector<GameUtils::Object> m_objects;
        std::unordered_map<sf::Keyboard::Scancode, std::shared_ptr<GameUtils::Input>> m_keyMaps;
        std::shared_ptr<LogicFunctions> m_logicFunction;
        int m_score = 0;
        int m_highscore = 0;
        int m_paused = 0;

        
        sf::Event m_keyboardEvent;

        std::chrono::time_point<std::chrono::steady_clock> m_lastFrameTime;

        void InitializeState() override;
        void CaptureKeyInput() override;
        void PauseLogic() override;
        void ExecuteLogic() override;
        void DrawSprites() override;
        void ClearScreen() override;
        void RespawnGame() override;
        void CleanupPointers() override;
        void CleanupGame() override;

        void GenerateSoundChannels();
        void CreateArrayObject(int rows, int columns, std::function<GameUtils::Object(sf::Vector2i, std::string)> objectBuilder);
        void BlockingTextScreen(std::string text);

    };

    using GameThreadPtr = std::shared_ptr<GameThread>;
}