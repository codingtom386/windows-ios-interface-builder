/*#include <windows.h> // 包含所有 Win32 API 函数和宏
#include <windowsx.h> // 用于 GET_X_LPARAM, GET_Y_LPARAM
#include <stdio.h>   // 用于 sprintf, swprintf
#include <string.h>  // 用于 wcstombs_s
#include <stdlib.h>  // 用于 _wtoi (Wide char to int)
#include <math.h>    // For round


// #include <locale.h>  // 移除：不再需要 setlocale

// --- 全局变量和宏定义 ---
// 窗口类名和窗口标题
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

// --- 函数声明 ---
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK CanvasPanelProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK toolpanelproc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam); // 工具箱面板的 WndProc
LRESULT CALLBACK CodePropertiesPanelProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam); // 新增：代码/属性面板的窗口过程

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
*/
// --- WinMain 函数：应用程序入口点 ---
#include "IOSBUILDER.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	WNDCLASSEX wc = {0}; // 主窗口类结构体
	WNDCLASSEX wcCanvas = {0}; // 画布面板窗口类结构体
	WNDCLASSEX wcTool = {0}; // 工具箱面板
	WNDCLASSEX wcCodeProp = {0}; // 新增：代码/属性面板窗口类结构体

	MSG msg;             // 消息结构体

	// 1. 注册主窗口类
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.cbSize        = sizeof(WNDCLASSEX);
	wc.lpfnWndProc   = WndProc;
	wc.hInstance     = hInstance;
	wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = CreateSolidBrush(RGB(66, 66, 66)); // 浅灰色背景
	wc.lpszClassName = MAIN_WINDOW_CLASS_NAME;

	if (!RegisterClassEx(&wc)) {
		MessageBox(NULL, TEXT("主窗口类注册失败!"), TEXT("错误"), MB_ICONERROR | MB_OK);
		return 0;
	}

	// 2. 注册画布面板窗口类 (需要自定义绘制)
	wcCanvas.style = CS_HREDRAW | CS_VREDRAW;
	wcCanvas.cbSize        = sizeof(WNDCLASSEX);
	wcCanvas.lpfnWndProc   = CanvasPanelProc; // 使用自定义的窗口过程函数
	wcCanvas.hInstance     = hInstance;
	wcCanvas.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wcCanvas.hbrBackground = (HBRUSH)GetStockObject(GRAY_BRUSH); // 白色背景
	wcCanvas.lpszClassName = TEXT("CanvasPanelClass"); // 自定义画布类名

	if (!RegisterClassEx(&wcCanvas)) {
		MessageBox(NULL, TEXT("画布面板类注册失败!"), TEXT("错误"), MB_ICONERROR | MB_OK);
		return 0;
	}

	// 3. 注册工具箱面板窗口类 (主要用于管理其子控件，WM_COMMAND 会转发到父窗口)
	wcTool.cbSize        = sizeof(WNDCLASSEX);
	wcTool.lpfnWndProc   = toolpanelproc; // 使用自定义的窗口过程函数，用于处理其内部按钮点击
	wcTool.hInstance     = hInstance;
	wcTool.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wcTool.hbrBackground = CreateSolidBrush(RGB(66, 66, 66)); // 灰色背景
	wcTool.lpszClassName = TEXT("ToolPanelClass"); // 自定义工具箱类名

	if (!RegisterClassEx(&wcTool)) {
		MessageBox(NULL, TEXT("工具栏类注册失败!"), TEXT("错误"), MB_ICONERROR | MB_OK);
		return 0;
	}

	// 4. 新增：注册代码/属性面板窗口类
	wcCodeProp.cbSize        = sizeof(WNDCLASSEX);
	wcCodeProp.lpfnWndProc   = CodePropertiesPanelProc; // 使用自定义的窗口过程函数
	wcCodeProp.hInstance     = hInstance;
	wcCodeProp.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wcCodeProp.hbrBackground = CreateSolidBrush(RGB(66, 66, 66)); // 浅灰色背景
	wcCodeProp.lpszClassName = TEXT("CodePropertiesPanelClass"); // 自定义类名

	if (!RegisterClassEx(&wcCodeProp)) {
		MessageBox(NULL, TEXT("代码/属性面板类注册失败!"), TEXT("错误"), MB_ICONERROR | MB_OK);
		return 0;
	}

	// 5. 创建主窗口
	HWND hMainWnd = CreateWindowEx(
	                    0,                                  // 扩展窗口样式
	                    MAIN_WINDOW_CLASS_NAME,             // 窗口类名
	                    MAIN_WINDOW_TITLE,                  // 窗口标题
	                    WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN, // 窗口样式 (WS_CLIPCHILDREN 防止子窗口重绘时闪烁)
	                    CW_USEDEFAULT, CW_USEDEFAULT,       // 初始X, 初始Y
	                    1500, 932,                          // 宽度, 高度 (示例尺寸)
	                    NULL,                               // 父窗口句柄 (桌面窗口)
	                    NULL,                               // 菜单句柄
	                    hInstance,                          // 应用程序实例句柄
	                    NULL                                // 创建参数
	                );

	if (!hMainWnd) {
		MessageBox(NULL, TEXT("主窗口创建失败!"), TEXT("错误"), MB_ICONERROR | MB_OK);
		return 0;
	}

	// 6. 创建子面板：工具箱、画布、代码输出
	// 这些是 STATIC 控件，作为容器使用，并设置 WS_BORDER 样式以便区分

	// 工具箱面板 (左侧)
	hToolboxPanel = CreateWindowEx(
	                    WS_EX_CLIENTEDGE,                   // 扩展样式：3D边框
	                    TEXT("ToolPanelClass"),             // 控件类名
	                    TEXT("控件栏"),                       // 文本
	                    WS_CHILD | WS_VISIBLE | SS_CENTER,  // 子窗口，可见，文本居中
	                    10, 10,                            // X, Y 坐标 (相对于主窗口)
	                    200, 800,                           // 宽度, 高度 (调整尺寸)
	                    hMainWnd,                           // 父窗口句柄
	                    (HMENU)IDC_TOOLBOX_PANEL,           // 控件ID
	                    hInstance,                          // 应用程序实例句柄
	                    NULL                                // 创建参数
	                );

	// 设计画布面板 (中间)
	// 这些是初始值，实际显示时会根据 WM_SIZE 中的比例逻辑调整
	g_hCanvasPanel = CreateWindowEx( // 存储到全局变量
	                     WS_EX_CLIENTEDGE,                   // 扩展样式：3D边框
	                     TEXT("CanvasPanelClass"),           // 使用我们自定义的画布类名
	                     TEXT(""),                           // 无文本，因为我们要自己绘制内容
	                     WS_CHILD | WS_VISIBLE,              // 子窗口，可见
	                     220, 10,                            // 初始 X, Y 坐标
	                     CANVAS_INITIAL_WIDTH, CANVAS_INITIAL_HEIGHT, // 初始 宽度, 高度 (会在 WM_SIZE 中按比例调整)
	                     hMainWnd,                           // 父窗口句柄
	                     (HMENU)IDC_CANVAS_PANEL,            // 控件ID
	                     hInstance,                          // 应用程序实例句柄
	                     NULL                                // 创建参数
	                 );

	// 代码/属性输出面板 (右侧) - 现在使用自定义类
	hCodeOutputPanel = CreateWindowEx(
	                       WS_EX_CLIENTEDGE,                   // 扩展样式：3D边框
	                       TEXT("CodePropertiesPanelClass"),   // 使用自定义类名
	                       TEXT(""),                           // 无文本，子控件会提供标题
	                       WS_CHILD | WS_VISIBLE | ES_AUTOVSCROLL,            // 子窗口，可见
	                       930, 10,                            // X, Y 坐标
	                       550, 800,                           // 宽度, 高度 (调整尺寸)
	                       hMainWnd,                           // 父窗口句柄
	                       (HMENU)IDC_CODE_OUTPUT_PANEL,       // 控件ID
	                       hInstance,                          // 应用程序实例句柄
	                       NULL                                // 创建参数
	                   );

	// 注意：代码输出面板内的子控件（g_hCodeEdit, g_hPropXEdit 等）的创建
	// 现在移到了 CodePropertiesPanelProc 的 WM_CREATE 消息处理中。
	// 这样可以确保这些控件是 hCodeOutputPanel 的子窗口。

	// 7. 在工具箱面板中添加可“拖拽”的控件按钮 (目前是点击模拟)
	CreateWindowEx(
	    0,                                  // 扩展样式
	    TEXT("BUTTON"),                     // 控件类名
	    TEXT("iOS 按钮"),                     // 按钮文本
	    WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, // 子窗口，可见，普通按钮
	    20, 50,                             // X, Y 坐标 (相对于 hToolboxPanel)
	    160, 40,                            // 宽度, 高度
	    hToolboxPanel,                      // 父窗口句柄 (工具箱面板)
	    (HMENU)IDC_TOOL_BUTTON,             // 控件ID
	    hInstance,                          // 应用程序实例句柄
	    NULL                                // 创建参数
	);

	CreateWindowEx(
	    0,
	    TEXT("BUTTON"),
	    TEXT("iOS 文本框"),
	    WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
	    20, 100,
	    160, 40,
	    hToolboxPanel,
	    (HMENU)IDC_TOOL_TEXTFIELD,
	    hInstance,
	    NULL
	);

	CreateWindowEx(
	    0,
	    TEXT("BUTTON"),
	    TEXT("iOS 表格视图"),
	    WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
	    20, 150,
	    160, 40,
	    hToolboxPanel,
	    (HMENU)IDC_TOOL_TABLEVIEW,
	    hInstance,
	    NULL
	);


	// 8. 显示和更新主窗口
	ShowWindow(hMainWnd, nCmdShow);
	UpdateWindow(hMainWnd);

	// 初始更新属性面板 (显示未选中状态)
	UpdatePropertiesPanel();
	// 初始代码生成
	RegenerateAllObjCCode();


	// 9. 消息循环
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg); // 翻译虚拟键消息为字符消息
		DispatchMessage(&msg);  // 分发消息给窗口过程函数
	}


	return (int)msg.wParam;
}

