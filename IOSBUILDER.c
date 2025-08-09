#include "IOSBUILDER.h"

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
}
