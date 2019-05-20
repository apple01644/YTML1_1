#include <DirectXMath.h>
#include <memory>
#include <vector>
#include <functional>
#include <iostream>
#include <unordered_map>
#include <fstream>
#include <string>
#include <string_view>
#include <charconv>

namespace YTML1_1
{
	enum class ElementFlag {
		RatioSizeWidth = 0b1,
		RatioSizeHeight = 0b10,
		RatioHorizontalAlign = 0b100,
		RatioVerticalAlign = 0b1000,
	};

	enum class ElementHorizontalAlign {
		Left, Center, Right
	};
	enum class ElementVerticalAlign {
		Top, Middle, Bottom
	};
	enum class ElementParentClipDirection {
		Horizontal, Vertical
	};

	struct FloatSize {
		float w = 0.f;
		float h = 0.f;
	};

	struct FloatRect {
		float x = 0.f;
		float y = 0.f;
		float w = 0.f;
		float h = 0.f;
	};

	void SplitByBlank(std::vector<std::string_view>& sv, const std::string& s)
	{
		size_t start = -1;
		for (size_t i = 0; i < s.size(); ++i)
		{
			if (s.at(i) == ' ')
			{
				if (start != -1)
				{
					sv.push_back(std::string_view(&s.at(start), i - start));
					//std::cout << s.substr(start, i - start) << std::endl;
					start = -1;
				}
			}
			else if (start == -1) start = i;
		}
		if (start != -1)
		{
			sv.push_back(std::string_view(&s.at(start), s.size() - start));
			//std::cout << s.substr(start, i - start) << std::endl;
			start = -1;
		}
	}

	struct Element {
		union {
			DirectX::XMFLOAT4 flt4;
			struct {
				float left;
				float top;
				float right;
				float bottom;
			};
		} margin;

		FloatSize size = { 0.f, 0.f };
		ElementHorizontalAlign halign = ElementHorizontalAlign::Left;
		ElementVerticalAlign valign = ElementVerticalAlign::Top;
		ElementParentClipDirection pclip = ElementParentClipDirection::Horizontal;

		uint16_t flags = 0;

		std::unordered_map<std::string, std::string> tuple;
		
		Element() {
			margin.flt4 = { 0.f, 0.f, 0.f, 0.f };
		}

		void tupleChanged(const std::string& key) {
			std::cout << "{" << key << ":" << tuple[key]  << "}" << std::endl;
			const auto& value = tuple[key];
			bool isWidth = key == "width";
			bool isHeight = key == "height";
			if (isWidth || isHeight)
			{
				size_t i, j, len;
				len = key.size();
				//Both Trip
				for (i = 0; i < value.size(); ++i)
				{
					if (value[i] != ' ') break;
				}
				for (j = value.size(); j > 1; --j)
				{
					if (value[j - 1] != ' ') break;
				}

				//Parse suffix %, px
				if (value[j - 2] == 'p' && value[j - 1] == 'x')
				{
					//std::cout << "|" << value.substr(i, j - i - 1) << "~px" << std::endl;
					if (isWidth)
					{
						std::from_chars(&value.at(i), &value.at(j - 1), size.w);
						//std::cout << value.substr(i, j - i - 1) << " = " << size.w << std::endl;
						//this->size.w = std::stof(std::string_view(&value.at(i), j - i - 1));
						flags &= ~(uint64_t)ElementFlag::RatioSizeWidth;
					}
					else if (isHeight)
					{
						std::from_chars(&value.at(i), &value.at(j - 1), size.h);
						//std::cout << value.substr(i, j - i - 1) << " = " << size.h << std::endl;
						flags &= ~(uint64_t)ElementFlag::RatioSizeHeight;
					}
				}
				else if (value[len - 1] == '%')
				{
					//std::cout << "|" << value.substr(i, j - i) << "~%" << std::endl;
					if (isWidth)
					{
						std::from_chars(&value.at(i), &value.at(j - 1), this->size.w);
						this->size.w /= 100.f;
						this->flags |= (uint64_t)ElementFlag::RatioSizeWidth;
					}
					else if (isHeight)
					{
						std::from_chars(&value.at(i), &value.at(j - 1), this->size.h);
						this->size.h /= 100.f;
						this->flags |= (uint64_t)ElementFlag::RatioSizeHeight;
					}
				}

				//std::cout << "|" << value.substr(i, j - i + 1) << "|" << value.substr(j - i, 1) << std::endl;

			}
			else if (key == "margin")
			{
				std::vector<std::string_view> sv;
				SplitByBlank(sv, value);
				int i = 0;
				if (i++ < sv.size())
				{
					const auto& v = sv.at(0);
					std::from_chars(v.data(), v.data() + v.size(), this->margin.left);
				}
				if (i++ < sv.size())
				{
					const auto& v = sv.at(1);
					std::from_chars(v.data(), v.data() + v.size(), this->margin.top);
				}
				if (i++ < sv.size())
				{
					const auto& v = sv.at(2);
					std::from_chars(v.data(), v.data() + v.size(), this->margin.right);
				}
				if (i++ < sv.size())
				{
					const auto& v = sv.at(3);
					std::from_chars(v.data(), v.data() + v.size(), this->margin.bottom);
				}
			}

		}
	};

