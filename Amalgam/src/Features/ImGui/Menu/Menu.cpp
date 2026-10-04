#include "Menu.h"

#include "Components.h"
#include "../../Configs/Configs.h"
#include "../../Binds/Binds.h"
#include "../../Visuals/Groups/Groups.h"
#include "../../Players/PlayerUtils.h"
#include "../../Spectate/Spectate.h"
#include "../../Resolver/Resolver.h"
#include "../../Visuals/Visuals.h"
#include "../../Misc/Misc.h"
#include "../../Output/Output.h"
#include "../nemesis.h"
#include "icons.h"
#include "stb_image.h"

// ── Variáveis estáticas do menu ─────────────────────────────────────────────
static std::vector<std::string> s_ConfigList;
static int                      s_SelectedConfig = 0;
static char                     s_NewConfigName[128] = "";
static IDirect3DTexture9* s_IconTexture = nullptr;
static bool                     s_IconLoaded = false;

// ── Helpers internos ────────────────────────────────────────────────────────
static ImU32 AccentColor()
{
	Color_t c = Vars::Menu::Theme::Accent.Value;
	return IM_COL32(c.r, c.g, c.b, c.a);
}

static IDirect3DTexture9* LoadTextureFromMemory(LPDIRECT3DDEVICE9 pDevice, const unsigned char* data, int data_size)
{
	int w, h, ch;
	unsigned char* img = stbi_load_from_memory(data, data_size, &w, &h, &ch, 4);
	if (!img) return nullptr;

	IDirect3DTexture9* tex = nullptr;
	if (SUCCEEDED(pDevice->CreateTexture(w, h, 1, D3DUSAGE_DYNAMIC, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &tex, nullptr)))
	{
		D3DLOCKED_RECT rc;
		if (SUCCEEDED(tex->LockRect(0, &rc, nullptr, 0)))
		{
			for (int y = 0; y < h; ++y)
				memcpy((uint8_t*)rc.pBits + rc.Pitch * y, img + w * 4 * y, w * 4);
			tex->UnlockRect(0);
		}
	}
	stbi_image_free(img);
	return tex;
}

// ── Acessa Map[DEFAULT_BIND] diretamente para evitar reset pelo bind system ─
#define VAR(x)  (x)[-1]
#define VARP(x) (&(x)[-1])

// ── Checkbox para variável de bitmask — só inverte quando o usuário clicou ─
// gui::checkbox modifica bool& in-place; comparamos antes/depois para detectar clique
#define BITMASK_CHECKBOX(label, bitmask_var, bit)           \
    {                                                        \
        bool _cur = (VAR(bitmask_var) & (1 << (bit))) != 0; \
        bool _prev = _cur;                                   \
        gui::checkbox(label, _cur);                          \
        if (_cur != _prev)                                   \
            VAR(bitmask_var) ^= (1 << (bit));                \
    }

