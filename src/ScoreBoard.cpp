/*********************************************************************
Matt Marchant 2014 - 2015
http://trederia.blogspot.com

Crush - Zlib license.

This software is provided 'as-is', without any express or
implied warranty. In no event will the authors be held
liable for any damages arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute
it freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented;
you must not claim that you wrote the original software.
If you use this software in a product, an acknowledgment
in the product documentation would be appreciated but
is not required.

2. Altered source versions must be plainly marked as such,
and must not be misrepresented as being the original software.

3. This notice may not be removed or altered from any
source distribution.
*********************************************************************/

#include <ScoreBoard.hpp>
#include <Game.hpp>
#include <Util.hpp>

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Text.hpp>

#include <sstream>
#include <cassert>
#include <iostream>

namespace
{
    sf::Text playerOneText;
    sf::Text playerTwoText;
    sf::Text npcText;

    const sf::Uint16 playerPoints = 100u; //points for killing other player
    const sf::Uint16 crushPoints = 500u; //points for crushing someone
    const sf::Uint16 suicidePoints = 200u; //points deducted for accidentally crushing self
    const sf::Uint16 itemPoints = 400u; //points for collecting item

    const float messageAcceleration = 232.f;
    const float initialMessageSpeed = 60.f;
}

ScoreBoard::ScoreBoard(StateStack& stack, State::Context context)
    : m_stack           (stack),
    m_context           (context),
    m_playerOneLives    (5),
    m_playerTwoLives    (-1),
    m_playerOneScore    (0u),
    m_playerTwoScore    (0u),
    m_playerOneExtinct  (false),
    m_playerTwoExtinct  (false),
    m_maxNpcs           (2u),
    m_spawnedNpcs       (0u),
    m_deadNpcs          (0u)
{
    playerOneText.setFont(context.gameInstance.getFont("res/fonts/VeraMono.ttf"));
    playerOneText.setPosition({ 60.f, 10.f });
    updateText(Category::PlayerOne);

    playerTwoText.setFont(*playerOneText.getFont());
    playerTwoText.setString("Press Start");
    playerTwoText.setPosition({ 1400.f, 10.f });

    npcText.setFont(*playerOneText.getFont());
}

//public
void ScoreBoard::update(float dt)
{
    m_messages.remove_if([](const Message& m){return m.stopped(); });

    for (auto& m : m_messages)
        m.update(dt);
}

