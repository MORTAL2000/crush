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

#ifndef UI_LABEL_H_
#define UI_LABEL_H_

#include <Resource.hpp>
#include <UIControl.hpp>

#include <SFML/Graphics/Text.hpp>

#include <string>

namespace ui
{
    class Label final : public Control
    {
    public:
        typedef std::shared_ptr<Label> Ptr;

        Label(const std::string& text, const sf::Font& font);
        ~Label() = default;

        bool selectable() const override;
        void handleEvent(const sf::Event& e, const sf::Vector2f& mousePos) override;

        void setAlignment(Alignment a) override;

        void setText(const std::string& text);
        void setTextColour(const sf::Color& c);
        void setFont(const sf::Font& font);
        void setFontSize(sf::Uint16 size);

    private:
        sf::Text m_text;
        Alignment m_alignment;
        void draw(sf::RenderTarget& rt, sf::RenderStates states) const override;
    };
}

#endif //UI_LABEL_H_