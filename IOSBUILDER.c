#define UNICODE     // Force Unicode for Win32 API
#define _UNICODE    // Force Unicode for C Runtime Library

#include "IOSBUILDER.h"
WCHAR generatedCodeBuffer[204800]; // Larger buffer to accommodate more code

const int labelWidth = 90;
const int editWidth = 80;
const int colorEditWidth = 40;
const int longEditWidth = 265;
const int controlHeight = 30;
const int rowSpacing = 40;
const int colSpacing = 10;
const int panelPadding = 10;

int propPanelStartY = 30 + 400 + 45;

// --- Global variable definitions (without extern) ---
iOSUIElement g_iOSUIElements[MAX_UI_ELEMENTS];
int g_iOSUIElementCount = 0;
int g_selectedElementIndex = -1;

HWND g_hPropTypeStatic;
HWND g_hPropXEdit, g_hPropYEdit, g_hPropWidthEdit, g_hPropHeightEdit, g_hPropTextEdit;
HWND g_hPropConfirmButton;

HWND g_hPropBgR_Edit, g_hPropBgG_Edit, g_hPropBgB_Edit;
HWND g_hPropTextR_Edit, g_hPropTextG_Edit, g_hPropTextB_Edit;
HWND g_hPropFontSize_Edit;

HWND g_hPropMinValue_Edit, g_hPropMaxValue_Edit, g_hPropCurrentValue_Edit;

HWND g_hCanvasPanel;
HWND g_hCodeEdit;
HWND hToolboxPanel;
HWND hCodeOutputPanel;

HBRUSH g_hbrPropertiesPanelBackground = NULL;

int combutx;
int combuty;
int currY;

const double CANVAS_ASPECT_RATIO = (double)CANVAS_INITIAL_WIDTH / CANVAS_INITIAL_HEIGHT;

UIElementDefinition g_elementDefinitions[] = {
	{
		TEXT("Button"), 1101, InitDefaultData, DrawButton, GenerateObjCButtonCode,
		{50, 50, 130, 50, TEXT("按钮"), 123, 123, 123, 255, 255, 255, 24.0, 0, 0, 0, 0},
		1, 1, 1, 1, 0, 0, 0, 0
	},
	{
		TEXT("TextField"), 1102, InitDefaultData, DrawTextField, GenerateObjCTextFieldCode,
		{50, 50, 240, 50, TEXT("搜索框"), 100, 100, 100, 255, 255, 255, 24.0, 0, 0, 0, 0},
		1, 1, 1, 1, 0, 0, 0, 0
	},
	{
		TEXT("TableView"), 1103, InitDefaultData, DrawTableView, GenerateObjCTableViewCode,
		{ 50, 50, 130, 130, TEXT(""), 233, 233, 233, 255, 255, 255, 24.0, 0, 0, 0, 44}
		, 0, 0, 0, 0, 0, 0, 0, 1
	},
	/*{
		TEXT("Slider"), 1104, InitDefaultData, DrawSlider, GenerateObjCSliderCode,
		{50, 50, 90, 30, TEXT(""), 0, 0, 0, 0, 0, 0, 0.0, 0, 100, 0, 0}
		, 0, 0, 0, 0, 1, 1, 1, 0
	},*/
	{
		TEXT("Label"), 1105, InitDefaultData, DrawLabel, GenerateObjCLabelCode,
		{50, 50, 100, 30, TEXT("label"), 100, 100, 100, 255, 255, 255, 24.0, 0, 0, 0, 0}
		, 1, 1, 1, 1, 0, 0, 0, 0
	},
	{
		TEXT("wtf"), 1106, InitDefaultData, DrawLabel, GenerateObjCLabelCode,
		{50, 50, 100, 30, TEXT("wtf"), 100, 100, 100, 255, 255, 255, 24.0, 0, 0, 0, 0}
		, 1, 1, 1, 1, 0, 0, 0, 0
	}


};
//int g_numElementDefinitions = _countof(g_elementDefinitions);
const int g_numElementDefinitions = sizeof(g_elementDefinitions) / sizeof(UIElementDefinition);


xyh edits[30];
propp labels[25];
WCHAR labelname[][20] = {
	TEXT("类型"),
	TEXT("X:"),
	TEXT("Y:"),
	TEXT("宽度:"),
	TEXT("高度:"),
	TEXT("文本:"),
	TEXT("背景色"),
	TEXT("文本色"),
	TEXT("字体大小:"),
	TEXT("最小值:"),
	TEXT("最大值:"),
	TEXT("当前值:"),
};





// --- Helper function implementations ---

void RgbToUIColorCode(int r, int g, int b, char* buffer, size_t bufferSize) {
	r = (r < 0) ? 0 : ((r > 255) ? 255 : r);
	g = (g < 0) ? 0 : ((g > 255) ? 255 : g);
	b = (b < 0) ? 0 : ((b > 255) ? 255 : b);

	sprintf_s(buffer, bufferSize, "[UIColor colorWithRed:%.3f  green:%.3f  blue:%.3f  alpha:1.0]",
	          (float)r / 255.0, (float)g / 255.0, (float)b / 255.0);
}

