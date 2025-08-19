#include <stdio.h>
#include <string.h>

int main() {
	// 声明变量来存储用户输入
	char type_name[100],  drawFunc[75], genFunc[75], text[30];
	int toolButtonId, defaultx, defaulty, defaultWidth, defaultHeight;
	int defaultbr, defaultbg, defaultbb, defaulttr, defaulttg, defaulttb;
	int defaultfontsize;
	int defaultminval, defaultmaxval, defaultcurrval, defaultrowheight;
	int hasText, hasBgColor, hasTextColor, hasFontSize, hasMinValue, hasMaxValue;
	int hasCurrentValue, hasRowHeight, hasAlignment;
	
	printf("请输入 dllmap.map 文件所需的字段值：\n\n");
	
	// 提示用户输入每个字段的值
	printf("请输入类型名称 (type_name): ");
	scanf("%99s", type_name);
	printf("请输入工具按钮ID (toolButtonId): ");
	scanf("%d", &toolButtonId);
	printf("请输入绘制函数 (drawFunc): ");
	scanf("%74s", drawFunc);
	printf("请输入生成函数 (genFunc): ");
	scanf("%74s", genFunc);
	printf("请输入默认X坐标 (defaultx): ");
	scanf("%d", &defaultx);
	printf("请输入默认Y坐标 (defaulty): ");
	scanf("%d", &defaulty);
	printf("请输入默认宽度 (defaultWidth): ");
	scanf("%d", &defaultWidth);
	printf("请输入默认高度 (defaultHeight): ");
	scanf("%d", &defaultHeight);
	printf("请输入按钮文本 (text): ");
	scanf("%29s", text);
	printf("请输入默认背景红色值 (defaultbr): ");
	scanf("%d", &defaultbr);
	printf("请输入默认背景绿色值 (defaultbg): ");
	scanf("%d", &defaultbg);
	printf("请输入默认背景蓝色值 (defaultbb): ");
	scanf("%d", &defaultbb);
	printf("请输入默认文本红色值 (defaulttr): ");
	scanf("%d", &defaulttr);
	printf("请输入默认文本绿色值 (defaulttg): ");
	scanf("%d", &defaulttg);
	printf("请输入默认文本蓝色值 (defaulttb): ");
	scanf("%d", &defaulttb);
	printf("请输入默认字体大小 (defaultfontsize): ");
	scanf("%d", &defaultfontsize);
	printf("请输入默认最小值 (defaultminval): ");
	scanf("%d", &defaultminval);
	printf("请输入默认最大值 (defaultmaxval): ");
	scanf("%d", &defaultmaxval);
	printf("请输入默认当前值 (defaultcurrval): ");
	scanf("%d", &defaultcurrval);
	printf("请输入默认行高 (defaultrowheight): ");
	scanf("%d", &defaultrowheight);
	printf("是否有文本 (hasText, 0 或 1): ");
	scanf("%d", &hasText);
	printf("是否有背景色 (hasBgColor, 0 或 1): ");
	scanf("%d", &hasBgColor);
	printf("是否有文本颜色 (hasTextColor, 0 或 1): ");
	scanf("%d", &hasTextColor);
	printf("是否有字体大小 (hasFontSize, 0 或 1): ");
	scanf("%d", &hasFontSize);
	printf("是否有最小值 (hasMinValue, 0 或 1): ");
	scanf("%d", &hasMinValue);
	printf("是否有最大值 (hasMaxValue, 0 或 1): ");
	scanf("%d", &hasMaxValue);
	printf("是否有当前值 (hasCurrentValue, 0 或 1): ");
	scanf("%d", &hasCurrentValue);
	printf("是否有行高 (hasRowHeight, 0 或 1): ");
	scanf("%d", &hasRowHeight);
	printf("是否有对齐方式 (hasAlignment, 0 或 1): ");
	scanf("%d", &hasAlignment);
	
	// 打印最终生成的字符串
	printf("\n生成的 dllmap.map 行内容如下：\n");
	printf(
		   "%s,%d,%s,%s,%d,%d,%d,%d,%s,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
		   type_name, toolButtonId,  drawFunc, genFunc,
		   defaultx, defaulty, defaultWidth, defaultHeight, text,
		   defaultbr, defaultbg, defaultbb, defaulttr, defaulttg, defaulttb,
		   defaultfontsize, defaultminval, defaultmaxval, defaultcurrval, defaultrowheight,
		   hasText, hasBgColor, hasTextColor, hasFontSize, hasMinValue, hasMaxValue,
		   hasCurrentValue, hasRowHeight, hasAlignment
		   );
	
	return 0;
}
