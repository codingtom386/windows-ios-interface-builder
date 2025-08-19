#define UNICODE     // Force Unicode for Win32 API
#define _UNICODE    // Force Unicode for C Runtime Library

#include "IOSBUILDER.h"

HBRUSH codeeditback;
HWND ctitle = NULL;
// --- Global variables for tool panel scrolling ---
static HWND g_hToolButtons[100];
static int g_numToolButtons = 0;

// Store the initial X and Y positions of the buttons relative to the tool panel's client area
static int g_toolButtonOriginalX[100]; // New: Store original X position
static int g_toolButtonOriginalY[100];

static int toolPanelScrollPos = 0;
static int toolPanelMaxScroll = 0;



int currentButtonY = 50;

void DeleteSelectedElement();
void CopySelectedElement(HWND hwnd);

//HBRUSH butbackbr = NULL;

int g_scrollPos = 0;
FILE* mapping = NULL;
HMODULE g_hBuilderCoreDll = NULL;
void inittool() {
	char type[100];
	char draw[75];
	char gen[75];
	char text[30];
	int count = 0;
	mapping = fopen("dllmap.map", "r");
	const char* form = " %100[^,], %d, %75[^,], %75[^,], %d, %d, %d, %d, %30[^,], %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d";
	while (fscanf(mapping, form, type,
	              &g_elementDefinitions[count].toolButtonId,
	              draw,
	              gen,
	              &g_elementDefinitions[count].Defaults.defaultx,
	              &g_elementDefinitions[count].Defaults.defaulty,
	              &g_elementDefinitions[count].Defaults.defaultWidth,
	              &g_elementDefinitions[count].Defaults.defaultHeight,
	              text,
	              &g_elementDefinitions[count].Defaults.defaultbr,
	              &g_elementDefinitions[count].Defaults.defaultbg,
	              &g_elementDefinitions[count].Defaults.defaultbb,
	              &g_elementDefinitions[count].Defaults.defaulttr,
	              &g_elementDefinitions[count].Defaults.defaulttg,
	              &g_elementDefinitions[count].Defaults.defaulttb,
	              &g_elementDefinitions[count].Defaults.defaultfontsize,
	              &g_elementDefinitions[count].Defaults.defaultminval,
	              &g_elementDefinitions[count].Defaults.defaultmaxval,
	              &g_elementDefinitions[count].Defaults.defaultcurrval,
	              &g_elementDefinitions[count].Defaults.defaultrowheight,
	              &g_elementDefinitions[count].hasText,
	              &g_elementDefinitions[count].hasBgColor,
	              &g_elementDefinitions[count].hasTextColor,
	              &g_elementDefinitions[count].hasFontSize,
	              &g_elementDefinitions[count].hasMinValue,
	              &g_elementDefinitions[count].hasMaxValue,
	              &g_elementDefinitions[count].hasCurrentValue,
	              &g_elementDefinitions[count].hasRowHeight,
	              &g_elementDefinitions[count].hasAlignment) == 29) {
		MultiByteToWideChar(CP_UTF8, 0, type, -1, g_elementDefinitions[count].typeName, _countof(g_elementDefinitions[count].typeName));
		g_elementDefinitions[count].drawFunc = (void *)GetProcAddress(g_hBuilderCoreDll, draw);
		if (!g_elementDefinitions[count].drawFunc)
			MessageBoxA(NULL, "drawFunc load error", NULL, MB_ICONERROR);
		g_elementDefinitions[count].generateCodeFunc = (void *)GetProcAddress(g_hBuilderCoreDll, gen);
		if (!g_elementDefinitions[count].generateCodeFunc)
			MessageBoxA(NULL, "genFunc load error", NULL, MB_ICONERROR);

		g_elementDefinitions[count].initDefaultDataFunc = InitDefaultData;
		MultiByteToWideChar(CP_UTF8, 0, text, -1, g_elementDefinitions[count].Defaults.defaultext, _countof(g_elementDefinitions[count].Defaults.defaultext));

		count++;


	}
	g_numElementDefinitions = count;
}
void SaveFile(HWND hwnd) {
	OPENFILENAME ofn;
	wchar_t szFile[MAX_PATH] = L"";

	ZeroMemory(&ofn, sizeof(ofn));

	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hwnd;
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = L"objective-c文件(*.m)\0*.m\0所有文件 (*.*)\0*.*\0";
	ofn.nFilterIndex = 1;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;
	ofn.lpstrDefExt = L"m";

	if (GetSaveFileName(&ofn) == TRUE) {
		HANDLE hFile = CreateFile(
		                   ofn.lpstrFile,
		                   GENERIC_WRITE,
		                   0,
		                   NULL,
		                   CREATE_ALWAYS,
		                   FILE_ATTRIBUTE_NORMAL,
		                   NULL
		               );

		if (hFile != INVALID_HANDLE_VALUE) {
			// 写入 UTF-8 BOM
			unsigned char bom[] = {0xEF, 0xBB, 0xBF};
			DWORD dwBytesWritten = 0;
			WriteFile(hFile, bom, sizeof(bom), &dwBytesWritten, NULL);

			// 将 Unicode 字符串转换为 UTF-8
			int nBytes = WideCharToMultiByte(CP_UTF8, 0, generatedCodeBuffer, -1, NULL, 0, NULL, NULL);
			char* pUTF8 = (char*)malloc(nBytes);
			if (pUTF8) {
				WideCharToMultiByte(CP_UTF8, 0, generatedCodeBuffer, -1, pUTF8, nBytes, NULL, NULL);
				WriteFile(hFile, pUTF8, nBytes - 1, &dwBytesWritten, NULL); // nBytes-1 是为了不写入终止符
				free(pUTF8);
			}

			MessageBox(hwnd, L"文件已成功保存！", L"保存", MB_OK | MB_ICONINFORMATION);
			CloseHandle(hFile);
		} else {
			MessageBox(hwnd, L"无法创建文件句柄。", L"错误", MB_OK | MB_ICONERROR);
		}
	}
}
HMENU CreateMainMenu(void) {
	HMENU hMenu = CreateMenu();
	HMENU hFilePopup = CreatePopupMenu();
	HMENU hHelpPopup = CreatePopupMenu();
	HMENU hEditPopup = CreatePopupMenu();

	// 创建“文件”菜单
	AppendMenu(hFilePopup, MF_STRING, IDM_FILE_SAVE, L"&保存");

	AppendMenu(hFilePopup, MF_STRING, IDM_FILE_EXIT, L"&退出");

	// 创建“帮助”菜单
	AppendMenu(hHelpPopup, MF_STRING, IDM_HELP_ABOUT, L"&关于...");
	//create edit menu
	AppendMenu(hEditPopup, MF_STRING, IDM_DELETE_ELEMENT, L"&删除");
	AppendMenu(hEditPopup, MF_STRING, IDM_COPY_ELEMENT, L"&复制");
	// 将子菜单附加到主菜单
	AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hFilePopup, L"&文件");
	AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hEditPopup, L"&编辑");
	AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hHelpPopup, L"&帮助");

	return hMenu;
}

