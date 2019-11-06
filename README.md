# YTML1_1
HTML과 유사한 형태의 코드를 이용해서 
문서 구조를 만들어주는 프로그램입니다.

그러나 모든 HTML코드의 문법과 CSS의 문법을 지원하지 않습니다.

# 요구사항

Visual studio C++17 혹은 이상의 버전

# Parse HTML code 예제

## somestyle.css
```css
.outter-block {
	width: 340px;
	height: 120px;
	border: 1 1 1 1;
	background-color: #aeaeae;
}	

.inner-block {
	margin:10 10 0 0;
	width: 100px;
	height: 100px;
	border: 1 1 1 1;
}	
```

## sample.html
```html
<div class = " outter-block ">
	<div class="inner-block"/>
	<div class="inner-block"/>
	<div class="inner-block"/>
</div>
```

## 코드
```cpp
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
```

## 결과
```
{class: outter-block }
{width:340px}
{height:120px}
{border:1 1 1 1}
{background-color:#aeaeae}
{class:inner-block}
{margin:10 10 0 0}
{width:100px}
{height:100px}
{border:1 1 1 1}
{class:inner-block}
{margin:10 10 0 0}
{width:100px}
{height:100px}
{border:1 1 1 1}
{class:inner-block}
{margin:10 10 0 0}
{width:100px}
{height:100px}
{border:1 1 1 1}
[    0,     0,  1280,   800]
[    0,     0,   340,   120]
[   10,    10,   100,   100]
[  120,    10,   100,   100]
[  230,    10,   100,   100]
```