	struct Tree {
		Element value;
		std::vector<Tree*> child;

		[[nodiscard]] Element* operator-> () {
			return &value;
		}
	};
		
	void RawLoopTree_L(const std::function<void(Element&)>& func, Tree& tree)
	{
		func(tree.value);
		for (size_t _ = 0; _ < tree.child.size(); ++_)
		{
			RawLoopTree_L(func, *tree.child[_]);
		}
	}

	void LoopTree_L(const std::function<FloatRect(Element&, FloatRect&)>& func, Tree& tree, FloatRect& rect)
	{
		auto r = func(tree.value, rect);
		for (size_t _ = 0; _ < tree.child.size(); ++_)
		{
			//std::cout << "index : " << _ << " / " << tree.child.size() << std::endl;
			LoopTree_L(func, *tree.child[_], rect);
		}
		rect = r;
	}
	void LoopTree_L(const std::function<FloatRect(Element&, FloatRect&)>& func, Tree& tree)
	{
		FloatRect rect = { 0.f, 0.f, tree->size.w, tree->size.h };
		auto r = func(tree.value, rect);
		
		for (size_t _ = 0; _ < tree.child.size(); ++_)
		{
			//std::cout << "index : " << _ << " / " << tree.child.size() << std::endl;
			LoopTree_L(func, *tree.child[_], rect);
		}
	}

	void RunYTML1_1(YTML1_1::Tree& MainDisplay, const std::function<void(Element&, FloatRect&)>& user_func)
	{
		YTML1_1::LoopTree_L([&](YTML1_1::Element & t, YTML1_1::FloatRect r)
			{
				YTML1_1::FloatRect rect = r;
				switch (t.halign)
				{
				case YTML1_1::ElementHorizontalAlign::Left:
					rect.x += t.margin.left;
					rect.w -= t.margin.left;
					break;
				case YTML1_1::ElementHorizontalAlign::Center:
					rect.x += (r.w - t.size.w) / 2;
					rect.w -= t.size.w;
					break;
				case YTML1_1::ElementHorizontalAlign::Right:
					rect.w -= t.margin.right;
					break;
				}
				switch (t.valign)
				{
				case YTML1_1::ElementVerticalAlign::Top:
					rect.y += t.margin.top;
					rect.h -= t.margin.top;
					break;
				case YTML1_1::ElementVerticalAlign::Middle:
					rect.y += (r.h - t.size.h) / 2;
					rect.h -= t.size.h;
					break;
				case YTML1_1::ElementVerticalAlign::Bottom:
					rect.h -= t.margin.bottom;
					break;
				}

				//[Will]Check Overflow
				rect.w = t.size.w;
				rect.h = t.size.h;
				//
				user_func(t, rect);

				//Parent Clip
				switch (t.pclip)
				{
				case ElementParentClipDirection::Horizontal:
					switch (t.halign)
					{
					case YTML1_1::ElementHorizontalAlign::Left:
						r.x += t.margin.left + rect.w;
						r.w -= t.margin.left + rect.w;
						break;
					case YTML1_1::ElementHorizontalAlign::Right:
						r.w -= t.margin.right + rect.w;
						break;
					}
					break;
				case ElementParentClipDirection::Vertical:
					switch (t.valign)
					{
					case YTML1_1::ElementVerticalAlign::Top:
						r.y += t.margin.top + rect.h;
						r.h -= t.margin.top + rect.h;
						break;
					case YTML1_1::ElementVerticalAlign::Bottom:
						r.h -= t.margin.bottom + rect.h;
						break;
					}
					break;
				}
				return r;
			},
			MainDisplay);
	}