void HideAllPropertiesUIControls(void) {
	ShowWindow(GetDlgItem(hCodeOutputPanel, IDC_PROP_X), SW_HIDE);
	EnableWindow(edits[IDE_PROP_X - IDE_PROP_TYPE].thing, FALSE);
	ShowWindow(GetDlgItem(hCodeOutputPanel, IDC_PROP_Y), SW_HIDE);
	EnableWindow(edits[IDE_PROP_Y - IDE_PROP_TYPE].thing, FALSE);
	ShowWindow(GetDlgItem(hCodeOutputPanel, IDC_PROP_WIDTH), SW_HIDE);
	EnableWindow(edits[IDE_PROP_WIDTH - IDE_PROP_TYPE].thing, FALSE);
	ShowWindow(GetDlgItem(hCodeOutputPanel, IDC_PROP_HEIGHT), SW_HIDE);
	EnableWindow(edits[IDE_PROP_HEIGHT - IDE_PROP_TYPE].thing, FALSE);

	ShowWindow(GetDlgItem(hCodeOutputPanel, IDC_PROP_TEXT), SW_HIDE);
	EnableWindow(edits[IDE_PROP_TEXT - IDE_PROP_TYPE].thing, FALSE);

	ShowWindow(GetDlgItem(hCodeOutputPanel, IDC_PROP_BG_R), SW_HIDE);
	EnableWindow(edits[IDE_PROP_BG_R - IDE_PROP_TYPE].thing, FALSE);
	ShowWindow(GetDlgItem(hCodeOutputPanel, IDE_PROP_BG_G), SW_HIDE);
	EnableWindow(edits[IDE_PROP_BG_G - IDE_PROP_TYPE].thing, FALSE);
	ShowWindow(GetDlgItem(hCodeOutputPanel, IDE_PROP_BG_B), SW_HIDE);
	EnableWindow(edits[IDE_PROP_BG_B - IDE_PROP_TYPE].thing, FALSE);

	ShowWindow(GetDlgItem(hCodeOutputPanel, IDC_PROP_TEXT_R), SW_HIDE);
	EnableWindow(edits[IDE_PROP_TEXT_R - IDE_PROP_TYPE].thing, FALSE);
	ShowWindow(GetDlgItem(hCodeOutputPanel, IDE_PROP_TEXT_G), SW_HIDE);
	EnableWindow(edits[IDE_PROP_TEXT_G - IDE_PROP_TYPE].thing, FALSE);
	ShowWindow(GetDlgItem(hCodeOutputPanel, IDE_PROP_TEXT_B), SW_HIDE);
	EnableWindow(edits[IDE_PROP_TEXT_B - IDE_PROP_TYPE].thing, FALSE);

	ShowWindow(GetDlgItem(hCodeOutputPanel, IDC_PROP_FONT_SIZE), SW_HIDE);
	EnableWindow(edits[IDE_PROP_FONT_SIZE - IDE_PROP_TYPE].thing, FALSE);

	ShowWindow(GetDlgItem(hCodeOutputPanel, IDC_PROP_MIN_VALUE), SW_HIDE);
	EnableWindow(edits[IDE_PROP_MIN_VALUE - IDE_PROP_TYPE].thing, FALSE);
	ShowWindow(GetDlgItem(hCodeOutputPanel, IDC_PROP_MAX_VALUE), SW_HIDE);
	EnableWindow(edits[IDE_PROP_MAX_VALUE - IDE_PROP_TYPE].thing, FALSE);
	ShowWindow(GetDlgItem(hCodeOutputPanel, IDC_PROP_CURRENT_VALUE), SW_HIDE);
	EnableWindow(edits[IDE_PROP_CURRENT_VALUE - IDE_PROP_TYPE].thing, FALSE);

	SetWindowText(edits[IDE_PROP_X - IDE_PROP_TYPE].thing, TEXT(""));
	SetWindowText(edits[IDE_PROP_Y - IDE_PROP_TYPE].thing, TEXT(""));
	SetWindowText(edits[IDE_PROP_WIDTH - IDE_PROP_TYPE].thing, TEXT(""));
	SetWindowText(edits[IDE_PROP_HEIGHT - IDE_PROP_TYPE].thing, TEXT(""));
	SetWindowText(edits[IDE_PROP_TEXT - IDE_PROP_TYPE].thing, TEXT(""));
	SetWindowText(edits[IDE_PROP_BG_R - IDE_PROP_TYPE].thing, TEXT(""));
	SetWindowText(edits[IDE_PROP_BG_G - IDE_PROP_TYPE].thing, TEXT(""));
	SetWindowText(edits[IDE_PROP_BG_B - IDE_PROP_TYPE].thing, TEXT(""));
	SetWindowText(edits[IDE_PROP_TEXT_R - IDE_PROP_TYPE].thing, TEXT(""));
	SetWindowText(edits[IDE_PROP_TEXT_G - IDE_PROP_TYPE].thing, TEXT(""));
	SetWindowText(edits[IDE_PROP_TEXT_B - IDE_PROP_TYPE].thing, TEXT(""));
	SetWindowText(edits[IDE_PROP_FONT_SIZE - IDE_PROP_TYPE].thing, TEXT(""));
	SetWindowText(edits[IDE_PROP_MIN_VALUE - IDE_PROP_TYPE].thing, TEXT(""));
	SetWindowText(edits[IDE_PROP_MAX_VALUE - IDE_PROP_TYPE].thing, TEXT(""));
	SetWindowText(edits[IDE_PROP_CURRENT_VALUE - IDE_PROP_TYPE].thing, TEXT(""));
}