// Helper function to move all tool buttons based on the current scroll position
void MoveToolButtons(HWND hwndToolPanel) {
	for (int i = 0; i < g_numToolButtons; i++) {
		if (g_hToolButtons[i] != NULL) {
			// Calculate new Y position based on original Y and current scroll position
			int newY = g_toolButtonOriginalY[i] - toolPanelScrollPos;

			// Use the stored original X position, which is already relative to the tool panel's client area.
			int newX = g_toolButtonOriginalX[i];

			// Get current button size (width and height) - these don't change for these fixed-size buttons.
			const int buttonWidth = 160;
			const int buttonHeight = 40;

			// Move the button to its new calculated Y position, using its original X
			MoveWindow(g_hToolButtons[i], newX, newY,
			           buttonWidth, buttonHeight, TRUE);
		}
	}
}
void movecode(HWND hwnd) {
	int count = 0;
	for (idclist i = IDC_PROP_TYPE; i < IDC_COUNT; i++) {
		count = i - IDC_PROP_TYPE;
		int newY = labels[count].y - g_scrollPos;

		// Use the stored original X position, which is already relative to the tool panel's client area.
		int newX = labels[count].lx;

		// Get current button size (width and height) - these don't change for these fixed-size buttons.
		const int buttonWidth = labelWidth;
		const int buttonHeight = controlHeight;

		// Move the button to its new calculated Y position, using its original X
		MoveWindow(labels[count].labelh, newX, newY,
		           buttonWidth, buttonHeight, TRUE);

	}
	count = 0;
	for (idelist j = IDE_PROP_TYPE; j < IDE_COUNT; j++) {
		count = j - IDE_PROP_TYPE;
		int newY = edits[count].y - g_scrollPos;

		// Use the stored original X position, which is already relative to the tool panel's client area.
		int newX = edits[count].x;

		// Get current button size (width and height) - these don't change for these fixed-size buttons.
		const int buttonWidth = edits[count].width;
		const int buttonHeight = controlHeight;

		// Move the button to its new calculated Y position, using its original X
		MoveWindow(edits[count].thing, newX, newY,
		           buttonWidth, buttonHeight, TRUE);

	}
	MoveWindow(g_hPropConfirmButton, panelPadding + labelWidth + colSpacing, currY + 10 - g_scrollPos, 80, 30, TRUE);
	InvalidateRect(hwnd, NULL, TRUE);
}





