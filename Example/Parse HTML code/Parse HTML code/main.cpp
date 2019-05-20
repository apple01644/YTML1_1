#include "../../../YTML1_1.hpp"

int main() {
	YTML1_1::Tree MainDisplay;
	MainDisplay->size = { 1280, 800 };

	YTML1_1::ReadYTML1_1("sample.html", MainDisplay);
};