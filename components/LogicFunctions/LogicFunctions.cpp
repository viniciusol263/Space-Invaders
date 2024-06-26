#include <cmath>
#include <iostream>
#include <random>
#include <functional>

#include "LogicFunctions.h"
#include "Input/Input.h"
#include "GameThread/GameThread.h"
#include "GameUtils/GameUtils.h"

namespace GameEngine
{

    void LogicFunctions::PlayerStartup(GameUtils::Object& obj, const sf::Vector2i& initialPos)
    {
        auto posX = std::abs(initialPos.x - obj.GetSprite().getTextureRect().getSize().x/2);
        auto posY = std::abs(initialPos.y - obj.GetSprite().getTextureRect().getSize().y/2);
 
        obj.GetSprite().setPosition(posX, posY);
    }

    void LogicFunctions::PlayerLogic(GameUtils::Object& obj)
    {
        //Horizontal Movement
        auto left = (m_gameThread->GetKeys()[sf::Keyboard::Scancode::A]->GetPressed()) ? -1 : 0;
        auto right = (m_gameThread->GetKeys()[sf::Keyboard::Scancode::D]->GetPressed()) ? 1 : 0;
        auto projectile = m_gameThread->GetKeys()[sf::Keyboard::Scancode::Space]->GetPressed();

        auto nextPosition = obj.GetSprite().getPosition().x + ((left + right) * playerShipVelocityX);
        if(nextPosition > (m_gameThread->GetRenderWindow()->getSize().x - obj.GetSprite().getLocalBounds().getSize().x)) nextPosition -= playerShipVelocityX;
        if(nextPosition < 0) nextPosition += playerShipVelocityX;
        obj.GetSprite().setPosition(nextPosition, obj.GetSprite().getPosition().y);

        auto ticks = static_cast<int>(obj.GetSprite().getPosition().y/(2*projectileVelocityY));
        //Projectile instantiation
        if(projectile && GameUtils::IsExpired(obj.GetAuxiliarTimeStamp()["Primary"], ticks))
        {
            obj.GetAuxiliarTimeStamp()["Primary"] = std::chrono::steady_clock::now();
            obj.SetupAnimatedAction(0, false);
            m_gameThread->CreateObject("1", GameUtils::ObjectType::PROJECTILE, "../resources/texture/animated-projectile.png", "../resources/sfx/player-shot.wav",
                std::bind(LogicFunctions::ProjectileSetup, this, std::placeholders::_1),
                std::bind(LogicFunctions::ProjectileLogic, this, std::placeholders::_1), 150ms, 1);
        }
    }
    void LogicFunctions::EnemyStartup(GameUtils::Object& obj, const sf::Vector2i& initialPos)
    {
        auto colorSeparator = stoi(obj.GetId()) / GameUtils::enemyQuantity[0];
        std::vector<uint32_t> colors({GameUtils::red, GameUtils::darkerRed});
        PixelColorSwap(*obj.GetTexture(), colors, -1, colorSeparator);

        obj.GetAuxiliarVars()["movementCounter"] = 0; // Orientation X-Axis counter
        obj.GetAuxiliarVars()["baseVelocityX"] = 3; // Base velocity of the enemy ship on X-Axis

        m_enemyQnt = GameUtils::enemyQuantity[0]*GameUtils::enemyQuantity[1];
        for(auto index = 0; index < m_enemyQnt; ++index)
            m_randomPos.push_back(index);
        auto posX = std::abs(initialPos.x - obj.GetSprite().getTextureRect().getSize().x/2);
        auto posY = std::abs(initialPos.y - obj.GetSprite().getTextureRect().getSize().y/2);
 
        obj.GetSprite().setPosition(posX, posY);
        obj.SetTimer(obj.GetAnimationFrametime(), true);
    }

