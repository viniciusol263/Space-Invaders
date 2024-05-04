#pragma once

#include <string>
#include <functional>
#include <chrono>
#include <memory>
#include <array>
#include <map>

#include "SFML/Window.hpp"
#include "SFML/Graphics.hpp"
#include "SFML/Audio.hpp"

using namespace std::chrono_literals;

namespace GameUtils
{
    enum class ObjectType : int
    {
        PLAYER = 0,
        PROJECTILE,
        ENEMY,
        ENEMY_PROJECTILE,
        BOSS
    };

    static std::string ObjectTypeToString(const ObjectType& type) 
    {
        switch(type)
        {
            case ObjectType::PLAYER: return "Player";
            case ObjectType::PROJECTILE: return "Projectile";
            case ObjectType::ENEMY: return "Enemy";
            case ObjectType::ENEMY_PROJECTILE: return "Enemy Projectile";
            case ObjectType::BOSS: return "Boss";
            default: return "";
        }
    }

    class Object 
    {
    public:
        Object(const std::string& id = "UNKNOWN", 
            const ObjectType& objType = ObjectType::PLAYER, 
            const std::string& texturePath = "", 
            const std::string& soundPath = "", 
            const std::function<void(GameUtils::Object&)>& startupHandler = [](GameUtils::Object&){}, 
            const std::function<void(GameUtils::Object&)>& logicHandler = [](GameUtils::Object&){}, 
            const std::chrono::milliseconds& animationFrametime = 166ms, const int& hitPoints = 1);

        ~Object() = default;

        constexpr bool operator==(const Object& other) const 
        {
            return (m_id == other.m_id) &&
                    (m_objType == other.m_objType);
        }


        static std::pair<int,int> TextureSizeFromObjectType(const ObjectType& type);

        std::string GetId() const;
        sf::Sprite& GetSprite();
        std::string GetDefaultSoundFilePath();
        sf::Sound& GetSound();
        ObjectType GetType() const;
        std::chrono::milliseconds GetAnimationFrametime() const;
        void SetTimer(const std::chrono::milliseconds& time, const bool& continous);
        bool TimerOverflown();
        int GetHitPoints() const;
        void SetHitPoints(const int& value);
        bool GetAnimRunning();
        void StepLogic();
        void SetupAnimatedAction(const int& textureRow, const bool& isLoop, const bool& destroyOnFinish = false, const std::function<void()>& destroyAction = []{});
        void DoAnimatedAction();
        void StopAnimatedAction();
        bool GetDestroy();
        std::map<std::string, int>& GetAuxiliarVars();
        std::chrono::time_point<std::chrono::steady_clock>& GetAuxiliarTimeStamp();

    private:
        std::string m_id;
        std::string m_soundBufferPath;
        ObjectType m_objType;
        std::function<void(GameUtils::Object&)> m_startupHandler;
        std::function<void(GameUtils::Object&)> m_logicHandler;
        std::shared_ptr<sf::Texture> m_objTexture;
        std::shared_ptr<sf::SoundBuffer> m_objSoundBuffer;
        sf::Sprite m_objSprite;
        sf::Sound m_objSound;
        std::chrono::milliseconds m_animationFrametime;
        std::shared_ptr<std::tuple<std::chrono::milliseconds, std::chrono::steady_clock::time_point, bool>> m_timer;
        int m_hitPoints;
        std::map<std::string, int> m_auxiliarVariables;
        std::chrono::time_point<std::chrono::steady_clock> m_auxiliarTimestamp;


        //TODO Animation class?
        // -------------------------------
        bool m_animRunning = false;
        sf::IntRect m_currentRenderRect;
        sf::Vector2u m_textureSize;
        sf::Vector2i m_renderRectSize;
        unsigned int m_frameQuantity;
        unsigned int m_animationHead;
        unsigned int m_animationStep;
        std::chrono::_V2::steady_clock::time_point m_animationStartTime;
        int m_textureRow; 
        bool m_isLoop;
        bool m_destroyOnFinish;
        bool m_destroy;
        std::function<void()> m_destroyAction;
        // --------------------------------
    };
}