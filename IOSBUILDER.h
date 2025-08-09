

#ifndef IOSBUILDER_H
#define IOSBUILDER_H

#include <windows.h> // 包含所有 Win32 API 函数和宏
#include <windowsx.h> // 用于 GET_X_LPARAM, GET_Y_LPARAM
#include <stdio.h>   // 用于 sprintf, swprintf
#include <string.h>  // 用于 wcstombs_s
#include <stdlib.h>  // 用于 _wtoi (Wide char to int)
#include <math.h>

#define MAIN_WINDOW_CLASS_NAME TEXT("Win32iOSUIBuilderClass")
#define MAIN_WINDOW_TITLE      TEXT("我的 iOS UI Builder (Win32) - 带属性编辑")

// 控件ID定义 (用于识别子窗口和菜单项)
#define IDC_TOOLBOX_PANEL      1001
#define IDC_CANVAS_PANEL       1002
#define IDC_CODE_OUTPUT_PANEL  1003 // 现在这个ID用于父窗口句柄
#define IDC_CODE_DISPLAY_EDIT  1004 // 代码输出面板中的 EDIT 控件

// 工具箱中的控件按钮ID
#define IDC_TOOL_BUTTON        1101 // “iOS 按钮”工具
#define IDC_TOOL_TEXTFIELD     1102 // “iOS 文本框”工具
#define IDC_TOOL_TABLEVIEW		1103 //ios列表工具

// 属性面板中的控件ID
#define IDC_PROPERTIES_PANEL   1200 // 属性面板的容器 (目前未用作独立窗口，而是用作区域)
#define IDC_PROP_TYPE_LABEL    1201 // 类型标签
#define IDC_PROP_TYPE_STATIC   1202 // 类型显示 (修复为静态文本)
#define IDC_PROP_X_LABEL       1203 // X坐标标签
#define IDC_PROP_X_EDIT        1204 // X坐标编辑框
#define IDC_PROP_Y_LABEL       1205 // Y坐标标签
#define IDC_PROP_Y_EDIT        1206 // Y坐标编辑框
#define IDC_PROP_WIDTH_LABEL   1207 // 宽度标签
#define IDC_PROP_WIDTH_EDIT    1208 // 宽度编辑框
#define IDC_PROP_HEIGHT_LABEL  1209 // 高度标签
#define IDC_PROP_HEIGHT_EDIT   1210 // 高度编辑框
#define IDC_PROP_TEXT_LABEL    1211 // 文本标签 (智能切换为“标题”、“占位符”等)
#define IDC_PROP_TEXT_EDIT     1212 // 文本编辑框
#define IDC_PROP_CONFIRM_BUTTON 1213 // 新增：确定按钮

// 新增属性控件ID
#define IDC_PROP_BG_R_LABEL    1214
#define IDC_PROP_BG_R_EDIT     1215
#define IDC_PROP_BG_G_EDIT     1216
#define IDC_PROP_BG_B_EDIT     1217

#define IDC_PROP_TEXT_R_LABEL  1218
#define IDC_PROP_TEXT_R_EDIT   1219
#define IDC_PROP_TEXT_G_EDIT   1220
#define IDC_PROP_TEXT_B_EDIT   1221

#define IDC_PROP_FONT_SIZE_LABEL 1222
#define IDC_PROP_FONT_SIZE_EDIT  1223


// 生成的 iOS UI 元素的基ID (用于在画布上识别)
#define IDC_GENERATED_UI_BASE  2000

// 定义原始画布尺寸，用于比例计算
#define CANVAS_INITIAL_WIDTH  393
#define CANVAS_INITIAL_HEIGHT 852
const double CANVAS_ASPECT_RATIO = (double)CANVAS_INITIAL_WIDTH / CANVAS_INITIAL_HEIGHT; // Width / Height

// 定义一个结构体来存储画布上放置的 iOS UI 元素的信息

