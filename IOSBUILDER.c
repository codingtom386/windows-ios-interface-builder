#define UNICODE     // Force Unicode for Win32 API
#define _UNICODE    // Force Unicode for C Runtime Library

#include "IOSBUILDER.h"

WCHAR generatedCodeBuffer[210000]; // Larger buffer to accommodate more code

const int labelWidth = 90;
const int editWidth = 80;
const int colorEditWidth = 40;
const int longEditWidth = 265;
const int controlHeight = 30;
const int rowSpacing = 40;
const int colSpacing = 10;
const int panelPadding = 10;

int propPanelStartY = 50;

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
		1, 1, 1, 1, 0, 0, 0, 0, 0
	},
	{
		TEXT("TextField"), 1102, InitDefaultData, DrawTextField, GenerateObjCTextFieldCode,
		{50, 50, 240, 50, TEXT("搜索框"), 100, 100, 100, 255, 255, 255, 24.0, 0, 0, 0, 0},
		1, 1, 1, 1, 0, 0, 0, 0, 0
	},
	{
		TEXT("TableView"), 1103, InitDefaultData, DrawTableView, GenerateObjCTableViewCode,
		{ 50, 50, 130, 130, TEXT(""), 233, 233, 233, 255, 255, 255, 24.0, 0, 0, 0, 44}
		, 0, 0, 0, 0, 0, 0, 0, 1, 0
	},
	/*{
		TEXT("Slider"), 1104, InitDefaultData, DrawSlider, GenerateObjCSliderCode,
		{50, 50, 90, 30, TEXT(""), 0, 0, 0, 0, 0, 0, 0.0, 0, 100, 0, 0}
		, 0, 0, 0, 0, 1, 1, 1, 0,0
	},*/
	{
		TEXT("Label"), 1105, InitDefaultData, DrawLabel, GenerateObjCLabelCode,
		{50, 50, 100, 30, TEXT("label"), 100, 100, 100, 255, 255, 255, 24.0, 0, 0, 0, 0}
		, 1, 1, 1, 1, 0, 0, 0, 0, 1
	},



};
const int g_numElementDefinitions = sizeof(g_elementDefinitions) / sizeof(UIElementDefinition);

xyh edits[30];
propp labels[25];
WCHAR labelname[][20] = {
	TEXT("类型:"),
	TEXT("变量名:"),
	TEXT("X:"),
	TEXT("Y:"),
	TEXT("宽度:"),
	TEXT("高度:"),
	TEXT("文本:"),
	TEXT("背景色:"),
	TEXT("文本色:"),
	TEXT("字体大小:"),
	TEXT("最小值:"),
	TEXT("最大值:"),
	TEXT("当前值:"),
	TEXT("链接函数:"),
	TEXT("对齐方式"),
};

WCHAR textAlign[][50] = {
	TEXT("Center"),
	TEXT("Right"),
	TEXT("Natural"),
	TEXT("Left"),
	TEXT("Justified")
};
int textAligns = _countof(textAlign);


// --- Helper function implementations ---

void RgbToUIColorCode(int r, int g, int b, char* buffer, size_t bufferSize) {
	r = (r < 0) ? 0 : ((r > 255) ? 255 : r);
	g = (g < 0) ? 0 : ((g > 255) ? 255 : g);
	b = (b < 0) ? 0 : ((b > 255) ? 255 : b);

	sprintf_s(buffer, bufferSize, "[UIColor colorWithRed:%.3f  green:%.3f  blue:%.3f  alpha:1.0]",
	          (float)r / 255.0, (float)g / 255.0, (float)b / 255.0);
}

void HideAllPropertiesUIControls(void) {
	for (idclist i = IDC_PROP_X; i < IDC_COUNT; i++)
		ShowWindow(GetDlgItem(hCodeOutputPanel, i), SW_HIDE);
	for (idelist j = IDE_PROP_X; j < IDE_COUNT; j++)
		EnableWindow(edits[j - IDE_PROP_TYPE].thing, FALSE);

	for (idelist i = IDE_PROP_X; i <= IDE_COUNT; i++) {
		SetWindowText(edits[i - IDE_PROP_TYPE].thing, TEXT(""));
	}

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
	if (wcscmp(element->type, TEXT("Button")) == 0) {
		ShowWindow(GetDlgItem(hCodeOutputPanel, IDC_PROP_CONPROC), SW_SHOW);
		EnableWindow(edits[IDE_PROP_CONPROC - IDE_PROP_TYPE].thing, TRUE);
		SetWindowText(edits[IDE_PROP_CONPROC - IDE_PROP_TYPE].thing, element->linkproc);
	}

	if (wcscmp(element->type, TEXT("Label")) == 0) {
		ShowWindow(GetDlgItem(hCodeOutputPanel, IDC_PROP_ALIGNMENT), SW_SHOW);
		EnableWindow(edits[IDE_PROP_ALIGNMENT - IDE_PROP_TYPE].thing, TRUE);
		ComboBox_SetCurSel(edits[IDE_PROP_ALIGNMENT - IDE_PROP_TYPE].thing, element->alignment);
	}
}