// --- WinMain Function: Application Entry Point ---
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	WNDCLASSEX wc = {0};
	WNDCLASSEX wcCanvas = {0};
	WNDCLASSEX wcTool = {0};
	WNDCLASSEX wcCodeProp = {0};

	MSG msg;

	INITCOMMONCONTROLSEX iccex;
	iccex.dwSize = sizeof(INITCOMMONCONTROLSEX);
	iccex.dwICC  = ICC_STANDARD_CLASSES;
	InitCommonControlsEx(&iccex);

	// 1. Register main window class
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.cbSize        = sizeof(WNDCLASSEX);
	wc.lpfnWndProc   = WndProc;
	wc.hInstance     = hInstance;
	wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = CreateSolidBrush(RGB(17, 17, 17)); // Main window background color
	wc.lpszClassName = MAIN_WINDOW_CLASS_NAME;

	if (!RegisterClassEx(&wc)) {
		MessageBox(NULL, TEXT("主窗口类注册失败!"), TEXT("错误"), MB_ICONERROR | MB_OK);
		return 0;
	}

	// 2. Register canvas panel window class (requires custom drawing)
	wcCanvas.style = CS_HREDRAW | CS_VREDRAW; // HREDRAW | VREDRAW for redraw on size change (though canvas is fixed)
	wcCanvas.cbSize        = sizeof(WNDCLASSEX);
	wcCanvas.lpfnWndProc   = CanvasPanelProc;
	wcCanvas.hInstance     = hInstance;
	wcCanvas.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wcCanvas.hbrBackground = CreateSolidBrush(RGB(30, 30, 30)); // Canvas background color
	wcCanvas.lpszClassName = TEXT("CanvasPanelClass");

	if (!RegisterClassEx(&wcCanvas)) {
		MessageBox(NULL, TEXT("画布面板类注册失败!"), TEXT("错误"), MB_ICONERROR | MB_OK);
		return 0;
	}

	// 3. Register toolbox panel window class (mainly for managing its child controls, WM_COMMAND will be forwarded to parent)
	wcTool.cbSize        = sizeof(WNDCLASSEX);
	wcTool.lpfnWndProc   = toolpanelproc;
	wcTool.hInstance     = hInstance;
	wcTool.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wcTool.hbrBackground = CreateSolidBrush(RGB(20, 20, 20)); // Toolbox background color
	wcTool.lpszClassName = TEXT("ToolPanelClass");

	if (!RegisterClassEx(&wcTool)) {
		MessageBox(NULL, TEXT("工具栏类注册失败!"), TEXT("错误"), MB_ICONERROR | MB_OK);
		return 0;
	}

	// 4. Register code/properties panel window class
	wcCodeProp.cbSize        = sizeof(WNDCLASSEX);
	wcCodeProp.lpfnWndProc   = CodePropertiesPanelProc;
	wcCodeProp.hInstance     = hInstance;
	wcCodeProp.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wcCodeProp.hbrBackground = CreateSolidBrush(RGB(20, 20, 20));
	wcCodeProp.lpszClassName = TEXT("CodePropertiesPanelClass");

	if (!RegisterClassEx(&wcCodeProp)) {
		MessageBox(NULL, TEXT("代码/属性面板类注册失败!"), TEXT("错误"), MB_ICONERROR | MB_OK);
		return 0;
	}



	HMENU hMainMenu = CreateMainMenu();

	// 5. Create main window
	HWND hMainWnd = CreateWindowEx(
	                    0,
	                    MAIN_WINDOW_CLASS_NAME,
	                    MAIN_WINDOW_TITLE,
	                    WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN, // WS_CLIPCHILDREN prevents parent from drawing over children
	                    CW_USEDEFAULT, CW_USEDEFAULT,
	                    1700, 982,                          // Initial window size
	                    NULL,
	                    hMainMenu,
	                    hInstance,
	                    NULL
	                );

	if (!hMainWnd) {
		MessageBox(NULL, TEXT("主窗口创建失败!"), TEXT("错误"), MB_ICONERROR | MB_OK);
		return 0;
	}



	// 6. Create child panels: toolbox, canvas, code output
	// Add WS_VSCROLL to hToolboxPanel for vertical scrolling
	hToolboxPanel = CreateWindowEx(
	                    WS_EX_CLIENTEDGE,                   // Sunken border
	                    TEXT("ToolPanelClass"),             // Registered class name
	                    TEXT("控件栏"),                       // Window title
	                    WS_CHILD | WS_VISIBLE | SS_CENTER | WS_VSCROLL, // Child window, visible, centered text, vertical scrollbar
	                    10, 10,                             // X, Y position (will be overridden by WM_SIZE)
	                    200, 800,                           // Width, Height (will be overridden by WM_SIZE)
	                    hMainWnd,                           // Parent window
	                    (HMENU)IDC_TOOLBOX_PANEL,           // Control ID
	                    hInstance,                          // Application instance
	                    NULL                                // Window creation data
	                );

	// Create Canvas Panel (fixed size)
	g_hCanvasPanel = CreateWindowEx(
	                     WS_EX_CLIENTEDGE,                   // Extended style: sunken border
	                     TEXT("CanvasPanelClass"),           // Registered class name
	                     TEXT(""),                           // No title
	                     WS_CHILD | WS_VISIBLE,              // Child window, visible
	                     220, 10,                            // X, Y position (will be overridden by WM_SIZE)
	                     CANVAS_INITIAL_WIDTH, CANVAS_INITIAL_HEIGHT, // Fixed width and height (will be overridden by WM_SIZE)
	                     hMainWnd,                           // Parent window
	                     (HMENU)IDC_CANVAS_PANEL,            // Control ID
	                     hInstance,                          // Application instance
	                     NULL                                // Window creation data
	                 );
	hCodeOutputPanel = CreateWindowEx(
	                       WS_EX_CLIENTEDGE,
	                       TEXT("CodePropertiesPanelClass"),
	                       TEXT(""),
	                       WS_CHILD | WS_VISIBLE | ES_AUTOVSCROLL,
	                       930, 10,                            // X, Y position (will be overridden by WM_SIZE)
	                       550, 800,                           // Width, Height (will be overridden by WM_SIZE)
	                       hMainWnd,
	                       (HMENU)IDC_CODE_OUTPUT_PANEL,
	                       hInstance,
	                       NULL
	                   );
	ctitle = CreateWindowEx(0, TEXT("STATIC"), TEXT("生成的 Objective-C 代码"), WS_CHILD | WS_VISIBLE | SS_CENTER,
	                        0, 0, 0, 0, hMainWnd, (HMENU)IDC_CODE_TITLE_STATIC, hInstance, NULL);

	g_hCodeEdit = CreateWindowEx(
	                  0,
	                  TEXT("EDIT"),
	                  TEXT("// 代码将在这里生成，并调用你的 csm 工厂方法...\r\n// 例如：[UIViewController createButtonWithFrame:...]\r\n\r\n"),
	                  WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL | ES_MULTILINE | ES_READONLY | ES_AUTOVSCROLL | ES_AUTOHSCROLL,
	                  0, 0,
	                  0, 0,
	                  hMainWnd,
	                  (HMENU)IDC_CODE_DISPLAY_EDIT,
	                  hInstance,
	                  NULL
	              );
	SetWindowTheme(g_hCodeEdit, L"DarkMode_Explorer", nullptr);

	g_hbrPropertiesPanelBackground = CreateSolidBrush(RGB(30, 30, 30));
	if (g_hbrPropertiesPanelBackground == NULL) {
		MessageBox(NULL, TEXT("属性面板背景画刷创建失败!"), TEXT("错误"), MB_ICONERROR | MB_OK);
		return 0;
	}
	//butbackbr = CreateSolidBrush(RGB(20, 20, 20));
	codeeditback = CreateSolidBrush(RGB(60, 60, 60));
	SetWindowTheme(hCodeOutputPanel, L"DarkMode_Explorer", nullptr);
	SetWindowTheme(hToolboxPanel, L"DarkMode_Explorer", nullptr);

	int currentButtonY = 50;
	const int buttonHeight = 60;
	const int buttonSpacing = BUTTON_SPACING; // Using the macro now
	const int buttonWidth = 180;
	const int buttonX = 29; // This is the client X position relative to hToolboxPanel
	g_hBuilderCoreDll = LoadLibraryW(L"libbuidll.dll");

	// For each button, create it and store its HWND and original Y position
	inittool();
	g_numToolButtons = 0;
	for (int i = 0; i < g_numElementDefinitions; ++i) {
		g_hToolButtons[g_numToolButtons] = CreateWindowEx(
		                                       0,
		                                       TEXT("BUTTON"),
		                                       g_elementDefinitions[i].typeName,
		                                       WS_CHILD  | WS_VISIBLE | BS_OWNERDRAW,
		                                       buttonX, currentButtonY,
		                                       buttonWidth, buttonHeight,
		                                       hToolboxPanel,
		                                       (HMENU)MAKEINTRESOURCE(g_elementDefinitions[i].toolButtonId),
		                                       hInstance,
		                                       NULL
		                                   );

		// Store the original position
		g_toolButtonOriginalX[g_numToolButtons] = buttonX;
		g_toolButtonOriginalY[g_numToolButtons] = currentButtonY;

		currentButtonY += buttonHeight + buttonSpacing;
		g_numToolButtons++;

	}

	HFONT hFont = CreateFont(-73, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
	                         DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
	                         DEFAULT_PITCH | FF_SWISS, TEXT("Segoe UI"));

	// 8. Display and update main window
	ShowWindow(hMainWnd, nCmdShow);
	UpdateWindow(hMainWnd);

	// Initial update of properties panel (shows unselected state)
	UpdatePropertiesPanel();
	// Initial code generation
	RegenerateAllObjCCode();



	// 9. Message loop
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	// Destroy main window, toolbox panel, and code/properties panel background brushes at the end of WinMain
	DeleteObject(wc.hbrBackground);
	DeleteObject(wcTool.hbrBackground);
	DeleteObject(wcCodeProp.hbrBackground);

	return (int)msg.wParam;
}