// --- 窗口过程函数实现 ---

// 主窗口过程函数
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	WCHAR debugMsg[256]; // 用于 MessageBox 的调试信息

	switch (msg) {
		case WM_CREATE: {
			if (bgbrush == NULL) {
				bgbrush = CreateSolidBrush(RGB(66, 66, 66));
			}
			if (bgbrush == NULL) {
				MessageBox(NULL, TEXT("画刷创建失败!"), TEXT("ERROR"), MB_ICONERROR | MB_OK);
				return 0;
			}
			break;
		}
		case WM_COMMAND: {
			int wmId = LOWORD(wParam); // 控件ID
			// int wmEvent = HIWORD(wParam); // 通知代码 (现在只在确定按钮时处理，不需要区分通知码)
			// HWND hCtrl = (HWND)lParam; // 控件句柄

			// 处理属性编辑框的通知 (现在只在点击确定按钮时处理)
			if (wmId == IDC_PROP_CONFIRM_BUTTON) {
				if (g_selectedElementIndex != -1) {
					iOSUIElement *element = &g_iOSUIElements[g_selectedElementIndex];
					WCHAR buffer[256]; // 临时缓冲区
					int value;
					float floatValue;

					// 读取 X 坐标
					GetWindowTextW(g_hPropXEdit, buffer, _countof(buffer));
					value = _wtoi(buffer);
					element->x = value;

					// 读取 Y 坐标
					GetWindowTextW(g_hPropYEdit, buffer, _countof(buffer));
					value = _wtoi(buffer);
					element->y = value;

					// 读取 Width
					GetWindowTextW(g_hPropWidthEdit, buffer, _countof(buffer));
					value = _wtoi(buffer);
					element->width = value;

					// 读取 Height
					GetWindowTextW(g_hPropHeightEdit, buffer, _countof(buffer));
					value = _wtoi(buffer);
					element->height = value;

					// 读取 Text (按钮标题或文本框内容)
					GetWindowTextW(g_hPropTextEdit, buffer, _countof(buffer));
					wcscpy_s(element->text, _countof(element->text), buffer);

					// 读取背景颜色 (R, G, B)
					GetWindowTextW(g_hPropBgR_Edit, buffer, _countof(buffer));
					element->bgColorR = _wtoi(buffer);
					GetWindowTextW(g_hPropBgG_Edit, buffer, _countof(buffer));
					element->bgColorG = _wtoi(buffer);
					GetWindowTextW(g_hPropBgB_Edit, buffer, _countof(buffer));
					element->bgColorB = _wtoi(buffer);

					// 读取文本颜色 (R, G, B)
					GetWindowTextW(g_hPropTextR_Edit, buffer, _countof(buffer));
					element->textColorR = _wtoi(buffer);
					GetWindowTextW(g_hPropTextG_Edit, buffer, _countof(buffer));
					element->textColorG = _wtoi(buffer);
					GetWindowTextW(g_hPropTextB_Edit, buffer, _countof(buffer));
					element->textColorB = _wtoi(buffer);

					// 读取字体大小
					GetWindowTextW(g_hPropFontSize_Edit, buffer, _countof(buffer));
					// 使用 _wtof 将宽字符浮点数转换为 float
					floatValue = (float)_wtof(buffer);
					element->fontSize = floatValue;

					// --- 移除导致 bug 的代码块：文本框内容改变第一个按钮文字的逻辑 ---
					// if (wcscmp(element->type, L"TextField") == 0) {
					//     for (int i = 0; i < g_iOSUIElementCount; i++) {
					//         if (wcscmp(g_iOSUIElements[i].type, L"Button") == 0) {
					//             wcscpy_s(g_iOSUIElements[i].text, _countof(g_iOSUIElements[i].text), buffer);
					//             break;
					//         }
					//     }
					// }
					// --- 移除结束 ---

					// 获取画布当前实际尺寸，用于计算反向比例
					RECT currentCanvasRect;
					GetClientRect(g_hCanvasPanel, &currentCanvasRect);
					int currentCanvasWidth = currentCanvasRect.right;

					if (currentCanvasWidth > 0) { // Avoid division by zero
						double currentScaleForElements = (double)currentCanvasWidth / CANVAS_INITIAL_WIDTH;

						// 反向计算并更新 original_x/y/width/height
						element->original_x = (int)round(element->x / currentScaleForElements);
						element->original_y = (int)round(element->y / currentScaleForElements);
						element->original_width = (int)round(element->width / currentScaleForElements);
						element->original_height = (int)round(element->height / currentScaleForElements);
					}

					// 强制画布重绘，显示属性变化
					InvalidateRect(g_hCanvasPanel, NULL, TRUE);

					// 重新生成代码
					RegenerateAllObjCCode();
				}
			}
			break;
		}
		case WM_SIZE: {
			// Adjust panel sizes on window resize
			RECT rc;
			GetClientRect(hwnd, &rc);
			int newWidth = rc.right - rc.left;
			int newHeight = rc.bottom - rc.top;

			int toolboxWidth = 200; // Fixed width
			int codePanelWidth = 550; // Fixed width for code/properties panel
			int spacing = 10; // Spacing between panels and window edges

			int canvasX = toolboxWidth + spacing;
			int codePanelX = newWidth - codePanelWidth - spacing;

			// Move Toolbox Panel
			MoveWindow(hToolboxPanel, spacing, spacing, toolboxWidth, newHeight - (2 * spacing), TRUE);
			// Move Code Output Panel (hCodeOutputPanel)
			MoveWindow(hCodeOutputPanel, codePanelX, spacing, codePanelWidth, newHeight - (2 * spacing), TRUE);

			// Calculate the available space for the canvas panel
			int canvasAvailableWidth = codePanelX - canvasX - spacing;
			int canvasAvailableHeight = newHeight - (2 * spacing);

			// Ensure minimum size
			if (canvasAvailableWidth < 1) canvasAvailableWidth = 1;
			if (canvasAvailableHeight < 1) canvasAvailableHeight = 1;

			int finalCanvasWidth = 0;
			int finalCanvasHeight = 0;

			// Calculate dimensions based on maintaining canvas's original aspect ratio (700x800)
			// Option 1: Constrained by available height
			int tempWidth = (int)(canvasAvailableHeight * CANVAS_ASPECT_RATIO);
			if (tempWidth <= canvasAvailableWidth) {
				// Height is the limiting factor, or fits perfectly
				finalCanvasWidth = tempWidth;
				finalCanvasHeight = canvasAvailableHeight;
			} else {
				// Width is the limiting factor
				finalCanvasWidth = canvasAvailableWidth;
				finalCanvasHeight = (int)(canvasAvailableWidth / CANVAS_ASPECT_RATIO);
			}

			// Ensure dimensions are not zero or negative
			if (finalCanvasWidth < 1) finalCanvasWidth = 1;
			if (finalCanvasHeight < 1) finalCanvasHeight = 1;

			// Calculate centered position within the available area for the canvas panel
			int centeredX = canvasX + (canvasAvailableWidth - finalCanvasWidth) / 2;
			int centeredY = spacing + (canvasAvailableHeight - finalCanvasHeight) / 2;

			// Move the canvas panel (g_hCanvasPanel) to its new size and centered position
			MoveWindow(g_hCanvasPanel, centeredX, centeredY, finalCanvasWidth, finalCanvasHeight, TRUE);

			// --- 重要：根据新的画布尺寸调整其上所有元素的尺寸和位置 ---
			// 计算当前画布相对于其原始尺寸的缩放比例
			if (CANVAS_INITIAL_WIDTH > 0) { // 避免除以零
				double currentScale = (double)finalCanvasWidth / CANVAS_INITIAL_WIDTH;

				for (int i = 0; i < g_iOSUIElementCount; i++) {
					iOSUIElement *element = &g_iOSUIElements[i];
					// 使用元素的 original_x/y/width/height 来计算其在当前缩放下的实际显示尺寸
					element->x = (int)round(element->original_x * currentScale);
					element->y = (int)round(element->original_y * currentScale);
					element->width = (int)round(element->original_width * currentScale);
					element->height = (int)round(element->original_height * currentScale);

					// 确保元素不会缩小到看不见
					if (element->width < 1) element->width = 1;
					if (element->height < 1) element->height = 1;
				}
				// 强制画布重绘所有元素，更新其显示
				InvalidateRect(g_hCanvasPanel, NULL, TRUE);
				// 更新属性面板以反映新的坐标（虽然是自动缩放的，但也要显示当前值）
				UpdatePropertiesPanel();
				// 重新生成代码以确保代码中的坐标是最新的（虽然代码是基于original_x，但为了统一显示）
				RegenerateAllObjCCode();
			}

			// Note: The child controls within hCodeOutputPanel are handled by CodePropertiesPanelProc's WM_SIZE
			break;
		}
		case WM_DESTROY: {
			if (bgbrush != NULL) {
				DeleteObject(bgbrush);
				bgbrush = NULL; // 防止重复删除
			}
			PostQuitMessage(0);
			break;
		}

		default:
			return DefWindowProc(hwnd, msg, wParam, lParam);

	}
	return 0;
}