void ScoreBoard::onNotify(Subject& s, const Event& evt)
{
    switch (evt.type)
    {
    case Event::Node:
        if (evt.node.action == Event::NodeEvent::Despawn)
        {
            switch (evt.node.type)
            {
            case Category::PlayerOne:
                m_playerOneLives--;
                
                if (m_playerOneLives < 0)
                {
                    disablePlayer(Category::PlayerOne);
                }

                updateText(evt.node.type);
                break;
            case Category::PlayerTwo:
                m_playerTwoLives--;
                
                if (m_playerTwoLives < 0)
                {
                    disablePlayer(Category::PlayerTwo);
                }
                updateText(evt.node.type);
                break;
            case Category::Npc:
                m_deadNpcs++;
                if (m_maxNpcs == m_deadNpcs)
                {
                    //game over, all dead
                    m_stack.pushState(States::ID::GameOver);

                    //disable player input
                    disablePlayer(Category::PlayerOne);
                    disablePlayer(Category::PlayerTwo);
                }
                
                break;
            default: break;
            }
        }
        else if (evt.node.action == Event::NodeEvent::Spawn)
        {
            switch (evt.node.type)
            {
            case Category::Npc:
                m_spawnedNpcs++;
                updateText(Category::Npc);

                if (m_spawnedNpcs == m_maxNpcs)
                {
                    //stop spawning
                    Event e;
                    e.type = Event::Game;
                    e.game.action = Event::GameEvent::NpcDisable;
                    notify(*this, e);
                }
                
                break;
            default: break;
            }
        }
        else if(evt.node.action == Event::NodeEvent::KilledNode)
        {
            auto textUpdateTarget = evt.node.type;
            switch (evt.node.type)
            {                
            case Category::Block:
                if (evt.node.owner == Category::PlayerOne)
                {
                        switch (evt.node.target)
                        {
                        case Category::PlayerTwo: //p1 crushed p2
                            m_playerOneLives++;
                            m_playerOneScore += playerPoints;

                            //show message
                            m_messages.emplace_back(std::to_string(playerPoints),
                                sf::Vector2f(evt.node.positionX, evt.node.positionY),
                                m_context.gameInstance.getFont("res/fonts/VeraMono.ttf"));


                        case Category::Npc: //p1 killed bad guy
                            m_playerOneScore += crushPoints;

                            //show message
                            m_messages.emplace_back(std::to_string(crushPoints),
                                sf::Vector2f(evt.node.positionX, evt.node.positionY),
                                m_context.gameInstance.getFont("res/fonts/VeraMono.ttf"));

                            break;
                        case Category::PlayerOne: //p1 killed self, doh
                        {
                            std::string msg = "-";
                            if (m_playerOneScore > suicidePoints)
                            {
                                m_playerOneScore -= suicidePoints;
                                msg += std::to_string(suicidePoints);
                            }
                            else
                            {
                                msg += std::to_string(m_playerOneScore);
                                m_playerOneScore = 0u;
                            }

                            m_messages.emplace_back(msg,
                                sf::Vector2f(evt.node.positionX, evt.node.positionY),
                                m_context.gameInstance.getFont("res/fonts/VeraMono.ttf"));
                        }
                            break;
                        default: break;
                        }
                }
                else if (evt.node.owner == Category::PlayerTwo)
                {
                        switch (evt.node.target)
                        {
                        case Category::PlayerOne: //p2 killed p1  
                            m_playerTwoLives++;
                            m_playerTwoScore += playerPoints;

                            //show message
                            m_messages.emplace_back(std::to_string(playerPoints),
                                sf::Vector2f(evt.node.positionX, evt.node.positionY),
                                m_context.gameInstance.getFont("res/fonts/VeraMono.ttf"));

                        case Category::Npc: //p2 killed bad guy
                            m_playerTwoScore += crushPoints;

                            //show message
                            m_messages.emplace_back(std::to_string(crushPoints),
                                sf::Vector2f(evt.node.positionX, evt.node.positionY),
                                m_context.gameInstance.getFont("res/fonts/VeraMono.ttf"));

                            break;
                        case Category::PlayerTwo: //p2 crushed self :S
                        {    
                            std::string msg = "-";
                            if (m_playerTwoScore > suicidePoints)
                            {
                                m_playerTwoScore -= suicidePoints;
                                msg += std::to_string(suicidePoints);
                            }
                            else
                            {
                                msg += std::to_string(m_playerTwoScore);
                                m_playerTwoScore = 0u;
                            }

                            m_messages.emplace_back(msg,
                                sf::Vector2f(evt.node.positionX, evt.node.positionY),
                                m_context.gameInstance.getFont("res/fonts/VeraMono.ttf"));
                        }
                            break;
                        default: break;
                        }
                }
                else if (evt.node.owner == Category::None)
                {
                    std::string msg = "-";
                    switch (evt.node.target)
                    {
                    case Category::PlayerOne: //p1 killed self, doh
                        if (m_playerOneScore > suicidePoints)
                        {
                            msg += std::to_string(suicidePoints);
                            m_playerOneScore -= suicidePoints;
                        }
                        else
                        {
                            msg += std::to_string(m_playerOneScore);
                            m_playerOneScore = 0u;
                        }
                        break;
                    case Category::PlayerTwo: //p2 crushed self :S
                        if (m_playerTwoScore > suicidePoints)
                        {
                            msg += std::to_string(suicidePoints);
                            m_playerTwoScore -= suicidePoints;
                        }
                        else
                        {
                            msg += std::to_string(m_playerTwoScore);
                            m_playerTwoScore = 0u;
                        }
                        break;
                    default: break;
                    }

                    m_messages.emplace_back(msg,
                        sf::Vector2f(evt.node.positionX, evt.node.positionY),
                        m_context.gameInstance.getFont("res/fonts/VeraMono.ttf"));
                }
                textUpdateTarget = evt.node.owner;
                break;
            default: break;
            }
            //if (textUpdateTarget != Category::None)
            updateText(textUpdateTarget);
        }
        break;
    case Event::Player:
    {
        std::string msg;
        switch (evt.player.action)
        {
        case Event::PlayerEvent::GotItem:
            switch (evt.player.item)
            {
            case Event::PlayerEvent::ExtraLife:
                (evt.player.playerId == Category::PlayerOne) ?
                    m_playerOneLives++ :
                    m_playerTwoLives++;
                msg = "Extra Life! ";
                break;
            case Event::PlayerEvent::Attraction:
                msg = "Nothing! ";
                break;
            case Event::PlayerEvent::ExtraSpeed:
                msg = "Extra Speed! ";
                break;
            case Event::PlayerEvent::JumpIncrease:
                msg = "Extra Jump! ";
                break;
            case Event::PlayerEvent::ReverseControls:
                msg = "Reverse Controls! ";
                break;
            default: break;
            }
            (evt.player.playerId == Category::PlayerOne) ?
                m_playerOneScore += itemPoints :
                m_playerTwoScore += itemPoints;

            //display message
            m_messages.emplace_back(msg + std::to_string(itemPoints),
                sf::Vector2f(evt.player.positionX, evt.player.positionY),
                m_context.gameInstance.getFont("res/fonts/VeraMono.ttf"));

            m_messages.back().setColour(sf::Color::Yellow);
            updateText(evt.player.playerId);
            break;
        default:break;
        }
    }
        break;
    default: break;
    }
}

