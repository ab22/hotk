#include <iostream>
#include <fstream>

#include "errors/errors.h"
#include "graphics/errors.h"
#include "graphics/graphics.h"

using namespace hotk;

int main()
{
	std::cout << "Starting program...\n";

	try {
		std::cout << "Initializing graphics module...\n";
		graphics::Graphics g;

		std::cout << "Capturing Screen...\n";
		auto screen_hbitmap = g.capture_screen();

		std::cout << "Grabbing image data...\n";
		auto bitmap = g.to_vector(screen_hbitmap.get());

		std::cout << "Saving to file..\n";
		// g.save_bitmap_to_file(L"Screenshot.bmp", screen_hbitmap.get());
		std::ofstream file("Screenshot.bmp", std::ios::binary);

		if (!file.is_open()) {
			std::cout << "Error: could not create file!\n";
			return 0;
		}

		file.write((const char*)bitmap.data(), bitmap.size());
		std::cout << "Done! Have a good day commander!\n";
	}
	catch (const errors::ErrorCode &err) {
		std::cout << "Unhandled error caught:\n"
			<< "   code: " << err.code() << "\n"
			<< "message: " << err.what() << "\n";
	}

	system("pause");
	return 0;
}