// --- Window Procedure function implementations ---

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
		case WM_CTLCOLORSTATIC:
		case WM_CTLCOLOREDIT: {
			HDC hdc = (HDC)wParam;
			SetTextColor(hdc, RGB(255, 255, 255));
			SetBkColor(hdc, RGB(60, 60, 60));
			return (INT_PTR)codeeditback;
		}

		case WM_COMMAND: {
			int wmId = LOWORD(wParam);
			HINSTANCE hInstance = (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE);

			// Handle tool button clicks
			for (int i = 0; i < g_numElementDefinitions; i++) {
				if (wmId == g_elementDefinitions[i].toolButtonId) { // Check if it's a tool button
					if (g_iOSUIElementCount < MAX_UI_ELEMENTS) {
						iOSUIElement* newElement = &g_iOSUIElements[g_iOSUIElementCount];
						ZeroMemory(newElement, sizeof(iOSUIElement)); // Clear new element memory

						newElement->id = IDC_GENERATED_UI_BASE + g_iOSUIElementCount; // Assign unique ID
						wcscpy_s(newElement->type, _countof(newElement->type), g_elementDefinitions[i].typeName);
						newElement->definition = &g_elementDefinitions[i]; // Link to its definition

						// Initialize default data using the function pointer
						if (g_elementDefinitions[i].initDefaultDataFunc) {
							g_elementDefinitions[i].initDefaultDataFunc(newElement);
						}

						// Set element's current position and size to its original (fixed) values
						newElement->x = newElement->original_x;
						newElement->y = newElement->original_y;
						newElement->width = newElement->original_width; // Use original_width/height directly for fixed size
						newElement->height = newElement->original_height;

						g_iOSUIElementCount++;
						g_selectedElementIndex = g_iOSUIElementCount - 1; // Select the newly added element

						InvalidateRect(g_hCanvasPanel, NULL, TRUE); // Redraw canvas
						UpdatePropertiesPanel(); // Update properties panel for new element
						RegenerateAllObjCCode(); // Regenerate code
					} else {
						MessageBox(hwnd, TEXT("画布已满，无法添加更多元素。"), TEXT("警告"), MB_ICONWARNING | MB_OK);
					}
					return 0; // Handled the command
				}
			}

			if (wmId == IDC_PROP_CONFIRM_BUTTON) {
				if (g_selectedElementIndex != -1) {
					ReadPropertiesFromUI(&g_iOSUIElements[g_selectedElementIndex]);
				}
				InvalidateRect(g_hCanvasPanel, NULL, TRUE);
				RegenerateAllObjCCode();
			}

			if (wmId == IDM_DELETE_ELEMENT) {

				DeleteSelectedElement();

			}
			if (wmId == IDM_FILE_EXIT)
				DestroyWindow(hwnd);
			if (wmId == IDM_HELP_ABOUT)
				MessageBox(NULL, TEXT("ios builder"), TEXT("about"), MB_OK);
			if (wmId == IDM_FILE_SAVE)
				SaveFile(hwnd);
			if (wmId == IDM_COPY_ELEMENT)
				CopySelectedElement(hwnd);
			if (HIWORD(wParam) == CBN_SELCHANGE) {
				// 强制将焦点移出下拉框
				SetFocus(hwnd);
			}

			break;
		}
		case WM_SIZE: {
			RECT rc;
			GetClientRect(hwnd, &rc);
			int newWidth = rc.right - rc.left;
			int newHeight = rc.bottom - rc.top;

			int toolboxWidth = 250;
			int codePanelWidth = 450;
			int spacing = 10;

			int canvasX = toolboxWidth + spacing;
			int codePanelX = newWidth - codePanelWidth - spacing - 500 - spacing * 2;

			int codeeditx = newWidth - 500 - spacing;

			MoveWindow(hToolboxPanel, spacing, spacing, toolboxWidth, newHeight - (2 * spacing), TRUE);
			MoveWindow(hCodeOutputPanel, codePanelX, spacing, codePanelWidth, newHeight - (2 * spacing), TRUE);
			MoveWindow(g_hCodeEdit, codeeditx, spacing + 20, 500, newHeight - 20 - (2 * spacing), TRUE);
			MoveWindow(ctitle, codeeditx, spacing, 470, 20, TRUE);

			// Fixed canvas dimensions
			int finalCanvasWidth = CANVAS_INITIAL_WIDTH;
			int finalCanvasHeight = CANVAS_INITIAL_HEIGHT;

			// Calculate available space for centering the fixed canvas
			int canvasAvailableWidth = codePanelX - canvasX - spacing;
			int canvasAvailableHeight = newHeight - (2 * spacing);

			// Calculate centered position for the fixed canvas
			int centeredX = canvasX + (canvasAvailableWidth - finalCanvasWidth) / 2;
			int centeredY = spacing + (canvasAvailableHeight - finalCanvasHeight) / 2;

			// Ensure centered position is not negative
			if (centeredX < canvasX) centeredX = canvasX;
			if (centeredY < spacing) centeredY = spacing;

			MoveWindow(g_hCanvasPanel, spacing + toolboxWidth + spacing, centeredY, finalCanvasWidth, finalCanvasHeight, TRUE);



			// No scaling of elements needed here, as the canvas is fixed and elements use original_x/y/width/height
			// We just need to ensure the canvas is redrawn.
			InvalidateRect(g_hCanvasPanel, NULL, TRUE);
			InvalidateRect(hToolboxPanel, NULL, TRUE);
			InvalidateRect(ctitle, NULL, TRUE);
			InvalidateRect(g_hCodeEdit, NULL, TRUE);
			UpdatePropertiesPanel(); // Update properties panel to reflect current state (even if no change)
			RegenerateAllObjCCode(); // Regenerate code (might not change if elements aren't scaled, but good practice)

			break;
		}
		case WM_DESTROY: {
			if (g_hbrPropertiesPanelBackground != NULL) {
				DeleteObject(g_hbrPropertiesPanelBackground);
				g_hbrPropertiesPanelBackground = NULL;
			}
			//DeleteObject(butbackbr);
			//butbackbr = NULL;
			DeleteObject(codeeditback);
			codeeditback = NULL;

			PostQuitMessage(0);
			break;
		}
		default:
			return DefWindowProc(hwnd, msg, wParam, lParam);

	}
	return 0;
}

