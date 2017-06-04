#include <iostream>

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
		HBITMAP hbScreen = g.capture_screen();

		std::cout << "Saving to file..\n";
		g.save_bitmap_to_file(L"Screenshot.bmp", hbScreen);

		std::cout << "Done! Have a good day commander!\n";
	}
	catch (errors::ErrorCode &err) {
		std::cout << "Unhandled error caught:\n"
			<< "   code: " << err.code() << "\n"
			<< "message: " << err.what() << "\n";
	}

	system("pause");
	return 0;
}