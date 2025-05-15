#ifndef BASELINEMETHODTEX_H
#define BASELINEMETHODTEX_H

#include "BaseLineMethod.hpp"

class BaseLineMethodTex : public BaseLineMethod
{
	Q_OBJECT
  public:
	BaseLineMethodTex();
	void render(Camera const& camera) override;

  public slots:
	std::string getName() const override { return "Base Line Texture"; };

  private:
	GLTexture tex;
};

#endif // BASELINEMETHOD_H
