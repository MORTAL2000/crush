/*********************************************************************
Matt Marchant 2014
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

#include <GameOverState.hpp>
#include <Game.hpp>
#include <Util.hpp>

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/RectangleShape.hpp>

namespace
{
    sf::Text placeholderText;
    std::string str =
        "       GAME OVER\n Press C to Continue";

    sf::RectangleShape rectangle;

    const float waitTime = 2.f;
    float waitedTime = 0.f;
}

GameOverState::GameOverState(StateStack& stack, Context context)
    : State(stack, context)
{
    getContext().renderWindow.setTitle("Menu Screen");
    getContext().renderWindow.setView(getContext().defaultView);

    placeholderText.setFont(getContext().gameInstance.getFont("res/fonts/VeraMono.ttf"));
    placeholderText.setString(str);
    placeholderText.setCharacterSize(60u);
    Util::Position::centreOrigin(placeholderText);
    placeholderText.setPosition(getContext().defaultView.getCenter());

    rectangle.setFillColor({ 0u, 0u, 0u, 148u });
    rectangle.setSize(context.defaultView.getSize());
}

void GameOverState::draw()
{
    if (waitedTime > waitTime)
    {
        getContext().renderWindow.draw(rectangle);
        getContext().renderWindow.draw(placeholderText);
    }
}

bool GameOverState::update(float dt)
{
    waitedTime += dt;
    return true; //return true so we can see remaining baddies bouncing about / finish death animations
}

bool GameOverState::handleEvent(const sf::Event& evt)
{
    if (evt.type == sf::Event::KeyPressed)
    {
        if (evt.key.code == sf::Keyboard::C)
        {
            requestStackClear();
            requestStackPush(States::ID::Menu);
        }
    }
    return false;
}