#include <h3/internal.h>
#include <imgui/imgui.h>

std::string gAssertMessage = "";

void H3Internal_SetAssertMessage(const std::string& msg)
{
	gAssertMessage = msg;
}

const std::string& H3Internal_GetAssertMessage()
{
	return gAssertMessage;
}

void H3Internal_HandleAssertPopup()
{

	if (ImGui::BeginPopupModal("ASSERTION FAILED"))
	{
		ImGui::Text(gAssertMessage.c_str());
		ImGui::Separator();
		if (ImGui::Button("Ok"))
		{
			H3Internal_SetAssertMessage("");
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
	else if (!gAssertMessage.empty())
		ImGui::OpenPopup("ASSERTION FAILED");
}