// 画布面板的窗口过程函数 (负责绘制放置的 UI 元素和处理点击选择、拖拽、缩放)
LRESULT CALLBACK CanvasPanelProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	static int mouseX, mouseY; // 当前鼠标位置
	static BOOL isDragging = FALSE; // 是否正在拖拽元素
	static int dragOffset_X, dragOffset_Y; // 拖拽时鼠标点击点与元素左上角的偏移
	static int resizeHandle = -1; // -1: 无, 0: 左上, 1: 右上, 2: 右下, 3: 左下
	const int GRAB_AREA = 5; // 边缘捕获区域大小

	WCHAR debugMsg[256]; // 用于 MessageBox 的调试信息

	switch (msg) {
		case WM_PAINT: {
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hwnd, &ps);

			// 遍历并绘制所有已放置的 iOS UI 元素
			for (int i = 0; i < g_iOSUIElementCount; i++) {
				iOSUIElement *element = &g_iOSUIElements[i];
				// 绘制使用 element->x/y/width/height，这些值已经在 WM_SIZE 中被更新为当前缩放后的值
				RECT elementRect = {element->x, element->y, element->x + element->width, element->y + element->height};

				// 绘制边框和填充
				HPEN hPen;
				HBRUSH hBrush;

				// 根据类型选择颜色和字体
				if (wcscmp(element->type, L"Button") == 0) {
					hPen = CreatePen(PS_SOLID, 1, RGB(element->bgColorR, element->bgColorG, element->bgColorB)); // 使用背景色作为边框色
					hBrush = CreateSolidBrush(RGB(element->bgColorR, element->bgColorG, element->bgColorB)); // 使用背景色填充
				} else if (wcscmp(element->type, L"TextField") == 0) {
					// TextField 边框和背景色通常是固定的或由系统控制，这里使用默认值
					hPen = CreatePen(PS_SOLID, 1, RGB(0, 100, 0)); // 绿色边框
					hBrush = CreateSolidBrush(RGB(200, 255, 200)); // 浅绿色填充
				} else if (wcscmp(element->type, L"TableView") == 0) {
					hPen = CreatePen(PS_SOLID, 1, RGB(100, 0, 100)); // 紫色边框
					hBrush = CreateSolidBrush(RGB(255, 200, 255)); // 浅紫色填充
				} else {
					hPen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0)); // 默认黑色边框
					hBrush = (HBRUSH)GetStockObject(WHITE_BRUSH); // 默认白色填充
				}

				HPEN hOldPen = SelectObject(hdc, hPen);
				HBRUSH hOldBrush = SelectObject(hdc, hBrush);

				Rectangle(hdc, elementRect.left, elementRect.top, elementRect.right, elementRect.bottom);

				SelectObject(hdc, hOldPen);
				SelectObject(hdc, hOldBrush);
				DeleteObject(hPen);
				DeleteObject(hBrush); // 记得删除创建的刷子

				// 绘制文本
				SetBkMode(hdc, TRANSPARENT); // 背景透明
				SetTextColor(hdc, RGB(element->textColorR, element->textColorG, element->textColorB)); // 设置文本颜色

				// 创建字体
				HFONT hFont = CreateFont(
				                  -(int)element->fontSize, // 负值表示字符高度
				                  0, 0, 0,
				                  FW_NORMAL, // 字重
				                  FALSE, FALSE, FALSE,
				                  DEFAULT_CHARSET,
				                  OUT_DEFAULT_PRECIS,
				                  CLIP_DEFAULT_PRECIS,
				                  DEFAULT_QUALITY,
				                  DEFAULT_PITCH | FF_SWISS,
				                  TEXT("Arial") // 示例字体，可以替换为其他字体
				              );
				HFONT hOldFont = SelectObject(hdc, hFont);

				DrawTextW(hdc, element->text, -1, &elementRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);

				SelectObject(hdc, hOldFont); // 恢复旧字体
				DeleteObject(hFont); // 删除创建的字体
			}

			EndPaint(hwnd, &ps);
			break;
		}
		case WM_LBUTTONDOWN: {
			mouseX = GET_X_LPARAM(lParam);
			mouseY = GET_Y_LPARAM(lParam);
			SetCapture(hwnd); // 捕获鼠标，确保即使鼠标移出窗口也能继续处理事件

			int oldSelected = g_selectedElementIndex; // 记录旧的选中索引
			g_selectedElementIndex = -1; // 默认未选中

			// 首先检查是否点击到缩放手柄 (如果已有选中元素)
			if (oldSelected != -1) {
				iOSUIElement *element = &g_iOSUIElements[oldSelected];
				RECT r = {element->x, element->y, element->x + element->width, element->y + element->height};

				if (mouseX >= r.left - GRAB_AREA && mouseX <= r.left + GRAB_AREA &&
				        mouseY >= r.top - GRAB_AREA && mouseY <= r.top + GRAB_AREA) {
					resizeHandle = 0; // Top-Left
					g_selectedElementIndex = oldSelected; // 保持选中
				} else if (mouseX >= r.right - GRAB_AREA && mouseX <= r.right + GRAB_AREA &&
				           mouseY >= r.top - GRAB_AREA && mouseY <= r.top + GRAB_AREA) {
					resizeHandle = 1; // Top-Right
					g_selectedElementIndex = oldSelected;
				} else if (mouseX >= r.right - GRAB_AREA && mouseX <= r.right + GRAB_AREA &&
				           mouseY >= r.bottom - GRAB_AREA && mouseY <= r.bottom + GRAB_AREA) {
					resizeHandle = 2; // Bottom-Right
					g_selectedElementIndex = oldSelected;
				} else if (mouseX >= r.left - GRAB_AREA && mouseX <= r.left + GRAB_AREA &&
				           mouseY >= r.bottom - GRAB_AREA && mouseY <= r.bottom + GRAB_AREA) {
					resizeHandle = 3; // Bottom-Left
					g_selectedElementIndex = oldSelected;
				}
			}

			if (resizeHandle == -1) { // 如果没有点击到缩放手柄，则检查是否点击到元素或空白处
				// 遍历所有元素，检查点击是否在某个元素的边界内
				for (int i = g_iOSUIElementCount - 1; i >= 0; i--) { // 从后往前遍历，确保点击到最上层的元素
					iOSUIElement *element = &g_iOSUIElements[i];
					RECT elementRect = {element->x, element->y, element->x + element->width, element->y + element->height};

					if (PtInRect(&elementRect, (POINT) {
					mouseX, mouseY
				})) {
						g_selectedElementIndex = i; // 选中这个元素
						isDragging = TRUE; // 开始拖拽
						dragOffset_X = mouseX - element->x;
						dragOffset_Y = mouseY - element->y;
						break; // 找到第一个就退出
					}
				}
			}

			// 如果选中状态改变（包括从选中到未选中），更新属性面板并重绘画布
			if (oldSelected != g_selectedElementIndex) {
				UpdatePropertiesPanel();
				InvalidateRect(hwnd, NULL, TRUE); // 重绘画布，更新选中高亮
			} else if (g_selectedElementIndex != -1 && (isDragging || resizeHandle != -1)) {
				// 如果是同一个元素，但开始了拖拽或缩放，也需要更新属性面板 (以便显示鼠标实时更新的坐标)
				UpdatePropertiesPanel();
				InvalidateRect(hwnd, NULL, TRUE);
			}
			break;
		}

		case WM_MOUSEMOVE: {
			mouseX = GET_X_LPARAM(lParam);
			mouseY = GET_Y_LPARAM(lParam);

			if (g_selectedElementIndex != -1 && (isDragging || resizeHandle != -1)) {
				iOSUIElement *element = &g_iOSUIElements[g_selectedElementIndex];

				// 获取画布当前实际尺寸
				RECT currentCanvasRect;
				GetClientRect(hwnd, &currentCanvasRect);
				int currentCanvasWidth = currentCanvasRect.right;
				int currentCanvasHeight = currentCanvasRect.bottom;

				// 计算当前画布相对于其原始尺寸的缩放比例
				double currentScaleForElements = (currentCanvasWidth > 0) ? ((double)currentCanvasWidth / CANVAS_INITIAL_WIDTH) : 1.0;

				int newX = element->x;
				int newY = element->y;
				int newWidth = element->width;
				int newHeight = element->height;

				if (isDragging) {
					newX = mouseX - dragOffset_X;
					newY = mouseY - dragOffset_Y;

					// 限制元素不超出画布边界
					if (newX < 0) newX = 0;
					if (newY < 0) newY = 0;
					if (newX + element->width > currentCanvasWidth) newX = currentCanvasWidth - element->width;
					if (newY + element->height > currentCanvasHeight) newY = currentCanvasHeight - element->height;

					element->x = newX;
					element->y = newY;

				} else if (resizeHandle != -1) {
					int originalElementX = element->x;
					int originalElementY = element->y;
					int originalElementWidth = element->width;
					int originalElementHeight = element->height;

					switch (resizeHandle) {
						case 0: // Top-Left
							newX = mouseX;
							newY = mouseY;
							newWidth = originalElementX + originalElementWidth - newX;
							newHeight = originalElementY + originalElementHeight - newY;
							break;
						case 1: // Top-Right
							newX = originalElementX; // X 保持不变
							newY = mouseY;
							newWidth = mouseX - originalElementX;
							newHeight = originalElementY + originalElementHeight - newY;
							break;
						case 2: // Bottom-Right
							newX = originalElementX; // X 保持不变
							newY = originalElementY; // Y 保持不变
							newWidth = mouseX - originalElementX;
							newHeight = mouseY - originalElementY;
							break;
						case 3: // Bottom-Left
							newX = mouseX;
							newY = originalElementY; // Y 保持不变
							newWidth = originalElementX + originalElementWidth - newX;
							newHeight = mouseY - element->original_y;
							break;
					}

					// 确保宽度和高度至少为1，并且不超出画布边界
					if (newWidth < 1) newWidth = 1;
					if (newHeight < 1) newHeight = 1;

					// 限制元素不超出画布边界 (针对缩放后的新位置和大小)
					if (newX < 0) {
						newWidth += newX; // Adjust width if X goes negative
						newX = 0;
					}
					if (newY < 0) {
						newHeight += newY; // Adjust height if Y goes negative
						newY = 0;
					}
					if (newX + newWidth > currentCanvasWidth) newWidth = currentCanvasWidth - newX;
					if (newY + newHeight > currentCanvasHeight) newHeight = currentCanvasHeight - newY;

					element->x = newX;
					element->y = newY;
					element->width = newWidth;
					element->height = newHeight;
				}

				// --- 实时更新元素的 original_* 属性 ---
				// 只有当 currentScaleForElements 不为零时才进行计算，避免除以零
				if (currentScaleForElements != 0) {
					element->original_x = (int)round(element->x / currentScaleForElements);
					element->original_y = (int)round(element->y / currentScaleForElements);
					element->original_width = (int)round(element->width / currentScaleForElements);
					element->original_height = (int)round(element->height / currentScaleForElements);
				}


				UpdatePropertiesPanel(); // 实时更新属性面板
				InvalidateRect(hwnd, NULL, TRUE); // 实时重绘
			} else {
				// 未拖拽或缩放时，根据鼠标位置改变光标形状
				if (g_selectedElementIndex != -1) {
					iOSUIElement *element = &g_iOSUIElements[g_selectedElementIndex];
					RECT r = {element->x, element->y, element->x + element->width, element->y + element->height};
					HCURSOR hCursor = NULL;

					if ((mouseX >= r.left - GRAB_AREA && mouseX <= r.left + GRAB_AREA &&
					        mouseY >= r.top - GRAB_AREA && mouseY <= r.top + GRAB_AREA) || // TL
					        (mouseX >= r.right - GRAB_AREA && mouseX <= r.right + GRAB_AREA &&
					         mouseY >= r.bottom - GRAB_AREA && mouseY <= r.bottom + GRAB_AREA)) { // BR
						hCursor = LoadCursor(NULL, IDC_SIZENWSE);
					} else if ((mouseX >= r.right - GRAB_AREA && mouseX <= r.right + GRAB_AREA &&
					            mouseY >= r.top - GRAB_AREA && mouseY <= r.top + GRAB_AREA) || // TR
					           (mouseX >= r.left - GRAB_AREA && mouseX <= r.left + GRAB_AREA &&
					            mouseY >= r.bottom - GRAB_AREA && mouseY <= r.bottom + GRAB_AREA)) { // BL
						hCursor = LoadCursor(NULL, IDC_SIZENESW);
					} else if (PtInRect(&r, (POINT) {
					mouseX, mouseY
				})) { // Inside the element
						hCursor = LoadCursor(NULL, IDC_SIZEALL); // Move cursor
					} else {
						hCursor = LoadCursor(NULL, IDC_ARROW); // Default cursor
					}
					if (hCursor) SetCursor(hCursor);
				} else {
					SetCursor(LoadCursor(NULL, IDC_ARROW)); // Default cursor
				}
			}
			break;
		}

		case WM_LBUTTONUP: {
			ReleaseCapture(); // 释放鼠标捕获
			isDragging = FALSE;
			resizeHandle = -1;
			// 拖拽或缩放结束后，重新生成代码，确保最新坐标被写入
			RegenerateAllObjCCode(); // 确保拖拽/缩放后代码也更新
			break;
		}
		case WM_SETCURSOR:
			// 允许 WM_MOUSEMOVE 消息处理中设置光标，避免闪烁
			if (LOWORD(lParam) == HTCLIENT) {
				return TRUE;
			}
			return DefWindowProc(hwnd, msg, wParam, lParam);
		default:
			return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	return 0;
}