void UpdatePropertiesPanelUI(iOSUIElement* element) {
	WCHAR buffer[256];

	// Position and size (always visible for any element)
	swprintf_s(buffer, _countof(buffer), TEXT("%d"), element->x);
	SetWindowText(edits[IDE_PROP_X - IDE_PROP_TYPE].thing, buffer);
	swprintf_s(buffer, _countof(buffer), TEXT("%d"), element->y);
	SetWindowText(edits[IDE_PROP_Y - IDE_PROP_TYPE].thing, buffer);
	swprintf_s(buffer, _countof(buffer), TEXT("%d"), element->width);
	SetWindowText(edits[IDE_PROP_WIDTH - IDE_PROP_TYPE].thing, buffer);
	swprintf_s(buffer, _countof(buffer), TEXT("%d"), element->height);
	SetWindowText(edits[IDE_PROP_HEIGHT - IDE_PROP_TYPE].thing, buffer);
	ShowWindow(GetDlgItem(hCodeOutputPanel, IDC_PROP_X), SW_SHOW);
	EnableWindow(edits[IDE_PROP_X - IDE_PROP_TYPE].thing, TRUE);
	ShowWindow(GetDlgItem(hCodeOutputPanel, IDC_PROP_Y), SW_SHOW);
	EnableWindow(edits[IDE_PROP_Y - IDE_PROP_TYPE].thing, TRUE);
	ShowWindow(GetDlgItem(hCodeOutputPanel, IDC_PROP_WIDTH), SW_SHOW);
	EnableWindow(edits[IDE_PROP_WIDTH - IDE_PROP_TYPE].thing, TRUE);
	ShowWindow(GetDlgItem(hCodeOutputPanel, IDC_PROP_HEIGHT), SW_SHOW);
	EnableWindow(edits[IDE_PROP_HEIGHT - IDE_PROP_TYPE].thing, TRUE);

	// Text (unified for title/placeholder/row height label)
	if (element->definition->hasText || element->definition->hasRowHeight) {
		ShowWindow(GetDlgItem(hCodeOutputPanel, IDC_PROP_TEXT), SW_SHOW);
		EnableWindow(edits[IDE_PROP_TEXT - IDE_PROP_TYPE].thing, TRUE);
		if (element->definition->hasText) {
			if (wcscmp(element->type, TEXT("Button")) == 0) {
				SetWindowText(GetDlgItem(hCodeOutputPanel, IDC_PROP_TEXT), TEXT("标题:"));
			} else if (wcscmp(element->type, TEXT("TextField")) == 0) {
				SetWindowText(GetDlgItem(hCodeOutputPanel, IDC_PROP_TEXT), TEXT("占位符:"));
			} else if (wcscmp(element->type, TEXT("Label")) == 0) {
				SetWindowText(GetDlgItem(hCodeOutputPanel, IDC_PROP_TEXT), TEXT("文本内容:"));
			}
			SetWindowText(edits[IDE_PROP_TEXT - IDE_PROP_TYPE].thing, element->text);
		} else if (element->definition->hasRowHeight) {
			SetWindowText(GetDlgItem(hCodeOutputPanel, IDC_PROP_TEXT), TEXT("行高:"));
			swprintf_s(buffer, _countof(buffer), TEXT("%d"), element->rowHeight);
			SetWindowText(edits[IDE_PROP_TEXT - IDE_PROP_TYPE].thing, buffer);
		}
	}

	// Background Color
	if (element->definition->hasBgColor) {
		ShowWindow(GetDlgItem(hCodeOutputPanel, IDC_PROP_BG_R), SW_SHOW);
		EnableWindow(edits[IDE_PROP_BG_R - IDE_PROP_TYPE].thing, TRUE);
		ShowWindow(GetDlgItem(hCodeOutputPanel, IDE_PROP_BG_G), SW_SHOW);
		EnableWindow(edits[IDE_PROP_BG_G - IDE_PROP_TYPE].thing, TRUE);
		ShowWindow(GetDlgItem(hCodeOutputPanel, IDE_PROP_BG_B), SW_SHOW);
		EnableWindow(edits[IDE_PROP_BG_B - IDE_PROP_TYPE].thing, TRUE);
		swprintf_s(buffer, _countof(buffer), TEXT("%d"), element->bgColorR);
		SetWindowText(edits[IDE_PROP_BG_R - IDE_PROP_TYPE].thing, buffer);
		swprintf_s(buffer, _countof(buffer), TEXT("%d"), element->bgColorG);
		SetWindowText(edits[IDE_PROP_BG_G - IDE_PROP_TYPE].thing, buffer);
		swprintf_s(buffer, _countof(buffer), TEXT("%d"), element->bgColorB);
		SetWindowText(edits[IDE_PROP_BG_B - IDE_PROP_TYPE].thing, buffer);
	}

	// Text Color
	if (element->definition->hasTextColor) {
		ShowWindow(GetDlgItem(hCodeOutputPanel, IDC_PROP_TEXT_R), SW_SHOW);
		EnableWindow(edits[IDE_PROP_TEXT_R - IDE_PROP_TYPE].thing, TRUE);
		ShowWindow(GetDlgItem(hCodeOutputPanel, IDE_PROP_TEXT_G), SW_SHOW);
		EnableWindow(edits[IDE_PROP_TEXT_G - IDE_PROP_TYPE].thing, TRUE);
		ShowWindow(GetDlgItem(hCodeOutputPanel, IDE_PROP_TEXT_B), SW_SHOW);
		EnableWindow(edits[IDE_PROP_TEXT_B - IDE_PROP_TYPE].thing, TRUE);
		swprintf_s(buffer, _countof(buffer), TEXT("%d"), element->textColorR);
		SetWindowText(edits[IDE_PROP_TEXT_R - IDE_PROP_TYPE].thing, buffer);
		swprintf_s(buffer, _countof(buffer), TEXT("%d"), element->textColorG);
		SetWindowText(edits[IDE_PROP_TEXT_G - IDE_PROP_TYPE].thing, buffer);
		swprintf_s(buffer, _countof(buffer), TEXT("%d"), element->textColorB);
		SetWindowText(edits[IDE_PROP_TEXT_B - IDE_PROP_TYPE].thing, buffer);
	}

	// Font Size
	if (element->definition->hasFontSize) {
		ShowWindow(GetDlgItem(hCodeOutputPanel, IDC_PROP_FONT_SIZE), SW_SHOW);
		EnableWindow(edits[IDE_PROP_FONT_SIZE - IDE_PROP_TYPE].thing, TRUE);
		swprintf_s(buffer, _countof(buffer), TEXT("%.1f"), element->fontSize);
		SetWindowText(edits[IDE_PROP_FONT_SIZE - IDE_PROP_TYPE].thing, buffer);
	}

	// Slider specific properties
	if (element->definition->hasMinValue) {
		ShowWindow(GetDlgItem(hCodeOutputPanel, IDC_PROP_MIN_VALUE), SW_SHOW);
		EnableWindow(edits[IDE_PROP_MIN_VALUE - IDE_PROP_TYPE].thing, TRUE);
		ShowWindow(GetDlgItem(hCodeOutputPanel, IDC_PROP_MAX_VALUE), SW_SHOW);
		EnableWindow(edits[IDE_PROP_MAX_VALUE - IDE_PROP_TYPE].thing, TRUE);
		ShowWindow(GetDlgItem(hCodeOutputPanel, IDC_PROP_CURRENT_VALUE), SW_SHOW);
		EnableWindow(edits[IDE_PROP_CURRENT_VALUE - IDE_PROP_TYPE].thing, TRUE);
		swprintf_s(buffer, _countof(buffer), TEXT("%.1f"), element->minValue);
		SetWindowText(edits[IDE_PROP_MIN_VALUE - IDE_PROP_TYPE].thing, buffer);
		swprintf_s(buffer, _countof(buffer), TEXT("%.1f"), element->maxValue);
		SetWindowText(edits[IDE_PROP_MAX_VALUE - IDE_PROP_TYPE].thing, buffer);
		swprintf_s(buffer, _countof(buffer), TEXT("%.1f"), element->currentValue);
		SetWindowText(edits[IDE_PROP_CURRENT_VALUE - IDE_PROP_TYPE].thing, buffer);
	}
}

