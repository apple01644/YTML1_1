#include "../../../YTML1_1.hpp"

int main() {
	YTML1_1::Tree MainDisplay;
	size_t max_eid = 1;
	std::unordered_map<std::string, std::string> style;

	MainDisplay->eid = 0;
	MainDisplay->size = { 1280, 800 };
	
	YTML1_1::ReadCSS("somestyle.css", style);

	YTML1_1::ReadYTML1_1("sample.html", MainDisplay, style, max_eid);

	char buf[256];
	YTML1_1::RunYTML1_1(MainDisplay,
		[&](YTML1_1::Element& e, bool running)
		{
			sprintf_s(buf, "[%5.0f, %5.0f, %5.0f, %5.0f]", e.size_in_display.x, e.size_in_display.y, e.size_in_display.w, e.size_in_display.h);
			std::cout << buf << std::endl;
		}
	);
};