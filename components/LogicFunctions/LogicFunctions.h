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
    constexpr int projectileVelocityY = 7;
    constexpr auto projectileMapIndex = std::make_pair(GameUtils::ObjectType::PROJECTILE, 0);
    constexpr auto enemyProjectileMapIndex = std::make_pair(GameUtils::ObjectType::ENEMY_PROJECTILE, 0);
    constexpr auto bossMapIndex = std::make_pair(GameUtils::ObjectType::BOSS, 0);

    struct LogicAssist
    {
        std::vector<int> auxVariables;
    };

    static std::map<GameUtils::ObjectType, LogicAssist> DefaultAssists = {{GameUtils::ObjectType::ENEMY, {{0, 3, 0}}}, 
        {GameUtils::ObjectType::PROJECTILE, {{0,0}}}, 
        {GameUtils::ObjectType::ENEMY_PROJECTILE, {{0,0,0}}}, 
        {GameUtils::ObjectType::BOSS, {{0,6,0}}}};

    class LogicFunctions 
    {
    public:
        LogicFunctions() = delete;
        LogicFunctions(const std::shared_ptr<IGameThread>& gameThread) 
            : m_gameThread(std::move(gameThread))
        {
            //Logic Assist for Enemies
           for(int index = 0; index <= 4*4; ++index)
           {
            auto number = std::make_pair(GameUtils::ObjectType::ENEMY, index);
            m_logicAssists[number] = DefaultAssists[GameUtils::ObjectType::ENEMY];
           }

           //Logic Assist for Projectile
           m_logicAssists[projectileMapIndex] = DefaultAssists[GameUtils::ObjectType::PROJECTILE];

           //Logic Assist for Enemy Projectile
           m_logicAssists[enemyProjectileMapIndex] = DefaultAssists[GameUtils::ObjectType::ENEMY_PROJECTILE];

           //Logic Assist for Boss
           m_logicAssists[bossMapIndex] = DefaultAssists[GameUtils::ObjectType::BOSS];
        }

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

    private:
        std::mutex m_mutex;
        std::shared_ptr<IGameThread> m_gameThread;
        std::map<std::pair<GameUtils::ObjectType, int>, LogicAssist> m_logicAssists;
        std::chrono::time_point<std::chrono::steady_clock> m_auxiliarTimestamp;
        std::vector<int> m_randomPos;
        int m_enemyQnt;

        std::vector<GameUtils::Object> GetAllObjectByTypes(const std::vector<GameUtils::ObjectType>& types);
        GameUtils::Object& GetObjectReference(const GameUtils::Object& obj);
        void DestroyObject(const GameUtils::Object& obj);
        bool ObjectCollison(GameUtils::Object& obj, const std::vector<GameUtils::ObjectType>& objTypes, const std::pair<GameUtils::ObjectType, int>& logicAssist, const GameUtils::SoundName& soundName, const int& textureRow = 0);
        void RandomShuffler(std::vector<int>& vector, int originalSize);
        void PixelColorSwap(sf::Texture& texture, const std::vector<uint32_t>& oldColors, const int& newColor = -1, const int& cycle = 0);
        uint32_t ColorCycling(const uint32_t& color, const int& cycle);
    };


}