LRESULT CALLBACK CanvasPanelProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	static int mouseX, mouseY;
	static BOOL isDragging = FALSE;
	static int dragOffset_X, dragOffset_Y;
	static int resizeHandle = -1;
	const int GRAB_AREA = 5;

	switch (msg) {
		case WM_PAINT: {
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hwnd, &ps);
			for (int i = 0; i < g_iOSUIElementCount; i++) {
				iOSUIElement *element = &g_iOSUIElements[i];
				// Use function pointer to call the corresponding drawing function
				if (element->definition && element->definition->drawFunc) {
					// Draw using original dimensions, as canvas is fixed
					element->x = element->original_x;
					element->y = element->original_y;
					element->width = element->original_width;
					element->height = element->original_height;
					element->definition->drawFunc(hdc, element);
				}
			}

			// Draw selection rectangle if an element is selected
			if (g_selectedElementIndex != -1) {
				iOSUIElement *selectedElement = &g_iOSUIElements[g_selectedElementIndex];
				HPEN hPen = CreatePen(PS_DOT, 1, RGB(255, 255, 0)); // Yellow dotted line
				HPEN hOldPen = SelectObject(hdc, hPen);
				HBRUSH hOldBrush = SelectObject(hdc, GetStockObject(NULL_BRUSH)); // No fill

				Rectangle(hdc, selectedElement->x, selectedElement->y,
				          selectedElement->x + selectedElement->width, selectedElement->y + selectedElement->height);

				// Draw resize handles
				int handleSize = GRAB_AREA * 2;
				HBRUSH hBrush = CreateSolidBrush(RGB(255, 0, 0)); // Red handles
				HBRUSH hOldBrush2 = SelectObject(hdc, hBrush);

				Ellipse(hdc, selectedElement->x - GRAB_AREA, selectedElement->y - GRAB_AREA,
				        selectedElement->x + GRAB_AREA, selectedElement->y + GRAB_AREA); // Top-left
				Ellipse(hdc, selectedElement->x + selectedElement->width - GRAB_AREA, selectedElement->y - GRAB_AREA,
				        selectedElement->x + selectedElement->width + GRAB_AREA, selectedElement->y + GRAB_AREA); // Top-right
				Ellipse(hdc, selectedElement->x + selectedElement->width - GRAB_AREA, selectedElement->y + selectedElement->height - GRAB_AREA,
				        selectedElement->x + selectedElement->width + GRAB_AREA, selectedElement->y + selectedElement->height + GRAB_AREA); // Bottom-right
				Ellipse(hdc, selectedElement->x - GRAB_AREA, selectedElement->y + selectedElement->height - GRAB_AREA,
				        selectedElement->x + GRAB_AREA, selectedElement->y + selectedElement->height + GRAB_AREA); // Bottom-left

				SelectObject(hdc, hOldPen);
				SelectObject(hdc, hOldBrush);
				SelectObject(hdc, hOldBrush2);
				DeleteObject(hPen);
				DeleteObject(hBrush);
			}

			EndPaint(hwnd, &ps);
			break;
		}
		case WM_LBUTTONDOWN: {
			mouseX = GET_X_LPARAM(lParam);
			mouseY = GET_Y_LPARAM(lParam);
			SetCapture(hwnd);

			int oldSelected = g_selectedElementIndex;
			g_selectedElementIndex = -1;

			if (oldSelected != -1) {
				iOSUIElement *element = &g_iOSUIElements[oldSelected];
				RECT r = {element->x, element->y, element->x + element->width, element->y + element->height};

				if (mouseX >= r.left - GRAB_AREA && mouseX <= r.left + GRAB_AREA &&
				        mouseY >= r.top - GRAB_AREA && mouseY <= r.top + GRAB_AREA) {
					resizeHandle = 0;
					g_selectedElementIndex = oldSelected;
				} else if (mouseX >= r.right - GRAB_AREA && mouseX <= r.right + GRAB_AREA &&
				           mouseY >= r.top - GRAB_AREA && mouseY <= r.top + GRAB_AREA) {
					resizeHandle = 1;
					g_selectedElementIndex = oldSelected;
				} else if (mouseX >= r.right - GRAB_AREA && mouseX <= r.right + GRAB_AREA &&
				           mouseY >= r.bottom - GRAB_AREA && mouseY <= r.bottom + GRAB_AREA) {
					resizeHandle = 2;
					g_selectedElementIndex = oldSelected;
				} else if (mouseX >= r.left - GRAB_AREA && mouseX <= r.left + GRAB_AREA &&
				           mouseY >= r.bottom - GRAB_AREA && mouseY <= r.bottom + GRAB_AREA) {
					resizeHandle = 3;
					g_selectedElementIndex = oldSelected;
				}
			}

			if (resizeHandle == -1) {
				for (int i = g_iOSUIElementCount - 1; i >= 0; i--) {
					iOSUIElement *element = &g_iOSUIElements[i];
					RECT elementRect = {element->x, element->y, element->x + element->width, element->y + element->height};

					if (PtInRect(&elementRect, (POINT) {
					mouseX, mouseY
				})) {
						g_selectedElementIndex = i;
						isDragging = TRUE;
						dragOffset_X = mouseX - element->x;
						dragOffset_Y = mouseY - element->y;
						break;
					}
				}
			}

			if (oldSelected != g_selectedElementIndex) {
				UpdatePropertiesPanel();
				InvalidateRect(hwnd, NULL, TRUE);
			} else if (g_selectedElementIndex != -1 && (isDragging || resizeHandle != -1)) {
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

				// Canvas dimensions are fixed, so these are CANVAS_INITIAL_WIDTH/HEIGHT
				int currentCanvasWidth = CANVAS_INITIAL_WIDTH;
				int currentCanvasHeight = CANVAS_INITIAL_HEIGHT;

				int newX = element->original_x; // Start with original values for calculations
				int newY = element->original_y;
				int newWidth = element->original_width;
				int newHeight = element->original_height;

				if (isDragging) {
					newX = mouseX - dragOffset_X;
					newY = mouseY - dragOffset_Y;

					// Clamp to canvas boundaries
					if (newX < 0) newX = 0;
					if (newY < 0) newY = 0;
					if (newX + element->original_width > currentCanvasWidth) newX = currentCanvasWidth - element->original_width;
					if (newY + element->original_height > currentCanvasHeight) newY = currentCanvasHeight - element->original_height;

					element->original_x = newX;
					element->original_y = newY;

				} else if (resizeHandle != -1) {
					int originalElementX = element->original_x;
					int originalElementY = element->original_y;
					int originalElementWidth = element->original_width;
					int originalElementHeight = element->original_height;

					switch (resizeHandle) {
						case 0: // Top-left
							newX = mouseX;
							newY = mouseY;
							newWidth = originalElementX + originalElementWidth - newX;
							newHeight = originalElementY + originalElementHeight - newY;
							break;
						case 1: // Top-right
							newX = originalElementX;
							newY = mouseY;
							newWidth = mouseX - originalElementX;
							newHeight = originalElementY + originalElementHeight - newY;
							break;
						case 2: // Bottom-right
							newX = originalElementX;
							newY = originalElementY;
							newWidth = mouseX - originalElementX;
							newHeight = mouseY - originalElementY;
							break;
						case 3: // Bottom-left
							newX = mouseX;
							newY = originalElementY;
							newWidth = originalElementX + originalElementWidth - newX;
							newHeight = mouseY - originalElementY;
							break;
					}

					// Ensure minimum size
					if (newWidth < 1) newWidth = 1;
					if (newHeight < 1) newHeight = 1;

					// Clamp to canvas boundaries
					if (newX < 0) {
						newWidth += newX;
						newX = 0;
					}
					if (newY < 0) {
						newHeight += newY;
						newY = 0;
					}
					if (newX + newWidth > currentCanvasWidth) newWidth = currentCanvasWidth - newX;
					if (newY + newHeight > currentCanvasHeight) newHeight = currentCanvasHeight - newY;

					element->original_x = newX;
					element->original_y = newY;
					element->original_width = newWidth;
					element->original_height = newHeight;
				}

				// Update current x, y, width, height for drawing based on original values
				element->x = element->original_x;
				element->y = element->original_y;
				element->width = element->original_width;
				element->height = element->original_height;

				UpdatePropertiesPanel();
				InvalidateRect(hwnd, NULL, TRUE);
				UpdateWindow(hwnd);
			} else {
				if (g_selectedElementIndex != -1) {
					iOSUIElement *element = &g_iOSUIElements[g_selectedElementIndex];
					RECT r = {element->x, element->y, element->x + element->width, element->y + element->height};
					HCURSOR hCursor = NULL;

					if ((mouseX >= r.left - GRAB_AREA && mouseX <= r.left + GRAB_AREA &&
					        mouseY >= r.top - GRAB_AREA && mouseY <= r.top + GRAB_AREA) ||
					        (mouseX >= r.right - GRAB_AREA && mouseX <= r.right + GRAB_AREA &&
					         mouseY >= r.bottom - GRAB_AREA && mouseY <= r.bottom + GRAB_AREA)) {
						hCursor = LoadCursor(NULL, IDC_SIZENWSE);
					} else if ((mouseX >= r.right - GRAB_AREA && mouseX <= r.right + GRAB_AREA &&
					            mouseY >= r.top - GRAB_AREA && mouseY <= r.top + GRAB_AREA) ||
					           (mouseX >= r.left - GRAB_AREA && mouseX <= r.left + GRAB_AREA &&
					            mouseY >= r.bottom - GRAB_AREA && mouseY <= r.bottom + GRAB_AREA)) {
						hCursor = LoadCursor(NULL, IDC_SIZENESW);
					} else if (PtInRect(&r, (POINT) {
					mouseX, mouseY
				})) {
						hCursor = LoadCursor(NULL, IDC_SIZEALL);
					} else {
						hCursor = LoadCursor(NULL, IDC_ARROW);
					}
					if (hCursor) SetCursor(hCursor);
				} else {
					SetCursor(LoadCursor(NULL, IDC_ARROW));
				}
			}
			break;
		}

		case WM_LBUTTONUP: {
			ReleaseCapture();
			isDragging = FALSE;
			resizeHandle = -1;
			UpdatePropertiesPanel();
			RegenerateAllObjCCode();

			break;
		}
		case WM_SETCURSOR:
			if (LOWORD(lParam) == HTCLIENT) {
				return TRUE;
			}
			return DefWindowProc(hwnd, msg, wParam, lParam);
		/*		case WM_RBUTTONDOWN: {
					// 获取鼠标点击的坐标
					int mouseX = LOWORD(lParam);
					int mouseY = HIWORD(lParam);

					// 检查是否有元素被选中
					if (g_selectedElementIndex != -1) {
						// 创建并显示右键菜单
						HMENU hPopupMenu = CreatePopupMenu();
						AppendMenu(hPopupMenu, MF_STRING, IDM_DELETE_ELEMENT, TEXT("删除"));

						// 将客户端坐标转换为屏幕坐标
						POINT pt;
						pt.x = mouseX;
						pt.y = mouseY;
						ClientToScreen(g_hCanvasPanel, &pt);

						TrackPopupMenu(hPopupMenu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, GetParent(hwnd), NULL);
						DestroyMenu(hPopupMenu);
					}
					break;
				}*/
		default:
			return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	return 0;
}

