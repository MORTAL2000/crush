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
#include <SFML/Graphics/RenderTarget.hpp>

#include <Node.hpp>
#include <Scene.hpp>

#include <cassert>

Node::Node(const std::string& name)
    : m_parent  (nullptr),
    m_name      (name),
    m_scene     (nullptr),
    m_camera    (nullptr),
    m_drawable  (nullptr),
    m_physObject(nullptr)
{

}

Node::~Node()
{
    if (m_camera)
        m_camera->m_node = nullptr;

    if (m_physObject)
        m_physObject->deleteObject();

}

//public
void Node::addChild(Node::Ptr& child)
{
    child->m_parent = this;
    child->m_scene = m_scene;

    if (child->m_camera && m_scene->getActiveCamera() == &Scene::defaultCamera)
        m_scene->setActiveCamera(child->m_camera);

    m_children.push_back(std::move(child));
}

Node::Ptr Node::removeChild(Node& child)
{
    auto result = std::find_if(m_children.begin(), m_children.end(), [&child](const Node::Ptr& p)
    {
        return(p.get() == &child);
    });
    assert(result != m_children.end());

    Ptr found = std::move(*result);
    found->m_parent = nullptr;
    found->m_scene = nullptr;
    m_children.erase(result);
    return found;

}

Node* Node::findChild(const std::string& name, bool recursive)
{
    auto result = std::find_if(m_children.begin(), m_children.end(), [&name](const Node::Ptr& p)
    {
        return (p->getName() == name);
    });

    if (result != m_children.end()) return result->get();

    Node* np = nullptr;
    if (recursive)
    {
        for (const auto& c : m_children)
        {
            np = c->findChild(name, true);
            if (np) return np;
        }
    }
    return np;
}

sf::Vector2f Node::getWorldPosition() const
{
    return getWorldTransform() * sf::Vector2f();
}

sf::Transform Node::getWorldTransform() const
{
    auto t = sf::Transform::Identity;
    for (const auto* node = this; node != nullptr; node = node->m_parent)
        t = node->getTransform() * t;

    return t;
}

void Node::setWorldPosition(sf::Vector2f position)
{
    if (m_parent)
        position -= m_parent->getWorldPosition();

    setPosition(position);
}

void Node::setScene(Scene* scene)
{
    m_scene = scene;
}

void Node::setCamera(Camera* cam)
{
    m_camera = cam;
    cam->m_node = this;
}

void Node::setDrawable(sf::Drawable* drawable)
{
    m_drawable = drawable;
}

void Node::setPhysObject(PhysWorld::PhysObject* po)
{
    if (m_physObject) m_physObject->deleteObject();

    m_physObject = po;
    if (po)
    {
        m_physObject->m_node = this;
        m_physObject->setPosition(getWorldPosition());
    }

}

Scene* Node::getScene() const
{
    return m_scene;
}

Camera* Node::getCamera() const
{
    return m_camera;
}

const std::string& Node::getName() const
{
    return m_name;
}

//private
void Node::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    states.transform *= getTransform();
    drawSelf(rt, states);
    drawChildren(rt, states);
}

void Node::drawSelf(sf::RenderTarget& rt, sf::RenderStates states) const
{
    if (m_drawable)
        rt.draw(*m_drawable, states);
}

void Node::drawChildren(sf::RenderTarget& rt, sf::RenderStates states) const
{
    for (const auto& c : m_children)
        c->draw(rt, states);
}