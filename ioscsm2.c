/*#include <windows.h> // �������� Win32 API �����ͺ�
#include <windowsx.h> // ���� GET_X_LPARAM, GET_Y_LPARAM
#include <stdio.h>   // ���� sprintf, swprintf
#include <string.h>  // ���� wcstombs_s
#include <stdlib.h>  // ���� _wtoi (Wide char to int)
#include <math.h>    // For round


// #include <locale.h>  // �Ƴ���������Ҫ setlocale

// --- ȫ�ֱ����ͺ궨�� ---
// ���������ʹ��ڱ���
#define MAIN_WINDOW_CLASS_NAME TEXT("Win32iOSUIBuilderClass")
#define MAIN_WINDOW_TITLE      TEXT("�ҵ� iOS UI Builder (Win32) - �����Ա༭")

// �ؼ�ID���� (����ʶ���Ӵ��ںͲ˵���)
#define IDC_TOOLBOX_PANEL      1001
#define IDC_CANVAS_PANEL       1002
#define IDC_CODE_OUTPUT_PANEL  1003 // �������ID���ڸ����ھ��
#define IDC_CODE_DISPLAY_EDIT  1004 // �����������е� EDIT �ؼ�

// �������еĿؼ���ťID
#define IDC_TOOL_BUTTON        1101 // ��iOS ��ť������
#define IDC_TOOL_TEXTFIELD     1102 // ��iOS �ı��򡱹���
#define IDC_TOOL_TABLEVIEW		1103 //ios�б���

// ��������еĿؼ�ID
#define IDC_PROPERTIES_PANEL   1200 // ������������ (Ŀǰδ�����������ڣ�������������)
#define IDC_PROP_TYPE_LABEL    1201 // ���ͱ�ǩ
#define IDC_PROP_TYPE_STATIC   1202 // ������ʾ (�޸�Ϊ��̬�ı�)
#define IDC_PROP_X_LABEL       1203 // X�����ǩ
#define IDC_PROP_X_EDIT        1204 // X����༭��
#define IDC_PROP_Y_LABEL       1205 // Y�����ǩ
#define IDC_PROP_Y_EDIT        1206 // Y����༭��
#define IDC_PROP_WIDTH_LABEL   1207 // ��ȱ�ǩ
#define IDC_PROP_WIDTH_EDIT    1208 // ��ȱ༭��
#define IDC_PROP_HEIGHT_LABEL  1209 // �߶ȱ�ǩ
#define IDC_PROP_HEIGHT_EDIT   1210 // �߶ȱ༭��
#define IDC_PROP_TEXT_LABEL    1211 // �ı���ǩ (�����л�Ϊ�����⡱����ռλ������)
#define IDC_PROP_TEXT_EDIT     1212 // �ı��༭��
#define IDC_PROP_CONFIRM_BUTTON 1213 // ������ȷ����ť

// �������Կؼ�ID
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


// ���ɵ� iOS UI Ԫ�صĻ�ID (�����ڻ�����ʶ��)
#define IDC_GENERATED_UI_BASE  2000

// ����ԭʼ�����ߴ磬���ڱ�������
#define CANVAS_INITIAL_WIDTH  393
#define CANVAS_INITIAL_HEIGHT 852
const double CANVAS_ASPECT_RATIO = (double)CANVAS_INITIAL_WIDTH / CANVAS_INITIAL_HEIGHT; // Width / Height

// ����һ���ṹ�����洢�����Ϸ��õ� iOS UI Ԫ�ص���Ϣ

typedef struct {
	int id;           // �ؼ���ΨһID (�����ڲ�ʶ�𣬲���Objective-C�����е�ID)
	WCHAR type[50];   // �ؼ����� (e.g., L"Button", L"TextField", L"TableView")

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

// ȫ���������洢�����ϵ� iOS UI Ԫ��
#define MAX_UI_ELEMENTS 100 // ���֧�ֵ� UI Ԫ������
iOSUIElement g_iOSUIElements[MAX_UI_ELEMENTS];
int g_iOSUIElementCount = 0; // ��ǰ�ѷ��õ� UI Ԫ������
// ��ǰѡ�е� UI Ԫ������ (-1 ��ʾδѡ��)
int g_selectedElementIndex = -1;

// ȫ�� HWNDs�������ڲ�ͬ�����з����������Ŀؼ��������
HWND g_hPropTypeStatic;
HWND g_hPropXEdit, g_hPropYEdit, g_hPropWidthEdit, g_hPropHeightEdit, g_hPropTextEdit;
HWND g_hPropConfirmButton; // ������ȷ����ť���

// �������Ա༭���ȫ�־��
HWND g_hPropBgR_Edit, g_hPropBgG_Edit, g_hPropBgB_Edit;
HWND g_hPropTextR_Edit, g_hPropTextG_Edit, g_hPropTextB_Edit;
HWND g_hPropFontSize_Edit;


HWND g_hCanvasPanel; // �������ľ��������ȫ�ַ���
HWND g_hCodeEdit;    // ����༭��ľ��������ȫ�ַ���
HWND hToolboxPanel;  // �����������
HWND hCodeOutputPanel; // ������������ (�������Զ�����ľ��)

static HBRUSH bgbrush = NULL;

// --- �������� ---
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK CanvasPanelProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK toolpanelproc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam); // ���������� WndProc
LRESULT CALLBACK CodePropertiesPanelProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam); // ����������/�������Ĵ��ڹ���

// ������������ RGB ��ɫֵת��Ϊ Objective-C �� UIColor �����ַ���
void RgbToUIColorCode(int r, int g, int b, char* buffer, size_t bufferSize);

// �������������� Objective-C ��ť�������� (����� WCHAR ������)
void GenerateObjCButtonCode(WCHAR* codeBuffer, size_t bufferSize, iOSUIElement* element);
// �������������� Objective-C �ı��򴴽����� (����� WCHAR ������)
void GenerateObjCTextFieldCode(WCHAR* codeBuffer, size_t bufferSize, iOSUIElement* element);
// �������������� Objective-C �����ͼ�������� (����� WCHAR ������)
void GenerateObjCTableViewCode(WCHAR* codeBuffer, size_t bufferSize, iOSUIElement* element);

// ���������������������������ʾ
void UpdatePropertiesPanel(void);
// �������������������������� Objective-C ����
void RegenerateAllObjCCode(void);
*/
// --- WinMain ������Ӧ�ó�����ڵ� ---
#include "IOSBUILDER.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	WNDCLASSEX wc = {0}; // ��������ṹ��
	WNDCLASSEX wcCanvas = {0}; // ������崰����ṹ��
	WNDCLASSEX wcTool = {0}; // ���������
	WNDCLASSEX wcCodeProp = {0}; // ����������/������崰����ṹ��

	MSG msg;             // ��Ϣ�ṹ��

	// 1. ע����������
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.cbSize        = sizeof(WNDCLASSEX);
	wc.lpfnWndProc   = WndProc;
	wc.hInstance     = hInstance;
	wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = CreateSolidBrush(RGB(66, 66, 66)); // ǳ��ɫ����
	wc.lpszClassName = MAIN_WINDOW_CLASS_NAME;

	if (!RegisterClassEx(&wc)) {
		MessageBox(NULL, TEXT("��������ע��ʧ��!"), TEXT("����"), MB_ICONERROR | MB_OK);
		return 0;
	}

	// 2. ע�ử����崰���� (��Ҫ�Զ������)
	wcCanvas.style = CS_HREDRAW | CS_VREDRAW;
	wcCanvas.cbSize        = sizeof(WNDCLASSEX);
	wcCanvas.lpfnWndProc   = CanvasPanelProc; // ʹ���Զ���Ĵ��ڹ��̺���
	wcCanvas.hInstance     = hInstance;
	wcCanvas.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wcCanvas.hbrBackground = (HBRUSH)GetStockObject(GRAY_BRUSH); // ��ɫ����
	wcCanvas.lpszClassName = TEXT("CanvasPanelClass"); // �Զ��廭������

	if (!RegisterClassEx(&wcCanvas)) {
		MessageBox(NULL, TEXT("���������ע��ʧ��!"), TEXT("����"), MB_ICONERROR | MB_OK);
		return 0;
	}

	// 3. ע�Ṥ������崰���� (��Ҫ���ڹ������ӿؼ���WM_COMMAND ��ת����������)
	wcTool.cbSize        = sizeof(WNDCLASSEX);
	wcTool.lpfnWndProc   = toolpanelproc; // ʹ���Զ���Ĵ��ڹ��̺��������ڴ������ڲ���ť���
	wcTool.hInstance     = hInstance;
	wcTool.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wcTool.hbrBackground = CreateSolidBrush(RGB(66, 66, 66)); // ��ɫ����
	wcTool.lpszClassName = TEXT("ToolPanelClass"); // �Զ��幤��������

	if (!RegisterClassEx(&wcTool)) {
		MessageBox(NULL, TEXT("��������ע��ʧ��!"), TEXT("����"), MB_ICONERROR | MB_OK);
		return 0;
	}

	// 4. ������ע�����/������崰����
	wcCodeProp.cbSize        = sizeof(WNDCLASSEX);
	wcCodeProp.lpfnWndProc   = CodePropertiesPanelProc; // ʹ���Զ���Ĵ��ڹ��̺���
	wcCodeProp.hInstance     = hInstance;
	wcCodeProp.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wcCodeProp.hbrBackground = CreateSolidBrush(RGB(66, 66, 66)); // ǳ��ɫ����
	wcCodeProp.lpszClassName = TEXT("CodePropertiesPanelClass"); // �Զ�������

	if (!RegisterClassEx(&wcCodeProp)) {
		MessageBox(NULL, TEXT("����/���������ע��ʧ��!"), TEXT("����"), MB_ICONERROR | MB_OK);
		return 0;
	}

	// 5. ����������
	HWND hMainWnd = CreateWindowEx(
	                    0,                                  // ��չ������ʽ
	                    MAIN_WINDOW_CLASS_NAME,             // ��������
	                    MAIN_WINDOW_TITLE,                  // ���ڱ���
	                    WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN, // ������ʽ (WS_CLIPCHILDREN ��ֹ�Ӵ����ػ�ʱ��˸)
	                    CW_USEDEFAULT, CW_USEDEFAULT,       // ��ʼX, ��ʼY
	                    1500, 932,                          // ���, �߶� (ʾ���ߴ�)
	                    NULL,                               // �����ھ�� (���洰��)
	                    NULL,                               // �˵����
	                    hInstance,                          // Ӧ�ó���ʵ�����
	                    NULL                                // ��������
	                );

	if (!hMainWnd) {
		MessageBox(NULL, TEXT("�����ڴ���ʧ��!"), TEXT("����"), MB_ICONERROR | MB_OK);
		return 0;
	}

	// 6. ��������壺�����䡢�������������
	// ��Щ�� STATIC �ؼ�����Ϊ����ʹ�ã������� WS_BORDER ��ʽ�Ա�����

	// ��������� (���)
	hToolboxPanel = CreateWindowEx(
	                    WS_EX_CLIENTEDGE,                   // ��չ��ʽ��3D�߿�
	                    TEXT("ToolPanelClass"),             // �ؼ�����
	                    TEXT("�ؼ���"),                       // �ı�
	                    WS_CHILD | WS_VISIBLE | SS_CENTER,  // �Ӵ��ڣ��ɼ����ı�����
	                    10, 10,                            // X, Y ���� (�����������)
	                    200, 800,                           // ���, �߶� (�����ߴ�)
	                    hMainWnd,                           // �����ھ��
	                    (HMENU)IDC_TOOLBOX_PANEL,           // �ؼ�ID
	                    hInstance,                          // Ӧ�ó���ʵ�����
	                    NULL                                // ��������
	                );

	// ��ƻ������ (�м�)
	// ��Щ�ǳ�ʼֵ��ʵ����ʾʱ����� WM_SIZE �еı����߼�����
	g_hCanvasPanel = CreateWindowEx( // �洢��ȫ�ֱ���
	                     WS_EX_CLIENTEDGE,                   // ��չ��ʽ��3D�߿�
	                     TEXT("CanvasPanelClass"),           // ʹ�������Զ���Ļ�������
	                     TEXT(""),                           // ���ı�����Ϊ����Ҫ�Լ���������
	                     WS_CHILD | WS_VISIBLE,              // �Ӵ��ڣ��ɼ�
	                     220, 10,                            // ��ʼ X, Y ����
	                     CANVAS_INITIAL_WIDTH, CANVAS_INITIAL_HEIGHT, // ��ʼ ���, �߶� (���� WM_SIZE �а���������)
	                     hMainWnd,                           // �����ھ��
	                     (HMENU)IDC_CANVAS_PANEL,            // �ؼ�ID
	                     hInstance,                          // Ӧ�ó���ʵ�����
	                     NULL                                // ��������
	                 );

	// ����/���������� (�Ҳ�) - ����ʹ���Զ�����
	hCodeOutputPanel = CreateWindowEx(
	                       WS_EX_CLIENTEDGE,                   // ��չ��ʽ��3D�߿�
	                       TEXT("CodePropertiesPanelClass"),   // ʹ���Զ�������
	                       TEXT(""),                           // ���ı����ӿؼ����ṩ����
	                       WS_CHILD | WS_VISIBLE | ES_AUTOVSCROLL,            // �Ӵ��ڣ��ɼ�
	                       930, 10,                            // X, Y ����
	                       550, 800,                           // ���, �߶� (�����ߴ�)
	                       hMainWnd,                           // �����ھ��
	                       (HMENU)IDC_CODE_OUTPUT_PANEL,       // �ؼ�ID
	                       hInstance,                          // Ӧ�ó���ʵ�����
	                       NULL                                // ��������
	                   );

	// ע�⣺�����������ڵ��ӿؼ���g_hCodeEdit, g_hPropXEdit �ȣ��Ĵ���
	// �����Ƶ��� CodePropertiesPanelProc �� WM_CREATE ��Ϣ�����С�
	// ��������ȷ����Щ�ؼ��� hCodeOutputPanel ���Ӵ��ڡ�

	// 7. �ڹ������������ӿɡ���ק���Ŀؼ���ť (Ŀǰ�ǵ��ģ��)
	CreateWindowEx(
	    0,                                  // ��չ��ʽ
	    TEXT("BUTTON"),                     // �ؼ�����
	    TEXT("iOS ��ť"),                     // ��ť�ı�
	    WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, // �Ӵ��ڣ��ɼ�����ͨ��ť
	    20, 50,                             // X, Y ���� (����� hToolboxPanel)
	    160, 40,                            // ���, �߶�
	    hToolboxPanel,                      // �����ھ�� (���������)
	    (HMENU)IDC_TOOL_BUTTON,             // �ؼ�ID
	    hInstance,                          // Ӧ�ó���ʵ�����
	    NULL                                // ��������
	);

	CreateWindowEx(
	    0,
	    TEXT("BUTTON"),
	    TEXT("iOS �ı���"),
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
	    TEXT("iOS �����ͼ"),
	    WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
	    20, 150,
	    160, 40,
	    hToolboxPanel,
	    (HMENU)IDC_TOOL_TABLEVIEW,
	    hInstance,
	    NULL
	);


	// 8. ��ʾ�͸���������
	ShowWindow(hMainWnd, nCmdShow);
	UpdateWindow(hMainWnd);

	// ��ʼ����������� (��ʾδѡ��״̬)
	UpdatePropertiesPanel();
	// ��ʼ��������
	RegenerateAllObjCCode();


	// 9. ��Ϣѭ��
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg); // �����������ϢΪ�ַ���Ϣ
		DispatchMessage(&msg);  // �ַ���Ϣ�����ڹ��̺���
	}


	return (int)msg.wParam;
}