// 工具箱面板过程 (处理其内部按钮点击，然后将事件传递给主窗口，或者直接在这里处理元素的创建)
LRESULT CALLBACK toolpanelproc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	WCHAR debugMsg[256];
	switch (msg) {
		case WM_COMMAND: {
			if (g_iOSUIElementCount >= MAX_UI_ELEMENTS) {
				MessageBox(hwnd, TEXT("画布已满，无法添加更多元素。"), TEXT("提示"), MB_OK | MB_ICONINFORMATION);
				break;
			}

			iOSUIElement newElement;
			ZeroMemory(&newElement, sizeof(newElement));

			newElement.original_x = 50; // 简单排布
			newElement.original_y = 50;
			newElement.id = IDC_GENERATED_UI_BASE + g_iOSUIElementCount;

			// 根据当前画布缩放比例，计算初始显示坐标
			RECT currentCanvasRect;
			GetClientRect(g_hCanvasPanel, &currentCanvasRect);
			int currentCanvasWidth = currentCanvasRect.right;
			double currentScale = (currentCanvasWidth > 0) ? ((double)currentCanvasWidth / CANVAS_INITIAL_WIDTH) : 1.0;

			switch (LOWORD(wParam)) {
				case IDC_TOOL_BUTTON: {
					newElement.hasText = TRUE;
					wcscpy_s(newElement.type, _countof(newElement.type), L"Button");
					wcscpy_s(newElement.text, _countof(newElement.text), L"点击我");
					newElement.original_width = 100;
					newElement.original_height = 30;
					// 按钮默认颜色
					newElement.hasBgColor = TRUE;
					newElement.hasTextColor = TRUE;
					newElement.bgColorR = 0;
					newElement.bgColorG = 122;
					newElement.bgColorB = 255; // iOS 蓝色
					newElement.textColorR = 255;
					newElement.textColorG = 255;
					newElement.textColorB = 255; // 白色
					newElement.fontSize = 17.0f;
					break;
				}
				case IDC_TOOL_TEXTFIELD: {
					newElement.hasText = TRUE;
					newElement.hasBgColor = TRUE;
					newElement.hasTextColor = TRUE;
					wcscpy_s(newElement.type, _countof(newElement.type), L"TextField");
					wcscpy_s(newElement.text, _countof(newElement.text), L"请输入...") // Placeholder
					newElement.original_width = 200;
					newElement.original_height = 30;
					// 文本框默认颜色
					newElement.bgColorR = 255;
					newElement.bgColorG = 255;
					newElement.bgColorB = 255; // 白色背景 (不直接使用，但存储)
					newElement.textColorR = 0;
					newElement.textColorG = 0;
					newElement.textColorB = 0; // 黑色
					newElement.fontSize = 17.0f;
					break;
				}
				case IDC_TOOL_TABLEVIEW: {
					newElement.hasText = TRUE;
					newElement.hasBgColor = TRUE;
					newElement.hasTextColor = TRUE;
					wcscpy_s(newElement.type, _countof(newElement.type), L"TableView");
					wcscpy_s(newElement.text, _countof(newElement.text), L"表格视图"); // 仅作显示，实际 tableView 无此属性
					newElement.original_width = 300;
					newElement.original_height = 200;
					// TableView 默认颜色 (不常用，但为了完整性)
					newElement.bgColorR = 242;
					newElement.bgColorG = 242;
					newElement.bgColorB = 247; // iOS 默认背景色
					newElement.textColorR = 0;
					newElement.textColorG = 0;
					newElement.textColorB = 0; // 黑色
					newElement.fontSize = 17.0f;
					break;
				}
				default:
					return DefWindowProc(hwnd, msg, wParam, lParam); // 其他命令，交给默认处理
			}

			// 计算元素在当前画布上的实际显示位置和大小
			newElement.x = (int)round(newElement.original_x * currentScale);
			newElement.y = (int)round(newElement.original_y * currentScale);
			newElement.width = (int)round(newElement.original_width * currentScale);
			newElement.height = (int)round(newElement.original_height * currentScale);


			// 添加新元素到全局数组
			g_iOSUIElements[g_iOSUIElementCount] = newElement;
			g_iOSUIElementCount++;

			// 选中新添加的元素
			g_selectedElementIndex = g_iOSUIElementCount - 1;

			// 强制画布重绘，显示新添加的元素和选中状态
			InvalidateRect(g_hCanvasPanel, NULL, TRUE); // TRUE 表示擦除背景

			// 更新属性面板和代码
			UpdatePropertiesPanel();
			RegenerateAllObjCCode();
			break;
		}
		default:
			return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	return 0;
}