LRESULT CALLBACK toolpanelproc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	HINSTANCE hInstance = (HINSTANCE)GetWindowLongPtr(GetParent(hwnd), GWLP_HINSTANCE);

	switch (msg) {
		case WM_DRAWITEM: {
			LPDRAWITEMSTRUCT dis = (LPDRAWITEMSTRUCT)lParam;
			if (dis->CtlType == ODT_BUTTON) {
				BOOL isHovered = (dis->itemState & ODS_HOTLIGHT);
				BOOL isPressed = (dis->itemState & ODS_SELECTED);

				// 背景色
				COLORREF bgColor = isPressed ? RGB(65, 65, 65) :
				                   isHovered ? RGB(55, 55, 65) : RGB(40, 40, 45);
				HBRUSH br = CreateSolidBrush(bgColor);
				HPEN pen = CreatePen(PS_SOLID, 1, RGB(90, 90, 100));
				HBRUSH oldBr = SelectObject(dis->hDC, br);
				HPEN oldPen = SelectObject(dis->hDC, pen);

				HBRUSH hFullBrush = CreateSolidBrush(bgColor);
				FillRect(dis->hDC, &dis->rcItem, hFullBrush);
				DeleteObject(hFullBrush);

				// 圆角矩形
				RoundRect(dis->hDC, dis->rcItem.left, dis->rcItem.top,
				          dis->rcItem.right, dis->rcItem.bottom, 8, 8);

				SelectObject(dis->hDC, oldBr);
				SelectObject(dis->hDC, oldPen);
				DeleteObject(br);
				DeleteObject(pen);

				// 文字
				SetBkMode(dis->hDC, TRANSPARENT);
				SetTextColor(dis->hDC, RGB(230, 230, 230));

				// 获取按钮文字
				WCHAR text[128];
				GetWindowText(dis->hwndItem, text, 128);
				HFONT hFont = CreateFont(32, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, TEXT("Arial"));
				HFONT hOldFont = SelectObject(dis->hDC, hFont);
				DrawText(dis->hDC, text, -1, (LPRECT)&dis->rcItem,
				         DT_CENTER | DT_VCENTER | DT_SINGLELINE);
				SelectObject(dis->hDC, hOldFont);
				DeleteObject(hFont);
				return TRUE;
			}
		}

		case WM_VSCROLL: {

			int nScrollCode = LOWORD(wParam);
			int nPos = HIWORD(wParam); // 仅对 SB_THUMBTRACK 和 SB_THUMBPOSITION 有效

			SCROLLINFO si;
			si.cbSize = sizeof(si);
			si.fMask = SIF_ALL; // 获取所有滚动条信息

			GetScrollInfo(hwnd, SB_VERT, &si);
			int oldPos = si.nPos; // 存储当前位置

			switch (nScrollCode) {
				case SB_TOP:
					si.nPos = si.nMin;
					break;        // 滚动到顶部
				case SB_BOTTOM:
					si.nPos = si.nMax;
					break;       // 滚动到底部
				case SB_LINEUP:
					si.nPos -= 10;
					break;           // 向上滚动一行 (任意10像素)
				case SB_LINEDOWN:
					si.nPos += 10;
					break;         // 向下滚动一行
				case SB_PAGEUP:
					si.nPos -= si.nPage;
					break;   // 向上滚动一页
				case SB_PAGEDOWN:
					si.nPos += si.nPage;
					break; // 向下滚动一页
				case SB_THUMBTRACK: // 用户正在拖动滑块
				case SB_THUMBPOSITION: // 用户释放了滑块
					si.nPos = nPos; // 设置位置为滑块所在位置
					break;
			}
			// 将新位置限制在有效范围内
			int maxScroll = si.nMax - si.nPage + 1; // 滚动条最大位置是 max - page + 1
			if (si.nPos < 0) si.nPos = 0;
			if (si.nPos > maxScroll) si.nPos = maxScroll;
			// 如果位置发生变化，更新滚动条和内容
			if (si.nPos != oldPos) {
				SetScrollInfo(hwnd, SB_VERT, &si, TRUE);

				toolPanelScrollPos = si.nPos; // 更新我们的全局滚动位置
				MoveToolButtons(hwnd); // 根据新的滚动位置移动子按钮


				SetScrollPos(hwnd, SB_VERT, toolPanelScrollPos, TRUE);

				// 使工具栏面板失效，确保它重新绘制
				InvalidateRect(hwnd, NULL, FALSE);



			}

			break;
		}

		case WM_SIZE: {
			// 当工具栏面板自身大小改变时发送此消息。
			// 我们需要在这里更新滚动条范围。

			RECT rcClient;
			GetClientRect(hwnd, &rcClient);

			// 计算所有按钮的总内容高度
			int totalContentHeight = 0;
			if (g_numToolButtons > 0) {
				for (int i = 0; i < g_numToolButtons; i++) {
					// 计算每个按钮的底部边缘相对于工具栏面板顶部的坐标
					// g_toolButtonOriginalY[i] 是按钮的顶部 Y 坐标
					// 40 是按钮的高度 (buttonHeight)
					int buttonBottom = g_toolButtonOriginalY[i] + 40;
					if (buttonBottom > totalContentHeight) {
						totalContentHeight = buttonBottom ;
					}
				}
				// 在所有按钮的最低点基础上增加额外的底部填充，确保最后一个按钮完全可见
				totalContentHeight += 20; // 增加20像素的底部填充
			}

			// 计算最大滚动位置
			// 它是总内容高度减去可见客户区高度
			toolPanelMaxScroll = max(0, totalContentHeight - rcClient.bottom);



			SCROLLINFO si;
			si.cbSize = sizeof(si);
			si.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
			si.nMin = 0;
			si.nMax = totalContentHeight;
			si.nPage = rcClient.bottom - rcClient.top;
			si.nPos = toolPanelScrollPos;

			// 将滚动条信息设置到自定义滚动条
			SetScrollInfo(hwnd, SB_VERT, &si, TRUE);

			// 如果当前滚动位置超出新范围，则进行调整
			if (toolPanelScrollPos > toolPanelMaxScroll) {
				toolPanelScrollPos = toolPanelMaxScroll;
				SetScrollPos(hwnd, SB_VERT, toolPanelScrollPos, TRUE);
			}

			// 根据新的滚动位置移动所有子按钮
			MoveToolButtons(hwnd);
			// 使工具栏面板失效，确保它重新绘制


			InvalidateRect(hwnd, NULL, FALSE);


			break;
		}

		case WM_MOUSEWHEEL: {

			// 获取滚轮滚动量
			short wheelDelta = HIWORD(wParam);

			// 根据滚动量调整滚动位置
			// wheelDelta 通常是 120 的倍数
			int yDelta = -wheelDelta / 3;

			int oldScrollPos = toolPanelScrollPos;
			toolPanelScrollPos += yDelta;

			SCROLLINFO si = {0};
			si.cbSize = sizeof(SCROLLINFO);
			si.fMask = SIF_RANGE | SIF_PAGE;
			GetScrollInfo(hwnd, SB_VERT, &si);

			// 限制滚动位置在有效范围内
			int maxScroll = si.nMax - si.nPage + 1; // 滚动条最大位置是 max - page + 1
			if (toolPanelScrollPos < 0) toolPanelScrollPos = 0;
			if (toolPanelScrollPos > maxScroll) toolPanelScrollPos = maxScroll;

			if (toolPanelScrollPos != oldScrollPos) {
				// 更新滚动条位置


				SetScrollPos(hwnd, SB_VERT, toolPanelScrollPos, TRUE);


				MoveToolButtons(hwnd);
				// 强制重绘，移动子控件
				InvalidateRect(hwnd, NULL, FALSE);
			}

			break;
		}
		case WM_COMMAND: {
			SendMessage(GetParent(hwnd), WM_COMMAND, wParam, lParam);
			break;
		}




		default:
			return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	return 0;
}

