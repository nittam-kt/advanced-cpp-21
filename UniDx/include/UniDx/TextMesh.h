#pragma once

#include "UIBehaviour.h"

#include <SpriteFont.h>


namespace UniDx {

class Font;

// --------------------
// TextMeshƒNƒ‰ƒX
// --------------------
class TextMesh : public UIBehaviour
{
public:
	virtual void Awake() override;
	virtual void Render(const Matrix& proj) const;

	wstring          text;
	shared_ptr<Font> font;

private:
	unique_ptr<DirectX::SpriteBatch> spriteBatch;
};

}
