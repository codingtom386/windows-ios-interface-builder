/* Replace "dll.h" with the name of your header */
#define UNICODE
#define _UNICODE
#include "dll.h"
WCHAR textAlign[][50] = {
	TEXT("Center"),
	TEXT("Right"),
	TEXT("Natural"),
	TEXT("Left"),
	TEXT("Justified")
};
void RgbToUIColorCode(int r, int g, int b, char* buffer, size_t bufferSize) {
	r = (r < 0) ? 0 : ((r > 255) ? 255 : r);
	g = (g < 0) ? 0 : ((g > 255) ? 255 : g);
	b = (b < 0) ? 0 : ((b > 255) ? 255 : b);

	sprintf_s(buffer, bufferSize, "[UIColor colorWithRed:%.3f  green:%.3f  blue:%.3f  alpha:1.0]",
	          (float)r / 255.0, (float)g / 255.0, (float)b / 255.0);
}
//button
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


	swprintf_s(codeBuffer, bufferSize,
	           TEXT("    UIButton *%ls = [UIViewController createButtonWithFrame:CGRectMake(%d, %d, %d, %d)\r\n")
	           TEXT("                                                          bgcolor:%ls\r\n")
	           TEXT("                                                        textColor:%ls\r\n")
	           TEXT("                                                             font:[UIFont fontWithName:@\"arial\" size:%.2f] // Font size\r\n")
	           TEXT("                                                            title:@\"%ls\"\r\n")
	           TEXT("                                                      borderWidth:0.0 // Example border width\r\n")
	           TEXT("                                                      borderColor:nil]; // Example border width\r\n")
	           TEXT("[%ls addTarget:self action:@selector(%ls) forControlEvents:UIContorlEventTouchUpInside];\r\n")
	           TEXT("    [self.view addSubview:%ls]; \r\n\r\n"),
	           element->Vname, element->original_x, element->original_y, element->original_width, element->original_height,
	           bgColorCode_wc, textColorCode_wc, element->fontSize, element->text, element->Vname,
	           element->linkproc, element->Vname
	          );
}

//textfield
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

	DrawText(hdc, element->text, -1, &elementRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);

	SelectObject(hdc, hOldFont);
	DeleteObject(hFont);
}
void GenerateObjCTextFieldCode(WCHAR* codeBuffer, size_t bufferSize, iOSUIElement * element) {

	swprintf_s(codeBuffer, bufferSize,
	           TEXT("    UITextField *%ls = [UIViewController createTextFieldWithFrame:CGRectMake(%d, %d, %d, %d)\r\n")
	           TEXT("                                                            borderWidth:1.0 // Example\r\n")
	           TEXT("                                                            borderColor:[UIColor lightGrayColor].CGColor // Example\r\n")
	           TEXT("                                                        	 clearButton:UITextFieldViewModeAlways // Example\r\n")
	           TEXT("                                                            placeHolder:@\"%ls\"\r\n")
	           TEXT("                                                          	 retKey:UIReturnKeyDone // Example\r\n")
	           TEXT("                                                            font:[UIFont fontWithName:@\"arial\" size:%.2f] // Font size\r\n")
	           TEXT("                                                    		 keyboardAppearance:UIKeyboardAppearanceDark]; // Example\r\n")
	           TEXT("    // %ls.delegate = self; // If delegate needs to be set\r\n")
	           TEXT("    [self.view addSubview:%ls];\r\n\r\n"),
	           element->Vname, element->original_x, element->original_y, element->original_width, element->original_height,
	           element->text,  element->fontSize, element->Vname, element->Vname
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
	           TEXT("    UITableView *%ls = [UIViewController createTableViewWithFrame:CGRectMake(%d, %d, %d, %d)\r\n")
	           TEXT("                                                                    style:UITableViewStylePlain // Example\r\n")
	           TEXT("                                                              borderw:1.0 // Example\r\n")
	           TEXT("                                                              borderColor:[UIColor lightGrayColor].CGColor // Example\r\n")
	           TEXT("                                                                ciden:@\"MyCellIdentifier%ls\"]; // Example\r\n")
	           TEXT("	 //%ls.dataSource=self; // Example\r\n")
	           TEXT("	 //%ls.delegate=self; // Example\r\n")
	           TEXT("    [self.view addSubview:%ls];\r\n\r\n"),
	           element->Vname, element->original_x, element->original_y, element->original_width, element->original_height,
	           element->Vname,
	           element->Vname, element->Vname, element->Vname
	          );
}

// Label functions
void DrawLabel(HDC hdc, iOSUIElement* element) {
	RECT elementRect = {element->x, element->y, element->x + element->width, element->y + element->height};

	// Only draw background if it's not transparent (all zeros)
	//if (element->bgColorR != 0 || element->bgColorG != 0 || element->bgColorB != 0) {
	HBRUSH hBrush = CreateSolidBrush(RGB(element->bgColorR, element->bgColorG, element->bgColorB));
	HBRUSH hOldBrush = SelectObject(hdc, hBrush);
	FillRect(hdc, &elementRect, hBrush);
	SelectObject(hdc, hOldBrush);
	DeleteObject(hBrush);
//	}

	SetBkMode(hdc, TRANSPARENT);
	SetTextColor(hdc, RGB(element->textColorR, element->textColorG, element->textColorB));
	HFONT hFont = CreateFont(-(int)element->fontSize, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, TEXT("Arial"));
	HFONT hOldFont = SelectObject(hdc, hFont);
	if (element->alignment == 3) {
		DrawText(hdc, element->text, -1, &elementRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
	} else if (element->alignment == 1) {
		DrawText(hdc, element->text, -1, &elementRect, DT_RIGHT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
	} else {
		DrawText(hdc, element->text, -1, &elementRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);

	}

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

	swprintf_s(codeBuffer, bufferSize,
	           TEXT("UILabel *%ls = [UIViewController createlabelwithframe:CGRectMake(%d,%d,%d,%d)\r\n")
	           TEXT("			backcolor:%ls\r\n")
	           TEXT("			font:[UIFont fontWithName:@\"arial\" size:%.1f]\r\n")
	           TEXT("			textcolor:%ls\r\n")
	           TEXT("			textalignment:NSTextAlignment%ls\r\n")
	           TEXT("			text:@\"%ls\"];\r\n")
	           TEXT("    [self.view addSubview:%ls];\r\n\r\n"),
	           element->Vname, element->original_x, element->original_y, element->original_width, element->original_height,
	           bgColorCode_wc,
	           element->fontSize,
	           textColorCode_wc,
	           textAlign[element->alignment],
	           element->text,
	           element->Vname
	          );

}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
	switch (fdwReason) {
		case DLL_PROCESS_ATTACH: {

			break;
		}
		case DLL_PROCESS_DETACH: {
			break;
		}
		case DLL_THREAD_ATTACH: {
			break;
		}
		case DLL_THREAD_DETACH: {
			break;
		}
	}

	/* Return TRUE on success, FALSE on failure */
	return TRUE;
}