LRESULT CALLBACK CodePropertiesPanelProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	HINSTANCE hInstance = (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE);

	switch (msg) {
		case WM_CREATE: {




			initprop(hwnd);
			CreateWindowEx(0, TEXT("static"), TEXT("属性"), WS_CHILD | WS_VISIBLE | SS_CENTER,
			               0, 0, 450, 20, hwnd, NULL, hInstance, NULL);

			g_hPropConfirmButton = CreateWindowEx(
			                           0,
			                           TEXT("BUTTON"),
			                           TEXT("确定"),
			                           WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
			                           panelPadding + labelWidth + colSpacing, currY + 10,
			                           80, 30,
			                           hwnd,
			                           (HMENU)IDC_PROP_CONFIRM_BUTTON,
			                           hInstance,
			                           NULL
			                       );



			break;
		}
		case WM_SIZE: {
			RECT rc;
			GetClientRect(hwnd, &rc);
			int panelheight = rc.bottom - rc.top;
			int totalPropertiesHeight = currY + 50; // 假设属性面板内容总高
			SCROLLINFO si = {0};
			si.cbSize = sizeof(SCROLLINFO);
			si.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
			si.nMin = 0;
			si.nMax = totalPropertiesHeight;
			si.nPage = panelheight - 10;
			SetScrollInfo(hwnd, SB_VERT, &si, TRUE);

			MoveWindow(g_hCodeEdit, panelPadding, 30, (rc.right - rc.left) - (2 * panelPadding), 400, TRUE);
			break;
		}
		case WM_VSCROLL: {

			int nScrollCode = LOWORD(wParam);
			int nPos = HIWORD(wParam); // 仅对 SB_THUMBTRACK 和 SB_THUMBPOSITION 有效

			SCROLLINFO si;
			si.cbSize = sizeof(si);
			si.fMask = SIF_ALL; // 获取所有滚动条信息

			GetScrollInfo(hwnd, SB_VERT, &si);
			int oldPos = si.nPos; // 存储当前位置

			switch (nScrollCode) {
				case SB_TOP:
					si.nPos = si.nMin;
					break;        // 滚动到顶部
				case SB_BOTTOM:
					si.nPos = si.nMax;
					break;       // 滚动到底部
				case SB_LINEUP:
					si.nPos -= 10;
					break;           // 向上滚动一行 (任意10像素)
				case SB_LINEDOWN:
					si.nPos += 10;
					break;         // 向下滚动一行
				case SB_PAGEUP:
					si.nPos -= si.nPage;
					break;   // 向上滚动一页
				case SB_PAGEDOWN:
					si.nPos += si.nPage;
					break; // 向下滚动一页
				case SB_THUMBTRACK: // 用户正在拖动滑块
				case SB_THUMBPOSITION: // 用户释放了滑块
					si.nPos = nPos; // 设置位置为滑块所在位置
					break;
			}

			// 将新位置限制在有效范围内
			si.nPos = max(si.nMin, min(si.nPos, si.nMax));
			if (g_scrollPos < si.nMin) {
				g_scrollPos = si.nMin;
			}
			if (g_scrollPos > si.nMax) {
				g_scrollPos = si.nMax;
			}


			// 如果位置发生变化，更新滚动条和内容
			if (si.nPos != oldPos) {
				SetScrollInfo(hwnd, SB_VERT, &si, TRUE); // 更新滚动条位置
				movecode(hwnd);
				g_scrollPos = si.nPos; // 更新我们的全局滚动位置
				InvalidateRect(hwnd, NULL, FALSE);

				// 使工具栏面板失效，确保它重新绘制

			}
			break;
		}
		case WM_MOUSEWHEEL: {

			// 获取滚轮滚动量
			short wheelDelta = HIWORD(wParam);

			// 根据滚动量调整滚动位置
			// wheelDelta 通常是 120 的倍数
			int yDelta = -wheelDelta / 3;

			int oldScrollPos = g_scrollPos;
			g_scrollPos += yDelta;

			SCROLLINFO si = {0};
			si.cbSize = sizeof(SCROLLINFO);
			si.fMask = SIF_RANGE | SIF_PAGE;
			GetScrollInfo(hwnd, SB_VERT, &si);

			// 限制滚动位置在有效范围内
			int maxScroll = si.nMax - si.nPage + 1; // 滚动条最大位置是 max - page + 1
			if (g_scrollPos < 0) g_scrollPos = 0;
			if (g_scrollPos > maxScroll) g_scrollPos = maxScroll;

			if (g_scrollPos != oldScrollPos) {
				// 更新滚动条位置


				SetScrollPos(hwnd, SB_VERT, g_scrollPos, TRUE);


				movecode(hwnd);
				// 强制重绘，移动子控件
				InvalidateRect(hwnd, NULL, FALSE);
			}

			break;
		}
		case WM_COMMAND: {
			SendMessage(GetParent(hwnd), WM_COMMAND, wParam, lParam);
			break;
		}
		case WM_CTLCOLORSTATIC:
		case WM_CTLCOLOREDIT: {
			HDC hdc = (HDC)wParam;
			SetTextColor(hdc, RGB(255, 255, 255));
			SetBkColor(hdc, RGB(30, 30, 30));
			return (INT_PTR)g_hbrPropertiesPanelBackground;
		}
		default:
			return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	return 0;
}
void CopySelectedElement(HWND hwnd) {
	if (g_selectedElementIndex == -1) {
		MessageBox(hwnd, L"请先选择一个元素来复制。", L"警告", MB_OK | MB_ICONWARNING);
		return;
	}

	if (g_iOSUIElementCount >= MAX_UI_ELEMENTS) {
		MessageBox(hwnd, L"画布已满，无法复制更多元素。", L"警告", MB_OK | MB_ICONWARNING);
		return;
	}

	// 获取被选中的元素
	iOSUIElement* selectedElement = &g_iOSUIElements[g_selectedElementIndex];

	// 创建新元素，使用memcpy复制所有数据
	iOSUIElement* newElement = &g_iOSUIElements[g_iOSUIElementCount];
	memcpy(newElement, selectedElement, sizeof(iOSUIElement));

	// 为新元素分配一个新的ID
	newElement->id = IDC_GENERATED_UI_BASE + g_iOSUIElementCount;

	// 为了避免新元素与旧元素完全重叠，稍微偏移它的位置
	newElement->x += 20;
	newElement->y += 20;
	newElement->original_x = newElement->x;
	newElement->original_y = newElement->y;

	g_iOSUIElementCount++;
	g_selectedElementIndex = g_iOSUIElementCount - 1; // 选中新复制的元素

	InvalidateRect(g_hCanvasPanel, NULL, TRUE); // 重绘画布
	UpdatePropertiesPanel(); // 更新属性面板
	RegenerateAllObjCCode(); // 重新生成代码
}
void DeleteSelectedElement() {

	if (g_selectedElementIndex == -1) {
		return; // 没有选中的元素，直接返回
	}

	int elementIndex = -1;
	// 找到选中元素在数组中的索引
	for (int i = 0; i < g_iOSUIElementCount; ++i) {
		if (i == g_selectedElementIndex) {
			elementIndex = i;
			break;
		}
	}

	if (elementIndex != -1) {

		// 将数组中所有后续元素前移
		for (int i = elementIndex; i < g_iOSUIElementCount - 1; ++i) {
			g_iOSUIElements[i] = g_iOSUIElements[i + 1];

		}

		for (int i = 0; i <= g_iOSUIElementCount - 1; i++) {
			g_iOSUIElements[i].id = IDC_GENERATED_UI_BASE + i;
		}

		// 减少元素总数
		g_iOSUIElementCount--;

		// 重置选中元素
		g_selectedElementIndex = -1;

		// 清空属性面板
		HideAllPropertiesUIControls();

		// 重新绘制画布
		InvalidateRect(g_hCanvasPanel, NULL, TRUE);

		UpdateWindow(g_hCanvasPanel);
		// 重新生成代码
		RegenerateAllObjCCode();
	}
}