// 新增：代码/属性面板的窗口过程函数
LRESULT CALLBACK CodePropertiesPanelProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	HINSTANCE hInstance = (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE);

	// 定义布局常量
	const int labelWidth = 80;
	const int editWidth = 80;
	const int colorEditWidth = 40; // For R, G, B components
	const int longEditWidth = 265; // For text/placeholder
	const int controlHeight = 30;
	const int rowSpacing = 35; // Vertical spacing between rows
	const int colSpacing = 20; // Horizontal spacing between columns
	const int panelPadding = 10; // Padding from panel edges

	// 属性面板的起始 Y 坐标 (在代码编辑框下方)
	int propPanelStartY = 30 + 400 + 25; // 与主窗口中计算方式保持一致

	switch (msg) {
		case WM_CREATE: {
			// 在这里创建代码输出面板内的所有子控件
			// 在代码输出面板中添加标题

			CreateWindowEx(0, TEXT("STATIC"), TEXT("生成的 Objective-C 代码"), WS_CHILD | WS_VISIBLE | SS_CENTER,
			               0, 5, 550, 20, hwnd, NULL, hInstance, NULL);

			// 在代码输出面板中创建一个多行编辑框来显示代码
			g_hCodeEdit = CreateWindowEx( // 存储到全局变量
			                  0,                                  // 扩展样式
			                  TEXT("EDIT"),                       // 控件类名
			                  TEXT("// 代码将在这里生成，并调用你的 csm 工厂方法...\r\n// 例如：[UIViewController createButtonWithFrame:...]\r\n\r\n"), // 初始文本
			                  WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL | ES_MULTILINE | ES_READONLY | ES_AUTOVSCROLL | ES_AUTOHSCROLL,
			                  panelPadding, 30,                             // X, Y 坐标 (相对于 hCodeOutputPanel)
			                  550 - (2 * panelPadding), 400,           // 宽度, 高度
			                  hwnd,                               // 父窗口句柄 (现在是 CodePropertiesPanelProc 自身的 hwnd)
			                  (HMENU)IDC_CODE_DISPLAY_EDIT,       // 控件ID
			                  hInstance,                          // 应用程序实例句柄
			                  NULL                                // 创建参数
			              );

			// --- 属性面板区域 ---
			// 属性面板标题
			CreateWindowEx(0, TEXT("STATIC"), TEXT("属性"), WS_CHILD | WS_VISIBLE | SS_CENTER,
			               0, propPanelStartY - 20, 550, 20, hwnd, NULL, hInstance, NULL);

			int currentY = propPanelStartY; // 用于跟踪当前行的Y坐标

			// 类型 (静态文本)
			CreateWindowEx(0, TEXT("STATIC"), TEXT("类型:"), WS_CHILD | WS_VISIBLE,
			               panelPadding, currentY, labelWidth, controlHeight, hwnd, (HMENU)IDC_PROP_TYPE_LABEL, hInstance, NULL);
			g_hPropTypeStatic = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("STATIC"), TEXT("无选中"), WS_CHILD | WS_VISIBLE | SS_CENTER,
			                                   panelPadding + labelWidth + colSpacing, currentY, longEditWidth, controlHeight, hwnd, (HMENU)IDC_PROP_TYPE_STATIC, hInstance, NULL);
			currentY += rowSpacing;

			// X 坐标
			CreateWindowEx(0, TEXT("STATIC"), TEXT("X:"), WS_CHILD | WS_VISIBLE,
			               panelPadding, currentY, labelWidth, controlHeight, hwnd, (HMENU)IDC_PROP_X_LABEL, hInstance, NULL);
			g_hPropXEdit = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("EDIT"), TEXT(""), WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER | ES_AUTOHSCROLL,
			                              panelPadding + labelWidth + colSpacing, currentY, editWidth, controlHeight, hwnd, (HMENU)IDC_PROP_X_EDIT, hInstance, NULL);

			// Y 坐标
			CreateWindowEx(0, TEXT("STATIC"), TEXT("Y:"), WS_CHILD | WS_VISIBLE,
			               panelPadding + labelWidth + colSpacing + editWidth + colSpacing, currentY, labelWidth, controlHeight, hwnd, (HMENU)IDC_PROP_Y_LABEL, hInstance, NULL);
			g_hPropYEdit = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("EDIT"), TEXT(""), WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER | ES_AUTOHSCROLL,
			                              panelPadding + labelWidth + colSpacing + editWidth + colSpacing + labelWidth + colSpacing, currentY, editWidth, controlHeight, hwnd, (HMENU)IDC_PROP_Y_EDIT, hInstance, NULL);
			currentY += rowSpacing;

			// 宽度
			CreateWindowEx(0, TEXT("STATIC"), TEXT("宽度:"), WS_CHILD | WS_VISIBLE,
			               panelPadding, currentY, labelWidth, controlHeight, hwnd, (HMENU)IDC_PROP_WIDTH_LABEL, hInstance, NULL);
			g_hPropWidthEdit = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("EDIT"), TEXT(""), WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER | ES_AUTOHSCROLL,
			                                  panelPadding + labelWidth + colSpacing, currentY, editWidth, controlHeight, hwnd, (HMENU)IDC_PROP_WIDTH_EDIT, hInstance, NULL);

			// 高度
			CreateWindowEx(0, TEXT("STATIC"), TEXT("高度:"), WS_CHILD | WS_VISIBLE,
			               panelPadding + labelWidth + colSpacing + editWidth + colSpacing, currentY, labelWidth, controlHeight, hwnd, (HMENU)IDC_PROP_HEIGHT_LABEL, hInstance, NULL);
			g_hPropHeightEdit = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("EDIT"), TEXT(""), WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER | ES_AUTOHSCROLL,
			                                   panelPadding + labelWidth + colSpacing + editWidth + colSpacing + labelWidth + colSpacing, currentY, editWidth, controlHeight, hwnd, (HMENU)IDC_PROP_HEIGHT_EDIT, hInstance, NULL);
			currentY += rowSpacing;

			// 文本/占位符
			CreateWindowEx(0, TEXT("STATIC"), TEXT("文本:"), WS_CHILD | WS_VISIBLE,
			               panelPadding, currentY, labelWidth, controlHeight, hwnd, (HMENU)IDC_PROP_TEXT_LABEL, hInstance, NULL);
			g_hPropTextEdit = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("EDIT"), TEXT(""), WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
			                                 panelPadding + labelWidth + colSpacing, currentY, longEditWidth, controlHeight, hwnd, (HMENU)IDC_PROP_TEXT_EDIT, hInstance, NULL);
			currentY += rowSpacing;

			// 背景颜色 (R, G, B)
			CreateWindowEx(0, TEXT("STATIC"), TEXT("背景色:"), WS_CHILD | WS_VISIBLE,
			               panelPadding, currentY, labelWidth, controlHeight, hwnd, (HMENU)IDC_PROP_BG_R_LABEL, hInstance, NULL);
			g_hPropBgR_Edit = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("EDIT"), TEXT(""), WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER | ES_AUTOHSCROLL,
			                                 panelPadding + labelWidth + colSpacing, currentY, colorEditWidth, controlHeight, hwnd, (HMENU)IDC_PROP_BG_R_EDIT, hInstance, NULL);
			g_hPropBgG_Edit = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("EDIT"), TEXT(""), WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER | ES_AUTOHSCROLL,
			                                 panelPadding + labelWidth + colSpacing + colorEditWidth + colSpacing, currentY, colorEditWidth, controlHeight, hwnd, (HMENU)IDC_PROP_BG_G_EDIT, hInstance, NULL);
			g_hPropBgB_Edit = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("EDIT"), TEXT(""), WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER | ES_AUTOHSCROLL,
			                                 panelPadding + labelWidth + colSpacing + (colorEditWidth + colSpacing) * 2, currentY, colorEditWidth, controlHeight, hwnd, (HMENU)IDC_PROP_BG_B_EDIT, hInstance, NULL);
			currentY += rowSpacing;

			// 文本颜色 (R, G, B)
			CreateWindowEx(0, TEXT("STATIC"), TEXT("文本色:"), WS_CHILD | WS_VISIBLE,
			               panelPadding, currentY, labelWidth, controlHeight, hwnd, (HMENU)IDC_PROP_TEXT_R_LABEL, hInstance, NULL);
			g_hPropTextR_Edit = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("EDIT"), TEXT(""), WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER | ES_AUTOHSCROLL,
			                                   panelPadding + labelWidth + colSpacing, currentY, colorEditWidth, controlHeight, hwnd, (HMENU)IDC_PROP_TEXT_R_EDIT, hInstance, NULL);
			g_hPropTextG_Edit = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("EDIT"), TEXT(""), WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER | ES_AUTOHSCROLL,
			                                   panelPadding + labelWidth + colSpacing + colorEditWidth + colSpacing, currentY, colorEditWidth, controlHeight, hwnd, (HMENU)IDC_PROP_TEXT_G_EDIT, hInstance, NULL);
			g_hPropTextB_Edit = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("EDIT"), TEXT(""), WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER | ES_AUTOHSCROLL,
			                                   panelPadding + labelWidth + colSpacing + (colorEditWidth + colSpacing) * 2, currentY, colorEditWidth, controlHeight, hwnd, (HMENU)IDC_PROP_TEXT_B_EDIT, hInstance, NULL);
			currentY += rowSpacing;

			// 字体大小
			CreateWindowEx(0, TEXT("STATIC"), TEXT("字体大小:"), WS_CHILD | WS_VISIBLE,
			               panelPadding, currentY, labelWidth, controlHeight, hwnd, (HMENU)IDC_PROP_FONT_SIZE_LABEL, hInstance, NULL);
			g_hPropFontSize_Edit = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("EDIT"), TEXT(""), WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER | ES_AUTOHSCROLL,
			                                      panelPadding + labelWidth + colSpacing, currentY, editWidth, controlHeight, hwnd, (HMENU)IDC_PROP_FONT_SIZE_EDIT, hInstance, NULL);
			currentY += rowSpacing;


			// 确定按钮
			g_hPropConfirmButton = CreateWindowEx(
			                           0,
			                           TEXT("BUTTON"),
			                           TEXT("确定"),
			                           WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
			                           panelPadding + labelWidth + colSpacing, currentY + 10, // 放置在文本编辑框下方，额外加点间距
			                           80, 30, // 按钮大小
			                           hwnd, // 父窗口句柄 (现在是 CodePropertiesPanelProc 自身的 hwnd)
			                           (HMENU)IDC_PROP_CONFIRM_BUTTON,
			                           hInstance,
			                           NULL
			                       );
			break;
		}
		case WM_SIZE: {
			// 当 CodePropertiesPanelProc 自身大小改变时，调整其内部子控件的位置和大小
			RECT rc;
			GetClientRect(hwnd, &rc);
			int panelWidth = rc.right - rc.left;
			// int panelHeight = rc.bottom - rc.top; // Not used for this fixed layout

			// 调整代码编辑框
			MoveWindow(g_hCodeEdit, panelPadding, 30, panelWidth - (2 * panelPadding), 400, TRUE);

			// 属性控件的位置是固定的，不需要在 WM_SIZE 中调整，因为它们是相对于父窗口的固定偏移
			// 如果需要属性面板也响应式布局，则需要在这里重新计算所有属性控件的坐标
			break;
		}
		case WM_COMMAND: {
			// 将子控件的 WM_COMMAND 消息转发给父窗口 (主窗口)
			SendMessage(GetParent(hwnd), WM_COMMAND, wParam, lParam);
			break;
		}
		case WM_CTLCOLORSTATIC:
		case WM_CTLCOLOREDIT: {
			HDC hdd = (HDC)wParam;
			SetTextColor(hdd, RGB(255, 255, 255));
			SetBkColor(hdd, RGB(66, 66, 66));
			return bgbrush;
		}
		default:
			return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	return 0;
}

