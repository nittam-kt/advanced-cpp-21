#pragma once

#include "UIBehaviour.h"

#include <algorithm>
#include <SpriteFont.h>
#include <UniDx/Mesh.h>


namespace UniDx {

class Texture;

// --------------------
// ImageƒNƒ‰ƒX
// --------------------
class Image : public UIBehaviour
{
public:
	Image();
	virtual void OnEnable() override;
	virtual void Render(const Matrix& proj) const override;

	std::shared_ptr<Texture> texture;
	void SetColor(Color c) { std::fill(colors.begin(), colors.end(), c); }

private:
	ComPtr<ID3D11Buffer> constantBuffer0;
	std::unique_ptr<SubMesh> mesh;
	std::vector<Color> colors;
};

}
