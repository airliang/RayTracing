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

	Point2f Distribution2D::SampleContinuous(const Point2f& uv, Float* pdf) const
	{
		Float pdfs[2];
		int vi = 0;
		//�Ȳ���v���������
		//p(v) = ��f(ui, v~)������v~ = floor(u.y * nv);
		//vȷ���������Ӧ�ı߼�
		Float v = pdfMarginV->SampleContinuous(uv.y, &pdfs[1], &vi);

		//p(u|v) = p(u,v)/p(v)
		Float u = pdfUinV[vi]->SampleContinuous(uv.x, &pdfs[0]);

		*pdf = pdfs[0] * pdfs[1];
		return Point2f(u, v);
	}

	Float Distribution2D::Pdf(const Point2f& u) const
	{
		int ui = Clamp(int(u.x * pdfUinV[0]->Count()), 0, pdfUinV[0]->Count() - 1);
		int vi = Clamp(int(u.y * pdfMarginV->Count()), 0, pdfMarginV->Count() - 1);

		//pdfMarginV->funcInt �൱�����������Ļ���
		return pdfUinV[vi]->func[ui] / pdfMarginV->funcInt;
	}
}
