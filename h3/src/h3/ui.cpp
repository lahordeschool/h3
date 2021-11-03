#include <h3/ui.h>

#include <imgui/imgui.h>

H3_CAPI bool H3_Ui_BeginWindow(const char* title)
{
	return ImGui::Begin(title);
}

H3_CAPI void H3_Ui_EndWindow()
{
	ImGui::End();
}

H3_CAPI void H3_Ui_Printf(const char* fmt, ...)
{
	va_list argp;
	va_start(argp, fmt);
	ImGui::TextV(fmt, argp);
	va_end(argp);
}

H3_CAPI bool H3_Ui_Button(const char* caption)
{
	return ImGui::Button(caption);
}

H3_CAPI bool H3_Ui_Checkbox(const char* caption, bool* b)
{
	return ImGui::Checkbox(caption, b);
}

H3_CAPI bool H3_Ui_EditFloat(const char* caption, float* v, float min, float max)
{
	return ImGui::SliderFloat(caption, v, min, max, "%.3f");
}

H3_CAPI bool H3_Ui_EditInt(const char* caption, int* v, int min, int max)
{
	return ImGui::SliderInt(caption, v, min, max);
}
