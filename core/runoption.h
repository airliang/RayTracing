#pragma once

namespace AIR
{
	struct RunOptions {
		RunOptions() {
			cropWindow[0][0] = 0;
			cropWindow[0][1] = 1;
			cropWindow[1][0] = 0;
			cropWindow[1][1] = 1;
		}
		int nThreads = 0;
		//bool quickRender = false;
		//bool quiet = false;
		//bool cat = false, toPly = false;
		std::string imageFile;
		// x0, x1, y0, y1
		Float cropWindow[2][2];
	};
}