// Reads property values from UI controls and updates the element's properties.
void ReadPropertiesFromUI(iOSUIElement* element) {
	WCHAR buffer[256];
	float floatValue;

	// Always read common position/size properties
	GetWindowText(edits[IDE_PROP_X - IDE_PROP_TYPE].thing, buffer, _countof(buffer));
	element->x = _wtoi(buffer);
	GetWindowText(edits[IDE_PROP_Y - IDE_PROP_TYPE].thing, buffer, _countof(buffer));
	element->y = _wtoi(buffer);
	GetWindowText(edits[IDE_PROP_WIDTH - IDE_PROP_TYPE].thing, buffer, _countof(buffer));
	element->width = _wtoi(buffer);
	GetWindowText(edits[IDE_PROP_HEIGHT - IDE_PROP_TYPE].thing, buffer, _countof(buffer));
	element->height = _wtoi(buffer);

	// Update original dimensions based on current canvas scale
	// Since canvas is now fixed, original_x/y/width/height are directly used for rendering
	// and are updated directly from UI input.
	element->original_x = element->x;
	element->original_y = element->y;
	element->original_width = element->width;
	element->original_height = element->height;

	// Read text
	if (element->definition->hasText) {
		GetWindowText(edits[IDE_PROP_TEXT - IDE_PROP_TYPE].thing, element->text, _countof(element->text));
	} else if (element->definition->hasRowHeight) { // For TableView, text edit box is repurposed for rowHeight
		GetWindowText(edits[IDE_PROP_TEXT - IDE_PROP_TYPE].thing, buffer, _countof(buffer));
		element->rowHeight = _wtoi(buffer);
	}

	// Read background color
	if (element->definition->hasBgColor) {
		GetWindowText(edits[IDE_PROP_BG_R - IDE_PROP_TYPE].thing, buffer, _countof(buffer));
		element->bgColorR = _wtoi(buffer);
		GetWindowText(edits[IDE_PROP_BG_G - IDE_PROP_TYPE].thing, buffer, _countof(buffer));
		element->bgColorG = _wtoi(buffer);
		GetWindowText(edits[IDE_PROP_BG_B - IDE_PROP_TYPE].thing, buffer, _countof(buffer));
		element->bgColorB = _wtoi(buffer);
	}

	// Read text color
	if (element->definition->hasTextColor) {
		GetWindowText(edits[IDE_PROP_TEXT_R - IDE_PROP_TYPE].thing, buffer, _countof(buffer));
		element->textColorR = _wtoi(buffer);
		GetWindowText(edits[IDE_PROP_TEXT_G - IDE_PROP_TYPE].thing, buffer, _countof(buffer));
		element->textColorG = _wtoi(buffer);
		GetWindowText(edits[IDE_PROP_TEXT_B - IDE_PROP_TYPE].thing, buffer, _countof(buffer));
		element->textColorB = _wtoi(buffer);
	}

	// Read font size
	if (element->definition->hasFontSize) {
		GetWindowText(edits[IDE_PROP_FONT_SIZE - IDE_PROP_TYPE].thing, buffer, _countof(buffer));
		floatValue = (float)_wtof(buffer);
		element->fontSize = floatValue;
	}

	// Read slider specific properties
	if (element->definition->hasMinValue) {
		GetWindowText(edits[IDE_PROP_MIN_VALUE - IDE_PROP_TYPE].thing, buffer, _countof(buffer));
		floatValue = (float)_wtof(buffer);
		element->minValue = floatValue;
		GetWindowText(edits[IDE_PROP_MAX_VALUE - IDE_PROP_TYPE].thing, buffer, _countof(buffer));
		floatValue = (float)_wtof(buffer);
		element->maxValue = floatValue;
		GetWindowText(edits[IDE_PROP_CURRENT_VALUE - IDE_PROP_TYPE].thing, buffer, _countof(buffer));
		floatValue = (float)_wtof(buffer);
		element->currentValue = floatValue;
	}
}


