#pragma once

#include "Object.h"

#include <SpriteFont.h>

namespace UniDx {

class Font : public Object
{
public:
	Font();

	bool Load(const wchar_t* filePath);
	bool Load(const wstring& filePath) { return Load(filePath.c_str()); }

	DirectX::SpriteFont* getSpriteFont() const;

private:
	wstring fileName;
	unique_ptr<DirectX::SpriteFont> spriteFont;
};

}