// --- ���ڹ��̺���ʵ�� ---

// �����ڹ��̺���
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	WCHAR debugMsg[256]; // ���� MessageBox �ĵ�����Ϣ

	switch (msg) {
		case WM_CREATE: {
			if (bgbrush == NULL) {
				bgbrush = CreateSolidBrush(RGB(66, 66, 66));
			}
			if (bgbrush == NULL) {
				MessageBox(NULL, TEXT("��ˢ����ʧ��!"), TEXT("ERROR"), MB_ICONERROR | MB_OK);
				return 0;
			}
			break;
		}
		case WM_COMMAND: {
			int wmId = LOWORD(wParam); // �ؼ�ID
			// int wmEvent = HIWORD(wParam); // ֪ͨ���� (����ֻ��ȷ����ťʱ��������Ҫ����֪ͨ��)
			// HWND hCtrl = (HWND)lParam; // �ؼ����

			// �������Ա༭���֪ͨ (����ֻ�ڵ��ȷ����ťʱ����)
			if (wmId == IDC_PROP_CONFIRM_BUTTON) {
				if (g_selectedElementIndex != -1) {
					iOSUIElement *element = &g_iOSUIElements[g_selectedElementIndex];
					WCHAR buffer[256]; // ��ʱ������
					int value;
					float floatValue;

					// ��ȡ X ����
					GetWindowTextW(g_hPropXEdit, buffer, _countof(buffer));
					value = _wtoi(buffer);
					element->x = value;

					// ��ȡ Y ����
					GetWindowTextW(g_hPropYEdit, buffer, _countof(buffer));
					value = _wtoi(buffer);
					element->y = value;

					// ��ȡ Width
					GetWindowTextW(g_hPropWidthEdit, buffer, _countof(buffer));
					value = _wtoi(buffer);
					element->width = value;

					// ��ȡ Height
					GetWindowTextW(g_hPropHeightEdit, buffer, _countof(buffer));
					value = _wtoi(buffer);
					element->height = value;

					// ��ȡ Text (��ť������ı�������)
					GetWindowTextW(g_hPropTextEdit, buffer, _countof(buffer));
					wcscpy_s(element->text, _countof(element->text), buffer);

					// ��ȡ������ɫ (R, G, B)
					GetWindowTextW(g_hPropBgR_Edit, buffer, _countof(buffer));
					element->bgColorR = _wtoi(buffer);
					GetWindowTextW(g_hPropBgG_Edit, buffer, _countof(buffer));
					element->bgColorG = _wtoi(buffer);
					GetWindowTextW(g_hPropBgB_Edit, buffer, _countof(buffer));
					element->bgColorB = _wtoi(buffer);

					// ��ȡ�ı���ɫ (R, G, B)
					GetWindowTextW(g_hPropTextR_Edit, buffer, _countof(buffer));
					element->textColorR = _wtoi(buffer);
					GetWindowTextW(g_hPropTextG_Edit, buffer, _countof(buffer));
					element->textColorG = _wtoi(buffer);
					GetWindowTextW(g_hPropTextB_Edit, buffer, _countof(buffer));
					element->textColorB = _wtoi(buffer);

					// ��ȡ�����С
					GetWindowTextW(g_hPropFontSize_Edit, buffer, _countof(buffer));
					// ʹ�� _wtof �����ַ�������ת��Ϊ float
					floatValue = (float)_wtof(buffer);
					element->fontSize = floatValue;

					// --- �Ƴ����� bug �Ĵ���飺�ı������ݸı��һ����ť���ֵ��߼� ---
					// if (wcscmp(element->type, L"TextField") == 0) {
					//     for (int i = 0; i < g_iOSUIElementCount; i++) {
					//         if (wcscmp(g_iOSUIElements[i].type, L"Button") == 0) {
					//             wcscpy_s(g_iOSUIElements[i].text, _countof(g_iOSUIElements[i].text), buffer);
					//             break;
					//         }
					//     }
					// }
					// --- �Ƴ����� ---

					// ��ȡ������ǰʵ�ʳߴ磬���ڼ��㷴�����
					RECT currentCanvasRect;
					GetClientRect(g_hCanvasPanel, &currentCanvasRect);
					int currentCanvasWidth = currentCanvasRect.right;

					if (currentCanvasWidth > 0) { // Avoid division by zero
						double currentScaleForElements = (double)currentCanvasWidth / CANVAS_INITIAL_WIDTH;

						// ������㲢���� original_x/y/width/height
						element->original_x = (int)round(element->x / currentScaleForElements);
						element->original_y = (int)round(element->y / currentScaleForElements);
						element->original_width = (int)round(element->width / currentScaleForElements);
						element->original_height = (int)round(element->height / currentScaleForElements);
					}

					// ǿ�ƻ����ػ棬��ʾ���Ա仯
					InvalidateRect(g_hCanvasPanel, NULL, TRUE);

					// �������ɴ���
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

			// --- ��Ҫ�������µĻ����ߴ������������Ԫ�صĳߴ��λ�� ---
			// ���㵱ǰ�����������ԭʼ�ߴ�����ű���
			if (CANVAS_INITIAL_WIDTH > 0) { // ���������
				double currentScale = (double)finalCanvasWidth / CANVAS_INITIAL_WIDTH;

				for (int i = 0; i < g_iOSUIElementCount; i++) {
					iOSUIElement *element = &g_iOSUIElements[i];
					// ʹ��Ԫ�ص� original_x/y/width/height ���������ڵ�ǰ�����µ�ʵ����ʾ�ߴ�
					element->x = (int)round(element->original_x * currentScale);
					element->y = (int)round(element->original_y * currentScale);
					element->width = (int)round(element->original_width * currentScale);
					element->height = (int)round(element->original_height * currentScale);

					// ȷ��Ԫ�ز�����С��������
					if (element->width < 1) element->width = 1;
					if (element->height < 1) element->height = 1;
				}
				// ǿ�ƻ����ػ�����Ԫ�أ���������ʾ
				InvalidateRect(g_hCanvasPanel, NULL, TRUE);
				// ������������Է�ӳ�µ����꣨��Ȼ���Զ����ŵģ���ҲҪ��ʾ��ǰֵ��
				UpdatePropertiesPanel();
				// �������ɴ�����ȷ�������е����������µģ���Ȼ�����ǻ���original_x����Ϊ��ͳһ��ʾ��
				RegenerateAllObjCCode();
			}

			// Note: The child controls within hCodeOutputPanel are handled by CodePropertiesPanelProc's WM_SIZE
			break;
		}
		case WM_DESTROY: {
			if (bgbrush != NULL) {
				DeleteObject(bgbrush);
				bgbrush = NULL; // ��ֹ�ظ�ɾ��
			}
			PostQuitMessage(0);
			break;
		}

		default:
			return DefWindowProc(hwnd, msg, wParam, lParam);

	}
	return 0;
}

// �������Ĵ��ڹ��̺��� (������Ʒ��õ� UI Ԫ�غʹ�����ѡ����ק������)
LRESULT CALLBACK CanvasPanelProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	static int mouseX, mouseY; // ��ǰ���λ��
	static BOOL isDragging = FALSE; // �Ƿ�������קԪ��
	static int dragOffset_X, dragOffset_Y; // ��קʱ���������Ԫ�����Ͻǵ�ƫ��
	static int resizeHandle = -1; // -1: ��, 0: ����, 1: ����, 2: ����, 3: ����
	const int GRAB_AREA = 5; // ��Ե���������С

	WCHAR debugMsg[256]; // ���� MessageBox �ĵ�����Ϣ

	switch (msg) {
		case WM_PAINT: {
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hwnd, &ps);

			// ���������������ѷ��õ� iOS UI Ԫ��
			for (int i = 0; i < g_iOSUIElementCount; i++) {
				iOSUIElement *element = &g_iOSUIElements[i];
				// ����ʹ�� element->x/y/width/height����Щֵ�Ѿ��� WM_SIZE �б�����Ϊ��ǰ���ź��ֵ
				RECT elementRect = {element->x, element->y, element->x + element->width, element->y + element->height};

				// ���Ʊ߿�����
				HPEN hPen;
				HBRUSH hBrush;

				// ��������ѡ����ɫ������
				if (wcscmp(element->type, L"Button") == 0) {
					hPen = CreatePen(PS_SOLID, 1, RGB(element->bgColorR, element->bgColorG, element->bgColorB)); // ʹ�ñ���ɫ��Ϊ�߿�ɫ
					hBrush = CreateSolidBrush(RGB(element->bgColorR, element->bgColorG, element->bgColorB)); // ʹ�ñ���ɫ���
				} else if (wcscmp(element->type, L"TextField") == 0) {
					// TextField �߿�ͱ���ɫͨ���ǹ̶��Ļ���ϵͳ���ƣ�����ʹ��Ĭ��ֵ
					hPen = CreatePen(PS_SOLID, 1, RGB(0, 100, 0)); // ��ɫ�߿�
					hBrush = CreateSolidBrush(RGB(200, 255, 200)); // ǳ��ɫ���
				} else if (wcscmp(element->type, L"TableView") == 0) {
					hPen = CreatePen(PS_SOLID, 1, RGB(100, 0, 100)); // ��ɫ�߿�
					hBrush = CreateSolidBrush(RGB(255, 200, 255)); // ǳ��ɫ���
				} else {
					hPen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0)); // Ĭ�Ϻ�ɫ�߿�
					hBrush = (HBRUSH)GetStockObject(WHITE_BRUSH); // Ĭ�ϰ�ɫ���
				}

				HPEN hOldPen = SelectObject(hdc, hPen);
				HBRUSH hOldBrush = SelectObject(hdc, hBrush);

				Rectangle(hdc, elementRect.left, elementRect.top, elementRect.right, elementRect.bottom);

				SelectObject(hdc, hOldPen);
				SelectObject(hdc, hOldBrush);
				DeleteObject(hPen);
				DeleteObject(hBrush); // �ǵ�ɾ��������ˢ��

				// �����ı�
				SetBkMode(hdc, TRANSPARENT); // ����͸��
				SetTextColor(hdc, RGB(element->textColorR, element->textColorG, element->textColorB)); // �����ı���ɫ

				// ��������
				HFONT hFont = CreateFont(
				                  -(int)element->fontSize, // ��ֵ��ʾ�ַ��߶�
				                  0, 0, 0,
				                  FW_NORMAL, // ����
				                  FALSE, FALSE, FALSE,
				                  DEFAULT_CHARSET,
				                  OUT_DEFAULT_PRECIS,
				                  CLIP_DEFAULT_PRECIS,
				                  DEFAULT_QUALITY,
				                  DEFAULT_PITCH | FF_SWISS,
				                  TEXT("Arial") // ʾ�����壬�����滻Ϊ��������
				              );
				HFONT hOldFont = SelectObject(hdc, hFont);

				DrawTextW(hdc, element->text, -1, &elementRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);

				SelectObject(hdc, hOldFont); // �ָ�������
				DeleteObject(hFont); // ɾ������������
			}

			EndPaint(hwnd, &ps);
			break;
		}
		case WM_LBUTTONDOWN: {
			mouseX = GET_X_LPARAM(lParam);
			mouseY = GET_Y_LPARAM(lParam);
			SetCapture(hwnd); // ������꣬ȷ����ʹ����Ƴ�����Ҳ�ܼ��������¼�

			int oldSelected = g_selectedElementIndex; // ��¼�ɵ�ѡ������
			g_selectedElementIndex = -1; // Ĭ��δѡ��

			// ���ȼ���Ƿ����������ֱ� (�������ѡ��Ԫ��)
			if (oldSelected != -1) {
				iOSUIElement *element = &g_iOSUIElements[oldSelected];
				RECT r = {element->x, element->y, element->x + element->width, element->y + element->height};

				if (mouseX >= r.left - GRAB_AREA && mouseX <= r.left + GRAB_AREA &&
				        mouseY >= r.top - GRAB_AREA && mouseY <= r.top + GRAB_AREA) {
					resizeHandle = 0; // Top-Left
					g_selectedElementIndex = oldSelected; // ����ѡ��
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

			if (resizeHandle == -1) { // ���û�е���������ֱ��������Ƿ�����Ԫ�ػ�հ״�
				// ��������Ԫ�أ�������Ƿ���ĳ��Ԫ�صı߽���
				for (int i = g_iOSUIElementCount - 1; i >= 0; i--) { // �Ӻ���ǰ������ȷ����������ϲ��Ԫ��
					iOSUIElement *element = &g_iOSUIElements[i];
					RECT elementRect = {element->x, element->y, element->x + element->width, element->y + element->height};

					if (PtInRect(&elementRect, (POINT) {
					mouseX, mouseY
				})) {
						g_selectedElementIndex = i; // ѡ�����Ԫ��
						isDragging = TRUE; // ��ʼ��ק
						dragOffset_X = mouseX - element->x;
						dragOffset_Y = mouseY - element->y;
						break; // �ҵ���һ�����˳�
					}
				}
			}

			// ���ѡ��״̬�ı䣨������ѡ�е�δѡ�У�������������岢�ػ滭��
			if (oldSelected != g_selectedElementIndex) {
				UpdatePropertiesPanel();
				InvalidateRect(hwnd, NULL, TRUE); // �ػ滭��������ѡ�и���
			} else if (g_selectedElementIndex != -1 && (isDragging || resizeHandle != -1)) {
				// �����ͬһ��Ԫ�أ�����ʼ����ק�����ţ�Ҳ��Ҫ����������� (�Ա���ʾ���ʵʱ���µ�����)
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

				// ��ȡ������ǰʵ�ʳߴ�
				RECT currentCanvasRect;
				GetClientRect(hwnd, &currentCanvasRect);
				int currentCanvasWidth = currentCanvasRect.right;
				int currentCanvasHeight = currentCanvasRect.bottom;

				// ���㵱ǰ�����������ԭʼ�ߴ�����ű���
				double currentScaleForElements = (currentCanvasWidth > 0) ? ((double)currentCanvasWidth / CANVAS_INITIAL_WIDTH) : 1.0;

				int newX = element->x;
				int newY = element->y;
				int newWidth = element->width;
				int newHeight = element->height;

				if (isDragging) {
					newX = mouseX - dragOffset_X;
					newY = mouseY - dragOffset_Y;

					// ����Ԫ�ز����������߽�
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
							newX = originalElementX; // X ���ֲ���
							newY = mouseY;
							newWidth = mouseX - originalElementX;
							newHeight = originalElementY + originalElementHeight - newY;
							break;
						case 2: // Bottom-Right
							newX = originalElementX; // X ���ֲ���
							newY = originalElementY; // Y ���ֲ���
							newWidth = mouseX - originalElementX;
							newHeight = mouseY - originalElementY;
							break;
						case 3: // Bottom-Left
							newX = mouseX;
							newY = originalElementY; // Y ���ֲ���
							newWidth = originalElementX + originalElementWidth - newX;
							newHeight = mouseY - element->original_y;
							break;
					}

					// ȷ����Ⱥ͸߶�����Ϊ1�����Ҳ����������߽�
					if (newWidth < 1) newWidth = 1;
					if (newHeight < 1) newHeight = 1;

					// ����Ԫ�ز����������߽� (������ź����λ�úʹ�С)
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

				// --- ʵʱ����Ԫ�ص� original_* ���� ---
				// ֻ�е� currentScaleForElements ��Ϊ��ʱ�Ž��м��㣬���������
				if (currentScaleForElements != 0) {
					element->original_x = (int)round(element->x / currentScaleForElements);
					element->original_y = (int)round(element->y / currentScaleForElements);
					element->original_width = (int)round(element->width / currentScaleForElements);
					element->original_height = (int)round(element->height / currentScaleForElements);
				}


				UpdatePropertiesPanel(); // ʵʱ�����������
				InvalidateRect(hwnd, NULL, TRUE); // ʵʱ�ػ�
			} else {
				// δ��ק������ʱ���������λ�øı�����״
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
			ReleaseCapture(); // �ͷ���겶��
			isDragging = FALSE;
			resizeHandle = -1;
			// ��ק�����Ž������������ɴ��룬ȷ���������걻д��
			RegenerateAllObjCCode(); // ȷ����ק/���ź����Ҳ����
			break;
		}
		case WM_SETCURSOR:
			// ���� WM_MOUSEMOVE ��Ϣ���������ù�꣬������˸
			if (LOWORD(lParam) == HTCLIENT) {
				return TRUE;
			}
			return DefWindowProc(hwnd, msg, wParam, lParam);
		default:
			return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	return 0;
}

// ������������ (�������ڲ���ť�����Ȼ���¼����ݸ������ڣ�����ֱ�������ﴦ��Ԫ�صĴ���)
LRESULT CALLBACK toolpanelproc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	WCHAR debugMsg[256];
	switch (msg) {
		case WM_COMMAND: {
			if (g_iOSUIElementCount >= MAX_UI_ELEMENTS) {
				MessageBox(hwnd, TEXT("�����������޷���Ӹ���Ԫ�ء�"), TEXT("��ʾ"), MB_OK | MB_ICONINFORMATION);
				break;
			}

			iOSUIElement newElement;
			ZeroMemory(&newElement, sizeof(newElement));

			newElement.original_x = 50; // ���Ų�
			newElement.original_y = 50;
			newElement.id = IDC_GENERATED_UI_BASE + g_iOSUIElementCount;

			// ���ݵ�ǰ�������ű����������ʼ��ʾ����
			RECT currentCanvasRect;
			GetClientRect(g_hCanvasPanel, &currentCanvasRect);
			int currentCanvasWidth = currentCanvasRect.right;
			double currentScale = (currentCanvasWidth > 0) ? ((double)currentCanvasWidth / CANVAS_INITIAL_WIDTH) : 1.0;

			switch (LOWORD(wParam)) {
				case IDC_TOOL_BUTTON: {
					newElement.hasText = TRUE;
					wcscpy_s(newElement.type, _countof(newElement.type), L"Button");
					wcscpy_s(newElement.text, _countof(newElement.text), L"�����");
					newElement.original_width = 100;
					newElement.original_height = 30;
					// ��ťĬ����ɫ
					newElement.hasBgColor = TRUE;
					newElement.hasTextColor = TRUE;
					newElement.bgColorR = 0;
					newElement.bgColorG = 122;
					newElement.bgColorB = 255; // iOS ��ɫ
					newElement.textColorR = 255;
					newElement.textColorG = 255;
					newElement.textColorB = 255; // ��ɫ
					newElement.fontSize = 17.0f;
					break;
				}
				case IDC_TOOL_TEXTFIELD: {
					newElement.hasText = TRUE;
					newElement.hasBgColor = TRUE;
					newElement.hasTextColor = TRUE;
					wcscpy_s(newElement.type, _countof(newElement.type), L"TextField");
					wcscpy_s(newElement.text, _countof(newElement.text), L"������...") // Placeholder
					newElement.original_width = 200;
					newElement.original_height = 30;
					// �ı���Ĭ����ɫ
					newElement.bgColorR = 255;
					newElement.bgColorG = 255;
					newElement.bgColorB = 255; // ��ɫ���� (��ֱ��ʹ�ã����洢)
					newElement.textColorR = 0;
					newElement.textColorG = 0;
					newElement.textColorB = 0; // ��ɫ
					newElement.fontSize = 17.0f;
					break;
				}
				case IDC_TOOL_TABLEVIEW: {
					newElement.hasText = TRUE;
					newElement.hasBgColor = TRUE;
					newElement.hasTextColor = TRUE;
					wcscpy_s(newElement.type, _countof(newElement.type), L"TableView");
					wcscpy_s(newElement.text, _countof(newElement.text), L"�����ͼ"); // ������ʾ��ʵ�� tableView �޴�����
					newElement.original_width = 300;
					newElement.original_height = 200;
					// TableView Ĭ����ɫ (�����ã���Ϊ��������)
					newElement.bgColorR = 242;
					newElement.bgColorG = 242;
					newElement.bgColorB = 247; // iOS Ĭ�ϱ���ɫ
					newElement.textColorR = 0;
					newElement.textColorG = 0;
					newElement.textColorB = 0; // ��ɫ
					newElement.fontSize = 17.0f;
					break;
				}
				default:
					return DefWindowProc(hwnd, msg, wParam, lParam); // �����������Ĭ�ϴ���
			}

			// ����Ԫ���ڵ�ǰ�����ϵ�ʵ����ʾλ�úʹ�С
			newElement.x = (int)round(newElement.original_x * currentScale);
			newElement.y = (int)round(newElement.original_y * currentScale);
			newElement.width = (int)round(newElement.original_width * currentScale);
			newElement.height = (int)round(newElement.original_height * currentScale);


			// �����Ԫ�ص�ȫ������
			g_iOSUIElements[g_iOSUIElementCount] = newElement;
			g_iOSUIElementCount++;

			// ѡ������ӵ�Ԫ��
			g_selectedElementIndex = g_iOSUIElementCount - 1;

			// ǿ�ƻ����ػ棬��ʾ����ӵ�Ԫ�غ�ѡ��״̬
			InvalidateRect(g_hCanvasPanel, NULL, TRUE); // TRUE ��ʾ��������

			// �����������ʹ���
			UpdatePropertiesPanel();
			RegenerateAllObjCCode();
			break;
		}
		default:
			return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	return 0;
}

// ����������/�������Ĵ��ڹ��̺���
LRESULT CALLBACK CodePropertiesPanelProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	HINSTANCE hInstance = (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE);

	// ���岼�ֳ���
	const int labelWidth = 80;
	const int editWidth = 80;
	const int colorEditWidth = 40; // For R, G, B components
	const int longEditWidth = 265; // For text/placeholder
	const int controlHeight = 30;
	const int rowSpacing = 35; // Vertical spacing between rows
	const int colSpacing = 20; // Horizontal spacing between columns
	const int panelPadding = 10; // Padding from panel edges

	// ����������ʼ Y ���� (�ڴ���༭���·�)
	int propPanelStartY = 30 + 400 + 25; // ���������м��㷽ʽ����һ��

	switch (msg) {
		case WM_CREATE: {
			// �����ﴴ�������������ڵ������ӿؼ�
			// �ڴ�������������ӱ���

			CreateWindowEx(0, TEXT("STATIC"), TEXT("���ɵ� Objective-C ����"), WS_CHILD | WS_VISIBLE | SS_CENTER,
			               0, 5, 550, 20, hwnd, NULL, hInstance, NULL);

			// �ڴ����������д���һ�����б༭������ʾ����
			g_hCodeEdit = CreateWindowEx( // �洢��ȫ�ֱ���
			                  0,                                  // ��չ��ʽ
			                  TEXT("EDIT"),                       // �ؼ�����
			                  TEXT("// ���뽫���������ɣ���������� csm ��������...\r\n// ���磺[UIViewController createButtonWithFrame:...]\r\n\r\n"), // ��ʼ�ı�
			                  WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL | ES_MULTILINE | ES_READONLY | ES_AUTOVSCROLL | ES_AUTOHSCROLL,
			                  panelPadding, 30,                             // X, Y ���� (����� hCodeOutputPanel)
			                  550 - (2 * panelPadding), 400,           // ���, �߶�
			                  hwnd,                               // �����ھ�� (������ CodePropertiesPanelProc ����� hwnd)
			                  (HMENU)IDC_CODE_DISPLAY_EDIT,       // �ؼ�ID
			                  hInstance,                          // Ӧ�ó���ʵ�����
			                  NULL                                // ��������
			              );

			// --- ����������� ---
			// ����������
			CreateWindowEx(0, TEXT("STATIC"), TEXT("����"), WS_CHILD | WS_VISIBLE | SS_CENTER,
			               0, propPanelStartY - 20, 550, 20, hwnd, NULL, hInstance, NULL);

			int currentY = propPanelStartY; // ���ڸ��ٵ�ǰ�е�Y����

			// ���� (��̬�ı�)
			CreateWindowEx(0, TEXT("STATIC"), TEXT("����:"), WS_CHILD | WS_VISIBLE,
			               panelPadding, currentY, labelWidth, controlHeight, hwnd, (HMENU)IDC_PROP_TYPE_LABEL, hInstance, NULL);
			g_hPropTypeStatic = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("STATIC"), TEXT("��ѡ��"), WS_CHILD | WS_VISIBLE | SS_CENTER,
			                                   panelPadding + labelWidth + colSpacing, currentY, longEditWidth, controlHeight, hwnd, (HMENU)IDC_PROP_TYPE_STATIC, hInstance, NULL);
			currentY += rowSpacing;

			// X ����
			CreateWindowEx(0, TEXT("STATIC"), TEXT("X:"), WS_CHILD | WS_VISIBLE,
			               panelPadding, currentY, labelWidth, controlHeight, hwnd, (HMENU)IDC_PROP_X_LABEL, hInstance, NULL);
			g_hPropXEdit = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("EDIT"), TEXT(""), WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER | ES_AUTOHSCROLL,
			                              panelPadding + labelWidth + colSpacing, currentY, editWidth, controlHeight, hwnd, (HMENU)IDC_PROP_X_EDIT, hInstance, NULL);

			// Y ����
			CreateWindowEx(0, TEXT("STATIC"), TEXT("Y:"), WS_CHILD | WS_VISIBLE,
			               panelPadding + labelWidth + colSpacing + editWidth + colSpacing, currentY, labelWidth, controlHeight, hwnd, (HMENU)IDC_PROP_Y_LABEL, hInstance, NULL);
			g_hPropYEdit = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("EDIT"), TEXT(""), WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER | ES_AUTOHSCROLL,
			                              panelPadding + labelWidth + colSpacing + editWidth + colSpacing + labelWidth + colSpacing, currentY, editWidth, controlHeight, hwnd, (HMENU)IDC_PROP_Y_EDIT, hInstance, NULL);
			currentY += rowSpacing;

			// ���
			CreateWindowEx(0, TEXT("STATIC"), TEXT("���:"), WS_CHILD | WS_VISIBLE,
			               panelPadding, currentY, labelWidth, controlHeight, hwnd, (HMENU)IDC_PROP_WIDTH_LABEL, hInstance, NULL);
			g_hPropWidthEdit = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("EDIT"), TEXT(""), WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER | ES_AUTOHSCROLL,
			                                  panelPadding + labelWidth + colSpacing, currentY, editWidth, controlHeight, hwnd, (HMENU)IDC_PROP_WIDTH_EDIT, hInstance, NULL);

			// �߶�
			CreateWindowEx(0, TEXT("STATIC"), TEXT("�߶�:"), WS_CHILD | WS_VISIBLE,
			               panelPadding + labelWidth + colSpacing + editWidth + colSpacing, currentY, labelWidth, controlHeight, hwnd, (HMENU)IDC_PROP_HEIGHT_LABEL, hInstance, NULL);
			g_hPropHeightEdit = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("EDIT"), TEXT(""), WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER | ES_AUTOHSCROLL,
			                                   panelPadding + labelWidth + colSpacing + editWidth + colSpacing + labelWidth + colSpacing, currentY, editWidth, controlHeight, hwnd, (HMENU)IDC_PROP_HEIGHT_EDIT, hInstance, NULL);
			currentY += rowSpacing;

			// �ı�/ռλ��
			CreateWindowEx(0, TEXT("STATIC"), TEXT("�ı�:"), WS_CHILD | WS_VISIBLE,
			               panelPadding, currentY, labelWidth, controlHeight, hwnd, (HMENU)IDC_PROP_TEXT_LABEL, hInstance, NULL);
			g_hPropTextEdit = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("EDIT"), TEXT(""), WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
			                                 panelPadding + labelWidth + colSpacing, currentY, longEditWidth, controlHeight, hwnd, (HMENU)IDC_PROP_TEXT_EDIT, hInstance, NULL);
			currentY += rowSpacing;

			// ������ɫ (R, G, B)
			CreateWindowEx(0, TEXT("STATIC"), TEXT("����ɫ:"), WS_CHILD | WS_VISIBLE,
			               panelPadding, currentY, labelWidth, controlHeight, hwnd, (HMENU)IDC_PROP_BG_R_LABEL, hInstance, NULL);
			g_hPropBgR_Edit = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("EDIT"), TEXT(""), WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER | ES_AUTOHSCROLL,
			                                 panelPadding + labelWidth + colSpacing, currentY, colorEditWidth, controlHeight, hwnd, (HMENU)IDC_PROP_BG_R_EDIT, hInstance, NULL);
			g_hPropBgG_Edit = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("EDIT"), TEXT(""), WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER | ES_AUTOHSCROLL,
			                                 panelPadding + labelWidth + colSpacing + colorEditWidth + colSpacing, currentY, colorEditWidth, controlHeight, hwnd, (HMENU)IDC_PROP_BG_G_EDIT, hInstance, NULL);
			g_hPropBgB_Edit = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("EDIT"), TEXT(""), WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER | ES_AUTOHSCROLL,
			                                 panelPadding + labelWidth + colSpacing + (colorEditWidth + colSpacing) * 2, currentY, colorEditWidth, controlHeight, hwnd, (HMENU)IDC_PROP_BG_B_EDIT, hInstance, NULL);
			currentY += rowSpacing;

			// �ı���ɫ (R, G, B)
			CreateWindowEx(0, TEXT("STATIC"), TEXT("�ı�ɫ:"), WS_CHILD | WS_VISIBLE,
			               panelPadding, currentY, labelWidth, controlHeight, hwnd, (HMENU)IDC_PROP_TEXT_R_LABEL, hInstance, NULL);
			g_hPropTextR_Edit = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("EDIT"), TEXT(""), WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER | ES_AUTOHSCROLL,
			                                   panelPadding + labelWidth + colSpacing, currentY, colorEditWidth, controlHeight, hwnd, (HMENU)IDC_PROP_TEXT_R_EDIT, hInstance, NULL);
			g_hPropTextG_Edit = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("EDIT"), TEXT(""), WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER | ES_AUTOHSCROLL,
			                                   panelPadding + labelWidth + colSpacing + colorEditWidth + colSpacing, currentY, colorEditWidth, controlHeight, hwnd, (HMENU)IDC_PROP_TEXT_G_EDIT, hInstance, NULL);
			g_hPropTextB_Edit = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("EDIT"), TEXT(""), WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER | ES_AUTOHSCROLL,
			                                   panelPadding + labelWidth + colSpacing + (colorEditWidth + colSpacing) * 2, currentY, colorEditWidth, controlHeight, hwnd, (HMENU)IDC_PROP_TEXT_B_EDIT, hInstance, NULL);
			currentY += rowSpacing;

			// �����С
			CreateWindowEx(0, TEXT("STATIC"), TEXT("�����С:"), WS_CHILD | WS_VISIBLE,
			               panelPadding, currentY, labelWidth, controlHeight, hwnd, (HMENU)IDC_PROP_FONT_SIZE_LABEL, hInstance, NULL);
			g_hPropFontSize_Edit = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("EDIT"), TEXT(""), WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER | ES_AUTOHSCROLL,
			                                      panelPadding + labelWidth + colSpacing, currentY, editWidth, controlHeight, hwnd, (HMENU)IDC_PROP_FONT_SIZE_EDIT, hInstance, NULL);
			currentY += rowSpacing;


			// ȷ����ť
			g_hPropConfirmButton = CreateWindowEx(
			                           0,
			                           TEXT("BUTTON"),
			                           TEXT("ȷ��"),
			                           WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
			                           panelPadding + labelWidth + colSpacing, currentY + 10, // �������ı��༭���·�������ӵ���
			                           80, 30, // ��ť��С
			                           hwnd, // �����ھ�� (������ CodePropertiesPanelProc ����� hwnd)
			                           (HMENU)IDC_PROP_CONFIRM_BUTTON,
			                           hInstance,
			                           NULL
			                       );
			break;
		}
		case WM_SIZE: {
			// �� CodePropertiesPanelProc �����С�ı�ʱ���������ڲ��ӿؼ���λ�úʹ�С
			RECT rc;
			GetClientRect(hwnd, &rc);
			int panelWidth = rc.right - rc.left;
			// int panelHeight = rc.bottom - rc.top; // Not used for this fixed layout

			// ��������༭��
			MoveWindow(g_hCodeEdit, panelPadding, 30, panelWidth - (2 * panelPadding), 400, TRUE);

			// ���Կؼ���λ���ǹ̶��ģ�����Ҫ�� WM_SIZE �е�������Ϊ����������ڸ����ڵĹ̶�ƫ��
			// �����Ҫ�������Ҳ��Ӧʽ���֣�����Ҫ���������¼����������Կؼ�������
			break;
		}
		case WM_COMMAND: {
			// ���ӿؼ��� WM_COMMAND ��Ϣת���������� (������)
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
// --- ��������ʵ�� ---

// �� RGB ��ɫֵת��Ϊ Objective-C �� UIColor �����ַ��� (char* �汾������ Objective-C ����������)
void RgbToUIColorCode(int r, int g, int b, char* buffer, size_t bufferSize) {
	// ȷ����ɫֵ�� 0-255 ��Χ��
	r = (r < 0) ? 0 : ((r > 255) ? 255 : r);
	g = (g < 0) ? 0 : ((g > 255) ? 255 : g);
	b = (b < 0) ? 0 : ((b > 255) ? 255 : b);

	sprintf_s(buffer, bufferSize, "[UIColor colorWithRed:%.3f green:%.3f blue:%.3f alpha:1.0]",
	          (float)r / 255.0f, (float)g / 255.0f, (float)b / 255.0f);
}

// ���� Objective-C ��ť�������� (����� WCHAR ������)
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


	// Step 1: �� WCHAR �ı�ת��Ϊ UTF-8 ����� char*
	char text_utf8[512]; // �㹻��Ļ��������洢 UTF-8 ���������
	int utf8_len = WideCharToMultiByte(CP_UTF8, 0, element->text, -1, NULL, 0, NULL, NULL);
	if (utf8_len > 0 && utf8_len <= sizeof(text_utf8)) {
		WideCharToMultiByte(CP_UTF8, 0, element->text, -1, text_utf8, utf8_len, NULL, NULL);
	} else {
		text_utf8[0] = '\0'; // ת��ʧ�ܻ򻺳������㣬����ַ���
	}

	// Step 2: �� UTF-8 ����� char* ת���� WCHAR (���� swprintf_s �� %ls)
	WCHAR text_wc[512]; // �㹻��Ļ��������洢ת����� WCHAR
	int wc_text_len = MultiByteToWideChar(CP_UTF8, 0, text_utf8, -1, NULL, 0);
	if (wc_text_len > 0 && wc_text_len <= _countof(text_wc)) {
		MultiByteToWideChar(CP_UTF8, 0, text_utf8, -1, text_wc, wc_text_len);
	} else {
		text_wc[0] = L'\0'; // ת��ʧ�ܻ򻺳������㣬����ַ���
	}

	// ʹ�� swprintf_s д�� WCHAR ������������ʹ�� %ls (СдL��СдS)
	// ��Ϊ text_wc, bgColorCode_wc, textColorCode_wc ���� wchar_t* ����
	swprintf_s(codeBuffer, bufferSize,
	           L"    UIButton *myButton%d = [UIViewController createButtonWithFrame:CGRectMake(%d, %d, %d, %d)\r\n"
	           L"                                                            title:@\"%ls\"\r\n" // ��ť�ı��⣬ʹ�� %ls
	           L"                                                          bgColor:%ls\r\n" // ����ɫ������ʹ�� %ls
	           L"                                                        textColor:%ls\r\n" // �ı���ɫ������ʹ�� %ls
	           L"                                                             font:[UIFont systemFontOfSize:%.1f] // �����С\r\n"
	           L"                                                      borderWidth:0.0 // ʾ���߿���\r\n"
	           L"                                                      borderColor:nil // ʾ���߿���ɫ\r\n"
	           L"                                                           target:self\r\n"
	           L"                                                         selector:@selector(buttonTapped:)]; // ʾ��selector\r\n"
	           L"    [self.view addSubview:myButton%d]; // �൱�� Win32 �� WS_CHILD ��ʽ�͸����ھ��\r\n\r\n",
	           element->id, element->original_x, element->original_y, element->original_width, element->original_height,
	           text_wc, bgColorCode_wc, textColorCode_wc, element->fontSize, element->id
	          );
}