// Button functions
void DrawButton(HDC hdc, iOSUIElement* element) {
	RECT elementRect = {element->x, element->y, element->x + element->width, element->y + element->height};

	HPEN hPen = CreatePen(PS_SOLID, 1, RGB(element->bgColorR, element->bgColorG, element->bgColorB));
	HBRUSH hBrush = CreateSolidBrush(RGB(element->bgColorR, element->bgColorG, element->bgColorB));
	HPEN hOldPen = SelectObject(hdc, hPen);
	HBRUSH hOldBrush = SelectObject(hdc, hBrush);

	Rectangle(hdc, elementRect.left, elementRect.top, elementRect.right, elementRect.bottom);

	SelectObject(hdc, hOldPen);
	SelectObject(hdc, hOldBrush);
	DeleteObject(hPen);
	DeleteObject(hBrush);

	SetBkMode(hdc, TRANSPARENT);
	SetTextColor(hdc, RGB(element->textColorR, element->textColorG, element->textColorB));
	HFONT hFont = CreateFont(-(int)element->fontSize, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, TEXT("Arial"));
	HFONT hOldFont = SelectObject(hdc, hFont);

	DrawText(hdc, element->text, -1, &elementRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);

	SelectObject(hdc, hOldFont);
	DeleteObject(hFont);
}
void GenerateObjCButtonCode(WCHAR* codeBuffer, size_t bufferSize, iOSUIElement * element) {
	char bgColorCode_mbcs[100];
	char textColorCode_mbcs[100];

	RgbToUIColorCode(element->bgColorR, element->bgColorG, element->bgColorB, bgColorCode_mbcs, sizeof(bgColorCode_mbcs));
	RgbToUIColorCode(element->textColorR, element->textColorG, element->textColorB, textColorCode_mbcs, sizeof(textColorCode_mbcs));

	WCHAR bgColorCode_wc[100];
	MultiByteToWideChar(CP_UTF8, 0, bgColorCode_mbcs, -1, bgColorCode_wc, _countof(bgColorCode_wc));
	WCHAR textColorCode_wc[100];
	MultiByteToWideChar(CP_UTF8, 0, textColorCode_mbcs, -1, textColorCode_wc, _countof(textColorCode_wc));

	char text_utf8[512];
	WideCharToMultiByte(CP_UTF8, 0, element->text, -1, text_utf8, _countof(text_utf8), NULL, NULL);
	WCHAR text_wc[512];
	MultiByteToWideChar(CP_UTF8, 0, text_utf8, -1, text_wc, _countof(text_wc));

	swprintf_s(codeBuffer, bufferSize,
	           TEXT("    UIButton *myButton%d = [UIViewController createButtonWithFrame:CGRectMake(%d, %d, %d, %d)\r\n")
	           TEXT("                                                          bgcolor:%ls\r\n")
	           TEXT("                                                        textColor:%ls\r\n")
	           TEXT("                                                             font:[UIFont fontWithName:@\"arial\" size:%.2f] // Font size\r\n")
	           TEXT("                                                            title:@\"%ls\"\r\n")
	           TEXT("                                                      borderWidth:0.0 // Example border width\r\n")
	           TEXT("                                                      borderColor:nil]; // Example border width\r\n")
	           TEXT("    [self.view addSubview:myButton%d]; \r\n\r\n"),
	           element->id, element->original_x, element->original_y, element->original_width, element->original_height,
	           bgColorCode_wc, textColorCode_wc, element->fontSize, text_wc, element->id
	          );
}


// Text Field functions
void DrawTextField(HDC hdc, iOSUIElement* element) {
	RECT elementRect = {element->x, element->y, element->x + element->width, element->y + element->height};

	HPEN hPen = CreatePen(PS_SOLID, 1, RGB(0, 100, 0));
	HBRUSH hBrush = CreateSolidBrush(RGB(element->bgColorR, element->bgColorG, element->bgColorB));
	HPEN hOldPen = SelectObject(hdc, hPen);
	HBRUSH hOldBrush = SelectObject(hdc, hBrush);

	Rectangle(hdc, elementRect.left, elementRect.top, elementRect.right, elementRect.bottom);

	SelectObject(hdc, hOldPen);
	SelectObject(hdc, hOldBrush);
	DeleteObject(hPen);
	DeleteObject(hBrush);

	SetBkMode(hdc, TRANSPARENT);
	SetTextColor(hdc, RGB(element->textColorR, element->textColorG, element->textColorB));
	HFONT hFont = CreateFont(-(int)element->fontSize, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, TEXT("Arial"));
	HFONT hOldFont = SelectObject(hdc, hFont);

	DrawText(hdc, element->text, -1, &elementRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);

	SelectObject(hdc, hOldFont);
	DeleteObject(hFont);
}
void GenerateObjCTextFieldCode(WCHAR* codeBuffer, size_t bufferSize, iOSUIElement * element) {
	char textColorCode_mbcs[100];
	RgbToUIColorCode(element->textColorR, element->textColorG, element->textColorB, textColorCode_mbcs, sizeof(textColorCode_mbcs));

	char bgColorCode_mbcs[100];
	RgbToUIColorCode(element->bgColorR, element->bgColorG, element->bgColorB, bgColorCode_mbcs, sizeof(bgColorCode_mbcs));


	WCHAR textColorCode_wc[100];
	MultiByteToWideChar(CP_UTF8, 0, textColorCode_mbcs, -1, textColorCode_wc, _countof(textColorCode_wc));

	WCHAR bgColorCode_wc[100];
	MultiByteToWideChar(CP_UTF8, 0, bgColorCode_mbcs, -1, bgColorCode_wc, _countof(bgColorCode_wc));

	char text_utf8[512];
	WideCharToMultiByte(CP_UTF8, 0, element->text, -1, text_utf8, _countof(text_utf8), NULL, NULL);
	WCHAR text_wc[512];
	MultiByteToWideChar(CP_UTF8, 0, text_utf8, -1, text_wc, _countof(text_wc));

	swprintf_s(codeBuffer, bufferSize,
	           TEXT("    UITextField *myTextField%d = [UIViewController createTextFieldWithFrame:CGRectMake(%d, %d, %d, %d)\r\n")
	           TEXT("                                                            borderWidth:1.0 // Example\r\n")
	           TEXT("                                                            borderColor:[UIColor lightGrayColor].CGColor // Example\r\n")
	           TEXT("                                                        	 clearButton:UITextFieldViewModeAlways // Example\r\n")
	           TEXT("                                                            placeHolder:@\"%ls\"\r\n")
	           TEXT("                                                          	 retKey:UIReturnKeyDone // Example\r\n")
	           TEXT("                                                            font:[UIFont fontWithName:@\"arial\" size:%.2f] // Font size\r\n")
	           TEXT("                                                    		 keyboardAppearance:UIKeyboardAppearanceDark]; // Example\r\n")
	           TEXT("    // myTextField%d.delegate = self; // If delegate needs to be set\r\n")
	           TEXT("    [self.view addSubview:myTextField%d];\r\n\r\n"),
	           element->id, element->original_x, element->original_y, element->original_width, element->original_height,
	           text_wc,  element->fontSize, element->id, element->id
	          );
}


