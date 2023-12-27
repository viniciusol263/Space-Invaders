#pragma once

#include <unordered_map>
#include <memory>
#include <chrono>
#include <vector>
#include <utility>

#include "IGameThread.h"
#include "LogicFunctions/LogicFunctions.h"

namespace GameEngine
{
    constexpr std::pair textureSize{32,32};
    
    class GameThread : public IGameThread
    {
    public:
        ~GameThread() = default;

        explicit GameThread(std::shared_ptr<sf::RenderWindow> window);

        std::shared_ptr<sf::RenderWindow> GetRenderWindow() override;
        std::vector<GameUtils::Object>& GetObjects() override;
        std::unordered_map<sf::Keyboard::Scancode, std::shared_ptr<GameUtils::Input>>& GetKeys() override;
        void PlayAudioChannel(GameUtils::SoundName soundName) override;
        void DoAnimatedAction(GameUtils::Object& obj, std::vector<sf::Vector2i> newSpritePos, bool isLoop = true, std::function<void()> actionFunc = [](){}) override;

        void GameWatcherThread() override;
    private: 
        GameThread() = default;
        std::shared_ptr<sf::RenderWindow> m_window;
        std::shared_ptr<sf::Texture> backgroundTexture;
        std::shared_ptr<std::future<void>> m_auxThread;
        std::unordered_map<std::string,std::pair<std::shared_ptr<sf::SoundBuffer>, sf::Sound>> m_generalSoundChannels;
        sf::Sprite backgroundSprite;
        
        std::vector<GameUtils::Object> m_objects;
        std::unordered_map<sf::Keyboard::Scancode, std::shared_ptr<GameUtils::Input>> m_keyMaps;
        std::shared_ptr<LogicFunctions> m_logicFunction;

        
        sf::Event m_keyboardEvent;

        std::chrono::time_point<std::chrono::steady_clock> m_lastFrameTime;

        void InitializeState() override;
        void DrawSprites() override;
        void CaptureKeyInput() override;
        void ExecuteLogic() override;
        void ClearScreen() override;
        
        void GenerateSoundChannels();
        void CreateArrayObject(int rows, int columns, std::function<GameUtils::Object(sf::Vector2i, std::string)> objectBuilder);

    };

    using GameThreadPtr = std::shared_ptr<GameThread>;
}