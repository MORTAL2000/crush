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

#include <GameState.hpp>
#include <Game.hpp>
#include <DebugShape.hpp>
#include <BodyBehaviour.hpp>
#include <Util.hpp>
#include <Particles.hpp>
#include <Map.hpp>

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/RectangleShape.hpp>

#include <iostream>

namespace
{
    Camera sceneCam;

    sf::RectangleShape groundShape;
    sf::RectangleShape wallShape;
    sf::RectangleShape shelfShape;

    DebugShape blockShape;
    DebugShape playerShape;
    DebugShape npcShape;

    const sf::Uint8 maxPlayers = 2u;

    sf::Vector2f blockSize(100.f, 70.f);
 
    TextureResource textureResource;

}

GameState::GameState(StateStack& stack, Context context)
    : State             (stack, context),
    m_collisionWorld    (70.f),
    m_npcController     (m_commandStack),
    m_scoreBoard        (stack, context),
    m_particleController(textureResource),
    m_mapController     (m_commandStack)
{
    //build world
    getContext().renderWindow.setTitle("Game Screen");
    
    Scene::defaultCamera.setView(getContext().defaultView);

    groundShape.setFillColor(sf::Color::Transparent);
    groundShape.setOutlineColor(sf::Color::Red);
    groundShape.setOutlineThickness(-3.f);

    wallShape = groundShape;
    shelfShape = groundShape;

    blockShape.setSize(blockSize);
    blockShape.setColour(sf::Color::Red);

    playerShape.setSize(blockSize);
    playerShape.setColour(sf::Color::Blue);

    npcShape.setSize(blockSize);
    npcShape.setColour(sf::Color::Green);

    groundShape.setSize({ 1920.f, 50.f });
    auto groundNode = std::make_unique<Node>("groundNode");
    groundNode->setDrawable(&groundShape);
    groundNode->setPosition(0.f, 1030.f);
    auto gb = m_collisionWorld.addBody(CollisionWorld::Body::Type::Solid, groundShape.getSize());
    groundNode->setCollisionBody(gb);
    m_scene.addNode(groundNode);

    wallShape.setSize({ 150.f, 1030.f });
    auto leftWallNode = std::make_unique<Node>("leftWall");
    leftWallNode->setDrawable(&wallShape);
    leftWallNode->setPosition(-100.0f, 0.f);
    leftWallNode->setCollisionBody(m_collisionWorld.addBody(CollisionWorld::Body::Type::Solid, wallShape.getSize()));
    m_scene.addNode(leftWallNode);

    auto rightWallNode = std::make_unique<Node>("rightWall");
    rightWallNode->setDrawable(&wallShape);
    rightWallNode->setPosition(1880.f, 0.f);
    rightWallNode->setCollisionBody(m_collisionWorld.addBody(CollisionWorld::Body::Type::Solid, wallShape.getSize()));
    m_scene.addNode(rightWallNode);

    shelfShape.setSize({ 200.f, 16.f });
    auto leftShelfNode = std::make_unique<Node>("leftShelf");
    leftShelfNode->setDrawable(&shelfShape);
    leftShelfNode->setPosition({ 50.f, 600.f });
    leftShelfNode->setCollisionBody(m_collisionWorld.addBody(CollisionWorld::Body::Type::Solid, shelfShape.getSize()));
    m_scene.addNode(leftShelfNode);

    auto rightShelfNode = std::make_unique<Node>("rightShelf");
    rightShelfNode->setDrawable(&shelfShape);
    rightShelfNode->setPosition({ 1680.f, 600.f });
    rightShelfNode->setCollisionBody(m_collisionWorld.addBody(CollisionWorld::Body::Type::Solid, shelfShape.getSize()));
    m_scene.addNode(rightShelfNode);


    //set up controllers
    m_players.reserve(2);
    m_players.emplace_back(m_commandStack, Category::PlayerOne);
    m_players.emplace_back(m_commandStack, Category::PlayerTwo);

    std::function<void(const sf::Vector2f&, Player&)> spawnFunc = std::bind(&GameState::addPlayer, this, std::placeholders::_1, std::placeholders::_2);
    m_players[0].setSpawnFunction(spawnFunc);
    m_players[1].setSpawnFunction(spawnFunc);

    std::function<void(const sf::Vector2f&)> f = std::bind(&GameState::addNpc, this, std::placeholders::_1);
    m_npcController.setSpawnFunction(f);

    m_scoreBoard.addObserver(m_players[0]);
    m_scoreBoard.addObserver(m_players[1]);
    m_scoreBoard.addObserver(m_npcController);
    m_scoreBoard.enablePlayer(Category::PlayerOne);

    m_scene.addObserver(m_scoreBoard);
    

    //must be done after controllers are initialised
    Map map("res/maps/testmap.crm");
    m_npcController.setNpcCount(map.getNpcCount());
    m_scoreBoard.setMaxNpcs(map.getNpcTotal());

    //TODO properly parse map with sprites / textures
    const auto& nodes = map.getNodes();
    for (const auto& n : nodes)
    {
        switch (n.type)
        {
        case Category::Block:
            addBlock(n.position);
            break;
        case Category::PlayerOne:
            //set spawn point
            m_players[0].setSpawnPosition(n.position);
            break;
        case Category::PlayerTwo:
            //set spawn point
            m_players[1].setSpawnPosition(n.position);
            break;
        default: break;
        }
    }
}