// ────────────────────────────────────────────────────────────────────────────
void CMenu::DrawMenu()
{
	using namespace ImGui;

	static int active_tab = 0;

	ImU32 accent_color = AccentColor();

	SetNextWindowSize(ImVec2(800, 600));
	Begin("SLwindow", nullptr,
		ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar |
		ImGuiWindowFlags_NoScrollWithMouse);
	{
		ImDrawList* draw_list = GetWindowDrawList();
		ImVec2 p = GetWindowPos();
		ImVec2 s = GetWindowSize();

		// ── Header gradient ──────────────────────────────────────────────────
		draw_list->AddRectFilledMultiColor(
			p, ImVec2(p.x + s.x, p.y + 60),
			IM_COL32(35, 35, 35, 255), IM_COL32(35, 35, 35, 255),
			IM_COL32(22, 22, 22, 255), IM_COL32(20, 20, 20, 255));

		// ── Fundo content area ───────────────────────────────────────────────
		draw_list->AddRectFilled(
			ImVec2(p.x + 1, p.y + 60),
			ImVec2(p.x + s.x - 1, p.y + s.y - 1),
			IM_COL32(22, 22, 22, 255));

		// ── Símbolos animados ────────────────────────────────────────────────
		{
			const char symbolSet[] = "$#&%*+-=~<>/\\|@?";
			ImFont* bg_font = GetFont();
			float   bg_font_size = GetFontSize() * 1.2f;
			ImU32   bg_char_color = IM_COL32(65, 65, 65, 160);
			float   char_spacing = 28.0f;
			float   row_spacing = 32.0f;
			int     char_offset = static_cast<int>(GetTime() * 0.5f) % (int)(sizeof(symbolSet) - 1);
			int     char_idx = 0;

			for (float y = p.y + 60 + 8; y < p.y + s.y; y += row_spacing)
			{
				for (float x = p.x + 12; x < p.x + s.x; x += char_spacing)
				{
					char ch = symbolSet[(char_idx + char_offset) % (int)(sizeof(symbolSet) - 1)];
					char buf[2] = { ch, '\0' };
					draw_list->AddText(bg_font, bg_font_size, ImVec2(x, y), bg_char_color, buf);
					char_idx++;
				}
			}
		}

		// ── Linhas decorativas ───────────────────────────────────────────────
		draw_list->AddRectFilled(ImVec2(p.x + 1, p.y + 1), ImVec2(p.x + s.x - 1, p.y + 3), accent_color);
		draw_list->AddRectFilled(ImVec2(p.x + 1, p.y + 59), ImVec2(p.x + s.x - 1, p.y + 60), IM_COL32(64, 64, 64, 255));
		draw_list->AddRect(p, ImVec2(p.x + s.x, p.y + s.y), IM_COL32(40, 40, 40, 255));

		// ── Ícone + título ───────────────────────────────────────────────────
		if (s_IconTexture)
		{
			ImVec4 tint_color = ImVec4(
				((accent_color >> 0) & 0xFF) / 255.f,
				((accent_color >> 8) & 0xFF) / 255.f,
				((accent_color >> 16) & 0xFF) / 255.f, 1.0f);

			ImVec2 icon_pos(p.x - 4, p.y - 2);
			draw_list->AddImage(
				(ImTextureID)(intptr_t)s_IconTexture,
				icon_pos,
				ImVec2(icon_pos.x + 78, icon_pos.y + 68),
				ImVec2(0, 0), ImVec2(1, 1),   // UV normal
				ColorConvertFloat4ToU32(tint_color));

			float icon_end = p.x + 60;
			PushFont(GetFont());
			SetWindowFontScale(1.5f);
			draw_list->AddText(GetFont(), GetFontSize(), ImVec2(icon_end + 1, p.y + 16), IM_COL32(5, 5, 5, 255), "HANTOM.CLUB");
			draw_list->AddText(GetFont(), GetFontSize(), ImVec2(icon_end, p.y + 15), accent_color, "HANTOM.CLUB");
			SetWindowFontScale(1.0f);
			PopFont();
		}
		else
		{
			PushFont(GetFont());
			SetWindowFontScale(1.5f);
			draw_list->AddText(GetFont(), GetFontSize(), ImVec2(p.x - 24, p.y + 21), IM_COL32(5, 5, 5, 255), "HANTOM.CLUB");
			draw_list->AddText(GetFont(), GetFontSize(), ImVec2(p.x - 25, p.y + 20), accent_color, "HANTOM.CLUB");
			SetWindowFontScale(1.0f);
			PopFont();
		}

		// ── Subtítulo "DEVELOPED BY SHOOT & VOID" ───────────────────────────
		draw_list->AddText(ImVec2(p.x + 60, p.y + 33), IM_COL32(5, 5, 5, 255), "DEVELOPED BY");
		draw_list->AddText(ImVec2(p.x + 61, p.y + 32), IM_COL32(255, 255, 255, 100), "DEVELOPED BY");
		float dev_width = CalcTextSize("DEVELOPED BY").x;
		draw_list->AddText(ImVec2(p.x + 60 + dev_width + 5, p.y + 33), IM_COL32(5, 5, 5, 255), "SHOOT & STAR.K");
		draw_list->AddText(ImVec2(p.x + 61 + dev_width + 4, p.y + 32), accent_color, "SHOOT & STAR.K");

		// ── Tabs ─────────────────────────────────────────────────────────────
		SetCursorPosX(430);
		SetCursorPosY(25);
		BeginGroup();
		{
			if (gui::TabButton("Aimbot", active_tab, 0)) {}
			ImVec2 ab_min = GetItemRectMin(), ab_max = GetItemRectMax();
			if (IsMouseHoveringRect(ab_min, ab_max) && IsMouseClicked(1))
				gui::g_aimbot_mode_state.context_open = true;
			gui::aimbot_mode_selector(&gui::g_aimbot_mode_state.selected_mode);

			SameLine();
			gui::TabButton("Anti-Aim", active_tab, 1);

			SameLine();
			if (gui::TabButton("Visuals", active_tab, 2)) {}
			ImVec2 vis_min = GetItemRectMin(), vis_max = GetItemRectMax();
			if (IsMouseHoveringRect(vis_min, vis_max) && IsMouseClicked(1))
				gui::g_visuals_mode_state.context_open = true;
			gui::visuals_mode_selector(&gui::g_visuals_mode_state.selected_mode);

			SameLine();
			gui::TabButton("Players", active_tab, 3);
			SameLine();
			gui::TabButton("Misc", active_tab, 4);
		}
		EndGroup();

		NewLine();
		NewLine();

		// ── Animação de troca de tab ─────────────────────────────────────────
		static int   s_prev_tab = active_tab;
		static float s_tab_anim = 1.0f; // 0=oculto, 1=revelado
		if (active_tab != s_prev_tab) {
			s_prev_tab = active_tab;
			s_tab_anim = 0.0f;
		}
		// interpola rápido: ~12 frames para completar
		s_tab_anim += (1.0f - s_tab_anim) * 0.22f;
		if (s_tab_anim > 0.99f) s_tab_anim = 1.0f;

		// ── Conteúdo principal ───────────────────────────────────────────────
		SetCursorPosX(15);
		BeginGroup();
		{
			// Aplica clipping da esquerda para a direita proporcional à animação
			{
				ImVec2 cp = GetCursorScreenPos();
				float  full = s.x - 15.f;          // largura total do conteúdo
				float  clip = full * s_tab_anim;   // porção revelada
				ImGui::PushClipRect(
					ImVec2(p.x, p.y + 60),
					ImVec2(p.x + 15.f + clip, p.y + s.y),
					true);
			}

			switch (active_tab)
			{

				// ─────────────────────────────────────────────────────────────────
			case 0: // AIMBOT
				// ─────────────────────────────────────────────────────────────────
			{
				int aim_mode = gui::g_aimbot_mode_state.selected_mode;

				// ── GERAL: comum a todos os modos ────────────────────────────────
				// Panel de configurações gerais (targets, ignore) sempre visível lado direito
				// Panel específico do modo à esquerda

				// ══════════════════════════════════════
				// SUB-TAB 0 : HITSCAN
				// ══════════════════════════════════════
				if (aim_mode == 0)
				{
					static const char* aim_types[] = { "Off", "Plain", "Smooth", "Silent", "Locking", "Assistive" };
					static const char* target_sel[] = { "FOV", "Distance", "Hybrid" };

					// LEFT: Hitscan settings
					if (gui::begin_group_scrollable("HITSCAN", ImVec2(380, 490), 5.f, 0.f))
					{
						// ── Linha 1: checkbox "Aimbot" + keybind alinhado à direita ──
						{
							bool bEnabled = VAR(Vars::Aimbot::General::AimType) != 0;
							bool bPrev = bEnabled;
							gui::checkbox("Aimbot", bEnabled);
							if (bEnabled != bPrev)
								VAR(Vars::Aimbot::General::AimType) = bEnabled ? 1 : 0;

							ImGui::SameLine(360.f);
							{ gui::keybind("##key_hitscan", &VAR(Vars::Aimbot::Hitscan::Key), &VAR(Vars::Aimbot::Hitscan::KeyType)); }
						}

						if (VAR(Vars::Aimbot::General::AimType) != 0)
						{
							gui::slider("FOV", VARP(Vars::Aimbot::General::AimFOV), 0.f, 180.f);
							gui::combo("Aim Type", VARP(Vars::Aimbot::General::AimType),
								aim_types, IM_ARRAYSIZE(aim_types));
							gui::checkbox("Auto Shoot", VAR(Vars::Aimbot::General::AutoShoot));
							gui::checkbox("FOV Circle", VAR(Vars::Aimbot::General::FOVCircle));
							gui::checkbox("No Spread", VAR(Vars::Aimbot::General::NoSpread));

							Spacing();
							gui::combo("Sort", VARP(Vars::Aimbot::General::TargetSelection),
								target_sel, IM_ARRAYSIZE(target_sel));
							gui::slider("Max Targets", (float*)VARP(Vars::Aimbot::General::MaxTargets), 1.f, 6.f);
							gui::slider("Assist Strength", VARP(Vars::Aimbot::General::AssistStrength), 0.f, 100.f);

							Spacing();

							// Hitboxes bitmask
							{
								static bool _b0 = (VAR(Vars::Aimbot::Hitscan::Hitboxes) & (1 << 0)) != 0;
								static bool _b1 = (VAR(Vars::Aimbot::Hitscan::Hitboxes) & (1 << 1)) != 0;
								static bool _b2 = (VAR(Vars::Aimbot::Hitscan::Hitboxes) & (1 << 2)) != 0;
								static bool _b3 = (VAR(Vars::Aimbot::Hitscan::Hitboxes) & (1 << 3)) != 0;
								static bool _b4 = (VAR(Vars::Aimbot::Hitscan::Hitboxes) & (1 << 4)) != 0;
								static bool _b5 = (VAR(Vars::Aimbot::Hitscan::Hitboxes) & (1 << 5)) != 0;
								static bool _b6 = (VAR(Vars::Aimbot::Hitscan::Hitboxes) & (1 << 6)) != 0;
								_b0 = (VAR(Vars::Aimbot::Hitscan::Hitboxes) & (1 << 0)) != 0;
								_b1 = (VAR(Vars::Aimbot::Hitscan::Hitboxes) & (1 << 1)) != 0;
								_b2 = (VAR(Vars::Aimbot::Hitscan::Hitboxes) & (1 << 2)) != 0;
								_b3 = (VAR(Vars::Aimbot::Hitscan::Hitboxes) & (1 << 3)) != 0;
								_b4 = (VAR(Vars::Aimbot::Hitscan::Hitboxes) & (1 << 4)) != 0;
								_b5 = (VAR(Vars::Aimbot::Hitscan::Hitboxes) & (1 << 5)) != 0;
								_b6 = (VAR(Vars::Aimbot::Hitscan::Hitboxes) & (1 << 6)) != 0;
								std::vector<gui::MultiComboItem> _items_Vars_Aimbot_Hitscan_Hitboxes = {
									{ "Head", &_b0 },
									{ "Body", &_b1 },
									{ "Pelvis", &_b2 },
									{ "Arms", &_b3 },
									{ "Legs", &_b4 },
									{ "Bodyaim If Lethal", &_b5 },
									{ "Headshot Only", &_b6 },
								};
								if (gui::multi_combo("Hitboxes", _items_Vars_Aimbot_Hitscan_Hitboxes, 350.f))
								{
									VAR(Vars::Aimbot::Hitscan::Hitboxes) = (_b0 ? VAR(Vars::Aimbot::Hitscan::Hitboxes) | (1 << 0) : VAR(Vars::Aimbot::Hitscan::Hitboxes) & ~(1 << 0));
									VAR(Vars::Aimbot::Hitscan::Hitboxes) = (_b1 ? VAR(Vars::Aimbot::Hitscan::Hitboxes) | (1 << 1) : VAR(Vars::Aimbot::Hitscan::Hitboxes) & ~(1 << 1));
									VAR(Vars::Aimbot::Hitscan::Hitboxes) = (_b2 ? VAR(Vars::Aimbot::Hitscan::Hitboxes) | (1 << 2) : VAR(Vars::Aimbot::Hitscan::Hitboxes) & ~(1 << 2));
									VAR(Vars::Aimbot::Hitscan::Hitboxes) = (_b3 ? VAR(Vars::Aimbot::Hitscan::Hitboxes) | (1 << 3) : VAR(Vars::Aimbot::Hitscan::Hitboxes) & ~(1 << 3));
									VAR(Vars::Aimbot::Hitscan::Hitboxes) = (_b4 ? VAR(Vars::Aimbot::Hitscan::Hitboxes) | (1 << 4) : VAR(Vars::Aimbot::Hitscan::Hitboxes) & ~(1 << 4));
									VAR(Vars::Aimbot::Hitscan::Hitboxes) = (_b5 ? VAR(Vars::Aimbot::Hitscan::Hitboxes) | (1 << 5) : VAR(Vars::Aimbot::Hitscan::Hitboxes) & ~(1 << 5));
									VAR(Vars::Aimbot::Hitscan::Hitboxes) = (_b6 ? VAR(Vars::Aimbot::Hitscan::Hitboxes) | (1 << 6) : VAR(Vars::Aimbot::Hitscan::Hitboxes) & ~(1 << 6));
								}
							}

							Spacing();

							// Modifiers bitmask
							{
								static bool _b0 = (VAR(Vars::Aimbot::Hitscan::Modifiers) & (1 << 0)) != 0;
								static bool _b1 = (VAR(Vars::Aimbot::Hitscan::Modifiers) & (1 << 1)) != 0;
								static bool _b2 = (VAR(Vars::Aimbot::Hitscan::Modifiers) & (1 << 2)) != 0;
								static bool _b3 = (VAR(Vars::Aimbot::Hitscan::Modifiers) & (1 << 3)) != 0;
								static bool _b4 = (VAR(Vars::Aimbot::Hitscan::Modifiers) & (1 << 4)) != 0;
								static bool _b5 = (VAR(Vars::Aimbot::Hitscan::Modifiers) & (1 << 5)) != 0;
								static bool _b6 = (VAR(Vars::Aimbot::Hitscan::Modifiers) & (1 << 6)) != 0;
								_b0 = (VAR(Vars::Aimbot::Hitscan::Modifiers) & (1 << 0)) != 0;
								_b1 = (VAR(Vars::Aimbot::Hitscan::Modifiers) & (1 << 1)) != 0;
								_b2 = (VAR(Vars::Aimbot::Hitscan::Modifiers) & (1 << 2)) != 0;
								_b3 = (VAR(Vars::Aimbot::Hitscan::Modifiers) & (1 << 3)) != 0;
								_b4 = (VAR(Vars::Aimbot::Hitscan::Modifiers) & (1 << 4)) != 0;
								_b5 = (VAR(Vars::Aimbot::Hitscan::Modifiers) & (1 << 5)) != 0;
								_b6 = (VAR(Vars::Aimbot::Hitscan::Modifiers) & (1 << 6)) != 0;
								std::vector<gui::MultiComboItem> _items_Vars_Aimbot_Hitscan_Modifiers = {
									{ "Tapfire", &_b0 },
									{ "Wait for Headshot", &_b1 },
									{ "Wait for Charge", &_b2 },
									{ "Scoped Only", &_b3 },
									{ "Auto Scope", &_b4 },
									{ "Auto Rev Minigun", &_b5 },
									{ "Extinguish Team", &_b6 },
								};
								if (gui::multi_combo("Modifiers", _items_Vars_Aimbot_Hitscan_Modifiers, 350.f))
								{
									VAR(Vars::Aimbot::Hitscan::Modifiers) = (_b0 ? VAR(Vars::Aimbot::Hitscan::Modifiers) | (1 << 0) : VAR(Vars::Aimbot::Hitscan::Modifiers) & ~(1 << 0));
									VAR(Vars::Aimbot::Hitscan::Modifiers) = (_b1 ? VAR(Vars::Aimbot::Hitscan::Modifiers) | (1 << 1) : VAR(Vars::Aimbot::Hitscan::Modifiers) & ~(1 << 1));
									VAR(Vars::Aimbot::Hitscan::Modifiers) = (_b2 ? VAR(Vars::Aimbot::Hitscan::Modifiers) | (1 << 2) : VAR(Vars::Aimbot::Hitscan::Modifiers) & ~(1 << 2));
									VAR(Vars::Aimbot::Hitscan::Modifiers) = (_b3 ? VAR(Vars::Aimbot::Hitscan::Modifiers) | (1 << 3) : VAR(Vars::Aimbot::Hitscan::Modifiers) & ~(1 << 3));
									VAR(Vars::Aimbot::Hitscan::Modifiers) = (_b4 ? VAR(Vars::Aimbot::Hitscan::Modifiers) | (1 << 4) : VAR(Vars::Aimbot::Hitscan::Modifiers) & ~(1 << 4));
									VAR(Vars::Aimbot::Hitscan::Modifiers) = (_b5 ? VAR(Vars::Aimbot::Hitscan::Modifiers) | (1 << 5) : VAR(Vars::Aimbot::Hitscan::Modifiers) & ~(1 << 5));
									VAR(Vars::Aimbot::Hitscan::Modifiers) = (_b6 ? VAR(Vars::Aimbot::Hitscan::Modifiers) | (1 << 6) : VAR(Vars::Aimbot::Hitscan::Modifiers) & ~(1 << 6));
								}
							}

							Spacing();
							gui::slider("Multipoint Scale", VARP(Vars::Aimbot::Hitscan::MultipointScale), 0.f, 100.f);
							gui::slider("Tapfire Distance", VARP(Vars::Aimbot::Hitscan::TapfireDistance), 250.f, 1000.f);
						}
					}
					gui::end_group_scrollable();

					// RIGHT: Targets & Ignore
					SameLine(390);
					if (gui::begin_group_scrollable("TARGETS", ImVec2(380, 490), 5.f, 5.f))
					{
						// Target types bitmask
						{
							static bool _b0 = (VAR(Vars::Aimbot::General::Target) & (1 << 0)) != 0;
							static bool _b1 = (VAR(Vars::Aimbot::General::Target) & (1 << 1)) != 0;
							static bool _b2 = (VAR(Vars::Aimbot::General::Target) & (1 << 2)) != 0;
							static bool _b3 = (VAR(Vars::Aimbot::General::Target) & (1 << 3)) != 0;
							static bool _b4 = (VAR(Vars::Aimbot::General::Target) & (1 << 4)) != 0;
							static bool _b5 = (VAR(Vars::Aimbot::General::Target) & (1 << 5)) != 0;
							static bool _b6 = (VAR(Vars::Aimbot::General::Target) & (1 << 6)) != 0;
							_b0 = (VAR(Vars::Aimbot::General::Target) & (1 << 0)) != 0;
							_b1 = (VAR(Vars::Aimbot::General::Target) & (1 << 1)) != 0;
							_b2 = (VAR(Vars::Aimbot::General::Target) & (1 << 2)) != 0;
							_b3 = (VAR(Vars::Aimbot::General::Target) & (1 << 3)) != 0;
							_b4 = (VAR(Vars::Aimbot::General::Target) & (1 << 4)) != 0;
							_b5 = (VAR(Vars::Aimbot::General::Target) & (1 << 5)) != 0;
							_b6 = (VAR(Vars::Aimbot::General::Target) & (1 << 6)) != 0;
							std::vector<gui::MultiComboItem> _items_Vars_Aimbot_General_Target = {
								{ "Players", &_b0 },
								{ "Sentries", &_b1 },
								{ "Dispensers", &_b2 },
								{ "Teleporters", &_b3 },
								{ "Stickies", &_b4 },
								{ "NPCs", &_b5 },
								{ "Bombs", &_b6 },
							};
							if (gui::multi_combo("Targets", _items_Vars_Aimbot_General_Target, 350.f))
							{
								VAR(Vars::Aimbot::General::Target) = (_b0 ? VAR(Vars::Aimbot::General::Target) | (1 << 0) : VAR(Vars::Aimbot::General::Target) & ~(1 << 0));
								VAR(Vars::Aimbot::General::Target) = (_b1 ? VAR(Vars::Aimbot::General::Target) | (1 << 1) : VAR(Vars::Aimbot::General::Target) & ~(1 << 1));
								VAR(Vars::Aimbot::General::Target) = (_b2 ? VAR(Vars::Aimbot::General::Target) | (1 << 2) : VAR(Vars::Aimbot::General::Target) & ~(1 << 2));
								VAR(Vars::Aimbot::General::Target) = (_b3 ? VAR(Vars::Aimbot::General::Target) | (1 << 3) : VAR(Vars::Aimbot::General::Target) & ~(1 << 3));
								VAR(Vars::Aimbot::General::Target) = (_b4 ? VAR(Vars::Aimbot::General::Target) | (1 << 4) : VAR(Vars::Aimbot::General::Target) & ~(1 << 4));
								VAR(Vars::Aimbot::General::Target) = (_b5 ? VAR(Vars::Aimbot::General::Target) | (1 << 5) : VAR(Vars::Aimbot::General::Target) & ~(1 << 5));
								VAR(Vars::Aimbot::General::Target) = (_b6 ? VAR(Vars::Aimbot::General::Target) | (1 << 6) : VAR(Vars::Aimbot::General::Target) & ~(1 << 6));
							}
						}

						Spacing();

						// Ignore bitmask
						{
							static bool _b0 = (VAR(Vars::Aimbot::General::Ignore) & (1 << 0)) != 0;
							static bool _b1 = (VAR(Vars::Aimbot::General::Ignore) & (1 << 1)) != 0;
							static bool _b2 = (VAR(Vars::Aimbot::General::Ignore) & (1 << 3)) != 0;
							static bool _b3 = (VAR(Vars::Aimbot::General::Ignore) & (1 << 4)) != 0;
							static bool _b4 = (VAR(Vars::Aimbot::General::Ignore) & (1 << 6)) != 0;
							static bool _b5 = (VAR(Vars::Aimbot::General::Ignore) & (1 << 8)) != 0;
							static bool _b6 = (VAR(Vars::Aimbot::General::Ignore) & (1 << 9)) != 0;
							static bool _b7 = (VAR(Vars::Aimbot::General::Ignore) & (1 << 10)) != 0;
							_b0 = (VAR(Vars::Aimbot::General::Ignore) & (1 << 0)) != 0;
							_b1 = (VAR(Vars::Aimbot::General::Ignore) & (1 << 1)) != 0;
							_b2 = (VAR(Vars::Aimbot::General::Ignore) & (1 << 3)) != 0;
							_b3 = (VAR(Vars::Aimbot::General::Ignore) & (1 << 4)) != 0;
							_b4 = (VAR(Vars::Aimbot::General::Ignore) & (1 << 6)) != 0;
							_b5 = (VAR(Vars::Aimbot::General::Ignore) & (1 << 8)) != 0;
							_b6 = (VAR(Vars::Aimbot::General::Ignore) & (1 << 9)) != 0;
							_b7 = (VAR(Vars::Aimbot::General::Ignore) & (1 << 10)) != 0;
							std::vector<gui::MultiComboItem> _items_Vars_Aimbot_General_Ignore = {
								{ "Friends", &_b0 },
								{ "Party", &_b1 },
								{ "Invulnerable", &_b2 },
								{ "Invisible", &_b3 },
								{ "Dead Ringer", &_b4 },
								{ "Disguised", &_b5 },
								{ "Taunting", &_b6 },
								{ "Team", &_b7 },
							};
							if (gui::multi_combo("Ignore", _items_Vars_Aimbot_General_Ignore, 350.f))
							{
								VAR(Vars::Aimbot::General::Ignore) = (_b0 ? VAR(Vars::Aimbot::General::Ignore) | (1 << 0) : VAR(Vars::Aimbot::General::Ignore) & ~(1 << 0));
								VAR(Vars::Aimbot::General::Ignore) = (_b1 ? VAR(Vars::Aimbot::General::Ignore) | (1 << 1) : VAR(Vars::Aimbot::General::Ignore) & ~(1 << 1));
								VAR(Vars::Aimbot::General::Ignore) = (_b2 ? VAR(Vars::Aimbot::General::Ignore) | (1 << 3) : VAR(Vars::Aimbot::General::Ignore) & ~(1 << 3));
								VAR(Vars::Aimbot::General::Ignore) = (_b3 ? VAR(Vars::Aimbot::General::Ignore) | (1 << 4) : VAR(Vars::Aimbot::General::Ignore) & ~(1 << 4));
								VAR(Vars::Aimbot::General::Ignore) = (_b4 ? VAR(Vars::Aimbot::General::Ignore) | (1 << 6) : VAR(Vars::Aimbot::General::Ignore) & ~(1 << 6));
								VAR(Vars::Aimbot::General::Ignore) = (_b5 ? VAR(Vars::Aimbot::General::Ignore) | (1 << 8) : VAR(Vars::Aimbot::General::Ignore) & ~(1 << 8));
								VAR(Vars::Aimbot::General::Ignore) = (_b6 ? VAR(Vars::Aimbot::General::Ignore) | (1 << 9) : VAR(Vars::Aimbot::General::Ignore) & ~(1 << 9));
								VAR(Vars::Aimbot::General::Ignore) = (_b7 ? VAR(Vars::Aimbot::General::Ignore) | (1 << 10) : VAR(Vars::Aimbot::General::Ignore) & ~(1 << 10));
							}
						}

						Spacing();
						gui::slider("Invisible Threshold", VARP(Vars::Aimbot::General::IgnoreInvisible), 0.f, 100.f);
					}
					gui::end_group_scrollable();
				}

				// ══════════════════════════════════════
				// SUB-TAB 1 : PROJECTILE
				// ══════════════════════════════════════
				else if (aim_mode == 1)
				{
					static const char* aim_types[] = { "Off", "Plain", "Smooth", "Silent", "Locking", "Assistive" };
					static const char* target_sel[] = { "FOV", "Distance", "Hybrid" };
					static const char* splash_pred[] = { "Off", "Include", "Prefer", "Only" };

					// LEFT: Projectile settings
					if (gui::begin_group_scrollable("PROJECTILE", ImVec2(380, 490), 5.f, 0.f))
					{
						// ── Linha 1: checkbox "Aimbot" + keybind alinhado à direita ──
						{
							bool bEnabled = VAR(Vars::Aimbot::General::AimType) != 0;
							bool bPrev = bEnabled;
							gui::checkbox("Aimbot", bEnabled);
							if (bEnabled != bPrev)
								VAR(Vars::Aimbot::General::AimType) = bEnabled ? 1 : 0;

							ImGui::SameLine(360.f);
							{ gui::keybind("##key_projectile", &VAR(Vars::Aimbot::Projectile::Key), &VAR(Vars::Aimbot::Projectile::KeyType)); }
						}

						if (VAR(Vars::Aimbot::General::AimType) != 0)
						{
							gui::slider("FOV", VARP(Vars::Aimbot::General::AimFOV), 0.f, 180.f);
							gui::combo("Aim Type", VARP(Vars::Aimbot::General::AimType),
								aim_types, IM_ARRAYSIZE(aim_types));
							gui::checkbox("Auto Shoot", VAR(Vars::Aimbot::General::AutoShoot));
							gui::checkbox("FOV Circle", VAR(Vars::Aimbot::General::FOVCircle));

							Spacing();
							gui::combo("Sort", VARP(Vars::Aimbot::General::TargetSelection),
								target_sel, IM_ARRAYSIZE(target_sel));
							gui::slider("Max Targets", (float*)VARP(Vars::Aimbot::General::MaxTargets), 1.f, 6.f);

							Spacing();

							// Hitboxes
							{
								static bool _b0 = (VAR(Vars::Aimbot::Projectile::Hitboxes) & (1 << 0)) != 0;
								static bool _b1 = (VAR(Vars::Aimbot::Projectile::Hitboxes) & (1 << 1)) != 0;
								static bool _b2 = (VAR(Vars::Aimbot::Projectile::Hitboxes) & (1 << 2)) != 0;
								static bool _b3 = (VAR(Vars::Aimbot::Projectile::Hitboxes) & (1 << 3)) != 0;
								static bool _b4 = (VAR(Vars::Aimbot::Projectile::Hitboxes) & (1 << 4)) != 0;
								static bool _b5 = (VAR(Vars::Aimbot::Projectile::Hitboxes) & (1 << 5)) != 0;
								_b0 = (VAR(Vars::Aimbot::Projectile::Hitboxes) & (1 << 0)) != 0;
								_b1 = (VAR(Vars::Aimbot::Projectile::Hitboxes) & (1 << 1)) != 0;
								_b2 = (VAR(Vars::Aimbot::Projectile::Hitboxes) & (1 << 2)) != 0;
								_b3 = (VAR(Vars::Aimbot::Projectile::Hitboxes) & (1 << 3)) != 0;
								_b4 = (VAR(Vars::Aimbot::Projectile::Hitboxes) & (1 << 4)) != 0;
								_b5 = (VAR(Vars::Aimbot::Projectile::Hitboxes) & (1 << 5)) != 0;
								std::vector<gui::MultiComboItem> _items_Vars_Aimbot_Projectile_Hitboxes = {
									{ "Auto", &_b0 },
									{ "Head", &_b1 },
									{ "Body", &_b2 },
									{ "Feet", &_b3 },
									{ "Bodyaim If Lethal", &_b4 },
									{ "Prioritize Feet", &_b5 },
								};
								if (gui::multi_combo("Hitboxes", _items_Vars_Aimbot_Projectile_Hitboxes, 350.f))
								{
									VAR(Vars::Aimbot::Projectile::Hitboxes) = (_b0 ? VAR(Vars::Aimbot::Projectile::Hitboxes) | (1 << 0) : VAR(Vars::Aimbot::Projectile::Hitboxes) & ~(1 << 0));
									VAR(Vars::Aimbot::Projectile::Hitboxes) = (_b1 ? VAR(Vars::Aimbot::Projectile::Hitboxes) | (1 << 1) : VAR(Vars::Aimbot::Projectile::Hitboxes) & ~(1 << 1));
									VAR(Vars::Aimbot::Projectile::Hitboxes) = (_b2 ? VAR(Vars::Aimbot::Projectile::Hitboxes) | (1 << 2) : VAR(Vars::Aimbot::Projectile::Hitboxes) & ~(1 << 2));
									VAR(Vars::Aimbot::Projectile::Hitboxes) = (_b3 ? VAR(Vars::Aimbot::Projectile::Hitboxes) | (1 << 3) : VAR(Vars::Aimbot::Projectile::Hitboxes) & ~(1 << 3));
									VAR(Vars::Aimbot::Projectile::Hitboxes) = (_b4 ? VAR(Vars::Aimbot::Projectile::Hitboxes) | (1 << 4) : VAR(Vars::Aimbot::Projectile::Hitboxes) & ~(1 << 4));
									VAR(Vars::Aimbot::Projectile::Hitboxes) = (_b5 ? VAR(Vars::Aimbot::Projectile::Hitboxes) | (1 << 5) : VAR(Vars::Aimbot::Projectile::Hitboxes) & ~(1 << 5));
								}
							}

							Spacing();

							// Splash prediction
							gui::combo("Splash Prediction", VARP(Vars::Aimbot::Projectile::SplashPrediction),
								splash_pred, IM_ARRAYSIZE(splash_pred));
							gui::slider("Max Simulation Time", VARP(Vars::Aimbot::Projectile::MaxSimulationTime), 0.1f, 2.5f);
							gui::slider("Hit Chance", VARP(Vars::Aimbot::Projectile::HitChance), 0.f, 100.f);
							gui::slider("Splash Radius", VARP(Vars::Aimbot::Projectile::SplashRadius), 0.f, 100.f);
							gui::slider("Auto Release", VARP(Vars::Aimbot::Projectile::AutoRelease), 0.f, 100.f);

							Spacing();

							// Modifiers
							{
								static bool _b0 = (VAR(Vars::Aimbot::Projectile::Modifiers) & (1 << 0)) != 0;
								static bool _b1 = (VAR(Vars::Aimbot::Projectile::Modifiers) & (1 << 1)) != 0;
								static bool _b2 = (VAR(Vars::Aimbot::Projectile::Modifiers) & (1 << 2)) != 0;
								_b0 = (VAR(Vars::Aimbot::Projectile::Modifiers) & (1 << 0)) != 0;
								_b1 = (VAR(Vars::Aimbot::Projectile::Modifiers) & (1 << 1)) != 0;
								_b2 = (VAR(Vars::Aimbot::Projectile::Modifiers) & (1 << 2)) != 0;
								std::vector<gui::MultiComboItem> _items_Vars_Aimbot_Projectile_Modifiers = {
									{ "Charge Weapon", &_b0 },
									{ "Cancel Charge", &_b1 },
									{ "Use Arm Time", &_b2 },
								};
								if (gui::multi_combo("Modifiers", _items_Vars_Aimbot_Projectile_Modifiers, 350.f))
								{
									VAR(Vars::Aimbot::Projectile::Modifiers) = (_b0 ? VAR(Vars::Aimbot::Projectile::Modifiers) | (1 << 0) : VAR(Vars::Aimbot::Projectile::Modifiers) & ~(1 << 0));
									VAR(Vars::Aimbot::Projectile::Modifiers) = (_b1 ? VAR(Vars::Aimbot::Projectile::Modifiers) | (1 << 1) : VAR(Vars::Aimbot::Projectile::Modifiers) & ~(1 << 1));
									VAR(Vars::Aimbot::Projectile::Modifiers) = (_b2 ? VAR(Vars::Aimbot::Projectile::Modifiers) | (1 << 2) : VAR(Vars::Aimbot::Projectile::Modifiers) & ~(1 << 2));
								}
							}

							Spacing();

							// Strafe prediction
							{
								static bool _b0 = (VAR(Vars::Aimbot::Projectile::StrafePrediction) & (1 << 0)) != 0;
								static bool _b1 = (VAR(Vars::Aimbot::Projectile::StrafePrediction) & (1 << 1)) != 0;
								_b0 = (VAR(Vars::Aimbot::Projectile::StrafePrediction) & (1 << 0)) != 0;
								_b1 = (VAR(Vars::Aimbot::Projectile::StrafePrediction) & (1 << 1)) != 0;
								std::vector<gui::MultiComboItem> _items_Vars_Aimbot_Projectile_StrafePrediction = {
									{ "Air Strafing", &_b0 },
									{ "Ground Strafing", &_b1 },
								};
								if (gui::multi_combo("Strafe Prediction", _items_Vars_Aimbot_Projectile_StrafePrediction, 350.f))
								{
									VAR(Vars::Aimbot::Projectile::StrafePrediction) = (_b0 ? VAR(Vars::Aimbot::Projectile::StrafePrediction) | (1 << 0) : VAR(Vars::Aimbot::Projectile::StrafePrediction) & ~(1 << 0));
									VAR(Vars::Aimbot::Projectile::StrafePrediction) = (_b1 ? VAR(Vars::Aimbot::Projectile::StrafePrediction) | (1 << 1) : VAR(Vars::Aimbot::Projectile::StrafePrediction) & ~(1 << 1));
								}
							}

							Spacing();

							// Auto detonate
							{
								static bool _b0 = (VAR(Vars::Aimbot::Projectile::AutoDetonate) & (1 << 0)) != 0;
								static bool _b1 = (VAR(Vars::Aimbot::Projectile::AutoDetonate) & (1 << 1)) != 0;
								static bool _b2 = (VAR(Vars::Aimbot::Projectile::AutoDetonate) & (1 << 2)) != 0;
								static bool _b3 = (VAR(Vars::Aimbot::Projectile::AutoDetonate) & (1 << 3)) != 0;
								_b0 = (VAR(Vars::Aimbot::Projectile::AutoDetonate) & (1 << 0)) != 0;
								_b1 = (VAR(Vars::Aimbot::Projectile::AutoDetonate) & (1 << 1)) != 0;
								_b2 = (VAR(Vars::Aimbot::Projectile::AutoDetonate) & (1 << 2)) != 0;
								_b3 = (VAR(Vars::Aimbot::Projectile::AutoDetonate) & (1 << 3)) != 0;
								std::vector<gui::MultiComboItem> _items_Vars_Aimbot_Projectile_AutoDetonate = {
									{ "Stickies", &_b0 },
									{ "Flares", &_b1 },
									{ "Prevent Self Damage", &_b2 },
									{ "Ignore Invisible", &_b3 },
								};
								if (gui::multi_combo("Auto Detonate", _items_Vars_Aimbot_Projectile_AutoDetonate, 350.f))
								{
									VAR(Vars::Aimbot::Projectile::AutoDetonate) = (_b0 ? VAR(Vars::Aimbot::Projectile::AutoDetonate) | (1 << 0) : VAR(Vars::Aimbot::Projectile::AutoDetonate) & ~(1 << 0));
									VAR(Vars::Aimbot::Projectile::AutoDetonate) = (_b1 ? VAR(Vars::Aimbot::Projectile::AutoDetonate) | (1 << 1) : VAR(Vars::Aimbot::Projectile::AutoDetonate) & ~(1 << 1));
									VAR(Vars::Aimbot::Projectile::AutoDetonate) = (_b2 ? VAR(Vars::Aimbot::Projectile::AutoDetonate) | (1 << 2) : VAR(Vars::Aimbot::Projectile::AutoDetonate) & ~(1 << 2));
									VAR(Vars::Aimbot::Projectile::AutoDetonate) = (_b3 ? VAR(Vars::Aimbot::Projectile::AutoDetonate) | (1 << 3) : VAR(Vars::Aimbot::Projectile::AutoDetonate) & ~(1 << 3));
								}
							}
							gui::slider("Autodet Radius", VARP(Vars::Aimbot::Projectile::AutodetRadius), 0.f, 100.f);

							Spacing();

							// Auto airblast
							{
								static bool _b0 = (VAR(Vars::Aimbot::Projectile::AutoAirblast) & (1 << 0)) != 0;
								static bool _b1 = (VAR(Vars::Aimbot::Projectile::AutoAirblast) & (1 << 1)) != 0;
								static bool _b2 = (VAR(Vars::Aimbot::Projectile::AutoAirblast) & (1 << 2)) != 0;
								_b0 = (VAR(Vars::Aimbot::Projectile::AutoAirblast) & (1 << 0)) != 0;
								_b1 = (VAR(Vars::Aimbot::Projectile::AutoAirblast) & (1 << 1)) != 0;
								_b2 = (VAR(Vars::Aimbot::Projectile::AutoAirblast) & (1 << 2)) != 0;
								std::vector<gui::MultiComboItem> _items_Vars_Aimbot_Projectile_AutoAirblast = {
									{ "Enabled", &_b0 },
									{ "Redirect", &_b1 },
									{ "Ignore FOV", &_b2 },
								};
								if (gui::multi_combo("Auto Airblast", _items_Vars_Aimbot_Projectile_AutoAirblast, 350.f))
								{
									VAR(Vars::Aimbot::Projectile::AutoAirblast) = (_b0 ? VAR(Vars::Aimbot::Projectile::AutoAirblast) | (1 << 0) : VAR(Vars::Aimbot::Projectile::AutoAirblast) & ~(1 << 0));
									VAR(Vars::Aimbot::Projectile::AutoAirblast) = (_b1 ? VAR(Vars::Aimbot::Projectile::AutoAirblast) | (1 << 1) : VAR(Vars::Aimbot::Projectile::AutoAirblast) & ~(1 << 1));
									VAR(Vars::Aimbot::Projectile::AutoAirblast) = (_b2 ? VAR(Vars::Aimbot::Projectile::AutoAirblast) | (1 << 2) : VAR(Vars::Aimbot::Projectile::AutoAirblast) & ~(1 << 2));
								}
							}
						}
					}
					gui::end_group_scrollable();

					// RIGHT: Targets & Ignore
					SameLine(390);
					if (gui::begin_group_scrollable("TARGETS", ImVec2(380, 490), 5.f, 5.f))
					{
						{
							static bool _b0 = (VAR(Vars::Aimbot::General::Target) & (1 << 0)) != 0;
							static bool _b1 = (VAR(Vars::Aimbot::General::Target) & (1 << 1)) != 0;
							static bool _b2 = (VAR(Vars::Aimbot::General::Target) & (1 << 2)) != 0;
							static bool _b3 = (VAR(Vars::Aimbot::General::Target) & (1 << 3)) != 0;
							static bool _b4 = (VAR(Vars::Aimbot::General::Target) & (1 << 4)) != 0;
							static bool _b5 = (VAR(Vars::Aimbot::General::Target) & (1 << 5)) != 0;
							static bool _b6 = (VAR(Vars::Aimbot::General::Target) & (1 << 6)) != 0;
							_b0 = (VAR(Vars::Aimbot::General::Target) & (1 << 0)) != 0;
							_b1 = (VAR(Vars::Aimbot::General::Target) & (1 << 1)) != 0;
							_b2 = (VAR(Vars::Aimbot::General::Target) & (1 << 2)) != 0;
							_b3 = (VAR(Vars::Aimbot::General::Target) & (1 << 3)) != 0;
							_b4 = (VAR(Vars::Aimbot::General::Target) & (1 << 4)) != 0;
							_b5 = (VAR(Vars::Aimbot::General::Target) & (1 << 5)) != 0;
							_b6 = (VAR(Vars::Aimbot::General::Target) & (1 << 6)) != 0;
							std::vector<gui::MultiComboItem> _items_Vars_Aimbot_General_Target = {
								{ "Players", &_b0 },
								{ "Sentries", &_b1 },
								{ "Dispensers", &_b2 },
								{ "Teleporters", &_b3 },
								{ "Stickies", &_b4 },
								{ "NPCs", &_b5 },
								{ "Bombs", &_b6 },
							};
							if (gui::multi_combo("Targets", _items_Vars_Aimbot_General_Target, 350.f))
							{
								VAR(Vars::Aimbot::General::Target) = (_b0 ? VAR(Vars::Aimbot::General::Target) | (1 << 0) : VAR(Vars::Aimbot::General::Target) & ~(1 << 0));
								VAR(Vars::Aimbot::General::Target) = (_b1 ? VAR(Vars::Aimbot::General::Target) | (1 << 1) : VAR(Vars::Aimbot::General::Target) & ~(1 << 1));
								VAR(Vars::Aimbot::General::Target) = (_b2 ? VAR(Vars::Aimbot::General::Target) | (1 << 2) : VAR(Vars::Aimbot::General::Target) & ~(1 << 2));
								VAR(Vars::Aimbot::General::Target) = (_b3 ? VAR(Vars::Aimbot::General::Target) | (1 << 3) : VAR(Vars::Aimbot::General::Target) & ~(1 << 3));
								VAR(Vars::Aimbot::General::Target) = (_b4 ? VAR(Vars::Aimbot::General::Target) | (1 << 4) : VAR(Vars::Aimbot::General::Target) & ~(1 << 4));
								VAR(Vars::Aimbot::General::Target) = (_b5 ? VAR(Vars::Aimbot::General::Target) | (1 << 5) : VAR(Vars::Aimbot::General::Target) & ~(1 << 5));
								VAR(Vars::Aimbot::General::Target) = (_b6 ? VAR(Vars::Aimbot::General::Target) | (1 << 6) : VAR(Vars::Aimbot::General::Target) & ~(1 << 6));
							}
						}

						Spacing();

						// Ignore bitmask
						{
							static bool _b0 = (VAR(Vars::Aimbot::General::Ignore) & (1 << 0)) != 0;
							static bool _b1 = (VAR(Vars::Aimbot::General::Ignore) & (1 << 1)) != 0;
							static bool _b2 = (VAR(Vars::Aimbot::General::Ignore) & (1 << 3)) != 0;
							static bool _b3 = (VAR(Vars::Aimbot::General::Ignore) & (1 << 4)) != 0;
							static bool _b4 = (VAR(Vars::Aimbot::General::Ignore) & (1 << 6)) != 0;
							static bool _b5 = (VAR(Vars::Aimbot::General::Ignore) & (1 << 8)) != 0;
							static bool _b6 = (VAR(Vars::Aimbot::General::Ignore) & (1 << 9)) != 0;
							static bool _b7 = (VAR(Vars::Aimbot::General::Ignore) & (1 << 10)) != 0;
							_b0 = (VAR(Vars::Aimbot::General::Ignore) & (1 << 0)) != 0;
							_b1 = (VAR(Vars::Aimbot::General::Ignore) & (1 << 1)) != 0;
							_b2 = (VAR(Vars::Aimbot::General::Ignore) & (1 << 3)) != 0;
							_b3 = (VAR(Vars::Aimbot::General::Ignore) & (1 << 4)) != 0;
							_b4 = (VAR(Vars::Aimbot::General::Ignore) & (1 << 6)) != 0;
							_b5 = (VAR(Vars::Aimbot::General::Ignore) & (1 << 8)) != 0;
							_b6 = (VAR(Vars::Aimbot::General::Ignore) & (1 << 9)) != 0;
							_b7 = (VAR(Vars::Aimbot::General::Ignore) & (1 << 10)) != 0;
							std::vector<gui::MultiComboItem> _items_Vars_Aimbot_General_Ignore = {
								{ "Friends", &_b0 },
								{ "Party", &_b1 },
								{ "Invulnerable", &_b2 },
								{ "Invisible", &_b3 },
								{ "Dead Ringer", &_b4 },
								{ "Disguised", &_b5 },
								{ "Taunting", &_b6 },
								{ "Team", &_b7 },
							};
							if (gui::multi_combo("Ignore", _items_Vars_Aimbot_General_Ignore, 350.f))
							{
								VAR(Vars::Aimbot::General::Ignore) = (_b0 ? VAR(Vars::Aimbot::General::Ignore) | (1 << 0) : VAR(Vars::Aimbot::General::Ignore) & ~(1 << 0));
								VAR(Vars::Aimbot::General::Ignore) = (_b1 ? VAR(Vars::Aimbot::General::Ignore) | (1 << 1) : VAR(Vars::Aimbot::General::Ignore) & ~(1 << 1));
								VAR(Vars::Aimbot::General::Ignore) = (_b2 ? VAR(Vars::Aimbot::General::Ignore) | (1 << 3) : VAR(Vars::Aimbot::General::Ignore) & ~(1 << 3));
								VAR(Vars::Aimbot::General::Ignore) = (_b3 ? VAR(Vars::Aimbot::General::Ignore) | (1 << 4) : VAR(Vars::Aimbot::General::Ignore) & ~(1 << 4));
								VAR(Vars::Aimbot::General::Ignore) = (_b4 ? VAR(Vars::Aimbot::General::Ignore) | (1 << 6) : VAR(Vars::Aimbot::General::Ignore) & ~(1 << 6));
								VAR(Vars::Aimbot::General::Ignore) = (_b5 ? VAR(Vars::Aimbot::General::Ignore) | (1 << 8) : VAR(Vars::Aimbot::General::Ignore) & ~(1 << 8));
								VAR(Vars::Aimbot::General::Ignore) = (_b6 ? VAR(Vars::Aimbot::General::Ignore) | (1 << 9) : VAR(Vars::Aimbot::General::Ignore) & ~(1 << 9));
								VAR(Vars::Aimbot::General::Ignore) = (_b7 ? VAR(Vars::Aimbot::General::Ignore) | (1 << 10) : VAR(Vars::Aimbot::General::Ignore) & ~(1 << 10));
							}
						}

						Spacing();
						gui::slider("Invisible Threshold", VARP(Vars::Aimbot::General::IgnoreInvisible), 0.f, 100.f);
					}
					gui::end_group_scrollable();
				}

				// ══════════════════════════════════════
				// SUB-TAB 2 : MELEE
				// ══════════════════════════════════════
				else if (aim_mode == 2)
				{
					static const char* aim_types[] = { "Off", "Plain", "Smooth", "Silent", "Locking", "Assistive" };

					// LEFT: Melee settings
					if (gui::begin_group_scrollable("MELEE", ImVec2(380, 490), 5.f, 0.f))
					{
						// ── Linha 1: checkbox "Aimbot" + keybind alinhado à direita ──
						{
							bool bEnabled = VAR(Vars::Aimbot::General::AimType) != 0;
							bool bPrev = bEnabled;
							gui::checkbox("Aimbot", bEnabled);
							if (bEnabled != bPrev)
								VAR(Vars::Aimbot::General::AimType) = bEnabled ? 1 : 0;

							ImGui::SameLine(360.f);
							{ gui::keybind("##key_melee", &VAR(Vars::Aimbot::Melee::Key), &VAR(Vars::Aimbot::Melee::KeyType)); }
						}

						if (VAR(Vars::Aimbot::General::AimType) != 0)
						{
							gui::slider("FOV", VARP(Vars::Aimbot::General::AimFOV), 0.f, 180.f);
							gui::combo("Aim Type", VARP(Vars::Aimbot::General::AimType),
								aim_types, IM_ARRAYSIZE(aim_types));
							gui::checkbox("Auto Shoot", VAR(Vars::Aimbot::General::AutoShoot));

							Spacing();
							gui::checkbox("Auto Backstab", VAR(Vars::Aimbot::Melee::AutoBackstab));
							gui::checkbox("Ignore Razorback", VAR(Vars::Aimbot::Melee::IgnoreRazorback));
							gui::checkbox("Swing Prediction", VAR(Vars::Aimbot::Melee::SwingPrediction));
							gui::checkbox("Whip Team", VAR(Vars::Aimbot::Melee::WhipTeam));
						}
					}
					gui::end_group_scrollable();

					// RIGHT: Targets & Ignore
					SameLine(390);
					if (gui::begin_group_scrollable("TARGETS", ImVec2(380, 490), 5.f, 5.f))
					{
						{
							static bool _b0 = (VAR(Vars::Aimbot::General::Target) & (1 << 0)) != 0;
							static bool _b1 = (VAR(Vars::Aimbot::General::Target) & (1 << 1)) != 0;
							static bool _b2 = (VAR(Vars::Aimbot::General::Target) & (1 << 2)) != 0;
							static bool _b3 = (VAR(Vars::Aimbot::General::Target) & (1 << 3)) != 0;
							_b0 = (VAR(Vars::Aimbot::General::Target) & (1 << 0)) != 0;
							_b1 = (VAR(Vars::Aimbot::General::Target) & (1 << 1)) != 0;
							_b2 = (VAR(Vars::Aimbot::General::Target) & (1 << 2)) != 0;
							_b3 = (VAR(Vars::Aimbot::General::Target) & (1 << 3)) != 0;
							std::vector<gui::MultiComboItem> _items_Vars_Aimbot_General_Target = {
								{ "Players", &_b0 },
								{ "Sentries", &_b1 },
								{ "Dispensers", &_b2 },
								{ "Teleporters", &_b3 },
							};
							if (gui::multi_combo("Targets", _items_Vars_Aimbot_General_Target, 350.f))
							{
								VAR(Vars::Aimbot::General::Target) = (_b0 ? VAR(Vars::Aimbot::General::Target) | (1 << 0) : VAR(Vars::Aimbot::General::Target) & ~(1 << 0));
								VAR(Vars::Aimbot::General::Target) = (_b1 ? VAR(Vars::Aimbot::General::Target) | (1 << 1) : VAR(Vars::Aimbot::General::Target) & ~(1 << 1));
								VAR(Vars::Aimbot::General::Target) = (_b2 ? VAR(Vars::Aimbot::General::Target) | (1 << 2) : VAR(Vars::Aimbot::General::Target) & ~(1 << 2));
								VAR(Vars::Aimbot::General::Target) = (_b3 ? VAR(Vars::Aimbot::General::Target) | (1 << 3) : VAR(Vars::Aimbot::General::Target) & ~(1 << 3));
							}
						}

						Spacing();

						{
							static bool _b0 = (VAR(Vars::Aimbot::General::Ignore) & (1 << 0)) != 0;
							static bool _b1 = (VAR(Vars::Aimbot::General::Ignore) & (1 << 1)) != 0;
							static bool _b2 = (VAR(Vars::Aimbot::General::Ignore) & (1 << 3)) != 0;
							static bool _b3 = (VAR(Vars::Aimbot::General::Ignore) & (1 << 4)) != 0;
							static bool _b4 = (VAR(Vars::Aimbot::General::Ignore) & (1 << 6)) != 0;
							static bool _b5 = (VAR(Vars::Aimbot::General::Ignore) & (1 << 8)) != 0;
							static bool _b6 = (VAR(Vars::Aimbot::General::Ignore) & (1 << 9)) != 0;
							static bool _b7 = (VAR(Vars::Aimbot::General::Ignore) & (1 << 10)) != 0;
							_b0 = (VAR(Vars::Aimbot::General::Ignore) & (1 << 0)) != 0;
							_b1 = (VAR(Vars::Aimbot::General::Ignore) & (1 << 1)) != 0;
							_b2 = (VAR(Vars::Aimbot::General::Ignore) & (1 << 3)) != 0;
							_b3 = (VAR(Vars::Aimbot::General::Ignore) & (1 << 4)) != 0;
							_b4 = (VAR(Vars::Aimbot::General::Ignore) & (1 << 6)) != 0;
							_b5 = (VAR(Vars::Aimbot::General::Ignore) & (1 << 8)) != 0;
							_b6 = (VAR(Vars::Aimbot::General::Ignore) & (1 << 9)) != 0;
							_b7 = (VAR(Vars::Aimbot::General::Ignore) & (1 << 10)) != 0;
							std::vector<gui::MultiComboItem> _items_Vars_Aimbot_General_Ignore = {
								{ "Friends", &_b0 },
								{ "Party", &_b1 },
								{ "Invulnerable", &_b2 },
								{ "Invisible", &_b3 },
								{ "Dead Ringer", &_b4 },
								{ "Disguised", &_b5 },
								{ "Taunting", &_b6 },
								{ "Team", &_b7 },
							};
							if (gui::multi_combo("Ignore", _items_Vars_Aimbot_General_Ignore, 350.f))
							{
								VAR(Vars::Aimbot::General::Ignore) = (_b0 ? VAR(Vars::Aimbot::General::Ignore) | (1 << 0) : VAR(Vars::Aimbot::General::Ignore) & ~(1 << 0));
								VAR(Vars::Aimbot::General::Ignore) = (_b1 ? VAR(Vars::Aimbot::General::Ignore) | (1 << 1) : VAR(Vars::Aimbot::General::Ignore) & ~(1 << 1));
								VAR(Vars::Aimbot::General::Ignore) = (_b2 ? VAR(Vars::Aimbot::General::Ignore) | (1 << 3) : VAR(Vars::Aimbot::General::Ignore) & ~(1 << 3));
								VAR(Vars::Aimbot::General::Ignore) = (_b3 ? VAR(Vars::Aimbot::General::Ignore) | (1 << 4) : VAR(Vars::Aimbot::General::Ignore) & ~(1 << 4));
								VAR(Vars::Aimbot::General::Ignore) = (_b4 ? VAR(Vars::Aimbot::General::Ignore) | (1 << 6) : VAR(Vars::Aimbot::General::Ignore) & ~(1 << 6));
								VAR(Vars::Aimbot::General::Ignore) = (_b5 ? VAR(Vars::Aimbot::General::Ignore) | (1 << 8) : VAR(Vars::Aimbot::General::Ignore) & ~(1 << 8));
								VAR(Vars::Aimbot::General::Ignore) = (_b6 ? VAR(Vars::Aimbot::General::Ignore) | (1 << 9) : VAR(Vars::Aimbot::General::Ignore) & ~(1 << 9));
								VAR(Vars::Aimbot::General::Ignore) = (_b7 ? VAR(Vars::Aimbot::General::Ignore) | (1 << 10) : VAR(Vars::Aimbot::General::Ignore) & ~(1 << 10));
							}
						}

						Spacing();
						gui::slider("Invisible Threshold", VARP(Vars::Aimbot::General::IgnoreInvisible), 0.f, 100.f);
					}
					gui::end_group_scrollable();
				}

				break;
			}

			// ─────────────────────────────────────────────────────────────────
			case 1: // ANTI-AIM
				// ─────────────────────────────────────────────────────────────────
			{
				static const char* pitch_real[] = { "None", "Up", "Down", "Zero", "Jitter", "Reverse Jitter" };
				static const char* pitch_fake[] = { "None", "Up", "Down", "Jitter", "Reverse Jitter" };
				static const char* yaw_opts[] = { "Forward", "Left", "Right", "Backwards", "Edge", "Jitter", "Spin" };

				if (gui::begin_group_scrollable("ANTI-AIM", ImVec2(380, 500), 5.f, 5.f))
				{

					gui::checkbox("Enabled", VAR(Vars::AntiAim::Enabled));

					if (VAR(Vars::AntiAim::Enabled))
					{
						gui::combo("Real Pitch", VARP(Vars::AntiAim::PitchReal),
							pitch_real, IM_ARRAYSIZE(pitch_real));
						gui::combo("Fake Pitch", VARP(Vars::AntiAim::PitchFake),
							pitch_fake, IM_ARRAYSIZE(pitch_fake));
						gui::combo("Real Yaw", VARP(Vars::AntiAim::YawReal),
							yaw_opts, IM_ARRAYSIZE(yaw_opts));
						gui::combo("Fake Yaw", VARP(Vars::AntiAim::YawFake),
							yaw_opts, IM_ARRAYSIZE(yaw_opts));
						gui::slider("Real Offset", VARP(Vars::AntiAim::RealYawOffset), -180.f, 180.f);
						gui::slider("Fake Offset", VARP(Vars::AntiAim::FakeYawOffset), -180.f, 180.f);
						gui::slider("Spin Speed", VARP(Vars::AntiAim::SpinSpeed), -30.f, 30.f);
						gui::checkbox("Minwalk", VAR(Vars::AntiAim::MinWalk));
						gui::checkbox("Anti-Overlap", VAR(Vars::AntiAim::AntiOverlap));
					}
				}
				gui::end_group_scrollable();
				break;
			}

			// ─────────────────────────────────────────────────────────────────
			case 2: // VISUALS  (ESP / Materials / World)
				// ─────────────────────────────────────────────────────────────────
			{
				int vis_mode = gui::g_visuals_mode_state.selected_mode;

				// ══════════════════════════════════════
				// SUB-TAB 0 : ESP
				// ══════════════════════════════════════
				if (vis_mode == 0)
				{
					if (gui::begin_group_scrollable("PLAYER ESP", ImVec2(380, 490), 5.f, 0.f))
					{
						static const char* box_styles[] = { "2D", "3D", "Corner", "Gradient" };
						static const char* health_positions[] = { "Left", "Right", "Top", "Bottom" };
						static const char* dist_positions[] = { "Side", "Bottom" };
						static const char* uberbar_modes[] = { "Off", "Static", "Gradient" };

						gui::checkbox("ESP Master", VAR(Vars::Visuals::ESP::Players));

						if (VAR(Vars::Visuals::ESP::Players))
						{
							gui::checkbox("Team Check", VAR(Vars::Visuals::ESP::ShowEnemies));
							gui::checkbox("Show Friends", VAR(Vars::Visuals::ESP::ShowFriends));
							gui::checkbox("Show Local Player", VAR(Vars::Visuals::ESP::ShowLocalPlayer));
							gui::checkbox("Hide Cloaked Players", VAR(Vars::Visuals::ESP::HideCloaked));

							Spacing();

							// ── Box Style ────────────────────────────────────────────────
							{
								static const char* box_styles_with_none[] = { "None", "2D", "3D", "Corner", "Gradient" };
								ImVec2 _p = ImGui::GetCursorScreenPos();
								gui::combo("Box Style", VARP(Vars::Visuals::ESP::BoxStyle), box_styles_with_none, IM_ARRAYSIZE(box_styles_with_none));
								ImVec2 _e = ImGui::GetCursorScreenPos();

								if (VAR(Vars::Visuals::ESP::BoxStyle) != 0)
								{
									if (VAR(Vars::Visuals::ESP::BoxStyle) != 4)
										gui::color_hint(_p, _e, 350.f, &VAR(Vars::Visuals::ESP::BoxColor));
									else
										gui::color_hint(_p, _e, 350.f, &VAR(Vars::Visuals::ESP::BoxColor), &VAR(Vars::Visuals::ESP::BoxGradientColor));
								}
							}

							// ── Name ─────────────────────────────────────────────────────
							gui::checkbox_color("Name", VAR(Vars::Visuals::ESP::Name), &VAR(Vars::Visuals::ESP::NameColor));
							gui::checkbox_color("Text Background",
								VAR(Vars::Visuals::ESP::TextBackground),
								&VAR(Vars::Visuals::ESP::TextBackgroundColor));

							// ── Health Bar (combo com None = desativado) ───────────────────
							// HealthType: 0=None 1=Left 2=Right 3=Top 4=Bottom
							{
								static const char* health_bar_opts[] = { "None", "Left", "Right", "Top", "Bottom" };
								ImVec2 _p = ImGui::GetCursorScreenPos();
								gui::combo("Health Bar", VARP(Vars::Visuals::ESP::HealthType), health_bar_opts, IM_ARRAYSIZE(health_bar_opts));
								ImVec2 _e = ImGui::GetCursorScreenPos();
								VAR(Vars::Visuals::ESP::Health) = (VAR(Vars::Visuals::ESP::HealthType) != 0);
								VAR(Vars::Visuals::ESP::HealthPosition) = std::max(0, VAR(Vars::Visuals::ESP::HealthType) - 1);
								VAR(Vars::Visuals::ESP::HealthGradientEnabled) = true;

								if (VAR(Vars::Visuals::ESP::HealthType) != 0)
									gui::color_hint(_p, _e, 350.f,
										&VAR(Vars::Visuals::ESP::HealthGradientHigh),
										&VAR(Vars::Visuals::ESP::HealthGradientMid),
										&VAR(Vars::Visuals::ESP::HealthGradientLow));
							}

							// ── Health Text ───────────────────────────────────────────────
							gui::checkbox_color("Health Text", VAR(Vars::Visuals::ESP::HealthText), &VAR(Vars::Visuals::ESP::HealthTextColor));

							// ── Class Icons ───────────────────────────────────────────────
							gui::checkbox("Class Icons", VAR(Vars::Visuals::ESP::ClassIcons));

							// ── Weapon ────────────────────────────────────────────────────
							gui::checkbox("Weapon Icon", VAR(Vars::Visuals::ESP::WeaponIcon));
							gui::checkbox_color("Weapon Text", VAR(Vars::Visuals::ESP::WeaponText), &VAR(Vars::Visuals::ESP::WeaponTextColor));

							// ── Uber Bar ──────────────────────────────────────────────────
							{
								ImVec2 _p = ImGui::GetCursorScreenPos();
								gui::combo("Uber Bar", VARP(Vars::Visuals::ESP::UberBarMode), uberbar_modes, IM_ARRAYSIZE(uberbar_modes));
								ImVec2 _e = ImGui::GetCursorScreenPos();
								if (VAR(Vars::Visuals::ESP::UberBarMode) != 0)
									gui::color_hint(_p, _e, 350.f,
										&VAR(Vars::Visuals::ESP::UberBarColorHigh),
										&VAR(Vars::Visuals::ESP::UberBarColorMid),
										&VAR(Vars::Visuals::ESP::UberBarColor));
							}

							Spacing();

							// ── Distance ──────────────────────────────────────────────────
							gui::checkbox("Distance", VAR(Vars::Visuals::ESP::Distance));
							if (VAR(Vars::Visuals::ESP::Distance))
								gui::combo("Distance Position", VARP(Vars::Visuals::ESP::DistancePosition), dist_positions, IM_ARRAYSIZE(dist_positions));

							// ── Skeleton ──────────────────────────────────────────────────
							gui::checkbox("Skeleton", VAR(Vars::Visuals::ESP::Bones));

							Spacing();

							gui::checkbox("Player Conditions", VAR(Vars::Visuals::ESP::PlayerConditions));
							gui::checkbox_color("Player Tracers", VAR(Vars::Visuals::ESP::PlayerTracers), &VAR(Vars::Visuals::ESP::TracerColor));
							gui::checkbox("Buffs", VAR(Vars::Visuals::ESP::Buffs));
							gui::checkbox("Debuffs", VAR(Vars::Visuals::ESP::Debuffs));
							gui::checkbox("Latency (Ping)", VAR(Vars::Visuals::ESP::Latency));
						}
					}
					gui::end_group_scrollable();
				}

				// ══════════════════════════════════════
				// SUB-TAB 1 : MATERIALS (chams + outline)
				// ══════════════════════════════════════
				else if (vis_mode == 1)
				{
					static const char* chams_modes[] = { "Off", "Flat", "Wireframe", "Shaded", "Glow", "Shine" };

					if (gui::begin_group_scrollable("CHAMS", ImVec2(380, 490), 5.f, 0.f))
					{

						gui::combo("Enemy Mode", VARP(Vars::Visuals::Chams::EnemyEnabled),
							chams_modes, IM_ARRAYSIZE(chams_modes));
						if (VAR(Vars::Visuals::Chams::EnemyEnabled) != 0)
						{
							gui::color_picker("Enemy Color", &VAR(Vars::Visuals::Chams::EnemyColor));
							gui::color_picker("Enemy XQZ Color", &VAR(Vars::Visuals::Chams::EnemyXQZColor));
						}

						Spacing();

						gui::combo("Team Mode", VARP(Vars::Visuals::Chams::TeamEnabled),
							chams_modes, IM_ARRAYSIZE(chams_modes));
						if (VAR(Vars::Visuals::Chams::TeamEnabled) != 0)
						{
							gui::color_picker("Team Color", &VAR(Vars::Visuals::Chams::TeamColor));
							gui::color_picker("Team XQZ Color", &VAR(Vars::Visuals::Chams::TeamXQZColor));
						}

						Spacing();

						gui::checkbox("XQZ (Through Walls)", VAR(Vars::Visuals::Chams::XQZ));
						gui::checkbox("Ignore Z", VAR(Vars::Visuals::Chams::IgnoreZ));

						Spacing();

						static const char* spell_foot[] = { "Off", "Color", "Team", "Halloween" };
						gui::combo("Spell Footsteps", VARP(Vars::Visuals::Effects::SpellFootsteps),
							spell_foot, IM_ARRAYSIZE(spell_foot));
						gui::checkbox("Icons Through Walls",
							VAR(Vars::Visuals::Effects::DrawIconsThroughWalls));
						gui::checkbox("Dmg Numbers Through Walls",
							VAR(Vars::Visuals::Effects::DrawDamageNumbersThroughWalls));
					}
					gui::end_group_scrollable();

					SameLine(390);
					if (gui::begin_group_scrollable("OUTLINE", ImVec2(380, 490), 5.f, 5.f))
					{

						gui::checkbox("Enabled", VAR(Vars::Visuals::Chams::OutlineEnabled));

						if (VAR(Vars::Visuals::Chams::OutlineEnabled))
						{
							gui::checkbox("Outline Enemies", VAR(Vars::Visuals::Chams::OutlineEnemy));
							gui::checkbox("Outline Team", VAR(Vars::Visuals::Chams::OutlineTeam));

							Spacing();

							if (VAR(Vars::Visuals::Chams::OutlineEnemy))
								gui::color_picker("Enemy Color", &VAR(Vars::Visuals::Chams::OutlineEnemyColor));
							if (VAR(Vars::Visuals::Chams::OutlineTeam))
								gui::color_picker("Team Color", &VAR(Vars::Visuals::Chams::OutlineTeamColor));

							Spacing();
							gui::slider("Thickness",
								VARP(Vars::Visuals::Chams::OutlineThickness), 0.5f, 6.f);
						}
					}
					gui::end_group_scrollable();
				}

				// ══════════════════════════════════════
				// SUB-TAB 2 : WORLD  (grid 2x2)
				// ══════════════════════════════════════
				else if (vis_mode == 2)
				{
					if (gui::begin_group_scrollable("WORLD", ImVec2(380, 238), 5.f, 0.f))
					{

						{
							static const char* world_tex_opts[] = { "Default", "Dev", "Camo", "Black", "White", "Gray", "Flat" };
							static int world_tex_idx = 0;
							std::string& texRef = VAR(Vars::Visuals::World::WorldTexture);
							for (int i = 0; i < IM_ARRAYSIZE(world_tex_opts); i++)
								if (texRef == world_tex_opts[i]) { world_tex_idx = i; break; }
							if (gui::combo("World Texture", &world_tex_idx,
								world_tex_opts, IM_ARRAYSIZE(world_tex_opts)))
								texRef = world_tex_opts[world_tex_idx];
						}

						gui::checkbox("Near Prop Fade", VAR(Vars::Visuals::World::NearPropFade));
						gui::checkbox("No Prop Fade", VAR(Vars::Visuals::World::NoPropFade));

						Spacing();

						BITMASK_CHECKBOX("World", Vars::Visuals::World::Modulations, 0);
						BITMASK_CHECKBOX("Sky", Vars::Visuals::World::Modulations, 1);
						BITMASK_CHECKBOX("Prop", Vars::Visuals::World::Modulations, 2);
						BITMASK_CHECKBOX("Particle", Vars::Visuals::World::Modulations, 3);
						BITMASK_CHECKBOX("Fog", Vars::Visuals::World::Modulations, 4);

						Spacing();

						gui::checkbox("Enabled", VAR(Vars::Visuals::Thirdperson::Enabled));
						gui::checkbox("Crosshair", VAR(Vars::Visuals::Thirdperson::Crosshair));
						gui::slider("Distance", VARP(Vars::Visuals::Thirdperson::Distance), 0.f, 400.f);
						gui::slider("Right", VARP(Vars::Visuals::Thirdperson::Right), -100.f, 100.f);
						gui::slider("Up", VARP(Vars::Visuals::Thirdperson::Up), -100.f, 100.f);
					}
					gui::end_group_scrollable();

					SameLine(390);
					if (gui::begin_group_scrollable("VIEWMODEL", ImVec2(380, 238), 5.f, 5.f))
					{

						gui::checkbox("Crosshair Aim", VAR(Vars::Visuals::Viewmodel::CrosshairAim));
						gui::checkbox("Viewmodel Aim", VAR(Vars::Visuals::Viewmodel::ViewmodelAim));
						gui::slider("Offset X", VARP(Vars::Visuals::Viewmodel::OffsetX), -45.f, 45.f);
						gui::slider("Offset Y", VARP(Vars::Visuals::Viewmodel::OffsetY), -45.f, 45.f);
						gui::slider("Offset Z", VARP(Vars::Visuals::Viewmodel::OffsetZ), -45.f, 45.f);
						gui::slider("Pitch", VARP(Vars::Visuals::Viewmodel::Pitch), -180.f, 180.f);
						gui::slider("Yaw", VARP(Vars::Visuals::Viewmodel::Yaw), -180.f, 180.f);
						gui::slider("Roll", VARP(Vars::Visuals::Viewmodel::Roll), -180.f, 180.f);
						gui::slider("Sway Scale", VARP(Vars::Visuals::Viewmodel::SwayScale), 0.f, 5.f);
						gui::slider("Sway Interp", VARP(Vars::Visuals::Viewmodel::SwayInterp), 0.f, 1.f);

						Spacing();

						static const char* path_styles[] = { "Off", "Line", "Separators", "Spaced", "Arrows", "Boxes" };
						gui::combo("Player Path", VARP(Vars::Visuals::Simulation::PlayerPath), path_styles, IM_ARRAYSIZE(path_styles));
						gui::combo("Projectile Path", VARP(Vars::Visuals::Simulation::ProjectilePath), path_styles, IM_ARRAYSIZE(path_styles));
						gui::combo("Trajectory Path", VARP(Vars::Visuals::Simulation::TrajectoryPath), path_styles, IM_ARRAYSIZE(path_styles));
						gui::combo("Shot Path", VARP(Vars::Visuals::Simulation::ShotPath), path_styles, IM_ARRAYSIZE(path_styles));

						gui::checkbox("Timed Path", VAR(Vars::Visuals::Simulation::Timed));
						gui::checkbox("Path Box", VAR(Vars::Visuals::Simulation::Box));
						gui::checkbox("Projectile Camera", VAR(Vars::Visuals::Simulation::ProjectileCamera));
						gui::checkbox("Swing Lines", VAR(Vars::Visuals::Simulation::SwingLines));
						gui::slider("Draw Duration", VARP(Vars::Visuals::Simulation::DrawDuration), 0.f, 10.f);
					}
					gui::end_group_scrollable();

					if (gui::begin_group_scrollable("INTERFACE", ImVec2(380, 238), 5.f, 0.f))
					{

						gui::checkbox("Reveal Scoreboard", VAR(Vars::Visuals::UI::RevealScoreboard));
						gui::checkbox("Scoreboard Colors", VAR(Vars::Visuals::UI::ScoreboardColors));
						gui::checkbox("Scoreboard Utility", VAR(Vars::Visuals::UI::ScoreboardUtility));
						gui::checkbox("Clean Screenshots", VAR(Vars::Visuals::UI::CleanScreenshots));

						Spacing();

						gui::slider("Field of View", VARP(Vars::Visuals::UI::FieldOfView), 0.f, 160.f);
						gui::slider("Zoom Field of View", VARP(Vars::Visuals::UI::ZoomFieldOfView), 0.f, 160.f);
						gui::slider("Aspect Ratio", VARP(Vars::Visuals::UI::AspectRatio), 0.f, 5.f);

						Spacing();

						static const char* streamer_modes[] = { "Off", "Local", "Friends", "Party", "All" };
						gui::combo("Streamer Mode", VARP(Vars::Visuals::UI::StreamerMode),
							streamer_modes, IM_ARRAYSIZE(streamer_modes));
					}
					gui::end_group_scrollable();

					SameLine(390);
					if (gui::begin_group_scrollable("REMOVALS", ImVec2(380, 238), 5.f, 5.f))
					{

						gui::checkbox("Remove Scope", VAR(Vars::Visuals::Removals::Scope));
						gui::checkbox("Remove Post Processing", VAR(Vars::Visuals::Removals::PostProcessing));
						gui::checkbox("Remove Screen Overlays", VAR(Vars::Visuals::Removals::ScreenOverlays));
						gui::checkbox("Remove Screen Effects", VAR(Vars::Visuals::Removals::ScreenEffects));
						gui::checkbox("Remove View Punch", VAR(Vars::Visuals::Removals::ViewPunch));
						gui::checkbox("Remove Disguises", VAR(Vars::Visuals::Removals::Disguises));
						gui::checkbox("Remove Taunts", VAR(Vars::Visuals::Removals::Taunts));
						gui::checkbox("Remove Ragdolls", VAR(Vars::Visuals::Removals::Ragdolls));
						gui::checkbox("Remove Gibs", VAR(Vars::Visuals::Removals::Gibs));
						gui::checkbox("Remove MOTD", VAR(Vars::Visuals::Removals::MOTD));
						gui::checkbox("Remove Interpolation", VAR(Vars::Visuals::Removals::Interpolation));
						gui::checkbox("Remove Lerp", VAR(Vars::Visuals::Removals::Lerp));
						gui::checkbox("Remove Angle Forcing", VAR(Vars::Visuals::Removals::AngleForcing));
					}
					gui::end_group_scrollable();
				}

				break;
			}

			case 3: // PLAYERS
				// ─────────────────────────────────────────────────────────────────
				if (gui::begin_group_scrollable("PLAYERS", ImVec2(380, 500), 5.f, 5.f))
				{
					Text("Configure players here");
				}
				gui::end_group_scrollable();
				break;

				// ─────────────────────────────────────────────────────────────────
			case 4: // MISC
				// ─────────────────────────────────────────────────────────────────
			{
				static const char* autostrafe_modes[] = { "Off", "Legit", "Directional" };
				static const char* anti_backstab[] = { "Off", "Yaw", "Pitch", "Fake" };

				if (gui::begin_group_scrollable("MOVEMENT", ImVec2(380, 245), 5.f, 0.f))
				{

					gui::combo("Auto Strafe", VARP(Vars::Misc::Movement::AutoStrafe),
						autostrafe_modes, IM_ARRAYSIZE(autostrafe_modes));
					gui::slider("Turn Scale",
						VARP(Vars::Misc::Movement::AutoStrafeTurnScale), 0.f, 1.f);
					gui::checkbox("Bunnyhop", VAR(Vars::Misc::Movement::Bunnyhop));
					gui::checkbox("Edge Jump", VAR(Vars::Misc::Movement::EdgeJump));
					gui::checkbox("Auto Jumpbug", VAR(Vars::Misc::Movement::AutoJumpbug));
					gui::checkbox("Auto Rocket Jump", VAR(Vars::Misc::Movement::AutoRocketJump));
					gui::checkbox("Auto CTap", VAR(Vars::Misc::Movement::AutoCTap));
					gui::checkbox("Fast Stop", VAR(Vars::Misc::Movement::FastStop));
					gui::checkbox("Fast Accelerate", VAR(Vars::Misc::Movement::FastAccelerate));
					gui::checkbox("Duck Speed", VAR(Vars::Misc::Movement::DuckSpeed));
					gui::checkbox("No Push", VAR(Vars::Misc::Movement::NoPush));
					gui::checkbox("Break Jump", VAR(Vars::Misc::Movement::BreakJump));
				}
				gui::end_group_scrollable();

				SameLine(390);
				if (gui::begin_group_scrollable("AUTOMATION", ImVec2(380, 245), 5.f, 5.f))
				{

					gui::combo("Anti-Backstab", VARP(Vars::Misc::Automation::AntiBackstab),
						anti_backstab, IM_ARRAYSIZE(anti_backstab));
					gui::checkbox("Anti-AFK", VAR(Vars::Misc::Automation::AntiAFK));
					gui::checkbox("Anti-Autobalance", VAR(Vars::Misc::Automation::AntiAutobalance));
					gui::checkbox("Taunt Control", VAR(Vars::Misc::Automation::TauntControl));
					gui::checkbox("Accept Item Drops", VAR(Vars::Misc::Automation::AcceptItemDrops));

					Spacing();

					gui::checkbox("Pure Bypass", VAR(Vars::Misc::Exploits::PureBypass));
					gui::checkbox("Cheats Bypass", VAR(Vars::Misc::Exploits::CheatsBypass));
					gui::checkbox("Unlock CVars", VAR(Vars::Misc::Exploits::UnlockCVars));
					gui::checkbox("Backpack Expander", VAR(Vars::Misc::Exploits::BackpackExpander));
					gui::checkbox("Noisemaker Spam", VAR(Vars::Misc::Exploits::NoisemakerSpam));
					gui::checkbox("Ping Reducer", VAR(Vars::Misc::Exploits::PingReducer));
				}
				gui::end_group_scrollable();

				if (gui::begin_group_scrollable("SOUND", ImVec2(380, 235), 5.f, 5.f))
				{

					gui::checkbox("Hitsound Always", VAR(Vars::Misc::Sound::HitsoundAlways));
					gui::checkbox("Remove DSP", VAR(Vars::Misc::Sound::RemoveDSP));
					gui::checkbox("Giant Weapon Sounds", VAR(Vars::Misc::Sound::GiantWeaponSounds));
				}
				gui::end_group_scrollable();

				SameLine(390);
				if (gui::begin_group_scrollable("GAME", ImVec2(380, 235), 5.f, 5.f))
				{

					gui::checkbox("Anti-Cheat Compat", VAR(Vars::Misc::Game::AntiCheatCompatibility));
					gui::checkbox("F2P Chat Bypass", VAR(Vars::Misc::Game::F2PChatBypass));
					gui::checkbox("Network Fix", VAR(Vars::Misc::Game::NetworkFix));
					gui::checkbox("Bones Optimization", VAR(Vars::Misc::Game::SetupBonesOptimization));

					Spacing();

					gui::checkbox("Doubletap", VAR(Vars::Doubletap::Doubletap));
					gui::checkbox("Warp", VAR(Vars::Doubletap::Warp));
					gui::checkbox("Anti-Warp", VAR(Vars::Doubletap::AntiWarp));
					gui::slider("Tick Limit", (float*)VARP(Vars::Doubletap::TickLimit), 2.f, 22.f);
				}
				gui::end_group_scrollable();

				break;
			}

			} // end switch

			ImGui::PopClipRect();
		}
		EndGroup();
	}
	End();
}