// ���� Objective-C �ı��򴴽����� (����� WCHAR ������)
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

	// Step 1: �� WCHAR �ı�ת��Ϊ UTF-8 ����� char*
	char text_utf8[512]; // �㹻��Ļ��������洢 UTF-8 ���������
	int utf8_len = WideCharToMultiByte(CP_UTF8, 0, element->text, -1, NULL, 0, NULL, NULL);
	if (utf8_len > 0 && utf8_len <= sizeof(text_utf8)) {
		WideCharToMultiByte(CP_UTF8, 0, element->text, -1, text_utf8, utf8_len, NULL, NULL);
	} else {
		text_utf8[0] = L'\0'; // ת��ʧ�ܻ򻺳������㣬����ַ���
	}

	// Step 2: �� UTF-8 ����� char* ת���� WCHAR (���� swprintf_s �� %ls)
	WCHAR text_wc[512]; // �㹻��Ļ��������洢ת����� WCHAR
	int wc_len = MultiByteToWideChar(CP_UTF8, 0, text_utf8, -1, NULL, 0);
	if (wc_len > 0 && wc_len <= _countof(text_wc)) {
		MultiByteToWideChar(CP_UTF8, 0, text_utf8, -1, text_wc, wc_len);
	} else {
		text_wc[0] = L'\0'; // ת��ʧ�ܻ򻺳������㣬����ַ���
	}

	// **�޸����ʹ�� text �����������ı��ֶεĳ�ʼ���ݣ���ʹ�� %ls**
	swprintf_s(codeBuffer, bufferSize,
	           L"    UITextField *myTextField%d = [UIViewController createTextFieldWithFrame:CGRectMake(%d, %d, %d, %d)\r\n"
	           L"                                                                   text:@\"%ls\"\r\n" // ʹ�� text �������ó�ʼ�ı���ʹ�� %ls
	           L"                                                            borderWidth:1.0 // ʾ��\r\n"
	           L"                                                            borderColor:[UIColor lightGrayColor].CGColor // ʾ��\r\n"
	           L"                                                        clearButtonMode:UITextFieldViewModeWhileEditing // ʾ��\r\n"
	           L"                                                          returnKeyType:UIReturnKeyDone // ʾ��\r\n"
	           L"                                                                   font:[UIFont systemFontOfSize:%.1f] // �����С\r\n"
	           L"                                                     keyboardAppearance:UIKeyboardAppearanceDefault]; // ʾ��\r\n"
	           L"    // myTextField%d.delegate = self; // �����Ҫ����ί��\r\n"
	           L"    [self.view addSubview:myTextField%d];\r\n\r\n",
	           element->id, element->original_x, element->original_y, element->original_width, element->original_height,
	           text_wc, element->fontSize, element->id, element->id
	          );
}