    void LogicFunctions::EnemyLogic(GameUtils::Object& obj)
    {
        auto enemyInstance = std::make_pair(GameUtils::ObjectType::ENEMY, stoi(obj.GetId()));

        if(obj.GetAuxiliarVars()["movementCounter"]++ == 30)
        {
            obj.GetAuxiliarVars()["baseVelocityX"] = -obj.GetAuxiliarVars()["baseVelocityX"];
            obj.GetAuxiliarVars()["movementCounter"] = 0;
        }
        int nextPosition = obj.GetSprite().getPosition().x - obj.GetAuxiliarVars()["baseVelocityX"];
        if(nextPosition > m_gameThread->GetRenderWindow()->getSize().x) nextPosition -= obj.GetAuxiliarVars()["baseVelocityX"];
        if(nextPosition <= 0) nextPosition += obj.GetAuxiliarVars()["baseVelocityX"];
        obj.GetSprite().setPosition(nextPosition, obj.GetSprite().getPosition().y);

        auto position = obj.GetSprite().getPosition();

        RandomShuffler(m_randomPos, m_enemyQnt);

        auto playerPosition = std::find_if(m_gameThread->GetObjects().begin(), m_gameThread->GetObjects().end(), [](const GameUtils::Object& obj){
            return obj.GetType() == GameUtils::ObjectType::PLAYER;
        })->GetSprite().getPosition();
        auto ticks = static_cast<int>((playerPosition.y - position.y)/(2*projectileVelocityY));

        if(stoi(obj.GetId()) == m_randomPos.back() && GameUtils::IsExpired(m_auxiliarTimestamp, ticks))
        {
            m_randomPos.pop_back();
            m_auxiliarTimestamp = std::chrono::steady_clock::now();
            m_gameThread->CreateObject("1", GameUtils::ObjectType::ENEMY_PROJECTILE, "../resources/texture/animated-enemy-projectile.png", "../resources/sfx/enemy-shot.wav",
                std::bind(LogicFunctions::EnemyProjectileSetup, this, std::placeholders::_1, sf::Vector2i{position.x,position.y}, enemyInstance),
                std::bind(LogicFunctions::EnemyProjectileLogic, this, std::placeholders::_1), 150ms, 1);
        }


    }

    void LogicFunctions::ProjectileSetup(GameUtils::Object& obj)
    {
        auto playerPosition = std::find_if(m_gameThread->GetObjects().begin(), m_gameThread->GetObjects().end(), [](const GameUtils::Object& obj){
            return obj.GetType() == GameUtils::ObjectType::PLAYER;
        });

        obj.GetAuxiliarVars()["pinMovement"] = 0;
        m_gameThread->PlayAudioChannel(GameUtils::SoundName::PLAYER_SHOT); 
        obj.GetSprite().setPosition(playerPosition->GetSprite().getPosition().x, playerPosition->GetSprite().getPosition().y - obj.GetSprite().getGlobalBounds().getSize().y);
    }

    void LogicFunctions::ProjectileLogic(GameUtils::Object& obj)
    {
        auto currentPosition = obj.GetSprite().getPosition();

        if(currentPosition.y >= 0)
        {
            if(obj.GetAuxiliarVars()["pinMovement"] == 0)
                obj.GetSprite().setPosition(currentPosition.x, currentPosition.y - projectileVelocityY);
        }
        else 
        {
            DestroyObject(obj);
            return;
        }

        ObjectCollison(obj, {GameUtils::ObjectType::ENEMY, GameUtils::ObjectType::BOSS}, GameUtils::SoundName::ENEMY_DEATH);
    }

    void LogicFunctions::EnemyProjectileSetup(GameUtils::Object& obj, const sf::Vector2i& initialPos, const std::pair<GameUtils::ObjectType, int>& assistId)
    {
        if(obj.GetType() == GameUtils::ObjectType::BOSS_PROJECTILE)
        {
            std::vector<uint32_t> colors({0xAC3232FF,0xDF7126FF,0xFBF236FF});
            PixelColorSwap(*obj.GetTexture(), colors, -1, 0);
        }

        auto playerPosition = std::find_if(m_gameThread->GetObjects().begin(), m_gameThread->GetObjects().end(), [](const GameUtils::Object& obj){
            return obj.GetType() == GameUtils::ObjectType::PLAYER;
        })->GetSprite().getPosition();
        auto ticks = static_cast<int>((playerPosition.y - initialPos.y)/projectileVelocityY);
        auto projectileVelX = static_cast<int>(std::ceil((playerPosition.x - initialPos.x)/ticks));

        obj.GetAuxiliarVars()["Direction"] = 0;
        obj.GetAuxiliarVars()["pinMovement"] = 0;
        obj.GetAuxiliarVars()["mutableProjectileVelocity"] = projectileVelocityY;
        obj.GetAuxiliarVars()["maximumSpacement"] = playerPosition.y - initialPos.y;

        m_gameThread->PlayAudioChannel(GameUtils::SoundName::ENEMY_SHOT); 
        if(static_cast<int>(playerPosition.x - initialPos.x) != 0)
            obj.GetAuxiliarVars()["Direction"] = projectileVelX;


        obj.GetSprite().setPosition(initialPos.x, initialPos.y + obj.GetSprite().getGlobalBounds().getSize().y);
    }

