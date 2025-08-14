#ifndef IOSBUILDER_H
#define IOSBUILDER_H

#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <commctrl.h>
#include <uxtheme.h>
#include <tchar.h>

// --- Global Constants and Macros ---
#define MAIN_WINDOW_CLASS_NAME TEXT("Win32iOSUIBuilderClass")
#define MAIN_WINDOW_TITLE      TEXT("我的 iOS UI Builder (Win32) - 统一属性管理")

// Control ID definitions
#define IDC_TOOLBOX_PANEL      1001
#define IDC_CANVAS_PANEL       1002
#define IDC_CODE_OUTPUT_PANEL  1003
#define IDC_CODE_DISPLAY_EDIT  1004
#define IDC_CODE_TITLE_STATIC  1005
#define IDC_PROPERTIES_TITLE_STATIC 1006

// Control button IDs in the toolbox
#define IDC_TOOL_BUTTON        1101
#define IDC_TOOL_TEXTFIELD     1102
#define IDC_TOOL_TABLEVIEW		1103
#define IDC_TOOL_SLIDER        1104 // New: Slider tool ID
#define IDC_TOOL_LABEL         1105 // New: Label tool ID

// Control IDs in the properties panel
#define IDC_PROPERTIES_PANEL   1200
#define IDC_PROP_TYPE_LABEL    1201
#define IDC_PROP_TYPE_STATIC   1202
#define IDC_PROP_X_LABEL       1203
#define IDC_PROP_X_EDIT        1204
#define IDC_PROP_Y_LABEL       1205
#define IDC_PROP_Y_EDIT        1206
#define IDC_PROP_WIDTH_LABEL   1207
#define IDC_PROP_WIDTH_EDIT    1208
#define IDC_PROP_HEIGHT_LABEL  1209
#define IDC_PROP_HEIGHT_EDIT   1210
#define IDC_PROP_TEXT_LABEL    1211
#define IDC_PROP_TEXT_EDIT     1212
#define IDC_PROP_CONFIRM_BUTTON 1213

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

#define IDC_PROP_MIN_VALUE_LABEL 1224 // New: Slider min value label
#define IDC_PROP_MIN_VALUE_EDIT  1225 // New: Slider min value edit box
#define IDC_PROP_MAX_VALUE_LABEL 1226 // New: Slider max value label
#define IDC_PROP_MAX_VALUE_EDIT  1227 // New: Slider max value edit box
#define IDC_PROP_CURRENT_VALUE_LABEL 1228 // New: Slider current value label
#define IDC_PROP_CURRENT_VALUE_EDIT  1229 // New: Slider current value edit box

//IDM_DELETE_ELEMENT
#define IDM_DELETE_ELEMENT 5001

// Base ID for generated iOS UI elements
#define IDC_GENERATED_UI_BASE  2000

// Define original canvas dimensions for scaling calculations
#define CANVAS_INITIAL_WIDTH  393
#define CANVAS_INITIAL_HEIGHT 852

#define BUTTON_SPACING 200
extern const double CANVAS_ASPECT_RATIO;

typedef struct UIElementDefinition UIElementDefinition;

typedef struct iOSUIElement {
	int id;
	WCHAR type[50];
	int x, y, width, height; // Current drawing dimensions (will be same as original for fixed canvas)
	int original_x, original_y, original_width, original_height; // Original dimensions (used for code generation and internal logic)
	WCHAR text[256];
	int bgColorR, bgColorG, bgColorB;
	int textColorR, textColorG, textColorB;
	float fontSize;
	float minValue;
	float maxValue;
	float currentValue;
	int rowHeight;
	UIElementDefinition* definition; // Pointer to its definition for polymorphic calls

} iOSUIElement;
// Forward declaration of UIElementDefinition struct
typedef struct UIDefaultData {
	int defaultx, defaulty;
	int defaultWidth, defaultHeight;  // Default width when first created
	WCHAR defaultext[256];
	int defaultbr, defaultbg, defaultbb;
	int defaulttr, defaulttg, defaulttb;
	int defaultfontsize;
	int defaultminval, defaultmaxval, defaultcurrval;
	int defaultrowheight;
} UIDefaultData;

typedef struct UIElementDefinition {
	WCHAR typeName[50]; // Control type name (e.g., L"Button")
	int toolButtonId;   // Associated tool button ID (e.g., IDC_TOOL_BUTTON)
	// Default height when first created

	// Pointers to functions that handle type-specific behavior
	void (*initDefaultDataFunc)(iOSUIElement* element);         // Initialize default data
	void (*drawFunc)(HDC hdc, iOSUIElement* element);           // Drawing function
	void (*generateCodeFunc)(WCHAR* codeBuffer, size_t bufferSize, iOSUIElement* element); // Code generation function
	UIDefaultData Defaults;

//has*
	BOOL hasText;
	BOOL hasBgColor;
	BOOL hasTextColor;
	BOOL hasFontSize;
	BOOL hasMinValue;
	BOOL hasMaxValue;
	BOOL hasCurrentValue;
	BOOL hasRowHeight;
} UIElementDefinition;
// iOS UI Element structure definition

// UI Element Definition structure