// ���� Objective-C �����ͼ�������� (����� WCHAR ������)
void GenerateObjCTableViewCode(WCHAR* codeBuffer, size_t bufferSize, iOSUIElement * element) {
	// **ע�⣺����ʹ�� original_x/y/width/height �����ɴ��룬ȷ�������ǻ���ԭʼ������**
	swprintf_s(codeBuffer, bufferSize,
	           L"    UITableView *myTableView%d = [UIViewController createTableViewWithFrame:CGRectMake(%d, %d, %d, %d)\r\n"
	           L"                                                                    style:UITableViewStylePlain // ʾ��\r\n"
	           L"                                                                rowHeight:44.0 // ʾ��\r\n"
	           L"                                                              borderWidth:1.0 // ʾ��\r\n"
	           L"                                                              borderColor:[UIColor lightGrayColor].CGColor // ʾ��\r\n"
	           L"                                                                cellIdent:@\"MyCellIdentifier%d\" // ʾ��\r\n"
	           L"                                                               dataSource:self // ʾ��\r\n"
	           L"                                                                 delegate:self]; // ʾ��\r\n"
	           L"    [self.view addSubview:myTableView%d];\r\n\r\n",
	           element->id, element->original_x, element->original_y, element->original_width, element->original_height,
	           element->id, // cellIdent ������ʹ��IDȷ��Ψһ��
	           element->id
	          );
}

