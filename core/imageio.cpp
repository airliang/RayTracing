#include "imageio.h"
#include "fileutil.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "spectrum.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"


//#include "../thirdparty/openexr/OpenEXR/IlmImf/ImfRgba.h"
//#include "../thirdparty/openexr/OpenEXR/IlmImf/ImfRgbaFile.h"

namespace AIR
{
	std::string ImageIO::imageLoadPath;
	/*
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
	*/

	static void WriteImagePNG(const std::string& name, const void* pixel, int width, int height)
	{
		stbi_write_png(name.c_str(), width, height, 3, pixel, 3 * width);
	}

	void ImageIO::InitPath(const std::string& root)
	{
		imageLoadPath = root + "resources\\images\\";
	}

	void ImageIO::WriteImage(const std::string &name, const Float *rgb, const Bounds2i &outputBounds, const Point2i &totalResolution)
	{
		Vector2i resolution = outputBounds.Diagonal();
		if (HasExtension(name, ".exr")) 
		{
			//WriteImageEXR(name, rgb, resolution.x, resolution.y, totalResolution.x,
			//	totalResolution.y, outputBounds.pMin.x,
			//	outputBounds.pMin.y);
		}
		else if (HasExtension(name, ".png"))
		{
			std::unique_ptr<uint8_t[]> rgb8(new uint8_t[3 * resolution.x * resolution.y]);
			uint8_t* dst = rgb8.get();
			for (int y = 0; y < resolution.y; ++y) {
				for (int x = 0; x < resolution.x; ++x) {
#define TO_BYTE(v) (uint8_t) Clamp(255.f * GammaCorrect(v), 0.f, 255.f)
					dst[0] = TO_BYTE(rgb[3 * (y * resolution.x + x) + 0]);
					dst[1] = TO_BYTE(rgb[3 * (y * resolution.x + x) + 1]);
					dst[2] = TO_BYTE(rgb[3 * (y * resolution.x + x) + 2]);
#undef TO_BYTE
					dst += 3;
				}
			}
			WriteImagePNG(name, rgb8.get(), outputBounds.Diagonal().x, outputBounds.Diagonal().y);
		}
	}

	std::unique_ptr<RGBSpectrum[]> ImageIO::ReadImage(const std::string& filename, Point2i& resolution)
	{
		unsigned char* rgb;
		int w, h;
		int channel;

		// 用stb库加载图片
		rgb = stbi_load((imageLoadPath + filename).c_str(), &w, &h, &channel, 4);
		if (!rgb) {
			throw std::runtime_error(filename + " load fail");
		}
		resolution.x = w;
		resolution.y = h;
		// 将rgb值转换为RGB光谱
		RGBSpectrum* ret = new RGBSpectrum[w * h];
		unsigned char* src = rgb;
		for (unsigned int y = 0; y < h; ++y) {
			for (unsigned int x = 0; x < w; ++x, src += 4) {
				Float c[3];
				c[0] = src[0] / 255.f;
				c[1] = src[1] / 255.f;
				c[2] = src[2] / 255.f;
				if (channel == 4) {
					// todo
				}
				ret[y * w + x] = RGBSpectrum::FromRGB(c);
			}
		}
		free(rgb);
		//VLOG(2) << StringPrintf("Read image %s (%d x %d)", name.c_str(), *width, *height);
		return std::unique_ptr<RGBSpectrum[]>(ret);
	}
}