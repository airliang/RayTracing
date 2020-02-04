#include "imageio.h"
#include "fileutil.h"
#include "../thirdparty/openexr/OpenEXR/IlmImf/ImfRgba.h"
#include "../thirdparty/openexr/OpenEXR/IlmImf/ImfRgbaFile.h"

namespace AIR
{
	static void WriteImageEXR(const std::string &name, const Float *pixels,
		int xRes, int yRes, int totalXRes, int totalYRes,
		int xOffset, int yOffset) {
		using namespace Imf;
		using namespace Imath;

		Rgba *hrgba = new Rgba[xRes * yRes];
		for (int i = 0; i < xRes * yRes; ++i)
			hrgba[i] = Rgba(pixels[3 * i], pixels[3 * i + 1], pixels[3 * i + 2]);

		// OpenEXR uses inclusive pixel bounds.
		Box2i displayWindow(V2i(0, 0), V2i(totalXRes - 1, totalYRes - 1));
		Box2i dataWindow(V2i(xOffset, yOffset),
			V2i(xOffset + xRes - 1, yOffset + yRes - 1));

		try {
			RgbaOutputFile file(name.c_str(), displayWindow, dataWindow,
				WRITE_RGB);
			file.setFrameBuffer(hrgba - xOffset - yOffset * xRes, 1, xRes);
			file.writePixels(yRes);
		}
		catch (const std::exception &exc) 
		{
			//Error("Error writing \"%s\": %s", name.c_str(), exc.what());
		}

		delete[] hrgba;
	}

	void ImageIO::WriteImage(const std::string &name, const Float *rgb, const Bounds2i &outputBounds, const Point2i &totalResolution)
	{
		Vector2i resolution = outputBounds.Diagonal();
		if (HasExtension(name, ".exr")) 
		{
			WriteImageEXR(name, rgb, resolution.x, resolution.y, totalResolution.x,
				totalResolution.y, outputBounds.pMin.x,
				outputBounds.pMin.y);
		}
	}
}