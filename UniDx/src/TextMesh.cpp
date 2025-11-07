#include "pch.h"

#include <UniDx/TextMesh.h>
#include <UniDx/D3DManager.h>
#include <UniDx/Font.h>

using namespace DirectX;

namespace UniDx {


void TextMesh::Awake()
{
	UIBehaviour::Awake();
	spriteBatch = std::make_unique<SpriteBatch>(D3DManager::getInstance()->GetContext().Get());
}


void TextMesh::Render(const Matrix& proj) const
{
	UIBehaviour::Render(proj);
    if(spriteBatch == nullptr || font == nullptr || font->getSpriteFont() == nullptr) return;

    // SpriteFont‚ðŽg‚Á‚½•`‰æ
    spriteBatch->Begin();

    Vector3 pos = transform->position;
    Vector2 drawPos(pos.x, pos.y);
    font->getSpriteFont()->DrawString(spriteBatch.get(), text.c_str(), drawPos);

    spriteBatch->End();
}

}
