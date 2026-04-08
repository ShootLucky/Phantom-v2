#pragma once
#include "../../../Utils/Macros/Macros.h"
#include "../../Definitions/Interfaces/IMatSystemSurface.h"
#include <ImGui/imgui.h>
#include <unordered_map>
#include <windows.h>

enum EFonts
{
	FONT_ESP,
	FONT_ESP_CONDS,
	FONT_ESP_SMALL,
	FONT_INDICATORS,
	FONT_MENU,
	FONT_VERDANA_BOLD,
	FONT_OTHER,
	FONT_SPLASH,
	FONT_PIXEL,
};

struct Font_t
{
	const char* m_szName = "";
	int           m_nTall = 12;
	int           m_nFlags = 0;
	int           m_nWeight = 0;
	unsigned long m_dwFont = 0;

	int GetStringWidth(const char* szString) const;
};

class CFonts
{
private:
	std::unordered_map<EFonts, Font_t>   m_mFonts = {};
	std::unordered_map<EFonts, ImFont*>  m_mImFonts = {};

public:
	void          Reload(float flDPI = 1.f);
	const Font_t& GetFont(EFonts eFont);
	int           GetFontHeight(EFonts eFont) const;

	// Retorna o ImFont* correspondente; fallback = ImGui::GetFont()
	ImFont* GetImFont(EFonts eFont) const;

	// Carrega as fontes equivalentes no ImGui com os mesmos specs do Surface
	// Deve ser chamado ANTES de ImGui::NewFrame(), durante o init do ImGui
	void          ReloadImGui(ImGuiIO& io, float flDPI = 1.f);
};

ADD_FEATURE_CUSTOM(CFonts, Fonts, H);