bool GameState::update(float dt)
{
    while (!m_commandStack.empty())
        m_scene.executeCommand(m_commandStack.pop(), dt);

    //update players
    for (auto& p : m_players)
        p.update(dt);

    //update AI
    m_npcController.update(dt);

    //update collision detection
    m_collisionWorld.step(dt);

    //update particles
    m_particleController.update(dt);

    m_scene.flush();
    return true;
}

void GameState::draw()
{
    getContext().renderWindow.draw(m_scene);
    getContext().renderWindow.draw(m_particleController);
    getContext().renderWindow.draw(m_scoreBoard);
}

bool GameState::handleEvent(const sf::Event& evt)
{
    switch (evt.type)
    {
    case sf::Event::MouseButtonPressed:
    {  
        auto position = getContext().renderWindow.mapPixelToCoords(sf::Mouse::getPosition(getContext().renderWindow));
        switch (evt.mouseButton.button)
        {
        case sf::Mouse::Left:
            addBlock(position);
            break;
        case sf::Mouse::Right:
            addNpc(position);
            break;
        default: break;
        }
    break;
    }
    case sf::Event::KeyPressed:
        switch (evt.key.code)
        {
        case sf::Keyboard::Num1:
            
            break;
        case sf::Keyboard::Num2:
            m_scoreBoard.enablePlayer(Category::PlayerTwo);
            break;
        case sf::Keyboard::P:
            requestStackPush(States::ID::Pause);
            break;
        default:break;
        }
        break;
    default: break;
    }
    
    return true;
}



//private
void GameState::addBlock(const sf::Vector2f& position)
{
    auto blockNode = std::make_unique<Node>("blockNode");
    blockNode->setPosition(position);
    blockNode->setDrawable(&blockShape);
    blockNode->setCategory(Category::Block);
    blockNode->setCollisionBody(m_collisionWorld.addBody(CollisionWorld::Body::Type::Block, blockShape.getSize()));
    blockNode->addObserver(m_players[0]);
    blockNode->addObserver(m_players[1]);
    blockNode->addObserver(m_scoreBoard);
    m_scene.addNode(blockNode);
}

void GameState::addPlayer(const sf::Vector2f& position, Player& player)
{
    if (player.canSpawn())
    {
        auto playerNode = std::make_unique<Node>("Player");
        playerNode->setPosition(position);
        playerNode->setDrawable(&playerShape);
        playerNode->setCategory(player.getType());
        playerNode->setCollisionBody(m_collisionWorld.addBody(CollisionWorld::Body::Type::Player, playerShape.getSize()));
        playerNode->addObserver(player);
        playerNode->addObserver(m_npcController);
        playerNode->addObserver(m_scoreBoard);
        playerNode->addObserver(m_particleController);
        m_scene.addNode(playerNode);

        player.setSpawnable(false);
        player.setSize(playerShape.getSize());
    }
}

void GameState::addNpc(const sf::Vector2f& position)
{
    auto npcNode = std::make_unique<Node>("npc");
    npcNode->setPosition(position);
    npcNode->setDrawable(&npcShape);
    npcNode->setCategory(Category::Npc);
    npcNode->setCollisionBody(m_collisionWorld.addBody(CollisionWorld::Body::Type::Npc, npcShape.getSize()));
    npcNode->addObserver(m_npcController);
    npcNode->addObserver(m_scoreBoard);
    npcNode->addObserver(m_particleController);
    m_scene.addNode(npcNode);
}