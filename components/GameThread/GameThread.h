#pragma once

#include <unordered_map>
#include <memory>
#include <chrono>
#include <vector>
#include <utility>
#include <mutex>
#include <thread>
#include <tuple>

#include "IGameThread.h"
#include "LogicFunctions/LogicFunctions.h"

namespace GameEngine
{
    class GameThread : public IGameThread
    {
    public:
        ~GameThread();

        explicit GameThread(const std::shared_ptr<sf::RenderWindow>& window);

        std::shared_ptr<sf::RenderWindow> GetRenderWindow() override;
        std::vector<GameUtils::Object>& GetObjects() override;
        GameUtils::Object& CreateObject(const std::string& id = "UNKNOWN", const GameUtils::ObjectType& objType = GameUtils::ObjectType::PLAYER, 
            const std::string& texturePath = "", const std::string& soundPath = "",
            const std::function<void(GameUtils::Object&)>& startupHandler = [](GameUtils::Object&){}, const std::function<void(GameUtils::Object&)>& logicHandler = [](GameUtils::Object&){}, 
            const std::chrono::milliseconds& animationFrametime = 166ms, const int& hitPoints = 1) override;
        std::unordered_map<sf::Keyboard::Scancode, std::shared_ptr<GameUtils::Input>>& GetKeys() override;
        GameUtils::Object& CreateObjectAnimated(const std::string& id = "UNKNOWN", const GameUtils::ObjectType& objType = GameUtils::ObjectType::PLAYER, 
            const std::string& texturePath = "", const std::string& soundPath = "",
            const std::function<void(GameUtils::Object&)>& startupHandler = [](GameUtils::Object&){}, const std::function<void(GameUtils::Object&)>& logicHandler = [](GameUtils::Object&){}, 
            const std::chrono::milliseconds& animationFrametime = 166ms, const int& hitPoints = 1, const int& textureRow = 0, const bool& isLoop = true) override;
        void DestroyObject(const GameUtils::Object& obj) override;
        void DestroyObjectAnimated(const GameUtils::Object& obj, const int& textureRow) override;
        int& GetScore() override;
        void SetScore(const int& score) override;
        void PlayAudioChannel(const GameUtils::SoundName& soundName) override;

        void GameWatcherThread() override;
    private: 
        GameThread() = default;

        std::shared_ptr<sf::RenderWindow> m_window;
        std::shared_ptr<sf::Texture> backgroundTexture;
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
        GameUtils::Progression m_progression;

        sf::Event m_keyboardEvent;

        std::chrono::time_point<std::chrono::steady_clock> m_lastFrameTime;

        void InitializeState() override;
        void CaptureKeyInput() override;
        void PauseLogic() override;
        void ExecuteLogic() override;
        void DrawSprites() override;
        void ClearScreen() override;
        void ProgressionCheck() override;
        void CleanupGame() override;

        void GenerateSoundChannels();
        void CreateArrayObject(const int& rows, const int& columns, const std::function<GameUtils::Object(sf::Vector2i, std::string)>& objectBuilder);
        void BlockingTextScreen(const std::string& text);
        std::tuple<int,int,int> GetLiveObjects(); 

    };

    using GameThreadPtr = std::shared_ptr<GameThread>;
}