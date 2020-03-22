#include "spectrum.h"


namespace AIR
{
//电介质的菲尼尔反射率
//cosThetaI 入射角和表面法线的夹角的cos值
//etaI 表面法线正方向的折射率
//etaT 表面法线反方向的折射率
//return 光照反射的比例Fr，折射比例就是1 - Fr（能量守恒）
Float FrDielectric(Float cosThetaI, Float etaI, Float etaT);

//导体的菲尼尔反射率
//cosThetaI 入射角和表面法线的夹角的cos值
//etaI 表面法线正方向的折射率
//etaT 表面法线反方向的折射率
//吸收系数
Spectrum FrConductor(Float cosThetaI, const Spectrum &etaI,
                     const Spectrum &etaT, const Spectrum &k);
}