// Reads property values from UI controls and updates the element's properties.
void ReadPropertiesFromUI(iOSUIElement* element) {
	WCHAR buffer[256];
	float floatValue;

	GetWindowText(edits[IDE_PROP_VNAME - IDE_PROP_TYPE].thing, element->Vname, _countof(element->Vname));
	GetWindowText(edits[IDE_PROP_CONPROC - IDE_PROP_TYPE].thing, element->linkproc, _countof(element->linkproc));

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
	if (wcscmp(element->type, TEXT("Button")) == 0) {
		GetWindowText(edits[IDE_PROP_CONPROC - IDE_PROP_TYPE].thing, element->linkproc, _countof(element->linkproc));
	}
	if (wcscmp(element->type, TEXT("Label")) == 0) {
		element->alignment = ComboBox_GetCurSel(edits[IDE_PROP_ALIGNMENT - IDE_PROP_TYPE].thing);
	}

}


// Button functions
void DrawButton(HDC hdc, iOSUIElement* element) {
	HMODULE g_hBuilderCoreDll = NULL;
	g_hBuilderCoreDll = LoadLibraryW(L"libbuidll.dll");
	if (g_hBuilderCoreDll == NULL)
		MessageBeep(MB_ICONERROR);
	void (*pfn)(HDC hdc, iOSUIElement * element);
	pfn = (void *)GetProcAddress(g_hBuilderCoreDll, "DrawButton");
	if (!pfn)
		MessageBeep(MB_ICONERROR);
	pfn(hdc, element);
	FreeLibrary(g_hBuilderCoreDll);

}

void GenerateObjCButtonCode(WCHAR* codeBuffer, size_t bufferSize, iOSUIElement * element) {
	HMODULE g_hBuilderCoreDll = NULL;
	g_hBuilderCoreDll = LoadLibraryW(L"libbuidll.dll");
	if (g_hBuilderCoreDll == NULL)
		MessageBeep(MB_ICONERROR);
	void (*pfn)(WCHAR * codeBuffer, size_t bufferSize, iOSUIElement * element);
	pfn = (void *)GetProcAddress(g_hBuilderCoreDll, "GenerateObjCButtonCode");
	if (!pfn)
		MessageBeep(MB_ICONERROR);
	pfn(codeBuffer, bufferSize, element);
	FreeLibrary(g_hBuilderCoreDll);

}

// Text Field functions
void DrawTextField(HDC hdc, iOSUIElement* element) {
	HMODULE g_hBuilderCoreDll = NULL;
	g_hBuilderCoreDll = LoadLibraryW(L"libbuidll.dll");
	if (g_hBuilderCoreDll == NULL)
		MessageBeep(MB_ICONERROR);
	void (*pfn)(HDC hdc, iOSUIElement * element);
	pfn = (void *)GetProcAddress(g_hBuilderCoreDll, "DrawTextField");
	if (!pfn)
		MessageBeep(MB_ICONERROR);
	pfn(hdc,  element);
	FreeLibrary(g_hBuilderCoreDll);

}
void GenerateObjCTextFieldCode(WCHAR* codeBuffer, size_t bufferSize, iOSUIElement * element) {

	HMODULE g_hBuilderCoreDll = NULL;
	g_hBuilderCoreDll = LoadLibraryW(L"libbuidll.dll");
	if (g_hBuilderCoreDll == NULL)
		MessageBeep(MB_ICONERROR);
	void (*pfn)(WCHAR * codeBuffer, size_t bufferSize, iOSUIElement * element);
	pfn = (void *)GetProcAddress(g_hBuilderCoreDll, "GenerateObjCTextFieldCode");
	if (!pfn)
		MessageBeep(MB_ICONERROR);
	pfn(codeBuffer, bufferSize, element);
	FreeLibrary(g_hBuilderCoreDll);

}


// Table View functions
void DrawTableView(HDC hdc, iOSUIElement* element) {
	HMODULE g_hBuilderCoreDll = NULL;
	g_hBuilderCoreDll = LoadLibraryW(L"libbuidll.dll");
	if (g_hBuilderCoreDll == NULL)
		MessageBeep(MB_ICONERROR);
	void (*pfn)(HDC hdc, iOSUIElement * element);
	pfn = (void *)GetProcAddress(g_hBuilderCoreDll, "DrawTableView");
	if (!pfn)
		MessageBeep(MB_ICONERROR);
	pfn(hdc, element);
	FreeLibrary(g_hBuilderCoreDll);

}
void GenerateObjCTableViewCode(WCHAR* codeBuffer, size_t bufferSize, iOSUIElement * element) {
	HMODULE g_hBuilderCoreDll = NULL;
	g_hBuilderCoreDll = LoadLibraryW(L"libbuidll.dll");
	if (g_hBuilderCoreDll == NULL)
		MessageBeep(MB_ICONERROR);
	void (*pfn)(WCHAR * codeBuffer, size_t bufferSize, iOSUIElement * element);
	pfn = (void *)GetProcAddress(g_hBuilderCoreDll, "GenerateObjCTableViewCode");
	if (!pfn)
		MessageBeep(MB_ICONERROR);
	pfn(codeBuffer, bufferSize, element);
	FreeLibrary(g_hBuilderCoreDll);

}

