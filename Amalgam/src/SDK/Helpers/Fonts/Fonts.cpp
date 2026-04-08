#include "Fonts.h"
#include <cstring>
#include "../../Definitions/Interfaces/IVEngineClient.h"
#include <ImGui/imgui.h>

// Mapeia nome de fonte Surface → caminho de arquivo TTF do Windows
static const char* FontNameToPath(const char* szName)
{
	if (!strcmp(szName, "Verdana"))     return "C:\\Windows\\Fonts\\verdana.ttf";
	if (!strcmp(szName, "Small Fonts")) return "C:\\Windows\\Fonts\\SMSF.TTF";   // Small Fonts não tem TTF padrão; fallback
	if (!strcmp(szName, "Tahoma"))      return "C:\\Windows\\Fonts\\tahoma.ttf";
	return "C:\\Windows\\Fonts\\verdana.ttf"; // fallback
}

// Versão bold dos fonts que o Surface usa com weight >= 600
static const char* FontNameToPathBold(const char* szName)
{
	if (!strcmp(szName, "Verdana")) return "C:\\Windows\\Fonts\\verdanab.ttf";
	if (!strcmp(szName, "Tahoma"))  return "C:\\Windows\\Fonts\\tahomabd.ttf";
	return "C:\\Windows\\Fonts\\verdanab.ttf";
}

void CFonts::Reload(float flDPI)
{
	// ── Fontes base ──────────────────────────────────────────────────────────
	m_mFonts[FONT_ESP] = { "Verdana",     int(12.f * flDPI), FONTFLAG_OUTLINE,   0 };
	m_mFonts[FONT_ESP_CONDS] = { "Small Fonts", int(9.f * flDPI), FONTFLAG_OUTLINE,   0 };
	m_mFonts[FONT_ESP_SMALL] = { "Small Fonts", int(11.f * flDPI), FONTFLAG_OUTLINE,   0 };
	m_mFonts[FONT_INDICATORS] = { "Verdana",     int(13.f * flDPI), FONTFLAG_ANTIALIAS, 0 };
	m_mFonts[FONT_MENU] = { "Tahoma",      int(14.f * flDPI), FONTFLAG_OUTLINE,   700 };
	m_mFonts[FONT_VERDANA_BOLD] = { "Verdana",     int(12.f * flDPI), FONTFLAG_ANTIALIAS, 700 };
	m_mFonts[FONT_OTHER] = { "Tahoma",      int(12.f * flDPI), FONTFLAG_NONE,      400 };
	m_mFonts[FONT_PIXEL] = { "Small Fonts", int(8.f * flDPI), FONTFLAG_NONE,      0 };

	// ── Splash: usa GetScreenSize do engine para calcular tamanho relativo ───
	{
		int iScreenW = 1920, iScreenH = 1080;
		I::EngineClient->GetScreenSize(iScreenW, iScreenH);
		int iSplashSize = iScreenH > 0 ? iScreenH / 8 : 120;
		m_mFonts[FONT_SPLASH] = { "Verdana", int(iSplashSize * flDPI), FONTFLAG_OUTLINE, 700 };
	}

	// ── Registra todas as fontes na surface ───────────────────────────────────
	for (auto& [_, fFont] : m_mFonts)
	{
		if (fFont.m_dwFont = I::MatSystemSurface->CreateFont())
			I::MatSystemSurface->SetFontGlyphSet(
				fFont.m_dwFont,
				fFont.m_szName,
				fFont.m_nTall,
				fFont.m_nWeight,
				0, // blur
				0, // scanlines
				fFont.m_nFlags);
	}
}

const Font_t& CFonts::GetFont(EFonts eFont)
{
	return m_mFonts[eFont];
}

int CFonts::GetFontHeight(EFonts eFont) const
{
	return m_mFonts.at(eFont).m_nTall;
}

int Font_t::GetStringWidth(const char* szString) const
{
	if (!m_dwFont || !szString)
		return 0;

	int wide = 0, tall = 0;
	size_t len = strlen(szString) + 1;
	wchar_t* wszBuffer = new wchar_t[len];
	MultiByteToWideChar(CP_UTF8, 0, szString, -1, wszBuffer, (int)len);
	I::MatSystemSurface->GetTextSize(m_dwFont, wszBuffer, wide, tall);
	delete[] wszBuffer;
	return wide;
}

// ── ImGui font integration ────────────────────────────────────────────────────

void CFonts::ReloadImGui(ImGuiIO& io, float flDPI)
{
	m_mImFonts.clear();

	// Garante que a fonte default do ImGui sempre existe
	io.Fonts->AddFontDefault();

	// Para cada fonte Surface registrada, cria a equivalente ImGui
	// usando o mesmo nome de arquivo e tamanho (em pontos → pixels ImGui usa px direto)
	struct ImGuiFontSpec { EFonts eFont; const char* szName; int nTall; int nWeight; };

	const ImGuiFontSpec aSpecs[] =
	{
		{ FONT_ESP,          "Verdana",     int(12.f * flDPI), 400 },
		{ FONT_ESP_CONDS,    "Small Fonts", int(9.f * flDPI), 400 },
		{ FONT_ESP_SMALL,    "Small Fonts", int(11.f * flDPI), 400 },
		{ FONT_INDICATORS,   "Verdana",     int(13.f * flDPI), 400 },
		{ FONT_MENU,         "Tahoma",      int(14.f * flDPI), 700 },
		{ FONT_VERDANA_BOLD, "Verdana",     int(12.f * flDPI), 700 },
		{ FONT_OTHER,        "Tahoma",      int(12.f * flDPI), 400 },
		{ FONT_PIXEL,        "Small Fonts", int(8.f * flDPI), 400 },
	};

	ImFontConfig cfg;
	cfg.OversampleH = 2;
	cfg.OversampleV = 2;
	cfg.PixelSnapH = true;

	for (auto& spec : aSpecs)
	{
		const char* szPath = (spec.nWeight >= 600)
			? FontNameToPathBold(spec.szName)
			: FontNameToPath(spec.szName);

		// Small Fonts não tem TTF; usa default do ImGui com tamanho ajustado
		bool bSmall = !strcmp(spec.szName, "Small Fonts");

		ImFont* pFont = nullptr;
		if (!bSmall)
		{
			pFont = io.Fonts->AddFontFromFileTTF(szPath, (float)spec.nTall, &cfg);
		}

		// Se não carregou (arquivo ausente ou Small Fonts), usa default
		if (!pFont)
			pFont = io.Fonts->Fonts[0]; // default adicionado acima

		m_mImFonts[spec.eFont] = pFont;
	}

	// FONT_SPLASH: gerada de forma dinâmica conforme resolução — usa default
	m_mImFonts[FONT_SPLASH] = io.Fonts->Fonts[0];

	io.Fonts->Build();
}

ImFont* CFonts::GetImFont(EFonts eFont) const
{
	auto it = m_mImFonts.find(eFont);
	if (it != m_mImFonts.end() && it->second)
		return it->second;
	return ImGui::GetFont(); // fallback: fonte atual do ImGui
}