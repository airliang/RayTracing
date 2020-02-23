#pragma once
#include "interaction.h"
#include "matrix.h"

namespace AIR
{
class TextureMapping2D
{
public:
	virtual ~TextureMapping2D() { }

	//返回对应的纹理坐标s,t
	//dstdx,dstdy返回屏幕x,y变化时st的变化值
	virtual Point2f Map(const Interaction& si, Float *dstdx, Float* dstdy) const = 0;

};

class UVMapping2D : public TextureMapping2D
{
public:
    UVMapping2D(Float su = 1, Float sv = 1, Float du = 0, Float dv = 0);
	virtual Point2f Map(const Interaction& si, Vector2f *dstdx, Vector2f* dstdy) const;
private:
    //su sv uv缩放
    const Float su;
	const Float sv;
	//du dv uv偏移
	const Float du;
	const Float dv;  
};

class SphericalMapping2D : public TextureMapping2D
{
public:
    SphericalMapping2D(const Matrix4f& world2Texture) : worldToTexture(world2Texture)
	{

	}
	virtual Point2f Map(const Interaction& si, Vector2f *dstdx, Vector2f* dstdy) const;
  
private:
    //返回p坐标对应的球面坐标[0,1]
    Point2f Sphere(const Vector3f& p) const;
    Matrix4f worldToTexture;
};



template <typename T>
class Texture 
{
public:
	// Texture Interface
	virtual T Evaluate(const Interaction &) const = 0;
	virtual ~Texture() {}
};
}