// Slider functions
/*
void DrawSlider(HDC hdc, iOSUIElement* element) {
	HMODULE g_hBuilderCoreDll = NULL;
		g_hBuilderCoreDll = LoadLibraryW(L"libbuidll.dll");
		if (g_hBuilderCoreDll == NULL)
			MessageBeep(MB_ICONERROR);
		void (*pfn)(HDC hdc, iOSUIElement* element);
		pfn = (void *)GetProcAddress(g_hBuilderCoreDll, "DrawSlider");
		if (!pfn)
			MessageBeep(MB_ICONERROR);
		pfn(hdc,  element);
	FreeLibrary(g_hBuilderCoreDll);

}
void GenerateObjCSliderCode(WCHAR* codeBuffer, size_t bufferSize, iOSUIElement* element) {
	HMODULE g_hBuilderCoreDll = NULL;
		g_hBuilderCoreDll = LoadLibraryW(L"libbuidll.dll");
		if (g_hBuilderCoreDll == NULL)
			MessageBeep(MB_ICONERROR);
		void (*pfn)(WCHAR * codeBuffer, size_t bufferSize, iOSUIElement * element);
		pfn = (void *)GetProcAddress(g_hBuilderCoreDll, "GenerateObjCSliderCode");
		if (!pfn)
			MessageBeep(MB_ICONERROR);
		pfn(codeBuffer, bufferSize, element);
			FreeLibrary(g_hBuilderCoreDll);

}*/

// Label functions
void DrawLabel(HDC hdc, iOSUIElement* element) {
	HMODULE g_hBuilderCoreDll = NULL;
	g_hBuilderCoreDll = LoadLibraryW(L"libbuidll.dll");
	if (g_hBuilderCoreDll == NULL)
		MessageBeep(MB_ICONERROR);
	void (*pfn)(HDC hdc, iOSUIElement * element);
	pfn = (void *)GetProcAddress(g_hBuilderCoreDll, "DrawLabel");
	if (!pfn)
		MessageBeep(MB_ICONERROR);
	pfn(hdc, element);
	FreeLibrary(g_hBuilderCoreDll);

}
void GenerateObjCLabelCode(WCHAR* codeBuffer, size_t bufferSize, iOSUIElement* element) {
	HMODULE g_hBuilderCoreDll = NULL;
	g_hBuilderCoreDll = LoadLibraryW(L"libbuidll.dll");
	if (g_hBuilderCoreDll == NULL)
		MessageBeep(MB_ICONERROR);
	void (*pfn)(WCHAR * codeBuffer, size_t bufferSize, iOSUIElement * element);
	pfn = (void *)GetProcAddress(g_hBuilderCoreDll, "GenerateObjCLabelCode");
	if (!pfn)
		MessageBeep(MB_ICONERROR);
	pfn(codeBuffer, bufferSize, element);
	FreeLibrary(g_hBuilderCoreDll);

}


// Updates the property panel display
void UpdatePropertiesPanel(void) {
	if (g_selectedElementIndex != -1) {
		iOSUIElement *element = &g_iOSUIElements[g_selectedElementIndex];

		SetWindowText(edits[IDE_PROP_TYPE - IDE_PROP_TYPE].thing, element->type);
		SetWindowText(edits[IDE_PROP_VNAME - IDE_PROP_TYPE].thing, element->Vname);
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
	swprintf_s(element->Vname, 100, TEXT("%s%d"), element->type, element->id);
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
			case IDE_PROP_VNAME: {
				edits[count].x = panelPadding + labelWidth + colSpacing ;
				edits[count].y = currY2;
				edits[count].width = longEditWidth;
				break;
			}
			case IDE_PROP_ALIGNMENT:
			case IDE_PROP_CONPROC: {
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
		else if (i == IDE_PROP_ALIGNMENT)
			edits[count].thing = CreateWindowEx(
			                         0, TEXT("COMBOBOX"), NULL,
			                         CBS_DROPDOWNLIST | CBS_HASSTRINGS | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE | WS_VSCROLL,
			                         edits[count].x, edits[count].y, edits[count].width, 980,
			                         hwnd, (HMENU)IDE_PROP_ALIGNMENT, hInstance, NULL);
		else
			edits[count].thing = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("EDIT"), TEXT(""), WS_CHILD | WS_VISIBLE | SS_CENTER | ES_AUTOHSCROLL,
			                                    edits[count].x, edits[count].y, edits[count].width, controlHeight, hwnd, (HMENU)i, hInstance, NULL);

		currY2 += rowSpacing;
	}

	for (int i = 0; i < textAligns; i++)
		ComboBox_AddString(edits[IDE_PROP_ALIGNMENT - IDE_PROP_TYPE].thing, textAlign[i]);
}