    void LogicFunctions::EnemyProjectileLogic(GameUtils::Object& obj)
    {
        auto currentPosition = obj.GetSprite().getPosition();

        if(obj.GetType() == GameUtils::ObjectType::BOSS_PROJECTILE)
        {
            obj.GetAuxiliarVars()["mutableProjectileVelocity"] = minorProjectileVelocityBossY;
            auto playerPosition = std::find_if(m_gameThread->GetObjects().begin(), m_gameThread->GetObjects().end(), [](const GameUtils::Object& obj){
                    return obj.GetType() == GameUtils::ObjectType::PLAYER;
                })->GetSprite().getPosition();
            if(std::abs(static_cast<int>(playerPosition.y - currentPosition.y)) <= (obj.GetAuxiliarVars()["maximumSpacement"]/7))
            {
                obj.GetAuxiliarVars()["mutableProjectileVelocity"] = 3;
            }
        }
        if(currentPosition.y < m_gameThread->GetRenderWindow()->getDefaultView().getSize().y)
        {
            if(obj.GetAuxiliarVars()["pinMovement"] == 0)
                obj.GetSprite().setPosition(currentPosition.x + obj.GetAuxiliarVars()["Direction"], currentPosition.y + obj.GetAuxiliarVars()["mutableProjectileVelocity"]);
        }
        else 
        {
            DestroyObject(obj);
            return;
        }

        ObjectCollison(obj, {GameUtils::ObjectType::PLAYER}, GameUtils::SoundName::ENEMY_DEATH, 1);
    }
    
    void LogicFunctions::BossStartup(GameUtils::Object& obj, const sf::Vector2i& initialPos)
    {
        auto posX = std::abs(initialPos.x - obj.GetSprite().getTextureRect().getSize().x/2);
        auto posY = std::abs(initialPos.y - obj.GetSprite().getTextureRect().getSize().y/2);
 
        obj.GetAuxiliarTimeStamp()["Second"] = std::chrono::steady_clock::now();
        obj.GetAuxiliarVars()["Direction"] = 6;
        obj.GetAuxiliarVars()["BulletPosition"] = 0;
        obj.GetAuxiliarVars()["0"] = 0;
        obj.GetAuxiliarVars()["1"] = -obj.GetSprite().getLocalBounds().getSize().x/2;
        obj.GetAuxiliarVars()["2"] = obj.GetSprite().getLocalBounds().getSize().x/2;

        obj.GetSprite().setPosition(posX, posY);
        obj.SetTimer(obj.GetAnimationFrametime(), true);
    }

    void LogicFunctions::BossLogic(GameUtils::Object& obj)
    {   
        const int movementRange = (m_gameThread->GetRenderWindow()->getDefaultView().getSize().x/4);

        if(obj.GetSprite().getPosition().x <= (movementRange - (obj.GetSprite().getTextureRect().getSize().x/2)) || obj.GetSprite().getPosition().x >= (3*movementRange))
        {
            obj.GetAuxiliarVars()["Direction"] = -obj.GetAuxiliarVars()["Direction"];
        }
        int nextPosition = obj.GetSprite().getPosition().x - obj.GetAuxiliarVars()["Direction"];
        if(nextPosition > m_gameThread->GetRenderWindow()->getSize().x) nextPosition = 0;
        if(nextPosition <= 0) nextPosition = m_gameThread->GetRenderWindow()->getSize().x;
        obj.GetSprite().setPosition(nextPosition, obj.GetSprite().getPosition().y);

        auto position = obj.GetSprite().getPosition();

        auto playerPosition = std::find_if(m_gameThread->GetObjects().begin(), m_gameThread->GetObjects().end(), [](const GameUtils::Object& obj){
            return obj.GetType() == GameUtils::ObjectType::PLAYER;
        })->GetSprite().getPosition();
        auto ticks = static_cast<int>((playerPosition.y - position.y)/(2*projectileVelocityY));
        auto minorTicks = ticks / 2;
        if(GameUtils::IsExpired(obj.GetAuxiliarTimeStamp()["Second"], minorTicks))
        {
            obj.SetupAnimatedAction(2, false, false, true, 1);
            obj.GetAuxiliarTimeStamp()["Second"] = std::chrono::steady_clock::now();
            m_gameThread->CreateObject("2", GameUtils::ObjectType::BOSS_PROJECTILE, "../resources/texture/animated-enemy-projectile.png", "",
                std::bind(LogicFunctions::EnemyProjectileSetup, this, std::placeholders::_1, sf::Vector2i{position.x + (obj.GetSprite().getLocalBounds().getSize().x/2) + obj.GetAuxiliarVars()[std::to_string(obj.GetAuxiliarVars()["BulletPosition"])], position.y + (obj.GetSprite().getLocalBounds().getSize().y/2)}, std::make_pair<GameUtils::ObjectType,int>(GameUtils::ObjectType::UNKNOWN,0)),
                std::bind(LogicFunctions::EnemyProjectileLogic, this, std::placeholders::_1), 200ms, 1);
            obj.GetAuxiliarVars()["BulletPosition"] = (obj.GetAuxiliarVars()["BulletPosition"] + 1) % 3;
        }
        if(GameUtils::IsExpired(obj.GetAuxiliarTimeStamp()["Primary"], ticks))
        {
            obj.SetupAnimatedAction(2, false, false, true, 1);
            obj.GetAuxiliarTimeStamp()["Primary"] = std::chrono::steady_clock::now();
            m_gameThread->CreateObject("1", GameUtils::ObjectType::BOSS_PROJECTILE, "../resources/texture/animated-boss-projectile.png", "../resources/sfx/enemy-shot.wav",
                std::bind(LogicFunctions::BossProjectileSetup, this, std::placeholders::_1, sf::Vector2i{position.x + (obj.GetSprite().getLocalBounds().getSize().x/2), position.y + (obj.GetSprite().getLocalBounds().getSize().y/2)}),
                std::bind(LogicFunctions::BossProjectileLogic, this, std::placeholders::_1), 200ms, 1);
        }

    }

