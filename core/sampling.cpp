#include "sampling.h"
#include "mathdef.h"

namespace AIR
{
	Float Distribution1D::SampleContinuous(Float u, Float* pdf, int* off) const
	{
		//用binary search 找到u在哪个cdf里
		int offset = FindInterval(cdf.size(), [&](int index) { return cdf[index] <= u; });
		if (off != nullptr)
			*off = offset;

		//和下一个cdf做插值
		Float du = u - cdf[offset];
		if ((cdf[offset + 1] - cdf[offset]) > 0)
			du /= (cdf[offset + 1] - cdf[offset]);

		if (pdf != nullptr)
		{
			*pdf = funcInt != 0 ? func[offset] / funcInt : 0;
		}

		//注意，这里不是返回cdf，而是cdf对应的pdf里的反函数的值
		//由于cdf的index对应着pdf的index，做一个简单的线性插值
		return (offset + du) / Count();
		//return Lerp(du, cdf[offset], cdf[offset + 1]);
	}

	int Distribution1D::SampleDiscrete(Float u, Float* pdf, Float* uRemapped) const
	{
		int offset = FindInterval(cdf.size(), [&](int index) { return cdf[index] <= u; });

		if (pdf != nullptr)
		{
			*pdf = funcInt != 0 ? func[offset] / funcInt : 0;
		}

		if (uRemapped)
		{
			*uRemapped = u - cdf[offset];
			if ((cdf[offset + 1] - cdf[offset]) > 0)
				*uRemapped /= (cdf[offset + 1] - cdf[offset]);
		}

		return offset;
	}
}