typedef struct {
	int id;           // 控件的唯一ID (用于内部识别，不是Objective-C代码中的ID)
	WCHAR type[50];   // 控件类型 (e.g., L"Button", L"TextField", L"TableView")

	// Position and size (always present for any element, no separate BOOL needed for visibility)
	int x, y, width, height;
	int original_x, original_y, original_width, original_height;

	// Unified text field for title, placeholder, etc.
	WCHAR text[256];
	BOOL hasText;     // Flag: Does this element have a 'text' property?

	// Background Color
	int bgColorR, bgColorG, bgColorB;
	BOOL hasBgColor;  // Flag: Does this element have a background color property?

	// Text Color
	int textColorR, textColorG, textColorB;
	BOOL hasTextColor; // Flag: Does this element have a text color property?

	// Font Size
	float fontSize;

	// Slider Specific Properties
	float minValue;
	BOOL hasMinValue; // Flag: Does this element have a min value property?
	float maxValue;
	BOOL hasMaxValue; // Flag: Does this element have a max value property?
	float currentValue;
	BOOL hasCurrentValue; // Flag: Does this element have a current value property?

	// TableView Specific Properties
	int rowHeight;
	BOOL hasRowHeight; // Flag: Does this element have a row height property?

	// Pointer to its definition for type-specific behavior (drawFunc, generateCodeFunc)
	struct UIElementDefinition* definition;

} iOSUIElement;

typedef struct UIElementDefinition {
	WCHAR typeName[50];
	int defaultWidth;
	int defaultHeight;

	// Pointers to functions that handle type-specific behavior
	void (*initDefaultDataFunc)(iOSUIElement* element);
	void (*drawFunc)(HDC hdc, iOSUIElement* element);
	void (*generateCodeFunc)(WCHAR* codeBuffer, size_t bufferSize, iOSUIElement* element);

} UIElementDefinition;

// 全局数组来存储画布上的 iOS UI 元素
#define MAX_UI_ELEMENTS 100 // 最大支持的 UI 元素数量
iOSUIElement g_iOSUIElements[MAX_UI_ELEMENTS];
int g_iOSUIElementCount = 0; // 当前已放置的 UI 元素数量
// 当前选中的 UI 元素索引 (-1 表示未选中)
int g_selectedElementIndex = -1;

// 全局 HWNDs，用于在不同函数中访问属性面板的控件和主面板
HWND g_hPropTypeStatic;
HWND g_hPropXEdit, g_hPropYEdit, g_hPropWidthEdit, g_hPropHeightEdit, g_hPropTextEdit;
HWND g_hPropConfirmButton; // 新增：确定按钮句柄

// 新增属性编辑框的全局句柄
HWND g_hPropBgR_Edit, g_hPropBgG_Edit, g_hPropBgB_Edit;
HWND g_hPropTextR_Edit, g_hPropTextG_Edit, g_hPropTextB_Edit;
HWND g_hPropFontSize_Edit;


HWND g_hCanvasPanel; // 画布面板的句柄，方便全局访问
HWND g_hCodeEdit;    // 代码编辑框的句柄，方便全局访问
HWND hToolboxPanel;  // 工具栏面板句柄
HWND hCodeOutputPanel; // 代码输出面板句柄 (现在是自定义类的句柄)

static HBRUSH bgbrush = NULL;


// 辅助函数：将 RGB 颜色值转换为 Objective-C 的 UIColor 代码字符串
void RgbToUIColorCode(int r, int g, int b, char* buffer, size_t bufferSize);

// 辅助函数：生成 Objective-C 按钮创建代码 (输出到 WCHAR 缓冲区)
void GenerateObjCButtonCode(WCHAR* codeBuffer, size_t bufferSize, iOSUIElement* element);
// 辅助函数：生成 Objective-C 文本框创建代码 (输出到 WCHAR 缓冲区)
void GenerateObjCTextFieldCode(WCHAR* codeBuffer, size_t bufferSize, iOSUIElement* element);
// 辅助函数：生成 Objective-C 表格视图创建代码 (输出到 WCHAR 缓冲区)
void GenerateObjCTableViewCode(WCHAR* codeBuffer, size_t bufferSize, iOSUIElement* element);

// 新增辅助函数：更新属性面板显示
void UpdatePropertiesPanel(void);
// 新增辅助函数：重新生成所有 Objective-C 代码
void RegenerateAllObjCCode(void);

#endif