    void LogicFunctions::BossProjectileSetup(GameUtils::Object& obj, const sf::Vector2i& initialPos)
    {
        auto playerPosition = std::find_if(m_gameThread->GetObjects().begin(), m_gameThread->GetObjects().end(), [](const GameUtils::Object& obj){
                return obj.GetType() == GameUtils::ObjectType::PLAYER;
            })->GetSprite().getPosition();

        obj.GetAuxiliarVars()["Direction"] = 0;
        obj.GetAuxiliarVars()["pinMovement"] = 0;
        obj.GetAuxiliarVars()["maximumSpacement"] = playerPosition.y - initialPos.y;

        m_gameThread->PlayAudioChannel(GameUtils::SoundName::ENEMY_SHOT); 
        obj.GetSprite().setPosition(initialPos.x, initialPos.y);
    }

    void LogicFunctions::BossProjectileLogic(GameUtils::Object& obj)
    {
        auto currentPosition = obj.GetSprite().getPosition();

        if(currentPosition.y < m_gameThread->GetRenderWindow()->getDefaultView().getSize().y)
        {
            if(obj.GetAuxiliarVars()["pinMovement"] == 0)
            {
                std::random_device dev;
                std::mt19937 rng(dev());
                std::uniform_int_distribution<int> deviationAdditive(5,10);

                auto playerPosition = std::find_if(m_gameThread->GetObjects().begin(), m_gameThread->GetObjects().end(), [](const GameUtils::Object& obj){
                        return obj.GetType() == GameUtils::ObjectType::PLAYER;
                    })->GetSprite().getPosition();
                auto ticks = static_cast<int>((playerPosition.y - currentPosition.y)/projectileVelocityBossY);
                auto projectileVelX = static_cast<int>(std::ceil((playerPosition.x - currentPosition.x)/ticks));

                obj.GetAuxiliarVars()["Direction"] = projectileVelX;

                if(std::abs(static_cast<int>(playerPosition.y - currentPosition.y)) <= (obj.GetAuxiliarVars()["maximumSpacement"]/7))
                {
                    obj.GetAuxiliarVars()["Direction"] = 0;

                }

                obj.GetSprite().setPosition(currentPosition.x + obj.GetAuxiliarVars()["Direction"], currentPosition.y + projectileVelocityBossY);
            }
        }
        else 
        {
            DestroyObject(obj);
            return;
        }

        ObjectCollison(obj, {GameUtils::ObjectType::PLAYER}, GameUtils::SoundName::ENEMY_DEATH, 0);
    }

    std::vector<GameUtils::Object> LogicFunctions::GetAllObjectByTypes(const std::vector<GameUtils::ObjectType>& types)
    {
        std::vector<GameUtils::Object> m_vector;
        for(auto index = 0; index < m_gameThread->GetObjects().size(); ++index)
        {
            for(auto type : types)
                if(m_gameThread->GetObjects()[index].GetType() == type)
                    m_vector.push_back(m_gameThread->GetObjects()[index]);
        }
        return m_vector;
    }