// Table View functions
void DrawTableView(HDC hdc, iOSUIElement* element) {
	RECT elementRect = {element->x, element->y, element->x + element->width, element->y + element->height};

	HPEN hPen = CreatePen(PS_SOLID, 1, RGB(100, 0, 100));
	HBRUSH hBrush = CreateSolidBrush(RGB(255, 200, 255));
	HPEN hOldPen = SelectObject(hdc, hPen);
	HBRUSH hOldBrush = SelectObject(hdc, hBrush);

	Rectangle(hdc, elementRect.left, elementRect.top, elementRect.right, elementRect.bottom);

	SelectObject(hdc, hOldPen);
	SelectObject(hdc, hOldBrush);
	DeleteObject(hPen);
	DeleteObject(hBrush);

	SetBkMode(hdc, TRANSPARENT);
	SetTextColor(hdc, RGB(0, 0, 0));
	hPen = CreatePen(PS_SOLID, 1, RGB(180, 180, 180));
	hOldPen = SelectObject(hdc, hPen);

	int currentY = elementRect.top + element->rowHeight;

	while (currentY < elementRect.bottom) {
		MoveToEx(hdc, elementRect.left, currentY, NULL);

		LineTo(hdc, elementRect.right, currentY);
		currentY += element->rowHeight;
	}

	SelectObject(hdc, hOldPen);
	DeleteObject(hPen);

	HFONT hFont = CreateFont(-17, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, TEXT("Arial"));
	HFONT hOldFont = SelectObject(hdc, hFont);
	DrawText(hdc, TEXT("表格视图"), -1, &elementRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	SelectObject(hdc, hOldFont);
	DeleteObject(hFont);

}
void GenerateObjCTableViewCode(WCHAR* codeBuffer, size_t bufferSize, iOSUIElement * element) {

	swprintf_s(codeBuffer, bufferSize,
	           TEXT("    UITableView *myTableView%d = [UIViewController createTableViewWithFrame:CGRectMake(%d, %d, %d, %d)\r\n")
	           TEXT("                                                                    style:UITableViewStylePlain // Example\r\n")
	           TEXT("                                                              borderw:1.0 // Example\r\n")
	           TEXT("                                                              borderColor:[UIColor lightGrayColor].CGColor // Example\r\n")
	           TEXT("                                                                ciden:@\"MyCellIdentifier%d\"]; // Example\r\n")
	           TEXT("	 //myTableView%d.dataSource=self; // Example\r\n")
	           TEXT("	 //myTableView%d.delegate=self; // Example\r\n")
	           TEXT("    [self.view addSubview:myTableView%d];\r\n\r\n"),
	           element->id, element->original_x, element->original_y, element->original_width, element->original_height,
	           element->id,
	           element->id, element->id, element->id
	          );
}

// Slider functions
/*
void DrawSlider(HDC hdc, iOSUIElement* element) {
	RECT elementRect = {element->x, element->y, element->x + element->width, element->y + element->height};

	int sliderCenterY = elementRect.top + (elementRect.bottom - elementRect.top) / 2;
	int thumbX = elementRect.left + (int)((element->currentValue - element->minValue) / (element->maxValue - element->minValue) * element->width);

	if (thumbX < elementRect.left) thumbX = elementRect.left;
	if (thumbX > elementRect.right) thumbX = elementRect.right;

	HPEN hPen = CreatePen(PS_SOLID, 2, RGB(150, 150, 150));
	HPEN hOldPen = SelectObject(hdc, hPen);
	MoveToEx(hdc, elementRect.left, sliderCenterY, NULL);
	LineTo(hdc, elementRect.right, sliderCenterY);
	SelectObject(hdc, hOldPen);
	DeleteObject(hPen);

	HBRUSH hBrush = CreateSolidBrush(RGB(0, 122, 255));
	HBRUSH hOldBrush = SelectObject(hdc, hBrush);
	Ellipse(hdc, thumbX - 8, sliderCenterY - 8, thumbX + 8, sliderCenterY + 8);
	SelectObject(hdc, hOldBrush);
	DeleteObject(hBrush);

	SetBkMode(hdc, TRANSPARENT);
	SetTextColor(hdc, RGB(255, 255, 255));
	WCHAR valueStr[50];
	swprintf_s(valueStr, _countof(valueStr), TEXT("%.1f"), element->currentValue);
	RECT valueRect = {elementRect.left, elementRect.top, elementRect.right, sliderCenterY - 10};
	HFONT hFont = CreateFont(-14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, TEXT("Arial"));
	HFONT hOldFont = SelectObject(hdc, hFont);
	DrawText(hdc, valueStr, -1, &valueRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	SelectObject(hdc, hOldFont);
	DeleteObject(hFont);
}
void GenerateObjCSliderCode(WCHAR* codeBuffer, size_t bufferSize, iOSUIElement* element) {
	swprintf_s(codeBuffer, bufferSize,
	           TEXT("    UISlider *mySlider%d = [[UISlider alloc] initWithFrame:CGRectMake(%d, %d, %d, %d)];\r\n")
	           TEXT("    mySlider%d.minimumValue = %.1f;\r\n")
	           TEXT("    mySlider%d.maximumValue = %.1f;\r\n")
	           TEXT("    mySlider%d.value = %.1f;\r\n")
	           TEXT("    [mySlider%d addTarget:self action:@selector(sliderValueChanged:) forControlEvents:UIControlEventValueChanged];\r\n")
	           TEXT("    [self.view addSubview:mySlider%d];\r\n\r\n"),
	           element->id, element->original_x, element->original_y, element->original_width, element->original_height,
	           element->id, element->minValue,
	           element->id, element->maxValue,
	           element->id, element->currentValue,
	           element->id,
	           element->id
	          );
}*/

// Label functions
void DrawLabel(HDC hdc, iOSUIElement* element) {
	RECT elementRect = {element->x, element->y, element->x + element->width, element->y + element->height};

	// Only draw background if it's not transparent (all zeros)
	if (element->bgColorR != 0 || element->bgColorG != 0 || element->bgColorB != 0) {
		HBRUSH hBrush = CreateSolidBrush(RGB(element->bgColorR, element->bgColorG, element->bgColorB));
		HBRUSH hOldBrush = SelectObject(hdc, hBrush);
		FillRect(hdc, &elementRect, hBrush);
		SelectObject(hdc, hOldBrush);
		DeleteObject(hBrush);
	}

	SetBkMode(hdc, TRANSPARENT);
	SetTextColor(hdc, RGB(element->textColorR, element->textColorG, element->textColorB));
	HFONT hFont = CreateFont(-(int)element->fontSize, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, TEXT("Arial"));
	HFONT hOldFont = SelectObject(hdc, hFont);

	DrawText(hdc, element->text, -1, &elementRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);

	SelectObject(hdc, hOldFont);
	DeleteObject(hFont);
}
void GenerateObjCLabelCode(WCHAR* codeBuffer, size_t bufferSize, iOSUIElement* element) {
	char textColorCode_mbcs[100];
		RgbToUIColorCode(element->textColorR, element->textColorG, element->textColorB, textColorCode_mbcs, sizeof(textColorCode_mbcs));
	
		char bgColorCode_mbcs[100];
		RgbToUIColorCode(element->bgColorR, element->bgColorG, element->bgColorB, bgColorCode_mbcs, sizeof(bgColorCode_mbcs));
	
	
		WCHAR textColorCode_wc[100];
		MultiByteToWideChar(CP_UTF8, 0, textColorCode_mbcs, -1, textColorCode_wc, _countof(textColorCode_wc));
	
		WCHAR bgColorCode_wc[100];
		MultiByteToWideChar(CP_UTF8, 0, bgColorCode_mbcs, -1, bgColorCode_wc, _countof(bgColorCode_wc));
	
		char text_utf8[512];
		WideCharToMultiByte(CP_UTF8, 0, element->text, -1, text_utf8, _countof(text_utf8), NULL, NULL);
		WCHAR text_wc[512];
		MultiByteToWideChar(CP_UTF8, 0, text_utf8, -1, text_wc, _countof(text_wc));
	

	swprintf_s(codeBuffer, bufferSize,
	           TEXT("UILabel *myLabel%d = [UIViewController createlabelwithframe:CGRectMake(%d,%d,%d,%d)\r\n")
	           TEXT("			backcolor:%ls\r\n")
	           TEXT("			font:[UIFont fontWithName:@\"arial\" size:%.1f]\r\n")
	           TEXT("			textcolor:%ls\r\n")
	           TEXT("			textalignment:NSTextAlignmentCenter\r\n")
	           TEXT("			text:@\"%ls\"];\r\n")
	           TEXT("    [self.view addSubview:myLabel%d];\r\n\r\n"),
	           element->id, element->original_x, element->original_y, element->original_width, element->original_height,
	           bgColorCode_wc,
	           element->fontSize,
	           textColorCode_wc,
	           text_wc, 
			   element->id
	          );

}


// Updates the property panel display
void UpdatePropertiesPanel(void) {
	if (g_selectedElementIndex != -1) {
		iOSUIElement *element = &g_iOSUIElements[g_selectedElementIndex];

		SetWindowText(edits[IDE_PROP_TYPE - IDE_PROP_TYPE].thing, element->type);

		HideAllPropertiesUIControls(); // First, hide all controls

		UpdatePropertiesPanelUI(element); // Then, display relevant controls based on selected element's properties

		EnableWindow(g_hPropConfirmButton, TRUE);

	} else {
		SetWindowText(edits[IDE_PROP_TYPE - IDE_PROP_TYPE].thing, TEXT("无选中"));
		HideAllPropertiesUIControls(); // Hide all controls
		EnableWindow(g_hPropConfirmButton, FALSE); // Disable confirm button when nothing is selected
	}
}

// Regenerates all Objective-C code
void RegenerateAllObjCCode(void) {
	//WCHAR generatedCodeBuffer[204800]; // Larger buffer to accommodate more code
	ZeroMemory(generatedCodeBuffer, sizeof(generatedCodeBuffer));

	wcscat_s(generatedCodeBuffer, _countof(generatedCodeBuffer), TEXT("// 引入你的通用 UI 工厂类别头文件 (csm.h)\r\n"));
	wcscat_s(generatedCodeBuffer, _countof(generatedCodeBuffer), TEXT("#import \"csm.h\" \r\n\r\n"));
	wcscat_s(generatedCodeBuffer, _countof(generatedCodeBuffer), TEXT("// 在你的 UIViewController 的 viewDidLoad 或其他方法中：\r\n"));
	wcscat_s(generatedCodeBuffer, _countof(generatedCodeBuffer), TEXT("- (void)setupGeneratedUI {\r\n"));
	wcscat_s(generatedCodeBuffer, _countof(generatedCodeBuffer), TEXT("    // 假设 'screen' 和 'self.view' 在当前上下文可用\r\n\r\n"));

	for (int i = 0; i < g_iOSUIElementCount; i++) {
		WCHAR elementCode[2048];
		ZeroMemory(elementCode, sizeof(elementCode));

		// Use function pointer to call the corresponding code generation function
		if (g_iOSUIElements[i].definition && g_iOSUIElements[i].definition->generateCodeFunc) {
			g_iOSUIElements[i].definition->generateCodeFunc(elementCode, _countof(elementCode), &g_iOSUIElements[i]);
		}
		wcscat_s(generatedCodeBuffer, _countof(generatedCodeBuffer), elementCode);
	}
	wcscat_s(generatedCodeBuffer, _countof(generatedCodeBuffer), TEXT("}\r\n"));

	SetWindowText(g_hCodeEdit, generatedCodeBuffer); // Set the text of the EDIT control

}


void InitDefaultData(iOSUIElement* element) {
	element->x = element->definition->Defaults.defaultx;
	element->y = element->definition->Defaults.defaulty;

	wcscpy_s(element->text, _countof(element->text), element->definition->Defaults.defaultext);
	element->bgColorR = element->definition->Defaults.defaultbr;
	element->bgColorG = element->definition->Defaults.defaultbg;
	element->bgColorB = element->definition->Defaults.defaultbb;

	element->textColorR = element->definition->Defaults.defaulttr;
	element->textColorG = element->definition->Defaults.defaulttg;
	element->textColorB = element->definition->Defaults.defaulttb;

	element->fontSize = element->definition->Defaults.defaultfontsize;

	element->minValue = element->definition->Defaults.defaultminval;
	element->maxValue = element->definition->Defaults.defaultmaxval;

	element->rowHeight = element->definition->Defaults.defaultrowheight;

	element->original_x = element->definition->Defaults.defaultx;
	element->original_y = element->definition->Defaults.defaulty;
	element->original_width = element->definition->Defaults.defaultWidth; // Default width for button
	element->original_height = element->definition->Defaults.defaultHeight;
}

void initprop(HWND hwnd) {
	int count = 0;
	currY = propPanelStartY;
	HINSTANCE hInstance = (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE);

	for (idclist i = IDC_PROP_TYPE; i < IDC_COUNT; ++i) {

		count = i - IDC_PROP_TYPE;
		labels[count].name = labelname[count];


		labels[count].lx = panelPadding; // 其他标签放置在第一列
		labels[count].y = currY;

		labels[count].labelh = CreateWindowEx(0, TEXT("STATIC"), labels[count].name, WS_CHILD | WS_VISIBLE,
		                                      labels[count].lx, labels[count].y, labelWidth, controlHeight, hwnd, (HMENU)i, hInstance, NULL);

		currY += rowSpacing;

	}
	count = 0;
	int currY2 = propPanelStartY;
	for (idelist i = IDE_PROP_TYPE; i < IDE_COUNT; ++i) {
		count = i - IDE_PROP_TYPE;
		switch (i) {
			case IDE_PROP_TYPE: {
				edits[count].x = panelPadding + labelWidth + colSpacing;
				edits[count].y = currY2;
				edits[count].width = longEditWidth;
				break;
			}
			case IDE_PROP_BG_R: {
				edits[count].x = panelPadding + labelWidth + colSpacing;
				edits[count].y = labels[IDC_PROP_BG_R - IDC_PROP_TYPE].y;
				edits[count].width = colorEditWidth;
				break;
			}
			case IDE_PROP_BG_G: {
				edits[count].x = panelPadding + labelWidth + colSpacing + colorEditWidth + colSpacing;
				edits[count].y = labels[IDC_PROP_BG_R - IDC_PROP_TYPE].y;
				edits[count].width = colorEditWidth;
				currY2 -= rowSpacing;

				break;
			}
			case IDE_PROP_BG_B: {
				edits[count].x = panelPadding + labelWidth + colSpacing + (colorEditWidth + colSpacing) * 2;
				edits[count].y = labels[IDC_PROP_BG_R - IDC_PROP_TYPE].y;
				edits[count].width = colorEditWidth;
				currY2 -= rowSpacing;

				break;
			}
			case IDE_PROP_TEXT_R: {
				edits[count].x = panelPadding + labelWidth + colSpacing;
				edits[count].y = labels[IDC_PROP_TEXT_R - IDC_PROP_TYPE].y;
				edits[count].width = colorEditWidth;
				break;
			}
			case IDE_PROP_TEXT_G: {
				edits[count].x = panelPadding + labelWidth + colSpacing + colorEditWidth + colSpacing;
				edits[count].y = labels[IDC_PROP_TEXT_R - IDC_PROP_TYPE].y;
				edits[count].width = colorEditWidth;
				currY2 -= rowSpacing;

				break;
			}
			case IDE_PROP_TEXT_B: {
				edits[count].x = panelPadding + labelWidth + colSpacing + (colorEditWidth + colSpacing) * 2;
				edits[count].y = labels[IDC_PROP_TEXT_R - IDC_PROP_TYPE].y;
				edits[count].width = colorEditWidth;
				currY2 -= rowSpacing;

				break;
			}
			case IDE_PROP_TEXT: {
				edits[count].x = panelPadding + labelWidth + colSpacing ;
				edits[count].y = currY2;
				edits[count].width = longEditWidth;
				break;
			}
			default:
				edits[count].x = panelPadding + labelWidth + colSpacing;
				edits[count].y = currY2;
				edits[count].width = editWidth;

				break;

		}
		if (i == IDE_PROP_TYPE)
			edits[count].thing = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("STATIC"), TEXT(""), WS_CHILD | WS_VISIBLE | SS_CENTER,
			                                    edits[count].x, edits[count].y, edits[count].width, controlHeight, hwnd, (HMENU)i, hInstance, NULL);
		else
			edits[count].thing = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("EDIT"), TEXT(""), WS_CHILD | WS_VISIBLE | SS_CENTER | ES_AUTOHSCROLL,
			                                    edits[count].x, edits[count].y, edits[count].width, controlHeight, hwnd, (HMENU)i, hInstance, NULL);

		currY2 += rowSpacing;
	}

}
