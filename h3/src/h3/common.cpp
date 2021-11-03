#include <h3/internal.h>

#include <imgui/imgui.h>
#include <iostream>

#ifndef NDEBUG
void H3_ASSERT(bool b, const char* msg)
{
#ifdef H3_USE_IMGUI_ASSERT
	if (!b)
		H3Internal_SetAssertMessage(std::string(msg));
#else /* H3_USE_IMGUI_ASSERT */
	H3_ASSERT_CONSOLE(b, msg);
#endif /* H3_USE_IMGUI_ASSERT */
}

void H3_ASSERT_CONSOLE(bool b, const char* msg)
{
	if (!b)
	{
		std::cerr << "ASSERTION FAILED: " << msg << std::endl;
		__debugbreak();
	}
}
#endif // !NDEBUG
