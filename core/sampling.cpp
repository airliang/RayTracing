#include "sampling.h"
#include "mathdef.h"

namespace AIR
{
	Float Distribution1D::SampleContinuous(Float u, Float* pdf, int* off) const
	{
		//��binary search �ҵ�u���ĸ�cdf��
		int offset = FindInterval(cdf.size(), [&](int index) { return cdf[index] <= u; });
		if (off != nullptr)
			*off = offset;

		//����һ��cdf����ֵ
		Float du = u - cdf[offset];
		if ((cdf[offset + 1] - cdf[offset]) > 0)
			du /= (cdf[offset + 1] - cdf[offset]);

		if (pdf != nullptr)
		{
			*pdf = funcInt != 0 ? func[offset] / funcInt : 0;
		}

		//ע�⣬���ﲻ�Ƿ���cdf������cdf��Ӧ��pdf��ķ�������ֵ
		//����cdf��index��Ӧ��pdf��index����һ���򵥵����Բ�ֵ
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