// �������������ʾ
void UpdatePropertiesPanel(void) {
	WCHAR buffer[256];
	if (g_selectedElementIndex != -1) {
		iOSUIElement *element = &g_iOSUIElements[g_selectedElementIndex];

		// �������ͱ�ǩ
		SetWindowTextW(g_hPropTypeStatic, element->type);

		// ���� X, Y, Width, Height (��ʾ��ǰʵ����Ⱦ�ĳߴ磬���� original_x)
		swprintf_s(buffer, _countof(buffer), L"%d", element->x);
		SetWindowTextW(g_hPropXEdit, buffer);
		swprintf_s(buffer, _countof(buffer), L"%d", element->y);
		SetWindowTextW(g_hPropYEdit, buffer);
		swprintf_s(buffer, _countof(buffer), L"%d", element->width);
		SetWindowTextW(g_hPropWidthEdit, buffer);
		swprintf_s(buffer, _countof(buffer), L"%d", element->height);
		SetWindowTextW(g_hPropHeightEdit, buffer);

		// ���ñ�����ɫ
		swprintf_s(buffer, _countof(buffer), L"%d", element->bgColorR);
		SetWindowTextW(g_hPropBgR_Edit, buffer);
		swprintf_s(buffer, _countof(buffer), L"%d", element->bgColorG);
		SetWindowTextW(g_hPropBgG_Edit, buffer);
		swprintf_s(buffer, _countof(buffer), L"%d", element->bgColorB);
		SetWindowTextW(g_hPropBgB_Edit, buffer);

		// �����ı���ɫ
		swprintf_s(buffer, _countof(buffer), L"%d", element->textColorR);
		SetWindowTextW(g_hPropTextR_Edit, buffer);
		swprintf_s(buffer, _countof(buffer), L"%d", element->textColorG);
		SetWindowTextW(g_hPropTextG_Edit, buffer);
		swprintf_s(buffer, _countof(buffer), L"%d", element->textColorB);
		SetWindowTextW(g_hPropTextB_Edit, buffer);

		// ���������С
		swprintf_s(buffer, _countof(buffer), L"%.1f", element->fontSize);
		SetWindowTextW(g_hPropFontSize_Edit, buffer);


		// ������������ͳߴ�༭��
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


		// ����Ԫ������������ʾ�ı�/ռλ���༭��������ɫ����
		if (wcscmp(element->type, L"Button") == 0) {
			SetWindowTextW(GetDlgItem(hCodeOutputPanel, IDC_PROP_TEXT_LABEL), L"����:"); // ʹ��GetDlgItem��ȡ��ǩ���
			SetWindowTextW(g_hPropTextEdit, element->text);
			EnableWindow(g_hPropTextEdit, TRUE);
			ShowWindow(GetDlgItem(hCodeOutputPanel, IDC_PROP_TEXT_LABEL), SW_SHOW);
			ShowWindow(g_hPropTextEdit, SW_SHOW);

			// ��ť�ı���ɫ���ı�ɫ���ɱ༭
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
			SetWindowTextW(GetDlgItem(hCodeOutputPanel, IDC_PROP_TEXT_LABEL), L"�ı�����:"); // ��ǩ��Ϊ���ı����ݡ�
			SetWindowTextW(g_hPropTextEdit, element->text);
			EnableWindow(g_hPropTextEdit, TRUE);
			ShowWindow(GetDlgItem(hCodeOutputPanel, IDC_PROP_TEXT_LABEL), SW_SHOW);
			ShowWindow(g_hPropTextEdit, SW_SHOW);

			// �ı���ͨ��ֻ���ı���ɫ�ɱ༭������ɫһ����͸����̶���ɫ
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

		} else { // TableView ������û���ı����Ե�����
			SetWindowTextW(GetDlgItem(hCodeOutputPanel, IDC_PROP_TEXT_LABEL), L""); // ��ձ�ǩ�ı�
			SetWindowTextW(g_hPropTextEdit, TEXT("")); // ��ձ༭��
			EnableWindow(g_hPropTextEdit, FALSE); // ����
			ShowWindow(GetDlgItem(hCodeOutputPanel, IDC_PROP_TEXT_LABEL), SW_HIDE); // ���ر�ǩ
			ShowWindow(g_hPropTextEdit, SW_HIDE); // ���ر༭��

			// ���ز�����������ɫ�������С����
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
		// ѡ��Ԫ��ʱ����ȷ����ť
		EnableWindow(g_hPropConfirmButton, TRUE);

	} else {
		// δѡ���κ�Ԫ�أ���ղ����ñ༭��
		SetWindowTextW(g_hPropTypeStatic, TEXT("��ѡ��"));
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


		// ȷ���ı���ǩ�ͱ༭����δѡ��ʱҲ��ʾĬ��״̬����������
		SetWindowTextW(GetDlgItem(hCodeOutputPanel, IDC_PROP_TEXT_LABEL), L"�ı�:"); // �ָ�Ĭ�ϱ�ǩ
		ShowWindow(GetDlgItem(hCodeOutputPanel, IDC_PROP_TEXT_LABEL), SW_SHOW);
		ShowWindow(g_hPropTextEdit, SW_SHOW);

		// ����������ɫ�������С����
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


		// δѡ��Ԫ��ʱ����ȷ����ť
		EnableWindow(g_hPropConfirmButton, FALSE);
	}
}

// ������������ Objective-C ����
void RegenerateAllObjCCode(void) {
	WCHAR generatedCodeBuffer[204800]; // ����Ļ���������Ӧ�������, ����Ϊ WCHAR
	ZeroMemory(generatedCodeBuffer, sizeof(generatedCodeBuffer)); // ����

	wcscat_s(generatedCodeBuffer, _countof(generatedCodeBuffer), L"// �������ͨ�� UI �������ͷ�ļ� (csm.h)\r\n");
	wcscat_s(generatedCodeBuffer, _countof(generatedCodeBuffer), L"#import \"csm.h\" \r\n\r\n");
	wcscat_s(generatedCodeBuffer, _countof(generatedCodeBuffer), L"// ����� UIViewController �� viewDidLoad �����������У�\r\n");
	wcscat_s(generatedCodeBuffer, _countof(generatedCodeBuffer), L"- (void)setupGeneratedUI {\r\n");
	wcscat_s(generatedCodeBuffer, _countof(generatedCodeBuffer), L"    // ���� 'screen' �� 'self.view' �ڵ�ǰ�����Ŀ���\r\n");
	wcscat_s(generatedCodeBuffer, _countof(generatedCodeBuffer), L"    // CGRect screen = [[UIScreen mainScreen] bounds];\\r\\n\\r\\n");

	for (int i = 0; i < g_iOSUIElementCount; i++) {
		WCHAR elementCode[2048]; // ÿ��Ԫ�صĴ��뻺����, ����Ϊ WCHAR
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

	SetWindowTextW(g_hCodeEdit, generatedCodeBuffer); // ���� EDIT �ؼ����ı� (ʹ�� WCHAR �汾)
}*/