    GameUtils::Object& LogicFunctions::GetObjectReference(const GameUtils::Object& obj)
    {
        return *std::find(m_gameThread->GetObjects().begin(), m_gameThread->GetObjects().end(), obj);
    }

    void LogicFunctions::DestroyObject(const GameUtils::Object& obj)
    {
        m_gameThread->GetObjects().erase(std::find(m_gameThread->GetObjects().begin(), m_gameThread->GetObjects().end(), obj));
    }

    void LogicFunctions::ObjectCollison(GameUtils::Object& obj, const std::vector<GameUtils::ObjectType>& objTypes, const GameUtils::SoundName& soundName, const int& textureRow)
    {
        auto currentPosition = sf::Vector2i(obj.GetSprite().getPosition()) + obj.GetHitBox().getPosition();
        auto enemyObjs = GetAllObjectByTypes(objTypes);
        if(enemyObjs.size() > 0)
        {
            for(auto index = 0; index < enemyObjs.size(); ++index)
            {
                auto enemyPosition = sf::Vector2i(enemyObjs[index].GetSprite().getPosition()) + enemyObjs[index].GetHitBox().getPosition();
                auto dx = (int)std::abs(currentPosition.x - enemyPosition.x);
                auto dy = (int)std::abs(currentPosition.y - enemyPosition.y);
                auto r = enemyObjs[index].GetHitBox().getSize().x;


                if(!obj.GetDestroyOnFinish() && (std::pow(dx,2) + std::pow(dy, 2) <= std::pow(r, 2))) 
                {
                    obj.GetAuxiliarVars()["pinMovement"] = 1;
                    obj.SetupAnimatedAction(textureRow, false, true, false, 0, [this, &obj] {
                        obj.GetAuxiliarVars()["pinMovement"] = 0;
                    });

                    auto& objRef = GetObjectReference(enemyObjs[index]);
                    m_gameThread->PlayAudioChannel(soundName);
                    objRef.SetHitPoints(objRef.GetHitPoints() - 1);
                    if(objRef.GetHitPoints() <= 0)
                    {
                        objRef.SetupAnimatedAction(textureRow, false, true, false, 0, [this, objTypes, objRef]() {
                            for(auto objType : objTypes)
                                if(objType == GameUtils::ObjectType::ENEMY)
                                    m_gameThread->SetScore(++m_gameThread->GetScore());
                        });
                    }
                }   
            }
        }
    }

    void LogicFunctions::RandomShuffler(std::vector<int>& vector, int originalSize)
    {
        std::random_device dev;
        std::mt19937 rng(dev());
        if(vector.size() == 0)
        {
            for(auto index = 0; index < originalSize; ++index)
                vector.push_back(index);
        }
        std::shuffle(vector.begin(), vector.end(), rng);
    }

    void LogicFunctions::PixelColorSwap(sf::Texture& texture, const std::vector<uint32_t>& oldColors, const int& newColor, const int& cycle)
    {
        auto image = texture.copyToImage();
        for(int indexY = 0; indexY < image.getSize().y; ++indexY)
        {
            for(int indexX = 0; indexX < image.getSize().x; ++indexX)
            {
                auto pixelColor = image.getPixel(indexX, indexY).toInteger();
                auto pixelIter = std::find_if(oldColors.begin(), oldColors.end(), [this, pixelColor](const int& color)
                {
                    return color == pixelColor;
                });
                if(pixelIter != oldColors.end())
                {
                    if(newColor == -1)
                    {
                        image.setPixel(indexX, indexY, sf::Color(ColorCycling(*pixelIter, cycle)));
                    }
                    else
                        image.setPixel(indexX, indexY, sf::Color(newColor));

                }
            }
        }
        texture.loadFromImage(image);
    }

    uint32_t LogicFunctions::ColorCycling(const uint32_t& color, const int& cycle)
    {
        union {
            struct {
                unsigned B : 8;
                unsigned G : 8;
                unsigned R : 8;
            };
            uint32_t hexColor;
        } primaryColor;

        primaryColor.hexColor = color;
        std::array<uint8_t, 3> primaryArray = {primaryColor.R, primaryColor.G, primaryColor.B};

        for(auto index = 0; index <= cycle; ++index)
        {
            std::swap(primaryArray[1], primaryArray[0]);
            std::swap(primaryArray[2], primaryArray[1]);
        }
        primaryColor.R = primaryArray[0];
        primaryColor.G = primaryArray[1];
        primaryColor.B = primaryArray[2];
        return primaryColor.hexColor;
    }



}