// --- extern declarations for global variables ---
// These variables are actually defined in IOSBUILDER.c
#define MAX_UI_ELEMENTS 100
extern iOSUIElement g_iOSUIElements[MAX_UI_ELEMENTS];
extern int g_iOSUIElementCount;
extern int g_selectedElementIndex;

extern HWND g_hPropTypeStatic;
extern HWND g_hPropXEdit, g_hPropYEdit, g_hPropWidthEdit, g_hPropHeightEdit, g_hPropTextEdit;
extern HWND g_hPropConfirmButton;

extern HWND g_hPropBgR_Edit, g_hPropBgG_Edit, g_hPropBgB_Edit;
extern HWND g_hPropTextR_Edit, g_hPropTextG_Edit, g_hPropTextB_Edit;
extern HWND g_hPropFontSize_Edit;

extern HWND g_hPropMinValue_Edit, g_hPropMaxValue_Edit, g_hPropCurrentValue_Edit; // New slider property edit box handles

extern HWND g_hCanvasPanel;
extern HWND g_hCodeEdit;
extern HWND hToolboxPanel;
extern HWND hCodeOutputPanel;

extern HBRUSH g_hbrPropertiesPanelBackground;

extern UIElementDefinition g_elementDefinitions[]; // UI element definition array
extern const int g_numElementDefinitions; // Number of UI element definitions


// --- Function Prototypes (Declarations) ---
// Window procedure functions for main window and panels
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK CanvasPanelProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK toolpanelproc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK CodePropertiesPanelProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Helper functions
void RgbToUIColorCode(int r, int g, int b, char* buffer, size_t bufferSize);

// Generic functions for property panel management
void HideAllPropertiesUIControls(void); // Hide all property controls
void UpdatePropertiesPanelUI(iOSUIElement* element); // Update UI display based on element properties
void ReadPropertiesFromUI(iOSUIElement* element); // Read properties from UI and update element

// Element-Specific Functions (prototypes)
void DrawButton(HDC hdc, iOSUIElement* element);
void GenerateObjCButtonCode(WCHAR* codeBuffer, size_t bufferSize, iOSUIElement* element);

void DrawTextField(HDC hdc, iOSUIElement* element);
void GenerateObjCTextFieldCode(WCHAR* codeBuffer, size_t bufferSize, iOSUIElement* element);

void DrawTableView(HDC hdc, iOSUIElement* element);
void GenerateObjCTableViewCode(WCHAR* codeBuffer, size_t bufferSize, iOSUIElement* element);

void DrawSlider(HDC hdc, iOSUIElement* element); // New: Slider drawing function
void GenerateObjCSliderCode(WCHAR* codeBuffer, size_t bufferSize, iOSUIElement* element); // New: Slider code generation function

void DrawLabel(HDC hdc, iOSUIElement* element); // New: Label drawing function
void GenerateObjCLabelCode(WCHAR* codeBuffer, size_t bufferSize, iOSUIElement* element); // New: Label code generation function

// UI update functions
void UpdatePropertiesPanel(void);
void RegenerateAllObjCCode(void);

void InitDefaultData(iOSUIElement* element);

extern int g_scrollPos;

typedef struct propp {
	WCHAR *name;
	int y;
	int lx;
	HWND labelh;
} propp;
typedef struct xyh {
	int x, y;
	int width;
	HWND thing;
} xyh;

typedef enum {
	IDC_PROP_TYPE = 1201,
	IDC_PROP_X,
	IDC_PROP_Y,
	IDC_PROP_WIDTH,
	IDC_PROP_HEIGHT,
	IDC_PROP_TEXT,
	IDC_PROP_BG_R,
	IDC_PROP_TEXT_R,
	IDC_PROP_FONT_SIZE,
	IDC_PROP_MIN_VALUE,
	IDC_PROP_MAX_VALUE,
	IDC_PROP_CURRENT_VALUE,
	IDC_COUNT
} idclist ;

typedef enum {
	IDE_PROP_TYPE = 2401,
	IDE_PROP_X,
	IDE_PROP_Y,
	IDE_PROP_WIDTH,
	IDE_PROP_HEIGHT,
	IDE_PROP_TEXT,
	IDE_PROP_BG_R,
	IDE_PROP_BG_G,
	IDE_PROP_BG_B,
	IDE_PROP_TEXT_R,
	IDE_PROP_TEXT_G,
	IDE_PROP_TEXT_B,
	IDE_PROP_FONT_SIZE,
	IDE_PROP_MIN_VALUE,
	IDE_PROP_MAX_VALUE,
	IDE_PROP_CURRENT_VALUE,
	IDE_COUNT
} idelist;
extern void initprop(HWND hwnd);

extern propp labels[25];
extern xyh edits[30];

extern const int labelWidth ;
extern const int editWidth;
extern const int colorEditWidth;
extern const int longEditWidth;
extern const int controlHeight;
extern const int rowSpacing ;
extern const int colSpacing;
extern const int panelPadding;

extern int propPanelStartY ;

extern int combutx;
extern int combuty;
extern int currY;

 extern WCHAR generatedCodeBuffer[204800]; // Larger buffer to accommodate more code

#endif // IOSBUILDER_H
