#ifndef _DLL_H_
#define _DLL_H_

#include "IOSBUILDER.h"
#include <windows.h>

#if BUILDING_DLL
	#define DLLIMPORT __declspec(dllexport)
#else
	#define DLLIMPORT __declspec(dllimport)
#endif

DLLIMPORT void DrawButton(HDC hdc, iOSUIElement* element);
DLLIMPORT void GenerateObjCButtonCode(WCHAR* codeBuffer, size_t bufferSize, iOSUIElement * element);
void RgbToUIColorCode(int r, int g, int b, char* buffer, size_t bufferSize);
DLLIMPORT void DrawTextField(HDC hdc, iOSUIElement* element);
DLLIMPORT void GenerateObjCTextFieldCode(WCHAR* codeBuffer, size_t bufferSize, iOSUIElement* element);

DLLIMPORT void DrawTableView(HDC hdc, iOSUIElement* element);
DLLIMPORT void GenerateObjCTableViewCode(WCHAR* codeBuffer, size_t bufferSize, iOSUIElement* element);

DLLIMPORT void DrawSlider(HDC hdc, iOSUIElement* element); // New: Slider drawing function
DLLIMPORT void GenerateObjCSliderCode(WCHAR* codeBuffer, size_t bufferSize, iOSUIElement* element); // New: Slider code generation function

DLLIMPORT void DrawLabel(HDC hdc, iOSUIElement* element); // New: Label drawing function
DLLIMPORT void GenerateObjCLabelCode(WCHAR* codeBuffer, size_t bufferSize, iOSUIElement* element); // New: Label code generation function


#endif
