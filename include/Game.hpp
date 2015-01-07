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

//main game class

#ifndef GAME_H_
#define GAME_H_

#include <StateStack.hpp>
#include <Resource.hpp>
#include <ShaderResource.hpp>
#include <Music.hpp>
#include <Console.hpp>

#include <SFML/Graphics/RenderWindow.hpp>

class Game final : private sf::NonCopyable
{
public:
    Game();
    ~Game() = default;

    void run();
    void pause();
    void resume();

    void setClearColour(sf::Color c);
    sf::Font& getFont(const std::string& path);

    TextureResource& getTextureResource();
    ShaderResource& getShaderResource();

    void playMusic(const std::string& title, bool loop = true);
    void stopMusic();
    void pauseMusic();
    void resumeMusic();

private: 

    bool m_paused;

    sf::RenderWindow m_renderWindow;
    sf::Color m_clearColour;

    StateStack m_stateStack;

    FontResource m_fontResource;
    TextureResource m_textureResource;
    ShaderResource m_shaderResource;

    MusicPlayer m_musicPlayer;

    Console m_console;

    void handleEvents();
    void update(float dt);
    void draw();

    void registerStates();
};

#endif //GAME_H_