// ────────────────────────────────────────────────────────────────────────────
void CMenu::Render()
{
	using namespace ImGui;

	if (!(GetIO().DisplaySize.x > 160.f && GetIO().DisplaySize.y > 28.f))
		return;

	static bool s_FirstRun = true;
	if (s_FirstRun)
	{
		s_FirstRun = false;
		gui::set_theme(ImVec4(0.f, 122.f / 255.f, 187.f / 255.f, 1.f));
	}

	m_bInKeybind = m_bWindowHovered = false;

	if (m_bIsOpen)
	{
		for (int k = 0; k < 256; ++k)
			U::KeyHandler.StoreKey(k);
	}
	else
	{
		U::KeyHandler.StoreKey(Vars::Menu::PrimaryKey.Value);
		U::KeyHandler.StoreKey(Vars::Menu::SecondaryKey.Value);
		U::KeyHandler.StoreKey(VK_F11);
	}

	if (U::KeyHandler.Pressed(Vars::Menu::PrimaryKey.Value) ||
		U::KeyHandler.Pressed(Vars::Menu::SecondaryKey.Value))
		I::MatSystemSurface->SetCursorAlwaysVisible(m_bIsOpen = !m_bIsOpen);

	PushFont(F::Render.FontRegular);

	if (m_bIsOpen)
	{
		DrawMenu();
		F::Render.Cursor = GetMouseCursor();
		m_bWindowHovered = IsWindowHovered(
			ImGuiHoveredFlags_AnyWindow |
			ImGuiHoveredFlags_AllowWhenBlockedByPopup |
			ImGuiHoveredFlags_AllowWhenBlockedByActiveItem);
	}

	PopFont();
}

// ────────────────────────────────────────────────────────────────────────────
void CMenu::AddOutput(const char* sFunction, const char* sLog, Color_t tColor)
{
	static size_t iID = 0;
	m_vOutput.emplace_back(sFunction, sLog, iID++, tColor);
	while (m_vOutput.size() > m_iMaxOutputSize)
		m_vOutput.pop_front();
}

// ────────────────────────────────────────────────────────────────────────────
void CMenu::SetIconTexture(IDirect3DTexture9* pTexture)
{
	s_IconTexture = pTexture;
	s_IconLoaded = true;
}

// ────────────────────────────────────────────────────────────────────────────
void CMenu::LoadIconTexture(LPDIRECT3DDEVICE9 pDevice)
{
	if (!s_IconLoaded && pDevice && !s_IconTexture)
	{
		s_IconTexture = LoadTextureFromMemory(pDevice, qo0_icons, sizeof(qo0_icons));
		if (s_IconTexture)
			s_IconLoaded = true;
	}
}
