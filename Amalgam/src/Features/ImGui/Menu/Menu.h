#pragma once
#include "../../../SDK/SDK.h"
#include "../Render.h"
#include <ImGui/TextEditor.h>
#include <mutex>

struct Output_t
{
	std::string m_sFunction;
	std::string m_sLog;
	size_t m_iID;

	Color_t tAccent;
};

class CMenu
{
private:
	void DrawMenu();

	std::deque<Output_t> m_vOutput = {};
	size_t m_iMaxOutputSize = 1000;

public:
	void Render();
	void AddOutput(const char* sFunction, const char* sLog, Color_t tColor = Vars::Menu::Theme::Accent.Value);
	void SetIconTexture(IDirect3DTexture9* pTexture);
	void LoadIconTexture(LPDIRECT3DDEVICE9 pDevice);

	bool m_bIsOpen = false;
	bool m_bInKeybind = false;
	bool m_bWindowHovered = false;

	std::mutex m_tMutex;
};

ADD_FEATURE(CMenu, Menu);