/*
// --- 辅助函数实现 ---

// 将 RGB 颜色值转换为 Objective-C 的 UIColor 代码字符串 (char* 版本，用于 Objective-C 代码字面量)
void RgbToUIColorCode(int r, int g, int b, char* buffer, size_t bufferSize) {
	// 确保颜色值在 0-255 范围内
	r = (r < 0) ? 0 : ((r > 255) ? 255 : r);
	g = (g < 0) ? 0 : ((g > 255) ? 255 : g);
	b = (b < 0) ? 0 : ((b > 255) ? 255 : b);

	sprintf_s(buffer, bufferSize, "[UIColor colorWithRed:%.3f green:%.3f blue:%.3f alpha:1.0]",
	          (float)r / 255.0f, (float)g / 255.0f, (float)b / 255.0f);
}

// 生成 Objective-C 按钮创建代码 (输出到 WCHAR 缓冲区)
void GenerateObjCButtonCode(WCHAR* codeBuffer, size_t bufferSize, iOSUIElement * element) {
	char bgColorCode_mbcs[100]; // Multi-byte char for background color
	char textColorCode_mbcs[100]; // Multi-byte char for text color

	RgbToUIColorCode(element->bgColorR, element->bgColorG, element->bgColorB, bgColorCode_mbcs, sizeof(bgColorCode_mbcs));
	RgbToUIColorCode(element->textColorR, element->textColorG, element->textColorB, textColorCode_mbcs, sizeof(textColorCode_mbcs));

	// Convert bgColorCode_mbcs (char*) to WCHAR for swprintf_s
	WCHAR bgColorCode_wc[100];
	// Calculate required buffer size for wide characters (including null terminator)
	int wc_len_bg = MultiByteToWideChar(CP_UTF8, 0, bgColorCode_mbcs, -1, NULL, 0);
	if (wc_len_bg > 0 && wc_len_bg <= _countof(bgColorCode_wc)) {
		MultiByteToWideChar(CP_UTF8, 0, bgColorCode_mbcs, -1, bgColorCode_wc, wc_len_bg);
	} else {
		bgColorCode_wc[0] = L'\0'; // Ensure null termination on failure
	}

	// Convert textColorCode_mbcs (char*) to WCHAR for swprintf_s
	WCHAR textColorCode_wc[100];
	int wc_len_text = MultiByteToWideChar(CP_UTF8, 0, textColorCode_mbcs, -1, NULL, 0);
	if (wc_len_text > 0 && wc_len_text <= _countof(textColorCode_wc)) {
		MultiByteToWideChar(CP_UTF8, 0, textColorCode_mbcs, -1, textColorCode_wc, wc_len_text);
	} else {
		textColorCode_wc[0] = L'\0'; // Ensure null termination on failure
	}


	// Step 1: 将 WCHAR 文本转换为 UTF-8 编码的 char*
	char text_utf8[512]; // 足够大的缓冲区来存储 UTF-8 编码的中文
	int utf8_len = WideCharToMultiByte(CP_UTF8, 0, element->text, -1, NULL, 0, NULL, NULL);
	if (utf8_len > 0 && utf8_len <= sizeof(text_utf8)) {
		WideCharToMultiByte(CP_UTF8, 0, element->text, -1, text_utf8, utf8_len, NULL, NULL);
	} else {
		text_utf8[0] = '\0'; // 转换失败或缓冲区不足，清空字符串
	}

	// Step 2: 将 UTF-8 编码的 char* 转换回 WCHAR (用于 swprintf_s 的 %ls)
	WCHAR text_wc[512]; // 足够大的缓冲区来存储转换后的 WCHAR
	int wc_text_len = MultiByteToWideChar(CP_UTF8, 0, text_utf8, -1, NULL, 0);
	if (wc_text_len > 0 && wc_text_len <= _countof(text_wc)) {
		MultiByteToWideChar(CP_UTF8, 0, text_utf8, -1, text_wc, wc_text_len);
	} else {
		text_wc[0] = L'\0'; // 转换失败或缓冲区不足，清空字符串
	}

	// 使用 swprintf_s 写入 WCHAR 缓冲区，现在使用 %ls (小写L，小写S)
	// 因为 text_wc, bgColorCode_wc, textColorCode_wc 都是 wchar_t* 类型
	swprintf_s(codeBuffer, bufferSize,
	           L"    UIButton *myButton%d = [UIViewController createButtonWithFrame:CGRectMake(%d, %d, %d, %d)\r\n"
	           L"                                                            title:@\"%ls\"\r\n" // 按钮的标题，使用 %ls
	           L"                                                          bgColor:%ls\r\n" // 背景色，现在使用 %ls
	           L"                                                        textColor:%ls\r\n" // 文本颜色，现在使用 %ls
	           L"                                                             font:[UIFont systemFontOfSize:%.1f] // 字体大小\r\n"
	           L"                                                      borderWidth:0.0 // 示例边框宽度\r\n"
	           L"                                                      borderColor:nil // 示例边框颜色\r\n"
	           L"                                                           target:self\r\n"
	           L"                                                         selector:@selector(buttonTapped:)]; // 示例selector\r\n"
	           L"    [self.view addSubview:myButton%d]; // 相当于 Win32 的 WS_CHILD 样式和父窗口句柄\r\n\r\n",
	           element->id, element->original_x, element->original_y, element->original_width, element->original_height,
	           text_wc, bgColorCode_wc, textColorCode_wc, element->fontSize, element->id
	          );
}

// 生成 Objective-C 文本框创建代码 (输出到 WCHAR 缓冲区)
void GenerateObjCTextFieldCode(WCHAR* codeBuffer, size_t bufferSize, iOSUIElement * element) {
	char textColorCode_mbcs[100];
	RgbToUIColorCode(element->textColorR, element->textColorG, element->textColorB, textColorCode_mbcs, sizeof(textColorCode_mbcs));

	// Convert textColorCode_mbcs (char*) to WCHAR for swprintf_s
	WCHAR textColorCode_wc[100];
	int wc_len_text = MultiByteToWideChar(CP_UTF8, 0, textColorCode_mbcs, -1, NULL, 0);
	if (wc_len_text > 0 && wc_len_text <= _countof(textColorCode_wc)) {
		MultiByteToWideChar(CP_UTF8, 0, textColorCode_mbcs, -1, textColorCode_wc, wc_len_text);
	} else {
		textColorCode_wc[0] = L'\0'; // Ensure null termination on failure
	}

	// Step 1: 将 WCHAR 文本转换为 UTF-8 编码的 char*
	char text_utf8[512]; // 足够大的缓冲区来存储 UTF-8 编码的中文
	int utf8_len = WideCharToMultiByte(CP_UTF8, 0, element->text, -1, NULL, 0, NULL, NULL);
	if (utf8_len > 0 && utf8_len <= sizeof(text_utf8)) {
		WideCharToMultiByte(CP_UTF8, 0, element->text, -1, text_utf8, utf8_len, NULL, NULL);
	} else {
		text_utf8[0] = L'\0'; // 转换失败或缓冲区不足，清空字符串
	}

	// Step 2: 将 UTF-8 编码的 char* 转换回 WCHAR (用于 swprintf_s 的 %ls)
	WCHAR text_wc[512]; // 足够大的缓冲区来存储转换后的 WCHAR
	int wc_len = MultiByteToWideChar(CP_UTF8, 0, text_utf8, -1, NULL, 0);
	if (wc_len > 0 && wc_len <= _countof(text_wc)) {
		MultiByteToWideChar(CP_UTF8, 0, text_utf8, -1, text_wc, wc_len);
	} else {
		text_wc[0] = L'\0'; // 转换失败或缓冲区不足，清空字符串
	}

	// **修改这里：使用 text 属性来设置文本字段的初始内容，并使用 %ls**
	swprintf_s(codeBuffer, bufferSize,
	           L"    UITextField *myTextField%d = [UIViewController createTextFieldWithFrame:CGRectMake(%d, %d, %d, %d)\r\n"
	           L"                                                                   text:@\"%ls\"\r\n" // 使用 text 属性设置初始文本，使用 %ls
	           L"                                                            borderWidth:1.0 // 示例\r\n"
	           L"                                                            borderColor:[UIColor lightGrayColor].CGColor // 示例\r\n"
	           L"                                                        clearButtonMode:UITextFieldViewModeWhileEditing // 示例\r\n"
	           L"                                                          returnKeyType:UIReturnKeyDone // 示例\r\n"
	           L"                                                                   font:[UIFont systemFontOfSize:%.1f] // 字体大小\r\n"
	           L"                                                     keyboardAppearance:UIKeyboardAppearanceDefault]; // 示例\r\n"
	           L"    // myTextField%d.delegate = self; // 如果需要设置委托\r\n"
	           L"    [self.view addSubview:myTextField%d];\r\n\r\n",
	           element->id, element->original_x, element->original_y, element->original_width, element->original_height,
	           text_wc, element->fontSize, element->id, element->id
	          );
}

// 生成 Objective-C 表格视图创建代码 (输出到 WCHAR 缓冲区)
void GenerateObjCTableViewCode(WCHAR* codeBuffer, size_t bufferSize, iOSUIElement * element) {
	// **注意：这里使用 original_x/y/width/height 来生成代码，确保代码是基于原始比例的**
	swprintf_s(codeBuffer, bufferSize,
	           L"    UITableView *myTableView%d = [UIViewController createTableViewWithFrame:CGRectMake(%d, %d, %d, %d)\r\n"
	           L"                                                                    style:UITableViewStylePlain // 示例\r\n"
	           L"                                                                rowHeight:44.0 // 示例\r\n"
	           L"                                                              borderWidth:1.0 // 示例\r\n"
	           L"                                                              borderColor:[UIColor lightGrayColor].CGColor // 示例\r\n"
	           L"                                                                cellIdent:@\"MyCellIdentifier%d\" // 示例\r\n"
	           L"                                                               dataSource:self // 示例\r\n"
	           L"                                                                 delegate:self]; // 示例\r\n"
	           L"    [self.view addSubview:myTableView%d];\r\n\r\n",
	           element->id, element->original_x, element->original_y, element->original_width, element->original_height,
	           element->id, // cellIdent 参数，使用ID确保唯一性
	           element->id
	          );
}

// 更新属性面板显示
void UpdatePropertiesPanel(void) {
	WCHAR buffer[256];
	if (g_selectedElementIndex != -1) {
		iOSUIElement *element = &g_iOSUIElements[g_selectedElementIndex];

		// 设置类型标签
		SetWindowTextW(g_hPropTypeStatic, element->type);

		// 设置 X, Y, Width, Height (显示当前实际渲染的尺寸，不是 original_x)
		swprintf_s(buffer, _countof(buffer), L"%d", element->x);
		SetWindowTextW(g_hPropXEdit, buffer);
		swprintf_s(buffer, _countof(buffer), L"%d", element->y);
		SetWindowTextW(g_hPropYEdit, buffer);
		swprintf_s(buffer, _countof(buffer), L"%d", element->width);
		SetWindowTextW(g_hPropWidthEdit, buffer);
		swprintf_s(buffer, _countof(buffer), L"%d", element->height);
		SetWindowTextW(g_hPropHeightEdit, buffer);

		// 设置背景颜色
		swprintf_s(buffer, _countof(buffer), L"%d", element->bgColorR);
		SetWindowTextW(g_hPropBgR_Edit, buffer);
		swprintf_s(buffer, _countof(buffer), L"%d", element->bgColorG);
		SetWindowTextW(g_hPropBgG_Edit, buffer);
		swprintf_s(buffer, _countof(buffer), L"%d", element->bgColorB);
		SetWindowTextW(g_hPropBgB_Edit, buffer);

		// 设置文本颜色
		swprintf_s(buffer, _countof(buffer), L"%d", element->textColorR);
		SetWindowTextW(g_hPropTextR_Edit, buffer);
		swprintf_s(buffer, _countof(buffer), L"%d", element->textColorG);
		SetWindowTextW(g_hPropTextG_Edit, buffer);
		swprintf_s(buffer, _countof(buffer), L"%d", element->textColorB);
		SetWindowTextW(g_hPropTextB_Edit, buffer);

		// 设置字体大小
		swprintf_s(buffer, _countof(buffer), L"%.1f", element->fontSize);
		SetWindowTextW(g_hPropFontSize_Edit, buffer);


		// 启用所有坐标和尺寸编辑框
		EnableWindow(g_hPropXEdit, TRUE);
		EnableWindow(g_hPropYEdit, TRUE);
		EnableWindow(g_hPropWidthEdit, TRUE);
		EnableWindow(g_hPropHeightEdit, TRUE);
		EnableWindow(g_hPropBgR_Edit, TRUE);
		EnableWindow(g_hPropBgG_Edit, TRUE);
		EnableWindow(g_hPropBgB_Edit, TRUE);
		EnableWindow(g_hPropTextR_Edit, TRUE);
		EnableWindow(g_hPropTextG_Edit, TRUE);
		EnableWindow(g_hPropTextB_Edit, TRUE);
		EnableWindow(g_hPropFontSize_Edit, TRUE);


		// 根据元素类型智能显示文本/占位符编辑框和相关颜色属性
		if (wcscmp(element->type, L"Button") == 0) {
			SetWindowTextW(GetDlgItem(hCodeOutputPanel, IDC_PROP_TEXT_LABEL), L"标题:"); // 使用GetDlgItem获取标签句柄
			SetWindowTextW(g_hPropTextEdit, element->text);
			EnableWindow(g_hPropTextEdit, TRUE);
			ShowWindow(GetDlgItem(hCodeOutputPanel, IDC_PROP_TEXT_LABEL), SW_SHOW);
			ShowWindow(g_hPropTextEdit, SW_SHOW);

			// 按钮的背景色和文本色都可编辑
			ShowWindow(GetDlgItem(hCodeOutputPanel, IDC_PROP_BG_R_LABEL), SW_SHOW);
			ShowWindow(g_hPropBgR_Edit, SW_SHOW);
			ShowWindow(g_hPropBgG_Edit, SW_SHOW);
			ShowWindow(g_hPropBgB_Edit, SW_SHOW);

			ShowWindow(GetDlgItem(hCodeOutputPanel, IDC_PROP_TEXT_R_LABEL), SW_SHOW);
			ShowWindow(g_hPropTextR_Edit, SW_SHOW);
			ShowWindow(g_hPropTextG_Edit, SW_SHOW);
			ShowWindow(g_hPropTextB_Edit, SW_SHOW);

			ShowWindow(GetDlgItem(hCodeOutputPanel, IDC_PROP_FONT_SIZE_LABEL), SW_SHOW);
			ShowWindow(g_hPropFontSize_Edit, SW_SHOW);

		} else if (wcscmp(element->type, L"TextField") == 0) {
			SetWindowTextW(GetDlgItem(hCodeOutputPanel, IDC_PROP_TEXT_LABEL), L"文本内容:"); // 标签改为“文本内容”
			SetWindowTextW(g_hPropTextEdit, element->text);
			EnableWindow(g_hPropTextEdit, TRUE);
			ShowWindow(GetDlgItem(hCodeOutputPanel, IDC_PROP_TEXT_LABEL), SW_SHOW);
			ShowWindow(g_hPropTextEdit, SW_SHOW);

			// 文本框通常只有文本颜色可编辑，背景色一般是透明或固定白色
			ShowWindow(GetDlgItem(hCodeOutputPanel, IDC_PROP_BG_R_LABEL), SW_HIDE);
			ShowWindow(g_hPropBgR_Edit, SW_HIDE);
			ShowWindow(g_hPropBgG_Edit, SW_HIDE);
			ShowWindow(g_hPropBgB_Edit, SW_HIDE);
			EnableWindow(g_hPropBgR_Edit, FALSE);
			EnableWindow(g_hPropBgG_Edit, FALSE);
			EnableWindow(g_hPropBgB_Edit, FALSE);


			ShowWindow(GetDlgItem(hCodeOutputPanel, IDC_PROP_TEXT_R_LABEL), SW_SHOW);
			ShowWindow(g_hPropTextR_Edit, SW_SHOW);
			ShowWindow(g_hPropTextG_Edit, SW_SHOW);
			ShowWindow(g_hPropTextB_Edit, SW_SHOW);

			ShowWindow(GetDlgItem(hCodeOutputPanel, IDC_PROP_FONT_SIZE_LABEL), SW_SHOW);
			ShowWindow(g_hPropFontSize_Edit, SW_SHOW);

		} else { // TableView 或其他没有文本属性的类型
			SetWindowTextW(GetDlgItem(hCodeOutputPanel, IDC_PROP_TEXT_LABEL), L""); // 清空标签文本
			SetWindowTextW(g_hPropTextEdit, TEXT("")); // 清空编辑框
			EnableWindow(g_hPropTextEdit, FALSE); // 禁用
			ShowWindow(GetDlgItem(hCodeOutputPanel, IDC_PROP_TEXT_LABEL), SW_HIDE); // 隐藏标签
			ShowWindow(g_hPropTextEdit, SW_HIDE); // 隐藏编辑框

			// 隐藏并禁用所有颜色和字体大小属性
			ShowWindow(GetDlgItem(hCodeOutputPanel, IDC_PROP_BG_R_LABEL), SW_HIDE);
			ShowWindow(g_hPropBgR_Edit, SW_HIDE);
			ShowWindow(g_hPropBgG_Edit, SW_HIDE);
			ShowWindow(g_hPropBgB_Edit, SW_HIDE);
			EnableWindow(g_hPropBgR_Edit, FALSE);
			EnableWindow(g_hPropBgG_Edit, FALSE);
			EnableWindow(g_hPropBgB_Edit, FALSE);

			ShowWindow(GetDlgItem(hCodeOutputPanel, IDC_PROP_TEXT_R_LABEL), SW_HIDE);
			ShowWindow(g_hPropTextR_Edit, SW_HIDE);
			ShowWindow(g_hPropTextG_Edit, SW_HIDE);
			ShowWindow(g_hPropTextB_Edit, SW_HIDE);
			EnableWindow(g_hPropTextR_Edit, FALSE);
			EnableWindow(g_hPropTextG_Edit, FALSE);
			EnableWindow(g_hPropTextB_Edit, FALSE);

			ShowWindow(GetDlgItem(hCodeOutputPanel, IDC_PROP_FONT_SIZE_LABEL), SW_HIDE);
			ShowWindow(g_hPropFontSize_Edit, SW_HIDE);
			EnableWindow(g_hPropFontSize_Edit, FALSE);
		}
		// 选中元素时启用确定按钮
		EnableWindow(g_hPropConfirmButton, TRUE);

	} else {
		// 未选中任何元素，清空并禁用编辑框
		SetWindowTextW(g_hPropTypeStatic, TEXT("无选中"));
		SetWindowTextW(g_hPropXEdit, TEXT(""));
		SetWindowTextW(g_hPropYEdit, TEXT(""));
		SetWindowTextW(g_hPropWidthEdit, TEXT(""));
		SetWindowTextW(g_hPropHeightEdit, TEXT(""));
		SetWindowTextW(g_hPropTextEdit, TEXT(""));

		SetWindowTextW(g_hPropBgR_Edit, TEXT(""));
		SetWindowTextW(g_hPropBgG_Edit, TEXT(""));
		SetWindowTextW(g_hPropBgB_Edit, TEXT(""));
		SetWindowTextW(g_hPropTextR_Edit, TEXT(""));
		SetWindowTextW(g_hPropTextG_Edit, TEXT(""));
		SetWindowTextW(g_hPropTextB_Edit, TEXT(""));
		SetWindowTextW(g_hPropFontSize_Edit, TEXT(""));


		EnableWindow(g_hPropXEdit, FALSE);
		EnableWindow(g_hPropYEdit, FALSE);
		EnableWindow(g_hPropWidthEdit, FALSE);
		EnableWindow(g_hPropHeightEdit, FALSE);
		EnableWindow(g_hPropTextEdit, FALSE);

		EnableWindow(g_hPropBgR_Edit, FALSE);
		EnableWindow(g_hPropBgG_Edit, FALSE);
		EnableWindow(g_hPropBgB_Edit, FALSE);
		EnableWindow(g_hPropTextR_Edit, FALSE);
		EnableWindow(g_hPropTextG_Edit, FALSE);
		EnableWindow(g_hPropTextB_Edit, FALSE);
		EnableWindow(g_hPropFontSize_Edit, FALSE);


		// 确保文本标签和编辑框在未选中时也显示默认状态，或者隐藏
		SetWindowTextW(GetDlgItem(hCodeOutputPanel, IDC_PROP_TEXT_LABEL), L"文本:"); // 恢复默认标签
		ShowWindow(GetDlgItem(hCodeOutputPanel, IDC_PROP_TEXT_LABEL), SW_SHOW);
		ShowWindow(g_hPropTextEdit, SW_SHOW);

		// 隐藏所有颜色和字体大小属性
		ShowWindow(GetDlgItem(hCodeOutputPanel, IDC_PROP_BG_R_LABEL), SW_HIDE);
		ShowWindow(g_hPropBgR_Edit, SW_HIDE);
		ShowWindow(g_hPropBgG_Edit, SW_HIDE);
		ShowWindow(g_hPropBgB_Edit, SW_HIDE);

		ShowWindow(GetDlgItem(hCodeOutputPanel, IDC_PROP_TEXT_R_LABEL), SW_HIDE);
		ShowWindow(g_hPropTextR_Edit, SW_HIDE);
		ShowWindow(g_hPropTextG_Edit, SW_HIDE);
		ShowWindow(g_hPropTextB_Edit, SW_HIDE);

		ShowWindow(GetDlgItem(hCodeOutputPanel, IDC_PROP_FONT_SIZE_LABEL), SW_HIDE);
		ShowWindow(g_hPropFontSize_Edit, SW_HIDE);


		// 未选中元素时禁用确定按钮
		EnableWindow(g_hPropConfirmButton, FALSE);
	}
}

// 重新生成所有 Objective-C 代码
void RegenerateAllObjCCode(void) {
	WCHAR generatedCodeBuffer[204800]; // 更大的缓冲区以适应更多代码, 更改为 WCHAR
	ZeroMemory(generatedCodeBuffer, sizeof(generatedCodeBuffer)); // 清零

	wcscat_s(generatedCodeBuffer, _countof(generatedCodeBuffer), L"// 引入你的通用 UI 工厂类别头文件 (csm.h)\r\n");
	wcscat_s(generatedCodeBuffer, _countof(generatedCodeBuffer), L"#import \"csm.h\" \r\n\r\n");
	wcscat_s(generatedCodeBuffer, _countof(generatedCodeBuffer), L"// 在你的 UIViewController 的 viewDidLoad 或其他方法中：\r\n");
	wcscat_s(generatedCodeBuffer, _countof(generatedCodeBuffer), L"- (void)setupGeneratedUI {\r\n");
	wcscat_s(generatedCodeBuffer, _countof(generatedCodeBuffer), L"    // 假设 'screen' 和 'self.view' 在当前上下文可用\r\n");
	wcscat_s(generatedCodeBuffer, _countof(generatedCodeBuffer), L"    // CGRect screen = [[UIScreen mainScreen] bounds];\\r\\n\\r\\n");

	for (int i = 0; i < g_iOSUIElementCount; i++) {
		WCHAR elementCode[2048]; // 每个元素的代码缓冲区, 更改为 WCHAR
		ZeroMemory(elementCode, sizeof(elementCode));

		if (wcscmp(g_iOSUIElements[i].type, L"Button") == 0) {
			GenerateObjCButtonCode(elementCode, _countof(elementCode), &g_iOSUIElements[i]);
		} else if (wcscmp(g_iOSUIElements[i].type, L"TextField") == 0) {
			GenerateObjCTextFieldCode(elementCode, _countof(elementCode), &g_iOSUIElements[i]);
		} else if (wcscmp(g_iOSUIElements[i].type, L"TableView") == 0) {
			GenerateObjCTableViewCode(elementCode, _countof(elementCode), &g_iOSUIElements[i]);
		}
		wcscat_s(generatedCodeBuffer, _countof(generatedCodeBuffer), elementCode);
	}
	wcscat_s(generatedCodeBuffer, _countof(generatedCodeBuffer), L"}\r\n");

	SetWindowTextW(g_hCodeEdit, generatedCodeBuffer); // 设置 EDIT 控件的文本 (使用 WCHAR 版本)
}*/