	bool PossibleVariablename(const char& c)
	{
		return c == '_' || (c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
	}

	void ParseInnerBracket(YTML1_1::Element & e, const std::string_view& s)
	{
		std::string_view header = "";
		std::string key;
		size_t last = 0, i;
		char c;

		//Get header
		for (i = 0; i < s.size(); ++i)
		{
			c = s[i];
			if (c == ' ') {
				header = std::string_view(&s.at(last), i - last);
				last = i + 1;
				break;
			}
		}

		if (last > 0 && last < s.size())
		{
			//Get Trainer
			bool wait_for_equal_sign = true;
			size_t str_index;
			bool str_open = false;
			bool wait_for_varname = true;

			for (i = last; i < s.size(); ++i)
			{
				c = s[i];
				if (wait_for_equal_sign) {
					if (PossibleVariablename(c))
					{
						if (!wait_for_varname)
						{
							last = i;
							wait_for_varname = true;
						}
					}
					else if (c == ' ') {
						wait_for_varname = false;
					}
					else if (c == '=') {
						size_t end = i;
						for (; s.at(end - 1) == ' '; --end);
						//Get key
						key = s.substr(last, end - last);
						wait_for_equal_sign = false;
					}
				}
				else {
					if (c == '"') {
						if (str_open) {
							//Get value

							e.tuple[key] = s.substr(str_index, i - str_index);
							e.tupleChanged(key);

							wait_for_equal_sign = true;
							last = i + 1;
						}
						else {
							str_index = i + 1;
						}
						str_open = !str_open;
					}
				}
			}
		}
		else
		{
			header = s;
		}
	}

	void ReadYTML1_1(const std::string& path, YTML1_1::Tree MainDisplay)
	{
		std::ifstream file(path);
		std::vector<size_t> ind;
		
		MainDisplay->size = { 1280, 800 };

		std::vector<YTML1_1::Tree*> Parent;
		Parent.push_back(&MainDisplay);


		if (file.bad()) return;

		std::string str((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

		bool forward_close = false;
		bool back_close = false;

		char c;
		int depth = 0;
		for (size_t i = 0; i < str.size(); ++i)
		{
			c = str[i];
			switch (c)
			{
			case '<':
				//cout << c;
				ind.push_back(i + 1);

				if (str[i + 1] == '/') forward_close = true;
				break;
			case '>':

				if (forward_close)
				{
					--depth;
					//cout << '/' << depth << c;
					Parent.pop_back();
				}
				else if (back_close)
				{
					//cout << depth << '/' << c;
					YTML1_1::Tree* e = new YTML1_1::Tree();

					ParseInnerBracket(e->value, std::string_view(&str.at(*ind.crbegin()), i - *ind.crbegin() - 1));
					//std::cout << "( " << e->value.size.w << ", " << e->value.size.h << " )" << std::endl;
					(*Parent.rbegin())->child.push_back(e);
				}
				else
				{
					//cout << depth << c;
					YTML1_1::Tree* e = new YTML1_1::Tree();
					
					ParseInnerBracket(e->value, std::string_view(&str.at(*ind.crbegin()), i - *ind.crbegin()));
					//std::cout << "( " << e->value.size.w << ", " << e->value.size.h << " )" << std::endl;

					(*Parent.rbegin())->child.push_back(e);
					Parent.push_back(e);

					++depth;
				}
				ind.pop_back();

				forward_close = false;
				back_close = false;
				break;
			case '/':
				if (str[i + 1] == '>') back_close = true;
				break;
			}
		}
		
		YTML1_1::RunYTML1_1(MainDisplay,
			[](YTML1_1::Element & e, YTML1_1::FloatRect & f) {
				std::cout << f.x << ", " << f.y << ", " << f.w << ", " << f.h << " <- ( " << e.size.w << ", " << e.size.h << ")"   << std::endl;
			}
		);
	}
}
