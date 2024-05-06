#pragma once

#include <string>
#include <variant>
#include <map>
#include <utility>
#include <mutex>
#include <vector>

#include "Object/Object.h"
#include "Sound/Sound.h"
#include "GameThread/IGameThread.h"

namespace GameEngine
{
    constexpr int projectileVelocityBossY = 3;
    constexpr int minorProjectileVelocityBossY = 4;
    constexpr int playerShipVelocityX = 10;
    constexpr int projectileVelocityY = 6;

    class LogicFunctions 
    {
    public:
        LogicFunctions() = delete;
        LogicFunctions(const std::shared_ptr<IGameThread>& gameThread) 
            : m_gameThread(std::move(gameThread))
        {}

        void PlayerStartup(GameUtils::Object& obj, const sf::Vector2i& initialPos);
        void PlayerLogic(GameUtils::Object& obj);
        void EnemyStartup(GameUtils::Object& obj, const sf::Vector2i& initialPos);
        void EnemyLogic(GameUtils::Object& obj);
        void ProjectileSetup(GameUtils::Object& obj);
        void ProjectileLogic(GameUtils::Object& obj);
        void EnemyProjectileSetup(GameUtils::Object& obj, const sf::Vector2i& initialPos, const std::pair<GameUtils::ObjectType, int>& assistId);
        void EnemyProjectileLogic(GameUtils::Object& obj);
        void BossStartup(GameUtils::Object& obj, const sf::Vector2i& initialPos);
        void BossLogic(GameUtils::Object& obj);
        void BossProjectileSetup(GameUtils::Object& obj, const sf::Vector2i& initialPos);
        void BossProjectileLogic(GameUtils::Object& obj);

    private:
        std::mutex m_mutex;
        std::shared_ptr<IGameThread> m_gameThread;
        std::chrono::time_point<std::chrono::steady_clock> m_auxiliarTimestamp;
        std::vector<int> m_randomPos;
        int m_enemyQnt;

        std::vector<GameUtils::Object> GetAllObjectByTypes(const std::vector<GameUtils::ObjectType>& types);
        GameUtils::Object& GetObjectReference(const GameUtils::Object& obj);
        void DestroyObject(const GameUtils::Object& obj);
        bool ObjectCollison(GameUtils::Object& obj, const std::vector<GameUtils::ObjectType>& objTypes, const GameUtils::SoundName& soundName, const int& textureRow = 0);
        void RandomShuffler(std::vector<int>& vector, int originalSize);
        void PixelColorSwap(sf::Texture& texture, const std::vector<uint32_t>& oldColors, const int& newColor = -1, const int& cycle = 0);
        uint32_t ColorCycling(const uint32_t& color, const int& cycle);
    };


}