void ScoreBoard::enablePlayer(Category::Type player)
{
    assert(player == Category::PlayerOne || player == Category::PlayerTwo);

    //prevent re-enabling players who have lost all lives
    if ((player == Category::PlayerOne && m_playerOneExtinct)
        || (player == Category::PlayerTwo && m_playerTwoExtinct))
    {
        return;
    }

    Event e;
    e.type = Event::Game;
    e.game.action = (player == Category::PlayerOne) ? 
        Event::GameEvent::PlayerOneEnable : 
        Event::GameEvent::PlayerTwoEnable;

    notify(*this, e);

    if (player == Category::PlayerTwo)
    {
        m_playerTwoLives = 5;
        updateText(Category::PlayerTwo);
    }
}

void ScoreBoard::setMaxNpcs(sf::Uint8 count)
{
    m_maxNpcs = count;
    updateText(Category::Npc);
}

//private
void ScoreBoard::updateText(Category::Type type)
{
    std::stringstream ss;    
    if (type == Category::PlayerOne)
    {
        if (m_playerOneLives >= 0)
        {
            ss << "Lives: " << m_playerOneLives << "    Score: " << m_playerOneScore;
            playerOneText.setString(ss.str());
        }
        else if (m_playerOneExtinct)
        {
            ss << "GAME OVER    Score: " << m_playerOneScore;
            playerOneText.setString(ss.str());
        }
        
    }
    else if (type == Category::PlayerTwo)
    {
        if (m_playerTwoLives >= 0)
        {
            ss << "Lives: " << m_playerTwoLives << "    Score: " << m_playerTwoScore;
            playerTwoText.setString(ss.str());
        }
        else if (m_playerTwoExtinct)
        {
            ss << "GAME OVER    Score: " << m_playerTwoScore;
            playerTwoText.setString(ss.str());
        }
    }
    else if (type == Category::Npc)
    {
        ss << "Enemies Remaining: " << (m_maxNpcs - m_spawnedNpcs) << std::endl;
        npcText.setString(ss.str());
        Util::Position::centreOrigin(npcText);
        npcText.setPosition(m_context.defaultView.getCenter() + sf::Vector2f(0.f, -500.f));
    }

    if (m_playerOneLives < 0 && m_playerTwoLives < 0)
    {
        //Gaaaaaaame Oveeeeer!!!
        m_stack.pushState(States::ID::GameOver);
    }
}

void ScoreBoard::disablePlayer(Category::Type player)
{
    assert(player == Category::PlayerOne || player == Category::PlayerTwo);

    Event e;
    e.type = Event::Game;
    e.game.action = (player == Category::PlayerOne) ? 
        Event::GameEvent::PlayerOneDisable :
        Event::GameEvent::PlayerTwoDisable;
    notify(*this, e);

    if (player == Category::PlayerOne)
        m_playerOneExtinct = true;
    else
        m_playerTwoExtinct = true;
}

void ScoreBoard::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    for (const auto& m : m_messages)
        m_context.renderWindow.draw(m);

    m_context.renderWindow.draw(playerOneText);
    m_context.renderWindow.draw(playerTwoText);
    m_context.renderWindow.draw(npcText);
}

//-----message class-----//
ScoreBoard::Message::Message(const std::string& text, const sf::Vector2f& position, const sf::Font& font)
    : m_colour          (sf::Color::White),
    m_transparency      (1.f),
    m_text              (text, font, 24u),
    m_messageSpeed      (initialMessageSpeed)
{
    Util::Position::centreOrigin(m_text);
    m_text.setPosition(position);
}

void ScoreBoard::Message::update(float dt)
{
    const float step = 1.4f * dt;
    m_transparency -= step;

    if (m_transparency > 0)
    {
        m_colour.a = static_cast<sf::Uint8>(255.f * m_transparency);
        m_text.setColor(m_colour);
    }

    m_messageSpeed += messageAcceleration * dt;
    m_text.move(0.f, -m_messageSpeed * dt);
}

void ScoreBoard::Message::setColour(const sf::Color& c)
{
    m_colour = c;
}

bool ScoreBoard::Message::stopped() const
{
    return (m_transparency <= 0);
}

void ScoreBoard::Message::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    rt.draw(m_text, states);
}