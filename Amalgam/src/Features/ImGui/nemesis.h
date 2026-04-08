#pragma once
#define IMGUI_DEFINE_MATH_OPERATORS
#include <ImGui/imgui.h>
#include <ImGui/imgui_internal.h>
#include "Render.h"
#include "../../SDK/SDK.h"
#include <Windows.h>
#include <string>

#define STB_IMAGE_IMPLEMENTATION
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>

namespace gui {
    inline ImFont* font_menu() { return F::Render.FontBold; }
    inline ImFont* font_indicator() { return F::Render.FontRegular; }
    inline ImFont* font_title() { return F::Render.FontTitleBold; }
    inline ImFont* font_small() { return F::Render.FontSmall; }

    static ImVec4 g_current_accent_color = ImVec4(0.f, 122.f / 255.f, 187.f / 255.f, 1.f);

    inline ImU32 GetAccentColor(int alpha = 255) {
        Color_t c = Vars::Menu::Theme::Accent.Value;
        return IM_COL32(c.r, c.g, c.b, alpha);
    }
    inline ImU32 GetAccentColorFaded(int alpha = 70) {
        Color_t c = Vars::Menu::Theme::Accent.Value;
        return IM_COL32(c.r, c.g, c.b, alpha);
    }

    // Mede texto com a fonte que será usada para renderizar, evitando
    // divergência com CalcTextSize (que usa a fonte do contexto atual).
    static float _MeasureMenuText(const char* text) {
        ImFont* f = gui::font_menu();
        if (!f || !f->IsLoaded()) return ImGui::CalcTextSize(text).x;
        return f->CalcTextSizeA(f->FontSize, FLT_MAX, 0.f, text).x;
    }

    void set_theme(ImVec4 accent_color = ImVec4(0.f, 122.f / 255.f, 187.f / 255.f, 1.f)) {
        g_current_accent_color = accent_color;
        ImGuiStyle& style = ImGui::GetStyle();
        ImVec4* colors = style.Colors;
        style.FrameBorderSize = 1.f;  style.WindowBorderSize = 1.f;
        style.ChildBorderSize = 0.f;  style.ChildRounding = 0.f;
        style.WindowRounding = 0.f;  style.FrameRounding = 3.f;
        style.WindowPadding = { 0.f,8.f }; style.FramePadding = { 5.f,3.f };
        style.WindowTitleAlign = { .5f,.5f }; style.ItemSpacing = { 0.f,4.f };
        style.ScrollbarSize = 4.f;  style.ScrollbarRounding = 2.f;

        colors[ImGuiCol_Text] = { 1,1,1,1 };
        colors[ImGuiCol_TextDisabled] = { .5f,.5f,.5f,1 };
        colors[ImGuiCol_WindowBg] = { .06f,.06f,.06f,.85f };
        colors[ImGuiCol_ChildBg] = { .07f,.07f,.07f,1 };
        colors[ImGuiCol_PopupBg] = { .08f,.08f,.08f,.94f };
        colors[ImGuiCol_Border] = { .16f,.16f,.16f,1 };
        colors[ImGuiCol_BorderShadow] = { 0,0,0,0 };
        colors[ImGuiCol_FrameBg] = { .17f,.17f,.17f,1 };
        colors[ImGuiCol_FrameBgHovered] = { .05f,.05f,.05f,1 };
        colors[ImGuiCol_FrameBgActive] = { .05f,.05f,.05f,1 };
        colors[ImGuiCol_TitleBg] = { .04f,.04f,.04f,1 };
        colors[ImGuiCol_TitleBgActive] = { .05f,.05f,.05f,1 };
        colors[ImGuiCol_TitleBgCollapsed] = { 0,0,0,.51f };
        colors[ImGuiCol_MenuBarBg] = { .14f,.14f,.14f,1 };
        colors[ImGuiCol_ScrollbarBg] = { 0,0,0,0 };
        colors[ImGuiCol_ScrollbarGrab] = { .2f,.2f,.2f,.8f };
        colors[ImGuiCol_ScrollbarGrabHovered] = { .3f,.3f,.3f,1 };
        colors[ImGuiCol_ScrollbarGrabActive] = { .4f,.4f,.4f,1 };
        colors[ImGuiCol_CheckMark] = accent_color;
        colors[ImGuiCol_SliderGrab] = accent_color;
        colors[ImGuiCol_SliderGrabActive] = accent_color;
        colors[ImGuiCol_Button] = { .07f,.08f,.09f,1 };
        colors[ImGuiCol_ButtonHovered] = accent_color;
        colors[ImGuiCol_ButtonActive] = accent_color;
        colors[ImGuiCol_Header] = accent_color;
        colors[ImGuiCol_HeaderHovered] = accent_color;
        colors[ImGuiCol_HeaderActive] = accent_color;
        colors[ImGuiCol_Separator] = accent_color;
        auto dim = [&](float f) { return ImVec4(accent_color.x * f, accent_color.y * f, accent_color.z * f, accent_color.w); };
        colors[ImGuiCol_SeparatorHovered] = dim(.5f);
        colors[ImGuiCol_SeparatorActive] = dim(.5f);
        colors[ImGuiCol_ResizeGrip] = ImVec4(accent_color.x * .5f, accent_color.y * .5f, accent_color.z * .5f, accent_color.w * .2f);
        colors[ImGuiCol_ResizeGripHovered] = dim(.67f);
        colors[ImGuiCol_ResizeGripActive] = dim(.95f);
        colors[ImGuiCol_TabHovered] = dim(.8f);
        colors[ImGuiCol_Tab] = ImVec4(accent_color.x * .58f, accent_color.y * .58f, accent_color.z * .58f, accent_color.w * .86f);
        colors[ImGuiCol_PlotLines] = { .61f,.61f,.61f,1 };
        colors[ImGuiCol_PlotLinesHovered] = { 1,.43f,.35f,1 };
        colors[ImGuiCol_PlotHistogram] = { .9f,.7f,0,1 };
        colors[ImGuiCol_PlotHistogramHovered] = { 1,.6f,0,1 };
        colors[ImGuiCol_TextSelectedBg] = ImVec4(accent_color.x * .35f, accent_color.y * .35f, accent_color.z * .35f, accent_color.w);
        colors[ImGuiCol_DragDropTarget] = { 1,1,0,.9f };
        colors[ImGuiCol_NavWindowingHighlight] = { 1,1,1,.7f };
        colors[ImGuiCol_NavWindowingDimBg] = { .8f,.8f,.8f,.2f };
        colors[ImGuiCol_ModalWindowDimBg] = { .8f,.8f,.8f,.35f };
    }

    // ─── MODE SELECTORS ─────────────────────────────────────────────────────
    struct VisualsModeState {
        bool context_open = false, prev_context_open = false;
        float anim = 0; int selected_mode = 0; ImVec2 anchor_pos = { 0,0 };
    };
    static VisualsModeState g_visuals_mode_state, g_aimbot_mode_state;

    static void _DrawModeSelector(VisualsModeState& s, int* mode,
        const std::vector<std::string>& items) {
        if (s.context_open && !s.prev_context_open) s.anchor_pos = ImGui::GetMousePos();
        s.anim = s.context_open ? std::min(100.f, s.anim + 10.f) : std::max(0.f, s.anim - 10.f);
        if (s.anim > .01f) {
            ImDrawList* fg = ImGui::GetForegroundDrawList();
            float ih = 18.f, lw = 89.f, lh = (ih * items.size() + 4.f) * (s.anim / 100.f);
            ImVec2 lp = s.anchor_pos;
            ImVec2 disp = ImGui::GetIO().DisplaySize;
            if (lp.x + lw > disp.x) lp.x = disp.x - lw - 10.f;
            if (lp.y + lh > disp.y) lp.y = disp.y - lh - 10.f;
            lp.x = ImMax(lp.x, 5.f); lp.y = ImMax(lp.y, 5.f);
            ImRect lr(lp, { lp.x + lw,lp.y + lh });
            fg->AddRectFilled({ lp.x + 1,lp.y + 1 }, { lp.x + lw - 1,lp.y + (ih * items.size() + 2.f) * (s.anim / 100.f) }, IM_COL32(60, 60, 60, 255));
            fg->AddRect(lp, { lp.x + lw,lp.y + lh }, IM_COL32(15, 15, 15, 155));
            if (s.anim >= 100.f) {
                for (size_t i = 0; i < items.size(); i++) {
                    ImVec2 ip(lp.x + 5, lp.y + 4 + i * ih);
                    ImRect ir(ip, { ip.x + 88,ip.y + 15 });
                    bool hov = ImGui::IsMouseHoveringRect(ir.Min, ir.Max), sel = (*mode == (int)i);
                    fg->AddText(ImVec2(lp.x + 24, lp.y + 4 + i * ih), (hov || sel) ? GetAccentColor(255) : IM_COL32(120, 120, 120, 255), items[i].c_str());
                    if (hov && ImGui::IsMouseClicked(0)) { *mode = (int)i; s.context_open = false; }
                }
            }
            if (s.context_open && s.anim >= 100.f && ImGui::IsMouseClicked(0) && !ImGui::IsMouseHoveringRect(lr.Min, lr.Max))
                s.context_open = false;
        }
        s.prev_context_open = s.context_open;
    }
    void visuals_mode_selector(int* mode) { _DrawModeSelector(g_visuals_mode_state, mode, { "ESP","Materials","World" }); }
    void aimbot_mode_selector(int* mode) { _DrawModeSelector(g_aimbot_mode_state, mode, { "Hitscan","Projectile","Melee" }); }

    // ─── TOOLTIP HELPER ──────────────────────────────────────────────────────
    inline void begin_tooltip(const char* text, float delay_sec = 0.6f) {
        if (!ImGui::IsItemHovered()) return;
        if (ImGui::GetCurrentContext()->HoveredIdTimer < delay_sec) return;
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 6,5 });
        ImGui::PushStyleColor(ImGuiCol_PopupBg, IM_COL32(28, 28, 28, 245));
        ImGui::PushStyleColor(ImGuiCol_Border, IM_COL32(15, 15, 15, 200));
        ImGui::BeginTooltip();
        ImGui::TextUnformatted(text);
        ImGui::EndTooltip();
        ImGui::PopStyleColor(2);
        ImGui::PopStyleVar();
    }

    // ─── SEPARATOR ───────────────────────────────────────────────────────────
    void separator(const char* label = nullptr, float width = 330.f) {
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (window->SkipItems) return;
        ImVec2 pos = ImGui::GetCursorScreenPos();
        ImDrawList* dl = window->DrawList;
        constexpr float h = 9.f;

        if (!label || label[0] == '\0') {
            dl->AddLine({ pos.x,pos.y + h * .5f }, { pos.x + width,pos.y + h * .5f }, IM_COL32(40, 40, 40, 255));
        }
        else {
            ImFont* sf = gui::font_small() ? gui::font_small() : gui::font_menu();
            float fs = sf ? sf->FontSize : ImGui::GetFontSize();
            float tw = sf ? sf->CalcTextSizeA(fs, FLT_MAX, 0.f, label).x
                : ImGui::CalcTextSize(label).x;
            float lw = tw + 10.f;
            float line_y = pos.y + h * .5f;
            float tx = pos.x + (width - lw) * .5f;
            dl->AddLine({ pos.x,line_y }, { tx - 4.f,line_y }, IM_COL32(38, 38, 38, 255));
            dl->AddCircleFilled({ tx - 2.f,line_y }, 2.f, GetAccentColor(80), 6);
            dl->AddText(sf, 0.f, { tx + 5.f,pos.y + 1.f }, IM_COL32(65, 65, 65, 255), label);
            dl->AddCircleFilled({ tx + lw + 2.f,line_y }, 2.f, GetAccentColor(80), 6);
            dl->AddLine({ tx + lw + 4.f,line_y }, { pos.x + width,line_y }, IM_COL32(38, 38, 38, 255));
        }
        ImGui::Dummy({ width,h });
    }

    // ─── CHECKBOX ────────────────────────────────────────────────────────────
    bool checkbox(const char* label, bool& value, float row_width = 350.f) {
        ImGuiContext* ctx = ImGui::GetCurrentContext();
        if (!ctx) return false;
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (window->SkipItems) return false;

        ImDrawList* draw = ImGui::GetWindowDrawList();
        ImVec2 pos = ImGui::GetCursorScreenPos();
        constexpr float box_oy = 3.f, text_ox = 20.f;
        const ImVec2 box_sz(14.f, 14.f);

        ImVec2 text_size = ImGui::CalcTextSize(label);
        ImVec2 total_size(row_width, ImMax(box_sz.y + box_oy, text_size.y));

        ImGui::InvisibleButton(label, total_size);
        bool hovered = ImGui::IsItemHovered();
        if (ImGui::IsItemClicked()) value = !value;

        if (hovered)
            draw->AddRectFilled(pos, { pos.x + row_width,pos.y + total_size.y }, GetAccentColorFaded(12), 2.f);

        ImGuiID id = ImGui::GetID(label);
        ImGuiStorage* stor = ImGui::GetStateStorage();
        float progress = stor->GetFloat(id, value ? 1.f : 0.f);
        progress += ((value ? 1.f : 0.f) - progress) * 0.15f;
        stor->SetFloat(id, progress);

        ImVec2 bp(pos.x, pos.y + box_oy), be(bp.x + box_sz.x, bp.y + box_sz.y);
        draw->AddRectFilledMultiColor(bp, be,
            IM_COL32(52, 52, 52, 255), IM_COL32(52, 52, 52, 255),
            IM_COL32(41, 41, 41, 255), IM_COL32(41, 41, 41, 255));
        if (progress > .01f) {
            draw->AddRectFilledMultiColor(bp, be,
                GetAccentColor((int)(255 * progress)), GetAccentColor((int)(255 * progress)),
                IM_COL32(25, 25, 25, (int)(255 * progress)), IM_COL32(25, 25, 25, (int)(255 * progress)));
            draw->AddRect(bp, be, GetAccentColor((int)(70 * progress)));
            int a = (int)(255 * progress);
            draw->AddLine({ bp.x + 2.5f,bp.y + 7.f }, { bp.x + 5.5f,bp.y + 10.5f }, IM_COL32(255, 255, 255, a), 1.5f);
            draw->AddLine({ bp.x + 5.5f,bp.y + 10.5f }, { bp.x + 11.5f,bp.y + 3.5f }, IM_COL32(255, 255, 255, a), 1.5f);
        }
        draw->AddRect(bp, be, IM_COL32(15, 15, 15, 150));
        ImU32 tc = IM_COL32((int)(120 + 80 * progress), (int)(120 + 80 * progress), (int)(120 + 80 * progress), 255);
        draw->AddText(gui::font_menu(), 0.f, { pos.x + text_ox,pos.y + 2.f }, tc, label);

        ImGui::SetCursorScreenPos({ pos.x,pos.y + total_size.y + ctx->Style.ItemSpacing.y });
        return value;
    }

    // ─── SLIDER ──────────────────────────────────────────────────────────────
    bool slider(const char* label, float* value, float min, float max,
        const char* suffix = "", float width = 330.f) {
        ImGuiContext* ctx = ImGui::GetCurrentContext();
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (window->SkipItems) return false;

        const ImGuiStyle& style = ctx->Style;
        ImGuiID id = window->GetID(label);
        constexpr float track_h = 8.f, thumb_r = 5.5f;
        ImVec2 pos = window->DC.CursorPos;
        ImVec2 total({ width + 10.f, 15.f + track_h + thumb_r + 2.f });

        ImRect bb(pos, pos + total);
        ImGui::ItemSize(total, style.FramePadding.y);
        if (!ImGui::ItemAdd(bb, id)) return false;

        bool hovered, held;
        ImGui::ButtonBehavior(bb, id, &hovered, &held);
        if (held) *value = min + ImClamp((ImGui::GetIO().MousePos.x - bb.Min.x) / width, 0.f, 1.f) * (max - min);

        float t = ImSaturate((*value - min) / (max - min));
        ImDrawList* dl = window->DrawList;

        char buf[64]; snprintf(buf, sizeof(buf), " %.0f%s", *value, suffix);
        dl->AddText(gui::font_menu(), 0.f, ImVec2(pos.x, pos.y),
            IM_COL32(120, 120, 120, 255), (std::string(label) + buf).c_str());

        ImVec2 tp(pos.x, pos.y + 15.f), te(tp.x + width, tp.y + track_h);
        dl->AddRectFilled(tp, te, IM_COL32(40, 40, 40, 255), 3.f);
        if (t > .0f) {
            ImVec2 fe(tp.x + width * t, tp.y + track_h);
            dl->AddRectFilledMultiColor(tp, fe,
                GetAccentColor(255), GetAccentColor(255), GetAccentColor(80), GetAccentColor(80));
        }
        dl->AddRect(tp, te, IM_COL32(15, 15, 15, 155), 3.f);
        float tx = tp.x + width * t, ty = tp.y + track_h * .5f;
        dl->AddCircleFilled({ tx,ty }, thumb_r, GetAccentColor(255), 12);
        dl->AddCircle({ tx,ty }, thumb_r, IM_COL32(10, 10, 10, 220), 12, 1.5f);

        ImGui::SetCursorScreenPos({ pos.x,pos.y + total.y + style.ItemSpacing.y });
        return held;
    }

    // ─── TAB BUTTON ──────────────────────────────────────────────────────────
    bool TabButton(const char* label, int& active_var, int value) {
        ImGuiContext* ctx = ImGui::GetCurrentContext();
        if (!ctx) return false;
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (window->SkipItems) return false;

        ImVec2 ts = ImGui::CalcTextSize(label);
        ImVec2 pos = ImGui::GetCursorScreenPos();
        ImVec2 sz(ts.x + 10.f, ts.y + 7.f);

        ImGui::InvisibleButton(label, sz);
        bool is_active = (active_var == value), hovered = ImGui::IsItemHovered();
        if (ImGui::IsItemClicked()) active_var = value;

        ImGuiID id = ImGui::GetID(label);
        ImGuiStorage* stor = ImGui::GetStateStorage();
        float prog = stor->GetFloat(id, is_active ? 1.f : 0.f);
        prog += ((is_active ? 1.f : 0.f) - prog) * 0.15f;
        stor->SetFloat(id, prog);

        Color_t ac = Vars::Menu::Theme::Accent.Value;
        ImVec4 coff(120.f / 255, 120.f / 255, 120.f / 255, 1), con(ac.r / 255.f, ac.g / 255.f, ac.b / 255.f, 1);
        ImU32 tc = ImGui::GetColorU32(ImLerp(coff, con, prog));
        ImDrawList* draw = ImGui::GetWindowDrawList();
        ImVec2 tp(pos.x + (sz.x - ts.x) * .5f, pos.y + (sz.y - ts.y - 3.f) * .5f);
        draw->AddText(gui::font_menu(), 0.f, tp, tc, label);
        if (hovered && !is_active)
            draw->AddText(gui::font_menu(), 0.f, tp, IM_COL32(255, 255, 255, 22), label);
        if (prog > .01f) {
            float bw = sz.x * prog, bx = pos.x + (sz.x - bw) * .5f, by = pos.y + sz.y - 2.f;
            draw->AddRectFilled({ bx,by }, { bx + bw,by + 2.f }, GetAccentColor((int)(200 * prog)), 1.f);
        }
        return is_active;
    }

    // ─── GROUP BOX ───────────────────────────────────────────────────────────
    float g_GroupBoxContentOffsetX = 0.f, g_GroupBoxContentOffsetY = 0.f;

    static void _DrawGroupHeader(ImDrawList* draw, ImVec2 pos, float width, float hdr_h, const char* label) {
        float mid = pos.x + width * .5f;
        ImU32 tr = GetAccentColorFaded(0), ct = GetAccentColorFaded(55);
        draw->AddRectFilledMultiColor(pos, { mid,pos.y + hdr_h }, tr, ct, ct, tr);
        draw->AddRectFilledMultiColor({ mid,pos.y }, { pos.x + width,pos.y + hdr_h }, ct, tr, tr, ct);
        draw->AddCircleFilled({ pos.x + 8.f,pos.y + hdr_h * .5f }, 3.f, GetAccentColor(200), 8);
        ImVec2 tp(pos.x + 16.f, pos.y + 2.f);
        draw->AddText(gui::font_title(), 0.f, { tp.x + 1,tp.y + 1 }, IM_COL32(10, 10, 10, 150), label);
        draw->AddText(gui::font_title(), 0.f, tp, IM_COL32(220, 220, 220, 255), label);
    }

    bool begin_group(const char* label, ImVec2 size, float cx = 0.f, float cy = 0.f) {
        g_GroupBoxContentOffsetX = cx; g_GroupBoxContentOffsetY = cy;
        ImGuiWindow* w = ImGui::GetCurrentWindow();
        if (w->SkipItems) return false;
        ImGui::BeginGroup();
        ImVec2 pos = ImGui::GetCursorScreenPos();
        ImDrawList* draw = ImGui::GetWindowDrawList();
        ImVec2 end(pos.x + size.x, pos.y + size.y);
        constexpr float R = 5.f, H = 16.f;
        ImGui::Dummy(size);
        draw->AddRectFilled(pos, end, IM_COL32(32, 32, 32, 255), R);
        draw->PushClipRect(pos, end, true);
        _DrawGroupHeader(draw, pos, size.x, H, label);
        draw->PopClipRect();
        draw->AddRect(pos, end, GetAccentColor(80), R);
        ImGui::SetCursorScreenPos({ pos.x + 5.f + cx,pos.y + H + 5.f + cy });
        ImGui::Indent(5.f + cx);
        return true;
    }
    void end_group() { ImGui::Unindent(5.f + g_GroupBoxContentOffsetX); ImGui::EndGroup(); }

    static ImVec2 g_ScrollGroupPos, g_ScrollGroupSize;
    bool begin_group_scrollable(const char* label, ImVec2 size, float cx = 0.f, float cy = 0.f) {
        g_GroupBoxContentOffsetX = cx; g_GroupBoxContentOffsetY = cy;
        ImGuiWindow* w = ImGui::GetCurrentWindow();
        if (w->SkipItems) return false;
        ImGui::BeginGroup();
        ImVec2 pos = ImGui::GetCursorScreenPos();
        ImDrawList* draw = ImGui::GetWindowDrawList();
        ImVec2 end(pos.x + size.x, pos.y + size.y);
        g_ScrollGroupPos = pos; g_ScrollGroupSize = size;
        constexpr float R = 5.f, H = 16.f;
        ImGui::Dummy(size);
        draw->AddRectFilled(pos, end, IM_COL32(32, 32, 32, 255), R);
        draw->PushClipRect(pos, end, true);
        _DrawGroupHeader(draw, pos, size.x, H, label);
        draw->PopClipRect();
        draw->AddRect(pos, end, GetAccentColor(80), R);
        ImGui::SetCursorScreenPos({ pos.x + 8.f,pos.y + H + 5.f });
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0,0 });
        ImGui::BeginChild(label, { size.x - 12.f,size.y - H - 10.f }, false, ImGuiWindowFlags_NoBackground);
        ImGui::PopStyleVar();
        return true;
    }
    void end_group_scrollable() { ImGui::EndChild(); ImGui::EndGroup(); }

    // ─── HOTKEY ──────────────────────────────────────────────────────────────
    bool hotkey(const char* label, int* key, float width = 70.f, float height = 15.f) {
        ImGuiWindow* w = ImGui::GetCurrentWindow();
        if (w->SkipItems) return false;
        ImGuiID id = w->GetID(label);
        ImVec2 pos = ImGui::GetCursorScreenPos();
        ImRect bb(pos, { pos.x + width,pos.y + height });
        ImGui::ItemSize({ width,height });
        if (!ImGui::ItemAdd(bb, id)) return false;

        bool hovered, held; bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held);
        static ImGuiID s_cap = 0;
        bool is_active = (s_cap == id);
        if (pressed) s_cap = id;

        ImGuiStorage* stor = ImGui::GetStateStorage();
        float prog = stor->GetFloat(id, 0.f);
        prog += ((is_active ? 1.f : 0.f) - prog) * .12f;
        prog = ImClamp(prog, 0.f, 1.f);
        stor->SetFloat(id, prog);

        if (is_active) for (int vk = 1; vk <= 255; vk++) {
            if (!(GetAsyncKeyState(vk) & 0x8000)) continue;
            if (vk == VK_LBUTTON && pressed) continue;
            *key = (vk == VK_ESCAPE) ? 0 : vk; s_cap = 0; break;
        }

        ImDrawList* draw = w->DrawList;
        ImU32 bg = is_active ? IM_COL32(18, 20, 28, 255) : hovered ? IM_COL32(45, 45, 45, 255) : IM_COL32(22, 24, 26, 255);
        draw->AddRectFilled(bb.Min, bb.Max, bg, 3.f);
        if (is_active) {
            float p = ImSin((float)ImGui::GetTime() * 5.f) * .5f + .5f;
            draw->AddRect(bb.Min, bb.Max, GetAccentColor((int)(100 + 155 * p)), 3.f, 0, 1.5f);
        }
        else {
            draw->AddRect(bb.Min, bb.Max, hovered ? IM_COL32(75, 75, 75, 255) : IM_COL32(45, 45, 45, 255), 3.f);
            if (*key != 0)
                draw->AddLine({ bb.Min.x + 5,bb.Max.y - 1 }, { bb.Max.x - 5,bb.Max.y - 1 }, GetAccentColor(70));
        }

        char buf[16]; const char* kn = "...";
        if (!is_active) {
            int vk = *key;
            if (vk == 0)kn = "Unbound";
            else if (vk == VK_LBUTTON)kn = "M1"; else if (vk == VK_RBUTTON)kn = "M2";
            else if (vk == VK_MBUTTON)kn = "M3"; else if (vk == VK_XBUTTON1)kn = "M4"; else if (vk == VK_XBUTTON2)kn = "M5";
            else if (vk == VK_SHIFT)kn = "Shift"; else if (vk == VK_CONTROL)kn = "Ctrl";
            else if (vk == VK_MENU)kn = "Alt"; else if (vk == VK_SPACE)kn = "Space"; else if (vk == VK_RETURN)kn = "Enter";
            else if ((vk >= '0' && vk <= '9') || (vk >= 'A' && vk <= 'Z')) { sprintf_s(buf, sizeof(buf), "%c", (char)vk); kn = buf; }
            else kn = "Unknown";
        }
        ImVec2 ts = ImGui::CalcTextSize(kn);
        ImVec2 tp(bb.Min.x + (width - ts.x) * .5f, bb.Min.y + (height - ts.y) * .5f);
        ImU32 tc;
        if (is_active) { float p = ImSin((float)ImGui::GetTime() * 5.f) * .5f + .5f; tc = GetAccentColor((int)(170 + 85 * p)); }
        else { ImVec4 off(.48f, .48f, .48f, .8f), on(.9f, .9f, .9f, 1.f); tc = ImGui::GetColorU32(ImLerp(off, on, prog)); }
        draw->AddText(gui::font_menu(), 0.f, tp, tc, kn);
        return pressed;
    }

    // ─── SPINNER ─────────────────────────────────────────────────────────────
    bool Spinner(const char* label, float radius, int thickness, const ImU32& color) {
        ImGuiWindow* w = ImGui::GetCurrentWindow(); if (w->SkipItems) return false;
        ImGuiContext& g = *GImGui; const ImGuiStyle& s = g.Style; ImGuiID id = w->GetID(label);
        ImVec2 pos = w->DC.CursorPos; ImVec2 sz(radius * 2, (radius + s.FramePadding.y) * 2);
        ImRect bb(pos, { pos.x + sz.x,pos.y + sz.y });
        ImGui::ItemSize(bb, s.FramePadding.y); if (!ImGui::ItemAdd(bb, id)) return false;
        ImVec2 c(pos.x + radius, pos.y + radius + s.FramePadding.y);
        float t = (float)ImGui::GetTime(), sa = t * IM_PI * 2.f;
        w->DrawList->PathClear();
        for (int i = 0; i <= 20; i++) { float a = sa + ((float)i / 20) * IM_PI * .66f; w->DrawList->PathLineTo({ c.x + ImCos(a) * radius,c.y + ImSin(a) * radius }); }
        w->DrawList->PathStroke(color, false, thickness);
        return true;
    }

    // ─── COMBO helpers ────────────────────────────────────────────────────────
    struct ComboState { bool is_open = false; int stored_id = -1; float animation = 0.f; int hovered_item = -1; };
    static std::unordered_map<ImGuiID, ComboState> combo_states;

    void DrawArrow(ImDrawList* draw, ImVec2 pos, bool down, ImU32 color) {
        if (down) {
            draw->AddLine(pos, { pos.x + 4,pos.y + 4 }, color);
            draw->AddLine({ pos.x + 8,pos.y }, { pos.x + 4,pos.y + 4 }, color);
        }
        else {
            draw->AddLine({ pos.x,pos.y + 4 }, { pos.x + 4,pos.y }, color);
            draw->AddLine({ pos.x + 8,pos.y + 4 }, { pos.x + 4,pos.y }, color);
        }
    }

    static void _DrawDropdown(ImDrawList* draw, ImVec2 pos, float width, float height) {
        draw->AddRectFilled({ pos.x + 1,pos.y + 1 }, { pos.x + width + 1,pos.y + height + 1 }, IM_COL32(10, 10, 10, 180), 3.f);
        draw->AddRectFilled(pos, { pos.x + width,pos.y + height }, IM_COL32(32, 32, 32, 255), 3.f);
        draw->AddRect(pos, { pos.x + width,pos.y + height }, IM_COL32(15, 15, 15, 180), 3.f);
    }

    static void _DrawDropCheckmark(ImDrawList* draw, ImVec2 pos, ImU32 color) {
        draw->AddLine(pos, { pos.x + 3.f,pos.y + 3.5f }, color, 1.4f);
        draw->AddLine({ pos.x + 3.f,pos.y + 3.5f }, { pos.x + 8.f,pos.y - 2.f }, color, 1.4f);
    }

    // ─── COMBO (const char*[]) ────────────────────────────────────────────────
    bool combo(const char* label, int* current_item, const char* const items[],
        int items_count, float width = 350.f) {
        ImGuiContext* ctx = ImGui::GetCurrentContext();
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (window->SkipItems) return false;
        const ImGuiStyle& style = ctx->Style;
        ImGuiID id = window->GetID(label);
        ComboState& state = combo_states[id];
        ImVec2 pos = window->DC.CursorPos;
        constexpr float ch = 23.f, lh = 15.f;

        window->DrawList->AddText(gui::font_menu(), 0.f, { pos.x,pos.y }, IM_COL32(120, 120, 120, 255), label);
        ImVec2 cp(pos.x, pos.y + lh); ImRect cbb(cp, { cp.x + width,cp.y + ch });

        ImGui::SetCursorScreenPos(cp);
        ImGui::InvisibleButton("##cb", { width,ch });
        if (ImGui::IsItemClicked()) { state.is_open = !state.is_open; state.stored_id = state.is_open ? (int)id : -1; }
        if (state.is_open && ImGui::IsKeyPressed(ImGuiKey_Escape)) { state.is_open = false; state.stored_id = -1; }

        state.animation += ((state.is_open ? 1.f : 0.f) - state.animation) * .15f;
        ImDrawList* draw = window->DrawList;
        draw->AddRectFilledMultiColor(cbb.Min, cbb.Max,
            IM_COL32(41, 41, 41, 255), IM_COL32(41, 41, 41, 255),
            IM_COL32(49, 49, 49, 255), IM_COL32(49, 49, 49, 255));
        draw->AddRect(cbb.Min, cbb.Max, IM_COL32(15, 15, 15, 155));

        const char* preview = state.is_open ? "Press ESC to close"
            : (*current_item >= 0 && *current_item < items_count) ? items[*current_item] : "Select...";
        draw->AddText(gui::font_menu(), 0.f, { cp.x + 8,cp.y + 5 }, IM_COL32(120, 120, 120, 255), preview);
        DrawArrow(draw, { cp.x + width - 18,cp.y + 9 }, !state.is_open, IM_COL32(170, 170, 170, 255));

        bool value_changed = false;
        if (state.animation > .01f) {
            float ih = 20.f, dh = (ih * items_count + 4.f) * state.animation;
            ImVec2 dp(cp.x, cp.y + ch + 3.f);
            ImRect dbb(dp, { dp.x + width,dp.y + dh });
            _DrawDropdown(draw, dp, width, dh);
            if (state.animation > .95f) {
                for (int i = 0; i < items_count; i++) {
                    ImVec2 ip(dp.x + 1, dp.y + 2 + i * ih);
                    ImRect ib(ip, { ip.x + width - 2,ip.y + ih - 1 });
                    bool ih2 = ImGui::IsMouseHoveringRect(ib.Min, ib.Max), is = (*current_item == i);
                    if (is)       draw->AddRectFilled(ib.Min, ib.Max, IM_COL32(40, 40, 40, 255));
                    else if (ih2) draw->AddRectFilled(ib.Min, ib.Max, IM_COL32(45, 45, 45, 100));
                    if (is) _DrawDropCheckmark(draw, { ip.x + 6.f,ip.y + 9.f }, GetAccentColor(220));
                    ImU32 tc = (ih2 || is) ? GetAccentColor(255) : IM_COL32(120, 120, 120, 255);
                    draw->AddText(gui::font_menu(), 0.f, { ip.x + 18.f,ip.y + 4 }, tc, items[i]);
                    if (ih2 && ImGui::IsMouseClicked(0)) { *current_item = i; state.is_open = false; state.stored_id = -1; value_changed = true; }
                }
            }
            if (state.is_open && ImGui::IsMouseClicked(0)) {
                ImVec2 mp = ImGui::GetMousePos();
                if (!cbb.Contains(mp) && !dbb.Contains(mp)) { state.is_open = false; state.stored_id = -1; }
            }
        }
        float th = lh + ch + style.ItemSpacing.y;
        if (state.animation > .01f) th += (20.f * items_count + 4.f) * state.animation + 2.f;
        ImGui::SetCursorScreenPos({ pos.x,pos.y + th });
        return value_changed;
    }

    // ─── COMBO (vector<string>) ───────────────────────────────────────────────
    bool combo(const char* label, int* current_item, const std::vector<std::string>& items, float width = 350.f) {
        ImGuiContext* ctx = ImGui::GetCurrentContext();
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (window->SkipItems) return false;
        const ImGuiStyle& style = ctx->Style;
        ImGuiID id = window->GetID(label);
        ComboState& state = combo_states[id];
        ImVec2 pos = window->DC.CursorPos;
        constexpr float ch = 23.f, lh = 15.f;

        window->DrawList->AddText(gui::font_menu(), 0.f, { pos.x,pos.y }, IM_COL32(120, 120, 120, 255), label);
        ImVec2 cp(pos.x, pos.y + lh); ImRect cbb(cp, { cp.x + width,cp.y + ch });

        ImGui::SetCursorScreenPos(cp);
        ImGui::InvisibleButton("##cb", { width,ch });
        if (ImGui::IsItemClicked()) { state.is_open = !state.is_open; state.stored_id = state.is_open ? (int)id : -1; }
        if (state.is_open && ImGui::IsKeyPressed(ImGuiKey_Escape)) { state.is_open = false; state.stored_id = -1; }

        state.animation += ((state.is_open ? 1.f : 0.f) - state.animation) * .15f;
        ImDrawList* draw = window->DrawList;
        draw->AddRectFilledMultiColor(cbb.Min, cbb.Max,
            IM_COL32(41, 41, 41, 255), IM_COL32(41, 41, 41, 255),
            IM_COL32(49, 49, 49, 255), IM_COL32(49, 49, 49, 255));
        draw->AddRect(cbb.Min, cbb.Max, IM_COL32(15, 15, 15, 155));

        const char* preview = state.is_open ? "Press ESC to close"
            : (*current_item >= 0 && *current_item < (int)items.size()) ? items[*current_item].c_str() : "Select...";
        draw->AddText(gui::font_menu(), 0.f, { cp.x + 8,cp.y + 5 }, IM_COL32(120, 120, 120, 255), preview);
        DrawArrow(draw, { cp.x + width - 18,cp.y + 9 }, !state.is_open, IM_COL32(170, 170, 170, 255));

        bool value_changed = false;
        if (state.animation > .01f) {
            float ih = 20.f, dh = (ih * items.size() + 4.f) * state.animation;
            ImVec2 dp(cp.x, cp.y + ch + 3.f);
            ImRect dbb(dp, { dp.x + width,dp.y + dh });
            _DrawDropdown(draw, dp, width, dh);
            if (state.animation > .95f) {
                for (int i = 0; i < (int)items.size(); i++) {
                    ImVec2 ip(dp.x + 1, dp.y + 2 + i * ih);
                    ImRect ib(ip, { ip.x + width - 2,ip.y + ih - 1 });
                    bool ihov = ImGui::IsMouseHoveringRect(ib.Min, ib.Max), is = (*current_item == i);
                    if (is)        draw->AddRectFilled(ib.Min, ib.Max, IM_COL32(40, 40, 40, 255));
                    else if (ihov) draw->AddRectFilled(ib.Min, ib.Max, IM_COL32(45, 45, 45, 100));
                    if (is) _DrawDropCheckmark(draw, { ip.x + 6.f,ip.y + 9.f }, GetAccentColor(220));
                    ImU32 tc = (ihov || is) ? GetAccentColor(255) : IM_COL32(120, 120, 120, 255);
                    draw->AddText(gui::font_menu(), 0.f, { ip.x + 18.f,ip.y + 4 }, tc, items[i].c_str());
                    if (ihov && ImGui::IsMouseClicked(0)) { *current_item = i; state.is_open = false; state.stored_id = -1; value_changed = true; }
                }
            }
            if (state.is_open && ImGui::IsMouseClicked(0)) {
                ImVec2 mp = ImGui::GetMousePos();
                if (!cbb.Contains(mp) && !dbb.Contains(mp)) { state.is_open = false; state.stored_id = -1; }
            }
        }
        float th = lh + ch + style.ItemSpacing.y;
        if (state.animation > .01f) th += (20.f * items.size() + 4.f) * state.animation + 2.f;
        ImGui::SetCursorScreenPos({ pos.x,pos.y + th });
        return value_changed;
    }

    // forward declare
    bool color_picker(const char* label, Color_t* color, bool show_alpha = true, float bw = 14.f, float bh = 14.f);

    // ─── COLOR HINT ──────────────────────────────────────────────────────────
    // FIX: IDs eram "##ch1/2/3" fixos → colisão entre múltiplos combos na mesma
    // janela. Agora usa o ponteiro da cor como parte do ID para unicidade global.
    inline void color_hint(ImVec2 cs, ImVec2 ce, float cw, Color_t* c1, Color_t* c2 = nullptr, Color_t* c3 = nullptr) {
        if (!c1) return;
        constexpr float bx = 14.f, gap = 2.f, lh = 15.f;
        float cnt = c3 ? 3.f : c2 ? 2.f : 1.f, tot = cnt * bx + (cnt - 1.f) * gap;
        float x = (cs.x + cw) - tot - 6.f, y = cs.y + (lh - bx) * .5f;

        // FIX: IDs únicos por ponteiro de Color_t
        char id1[32], id2[32], id3[32];
        snprintf(id1, sizeof(id1), "##ch_%p", (void*)c1);
        ImGui::SetCursorScreenPos({ x,y }); color_picker(id1, c1, false, bx, bx);
        if (c2) {
            snprintf(id2, sizeof(id2), "##ch_%p", (void*)c2);
            ImGui::SetCursorScreenPos({ x + bx + gap,y }); color_picker(id2, c2, false, bx, bx);
        }
        if (c3) {
            snprintf(id3, sizeof(id3), "##ch_%p", (void*)c3);
            ImGui::SetCursorScreenPos({ x + (bx + gap) * 2,y }); color_picker(id3, c3, false, bx, bx);
        }
        ImGui::SetCursorScreenPos(ce);
    }

    // ─── MULTI COMBO ─────────────────────────────────────────────────────────
    struct MultiComboItem {
        std::string name; bool* value; Color_t* color;
        MultiComboItem(std::string n, bool* v, Color_t* c = nullptr) :name(n), value(v), color(c) {}
    };
    struct MultiComboState { bool is_open = false; int stored_id = -1; float animation = 0.f; std::vector<MultiComboItem>items; };
    static std::unordered_map<ImGuiID, MultiComboState> multi_combo_states;

    bool multi_combo(const char* label, std::vector<MultiComboItem>& items, float width = 350.f) {
        ImGuiContext* ctx = ImGui::GetCurrentContext();
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (window->SkipItems) return false;
        const ImGuiStyle& style = ctx->Style;
        ImGuiID id = window->GetID(label);
        MultiComboState& state = multi_combo_states[id];

        ImVec2 pos = window->DC.CursorPos;
        constexpr float ch = 23.f, lh = 15.f, cpH = 15.f, cpX = 5.f, cpG = 3.f;
        const float avail = width - 22.f;

        ImFont* fm = gui::font_menu();
        const float xw = fm ? fm->CalcTextSizeA(fm->FontSize, FLT_MAX, 0.f, "x").x
            : ImGui::CalcTextSize("x").x;
        auto ChipWidth = [&](float tw)->float { return cpX + tw + 4.f + xw + cpX; };

        window->DrawList->AddText(gui::font_menu(), 0.f, { pos.x,pos.y }, IM_COL32(120, 120, 120, 255), label);
        ImVec2 cp(pos.x, pos.y + lh); ImRect cbb(cp, { cp.x + width,cp.y + ch });

        // ── Fase 1: hit-test "x" ─────────────────────────────────────────────
        // FIX: usa continue (não break) para processar todos os chips visíveis
        // e usar a mesma lógica de visibilidade do loop de renderização.
        bool chip_clicked = false, value_changed = false;
        if (!state.is_open) {
            float cx2 = cp.x + 5.f, cy2 = cp.y + (ch - cpH) * .5f;
            for (auto& it : items) {
                if (!*it.value) continue;
                float tw = fm ? fm->CalcTextSizeA(fm->FontSize, FLT_MAX, 0.f, it.name.c_str()).x
                    : ImGui::CalcTextSize(it.name.c_str()).x;
                float cw2 = ChipWidth(tw);
                if (cx2 + cw2 > cp.x + avail) continue; // pula mas continua tentando próximos
                float x_start = cx2 + cpX + tw + 4.f;
                ImRect xr({ x_start - 1.f,cy2 }, { cx2 + cw2,cy2 + cpH });
                if (ImGui::IsMouseHoveringRect(xr.Min, xr.Max) && ImGui::IsMouseClicked(0)) {
                    *it.value = false; chip_clicked = true; value_changed = true;
                }
                cx2 += cw2 + cpG;
            }
        }

        ImGui::SetCursorScreenPos(cp);
        ImGui::InvisibleButton("##mc", { width,ch });
        if (ImGui::IsItemClicked() && !chip_clicked) {
            state.is_open = !state.is_open; state.stored_id = state.is_open ? (int)id : -1;
        }
        if (state.is_open && ImGui::IsKeyPressed(ImGuiKey_Escape)) { state.is_open = false; state.stored_id = -1; }
        state.animation += ((state.is_open ? 1.f : 0.f) - state.animation) * .15f;

        ImDrawList* draw = window->DrawList;
        draw->AddRectFilledMultiColor(cbb.Min, cbb.Max,
            IM_COL32(41, 41, 41, 255), IM_COL32(41, 41, 41, 255),
            IM_COL32(49, 49, 49, 255), IM_COL32(49, 49, 49, 255));
        draw->AddRect(cbb.Min, cbb.Max, IM_COL32(15, 15, 15, 155));

        // ── Fase 2: renderização dos chips ────────────────────────────────────
        // FIX: loop único com continue → conta overflow corretamente em uma passagem
        if (state.is_open) {
            draw->AddText(gui::font_menu(), 0.f, { cp.x + 8,cp.y + 5 }, IM_COL32(80, 80, 80, 255), "Press ESC to close");
        }
        else {
            float cx2 = cp.x + 5.f, cy2 = cp.y + (ch - cpH) * .5f;
            int overflow = 0;
            bool any = false;

            for (auto& it : items) {
                if (!*it.value) continue;
                float tw = fm ? fm->CalcTextSizeA(fm->FontSize, FLT_MAX, 0.f, it.name.c_str()).x
                    : ImGui::CalcTextSize(it.name.c_str()).x;
                float cw2 = ChipWidth(tw);

                if (cx2 + cw2 > cp.x + avail) { overflow++; continue; } // FIX: continue, conta todos

                any = true;
                draw->AddRectFilled({ cx2,cy2 }, { cx2 + cw2,cy2 + cpH }, GetAccentColorFaded(50), 3.f);
                draw->AddRect({ cx2,cy2 }, { cx2 + cw2,cy2 + cpH }, GetAccentColor(110), 3.f);
                draw->AddText(fm, 0.f, { cx2 + cpX,cy2 + 1.f }, IM_COL32(210, 210, 210, 255), it.name.c_str());

                float x_start = cx2 + cpX + tw + 4.f;
                ImRect xr({ x_start - 1.f,cy2 }, { cx2 + cw2,cy2 + cpH });
                bool xhov = ImGui::IsMouseHoveringRect(xr.Min, xr.Max);
                draw->AddText(fm, 0.f, { x_start,cy2 + 1.f },
                    xhov ? IM_COL32(255, 90, 90, 255) : IM_COL32(150, 150, 150, 255), "x");

                cx2 += cw2 + cpG;
            }

            // FIX: "+N" renderizado logo após o último chip visível
            if (overflow > 0) {
                char ob[8]; snprintf(ob, sizeof(ob), "+%d", overflow);
                float cy2_ = cp.y + (ch - cpH) * .5f;
                draw->AddText(fm, 0.f, { cx2 + 2.f,cy2_ + 1.f }, IM_COL32(100, 100, 100, 255), ob);
            }
            if (!any && overflow == 0)
                draw->AddText(gui::font_menu(), 0.f, { cp.x + 8,cp.y + 5 }, IM_COL32(70, 70, 70, 255), "None");
        }
        DrawArrow(draw, { cp.x + width - 18,cp.y + 9 }, !state.is_open, IM_COL32(170, 170, 170, 255));

        // ── Dropdown ─────────────────────────────────────────────────────────
        if (state.animation > .01f) {
            float ih = 20.f; int n = (int)items.size(); float dh = (ih * n + 4.f) * state.animation;
            ImVec2 dp(cp.x, cp.y + ch + 3.f);
            ImRect dbb(dp, { dp.x + width,dp.y + dh });
            _DrawDropdown(draw, dp, width, dh);

            if (state.animation > .95f) {
                for (int i = 0; i < n; i++) {
                    ImVec2 ip(dp.x + 1, dp.y + 2 + i * ih);
                    ImRect ib(ip, { ip.x + width - 2,ip.y + ih - 1 });
                    bool ihov = ImGui::IsMouseHoveringRect(ib.Min, ib.Max), isel = *items[i].value;
                    if (isel)       draw->AddRectFilled(ib.Min, ib.Max, IM_COL32(40, 40, 40, 255));
                    else if (ihov)  draw->AddRectFilled(ib.Min, ib.Max, IM_COL32(45, 45, 45, 100));
                    if (isel) _DrawDropCheckmark(draw, { ip.x + 6.f,ip.y + 9.f }, GetAccentColor(220));
                    draw->AddText(gui::font_menu(), 0.f, { ip.x + 18.f,ip.y + 4 },
                        (ihov || isel) ? GetAccentColor(255) : IM_COL32(120, 120, 120, 255),
                        items[i].name.c_str());

                    bool clr_click = false;
                    if (items[i].color) {
                        constexpr float cbw = 14, cbh = 14;
                        ImVec2 cbp(dp.x + width - cbw - 1, ip.y + (ih - cbh) * .5f);
                        ImRect cbr(cbp, { cbp.x + cbw,cbp.y + cbh });
                        clr_click = ImGui::IsMouseHoveringRect(cbr.Min, cbr.Max) && ImGui::IsMouseClicked(0);
                        ImVec2 prev = ImGui::GetCursorScreenPos();
                        ImGui::SetCursorScreenPos(cbp);
                        char cid[64]; snprintf(cid, sizeof(cid), "##mc_clr_%p_%d", (void*)id, i);
                        gui::color_picker(cid, items[i].color, false, cbw, cbh);
                        ImGui::SetCursorScreenPos(prev);
                    }
                    if (!clr_click && ihov && ImGui::IsMouseClicked(0)) { *items[i].value = !*items[i].value; value_changed = true; }
                }
            }
            if (state.is_open && ImGui::IsMouseClicked(0)) {
                ImVec2 mp = ImGui::GetMousePos();
                if (!cbb.Contains(mp) && !dbb.Contains(mp)) { state.is_open = false; state.stored_id = -1; }
            }
        }
        float th = lh + ch + style.ItemSpacing.y;
        if (state.animation > .01f) th += (20.f * items.size() + 4.f) * state.animation + 2.f;
        ImGui::SetCursorScreenPos({ pos.x,pos.y + th });
        return value_changed;
    }

    // ─── LISTBOX ─────────────────────────────────────────────────────────────
    std::string ToLower(std::string s) {
        std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) {return std::tolower(c); });
        return s;
    }
    struct ListboxState { int scroll_pos = 0; size_t last_temp_size = 0; };
    static std::unordered_map<ImGuiID, ListboxState> listbox_states;

    bool listbox(const char* label, int* current_item, const std::vector<std::string>& items,
        int visible_items, float width = 350.f, float height = 200.f, std::string* filter = nullptr) {
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (window->SkipItems) return false;
        ImGuiID id = ImGui::GetID(label);
        ListboxState& state = listbox_states[id];

        ImVec2 cp = ImGui::GetCursorScreenPos();
        if (label[0] != '#')
            window->DrawList->AddText(gui::font_menu(), 0.f, { cp.x + 5,cp.y + 4 }, IM_COL32(120, 120, 120, 255), label);

        ImVec2 bp(cp.x + 5, cp.y + 18);
        ImGui::Dummy({ width,18.f + height + 5.f });

        // FIX: scroll rect cobre a largura total do listbox
        ImRect wr(bp, { bp.x + width,bp.y + height });
        if (ImGui::IsMouseHoveringRect(wr.Min, wr.Max) && ImGui::GetIO().MouseWheel != 0.f)
            state.scroll_pos -= (int)ImGui::GetIO().MouseWheel;

        std::vector<std::pair<std::string, int>> temp;
        if (filter && !filter->empty()) {
            if (std::islower((*filter)[0])) (*filter)[0] = std::toupper((*filter)[0]);
            std::string lf = ToLower(*filter);
            for (size_t i = 0; i < items.size(); ++i)
                if (ToLower(items[i]).find(lf) != std::string::npos)
                    temp.emplace_back(items[i], (int)i);
        }
        else {
            for (size_t i = 0; i < items.size(); ++i) temp.emplace_back(items[i], (int)i);
        }
        if (temp.size() != state.last_temp_size) { state.scroll_pos = 0; state.last_temp_size = temp.size(); }
        state.scroll_pos = ImClamp(state.scroll_pos, 0, std::max(0, (int)temp.size() - visible_items));

        ImDrawList* draw = window->DrawList;
        draw->AddRectFilled(bp, bp + ImVec2(width, height), IM_COL32(45, 45, 45, 255));
        draw->AddRect(bp, bp + ImVec2(width, height), IM_COL32(15, 15, 15, 150));

        bool changed = false;
        if (!temp.empty()) {
            float ih = 22.f; int drawn = 0;
            for (size_t i = state.scroll_pos; i < temp.size() && drawn < visible_items; ++i, ++drawn) {
                ImVec2 ip(bp.x + 12, bp.y + 6 + drawn * ih);
                // FIX: hit rect cobre a linha inteira (full width e full height)
                // Antes era apenas 120px — clicar à direita do texto não selecionava o item
                ImRect ir({ bp.x,bp.y + drawn * ih }, { bp.x + width,bp.y + (drawn + 1) * ih });
                bool ihov = ImGui::IsMouseHoveringRect(ir.Min, ir.Max), sel = (*current_item == temp[i].second);

                if (!sel && drawn % 2 == 1)
                    draw->AddRectFilled({ bp.x,bp.y + drawn * ih }, { bp.x + width,bp.y + (drawn + 1) * ih }, IM_COL32(42, 42, 42, 255));
                if (sel)
                    draw->AddRectFilled({ bp.x,bp.y + drawn * ih }, { bp.x + width,bp.y + (drawn + 1) * ih }, IM_COL32(60, 60, 60, 255));
                draw->AddText(gui::font_menu(), 0.f, ip, (ihov || sel) ? GetAccentColor(255) : IM_COL32(160, 160, 160, 255), temp[i].first.c_str());
                if (ihov && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) { *current_item = temp[i].second; changed = true; }
            }
        }
        return changed;
    }

    // ─── KEYBIND ─────────────────────────────────────────────────────────────
    struct KeybindState { bool binding = false; std::string name = "NONE"; bool list_open = false; float anim = 0, anim_text = 0; };
    static std::unordered_map<ImGuiID, KeybindState> keybind_states;

    const char* keybind_keys[254] = {
        "NONE","M1","M2","BRK","M3","M4","M5","NONE","Bspc","Tab","NONE","NONE","NONE","Enter",
        "NONE","NONE","Shift","Ctrl","Alt","Pau","Caps","NONE","NONE","NONE","NONE","NONE","NONE","Esc",
        "NONE","NONE","NONE","NONE","Space","PgUp","PgDn","End","Home","Left","Up","Right","Down","NONE",
        "Prnt","NONE","PrtScr","Ins","Del","NONE","0","1","2","3","4","5","6","7","8","9",
        "NONE","NONE","NONE","NONE","NONE","NONE","NONE","A","B","C","D","E","F","G","H","I","J","K","L",
        "M","N","O","P","Q","R","S","T","U","V","W","X","Y","Z","LWin","RWin","NONE","NONE","NONE",
        "Num0","Num1","Num2","Num3","Num4","Num5","Num6","Num7","Num8","Num9","*","+","_","-",".","/",
        "F1","F2","F3","F4","F5","F6","F7","F8","F9","F10","F11","F12","F13","F14","F15","F16",
        "F17","F18","F19","F20","F21","F22","F23","F24","NONE","NONE","NONE","NONE","NONE","NONE","NONE","NONE",
        "NumLk","ScrLk","NONE","NONE","NONE","NONE","NONE","NONE","NONE","NONE","NONE","NONE","NONE","NONE","NONE","NONE",
        "LShft","RShft","LCtrl","RCtrl","LAlt","RAlt","NONE","NONE","NONE","NONE","NONE","NONE","NONE","NONE","NONE","NONE",
        "NTrk","PTrk","Stop","Play","NONE","NONE","NONE","NONE","NONE","NONE",";","+",",","-",".","/~",
        "NONE","NONE","NONE","NONE","NONE","NONE","NONE","NONE","NONE","NONE","NONE","NONE","NONE","NONE","NONE","NONE",
        "NONE","NONE","NONE","NONE","NONE","NONE","NONE","NONE","NONE","NONE","NONE","NONE","NONE","NONE","NONE","NONE",
        "NONE","NONE","NONE","NONE","NONE","NONE","NONE","NONE","NONE","NONE","{","\\|","}" };

    bool keybind(const char* label, int* value, int* bind_type) {
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (window->SkipItems) return false;
        ImGuiID id = window->GetID(label);
        KeybindState& s = keybind_states[id];

        char buf[128]; bool good = false;
        if (s.binding) s.name = "...";
        else {
            if (*value >= 0 && *value < 254 && keybind_keys[*value]) { s.name = keybind_keys[*value]; good = true; }
            else if (GetKeyNameTextA(*value << 16, buf, 127)) { s.name = buf; good = true; }
            if (!good) s.name = "NONE";
        }

        ImVec2 pos = ImGui::GetCursorScreenPos();
        float tw = _MeasureMenuText(s.name.c_str());
        constexpr float px = 6.f, py = 2.f;
        ImVec2 csz(tw + px * 2.f, ImGui::GetTextLineHeight() + py * 2.f);
        ImVec2 cpos(pos.x - csz.x, pos.y), cend(pos.x, pos.y + csz.y);

        ImDrawList* draw = window->DrawList;
        draw->AddRectFilled(cpos, cend, IM_COL32(22, 24, 26, 255), 3.f);
        draw->AddRect(cpos, cend, IM_COL32(50, 50, 50, 255), 3.f);
        draw->AddLine({ cpos.x + 2,cend.y + 1 }, { cend.x - 2,cend.y + 1 }, IM_COL32(35, 35, 35, 255));
        draw->AddText(gui::font_menu(), 0.f, { cpos.x + px,cpos.y + py }, IM_COL32(120, 120, 120, 255), s.name.c_str());

        ImGui::SetCursorScreenPos(cpos);
        ImGui::InvisibleButton("##kb", csz);
        bool changed = false;
        if (ImGui::IsItemClicked(0)) s.binding = true;
        if (s.binding) for (int i = 0; i < 255; i++) {
            if (!(GetAsyncKeyState(i) & 0x8000)) continue;
            if (i == VK_LBUTTON) continue;
            *value = (i == VK_ESCAPE) ? -1 : i; s.binding = false; changed = true; break;
        }
        if (ImGui::IsItemClicked(1)) s.list_open = !s.list_open;

        s.anim = s.list_open ? std::min(100.f, s.anim + 10.f) : std::max(0.f, s.anim - 10.f);
        s.anim_text = s.list_open ? std::min(255.f, s.anim_text + 15.f) : std::max(0.f, s.anim_text - 15.f);

        if (s.anim > .01f) {
            ImDrawList* fg = ImGui::GetForegroundDrawList();
            std::vector<std::string> ti = { "Always","Hold on","Toggle","Hold off" };
            float ih = 18.f, lw = 89.f, lh = (ih * ti.size() + 4.f) * (s.anim / 100.f);
            ImVec2 lp(pos.x + 4, pos.y);
            fg->AddRectFilled({ lp.x + 1,lp.y + 1 }, { lp.x + lw - 1,lp.y + (ih * ti.size() + 2.f) * (s.anim / 100.f) }, IM_COL32(60, 60, 60, 255));
            fg->AddRect(lp, { lp.x + lw,lp.y + lh }, IM_COL32(15, 15, 15, 155));
            if (s.anim >= 100.f) for (size_t t = 0; t < ti.size(); t++) {
                ImVec2 ip(lp.x + 5, lp.y + 4 + t * ih);
                ImRect ib(ip, { ip.x + 88,ip.y + 15 });
                bool hov = ImGui::IsMouseHoveringRect(ib.Min, ib.Max, false), sel = (*bind_type == (int)t);
                if (sel)       fg->AddRectFilled(ib.Min, ib.Max, IM_COL32(40, 40, 40, 255));
                else if (hov)  fg->AddRectFilled(ib.Min, ib.Max, IM_COL32(45, 45, 45, 100));
                fg->AddText(gui::font_menu(), 0.f, { lp.x + 24,lp.y + 4 + t * ih },
                    (hov || sel) ? GetAccentColor(255) : IM_COL32(120, 120, 120, 255), ti[t].c_str());
                if (hov && ImGui::GetIO().MouseClicked[0]) { *bind_type = (int)t; s.list_open = false; changed = true; }
            }
            if (s.list_open && s.anim >= 100.f && ImGui::GetIO().MouseClicked[0] &&
                !ImGui::IsMouseHoveringRect(lp, { lp.x + lw,lp.y + (ih * ti.size() + 4.f) }, false))
                s.list_open = false;
        }
        ImGui::SetCursorScreenPos({ ImGui::GetCurrentWindow()->DC.CursorStartPos.x,
            pos.y + csz.y + 1.f + ImGui::GetStyle().ItemSpacing.y });
        return changed;
    }

    // ─── RGB / HSV ────────────────────────────────────────────────────────────
    struct RGB { double r, g, b; };
    struct HSV { double h, s, v; };
    inline HSV rgb_to_hsv(const RGB& In) {
        HSV m; double mn = std::min({ In.r,In.g,In.b }), mx = std::max({ In.r,In.g,In.b });
        m.v = mx; double d = mx - mn;
        if (d < .0001) { m.s = m.h = 0; return m; }
        if (mx > 0) m.s = d / mx; else { m.s = m.h = 0; return m; }
        if (In.r >= mx) m.h = (In.g - In.b) / d;
        else if (In.g >= mx) m.h = 2 + (In.b - In.r) / d;
        else m.h = 4 + (In.r - In.g) / d;
        m.h *= 60; if (m.h < 0) m.h += 360; return m;
    }
    inline RGB hsv_to_rgb(const HSV& In) {
        RGB m;
        if (In.s <= 0) { m.r = m.g = m.b = In.v; return m; }
        double HH = (In.h >= 360 ? 0 : In.h) / 60; long i = (long)HH; double f = HH - i;
        double P = In.v * (1 - In.s), Q = In.v * (1 - In.s * f), T = In.v * (1 - In.s * (1 - f));
        switch (i) {
        case 0:m.r = In.v; m.g = T; m.b = P; break; case 1:m.r = Q; m.g = In.v; m.b = P; break;
        case 2:m.r = P; m.g = In.v; m.b = T; break; case 3:m.r = P; m.g = Q; m.b = In.v; break;
        case 4:m.r = T; m.g = P; m.b = In.v; break; default:m.r = In.v; m.g = P; m.b = Q; break;
        }
        return m;
    }

    // ─── COLOR PICKER ─────────────────────────────────────────────────────────
    // FIX PRINCIPAL: O popup do color picker agora fica "pinado" na posição de
    // tela onde foi aberto (SetNextWindowPos com ImGuiCond_Always). Antes, quando
    // chamado dentro de regiões scrolláveis (begin_group_scrollable, dropdown do
    // multi_combo), o popup se deslocava com o scroll do pai ou ficava clippado.
    // Agora a posição é calculada e clamped ao viewport no momento do clique e
    // reimposta todo frame → o popup nunca se move, independentemente do scroll.
    struct ColorPickerState {
        bool is_open = false; float hue = 0; ImVec2 sv_cursor = { 0,0 }; float alpha = 1;
        char hex_buf[8] = {};
        ImVec2 popup_pos = { 0,0 }; // posição de tela clamped, calculada ao abrir
    };
    static std::unordered_map<ImGuiID, ColorPickerState> g_color_picker_states;
    static Color_t rainbow_colors[7] = {
        {255,0,0,255},{255,255,0,255},{0,255,0,255},{0,255,255,255},
        {0,0,255,255},{255,0,255,255},{255,0,0,255} };

    static void _DrawChecker(ImDrawList* dl, ImVec2 pos, float w, float h) {
        dl->PushClipRect(pos, { pos.x + w,pos.y + h }, true);
        constexpr int sz = 3;
        int cols = (int)ceilf(w / sz), rows = (int)ceilf(h / sz);
        for (int r = 0; r < rows; r++) for (int c = 0; c < cols; c++) {
            ImU32 cc = ((r + c) % 2 == 0) ? IM_COL32(90, 90, 90, 255) : IM_COL32(55, 55, 55, 255);
            float x = pos.x + c * sz, y = pos.y + r * sz;
            dl->AddRectFilled({ x,y }, { x + (float)sz,y + (float)sz }, cc);
        }
        dl->PopClipRect();
    }

    bool color_picker(const char* label, Color_t* color, bool show_alpha, float box_width, float box_height) {
        if (!color) return false;
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (window->SkipItems) return false;
        ImGuiID id = window->GetID(label);
        ColorPickerState& state = g_color_picker_states[id];

        ImVec2 pos = ImGui::GetCursorScreenPos();
        ImDrawList* draw = window->DrawList;

        _DrawChecker(draw, pos, box_width, box_height);
        draw->AddRectFilled(pos, { pos.x + box_width,pos.y + box_height },
            IM_COL32(color->r, color->g, color->b, color->a));
        draw->AddRect(pos, { pos.x + box_width,pos.y + box_height }, IM_COL32(15, 15, 15, 155));

        ImGui::SetCursorScreenPos(pos);
        ImGui::InvisibleButton(label, { box_width,box_height });
        bool value_changed = false;

        // Calcula dimensões do popup antes do IsItemClicked para poder clampar
        constexpr float sv_sz = 180.f, hw = 15.f, sp = 8.f, abh = 18.f, hexh = 20.f;
        const float pw = sv_sz + hw + sp + 16.f;
        const float ph = sv_sz + (show_alpha ? abh + sp : 0.f) + sp + hexh + 16.f;

        if (ImGui::IsItemClicked()) {
            ImGui::OpenPopup(label);
            HSV hsv = rgb_to_hsv({ color->r / 255.,color->g / 255.,color->b / 255. });
            state.hue = (float)(hsv.h / 360.);
            state.sv_cursor = { (float)(hsv.s * sv_sz),(float)((1. - hsv.v) * sv_sz) };
            state.alpha = color->a / 255.f;
            snprintf(state.hex_buf, sizeof(state.hex_buf), "%02X%02X%02X", color->r, color->g, color->b);

            // FIX: calcula posição de abertura em coordenadas de tela absolutas
            // e clamp dentro do viewport agora, ao abrir — não frame a frame.
            ImVec2 disp = ImGui::GetIO().DisplaySize;
            state.popup_pos = { pos.x, pos.y + box_height + 2.f };
            if (state.popup_pos.x + pw > disp.x) state.popup_pos.x = disp.x - pw - 5.f;
            if (state.popup_pos.y + ph > disp.y) state.popup_pos.y = pos.y - ph - 2.f; // abre acima
            state.popup_pos.x = ImMax(state.popup_pos.x, 5.f);
            state.popup_pos.y = ImMax(state.popup_pos.y, 5.f);
        }

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 8,8 });
        ImGui::PushStyleColor(ImGuiCol_PopupBg, IM_COL32(35, 35, 35, 255));
        ImGui::PushStyleColor(ImGuiCol_Border, IM_COL32(15, 15, 15, 255));

        // FIX: ImGuiCond_Always → o popup fica fixo na posição calculada ao abrir,
        // independentemente de qualquer scroll no pai. Sem isso, o popup "flutuava"
        // com a região scrollável que o continha.
        ImGui::SetNextWindowPos(state.popup_pos, ImGuiCond_Always);
        ImGui::SetNextWindowSize({ pw,ph }, ImGuiCond_Always);

        if (ImGui::BeginPopup(label, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize)) {
            ImDrawList* dl = ImGui::GetWindowDrawList();
            ImVec2 pp = ImGui::GetCursorScreenPos(), svp = pp;

            RGB hc = hsv_to_rgb({ state.hue * 360.,1,1 });
            ImU32 hcol = IM_COL32((int)(hc.r * 255), (int)(hc.g * 255), (int)(hc.b * 255), 255);
            dl->AddRectFilledMultiColor(svp, { svp.x + sv_sz,svp.y + sv_sz },
                IM_COL32(255, 255, 255, 255), hcol, hcol, IM_COL32(255, 255, 255, 255));
            dl->AddRectFilledMultiColor(svp, { svp.x + sv_sz,svp.y + sv_sz },
                IM_COL32(0, 0, 0, 0), IM_COL32(0, 0, 0, 0), IM_COL32(0, 0, 0, 255), IM_COL32(0, 0, 0, 255));
            dl->AddRect(svp, { svp.x + sv_sz,svp.y + sv_sz }, IM_COL32(15, 15, 15, 200));

            float cx2 = svp.x + state.sv_cursor.x, cy2 = svp.y + state.sv_cursor.y;
            dl->AddCircleFilled({ cx2,cy2 }, 6.f, IM_COL32(0, 0, 0, 200), 16);
            dl->AddCircle({ cx2,cy2 }, 5.f, IM_COL32(255, 255, 255, 255), 16, 2.f);

            ImVec2 hp(svp.x + sv_sz + sp, svp.y);
            for (int i = 0; i < 6; i++) {
                float sh = sv_sz / 6.f;
                dl->AddRectFilledMultiColor({ hp.x,hp.y + sh * i }, { hp.x + hw,hp.y + sh * (i + 1) },
                    IM_COL32(rainbow_colors[i].r, rainbow_colors[i].g, rainbow_colors[i].b, 255),
                    IM_COL32(rainbow_colors[i].r, rainbow_colors[i].g, rainbow_colors[i].b, 255),
                    IM_COL32(rainbow_colors[i + 1].r, rainbow_colors[i + 1].g, rainbow_colors[i + 1].b, 255),
                    IM_COL32(rainbow_colors[i + 1].r, rainbow_colors[i + 1].g, rainbow_colors[i + 1].b, 255));
            }
            dl->AddRect(hp, { hp.x + hw,hp.y + sv_sz }, IM_COL32(15, 15, 15, 200));
            float hiy = hp.y + sv_sz * state.hue;
            dl->AddLine({ hp.x - 2,hiy }, { hp.x + hw + 2,hiy }, IM_COL32(0, 0, 0, 255), 3.f);
            dl->AddLine({ hp.x - 2,hiy }, { hp.x + hw + 2,hiy }, IM_COL32(255, 255, 255, 255), 1.5f);

            ImVec2 mouse = ImGui::GetMousePos(); bool mdn = ImGui::IsMouseDown(0);

            if (show_alpha) {
                ImVec2 ab(svp.x, svp.y + sv_sz + sp);
                for (int y = 0; y < 2; y++) for (int x = 0; x < 15; x++) {
                    ImU32 cc = ((x + y) % 2 == 0) ? IM_COL32(200, 200, 200, 255) : IM_COL32(140, 140, 140, 255);
                    dl->AddRectFilled({ ab.x + x * 12.f,ab.y + y * 9.f }, { ab.x + x * 12.f + 12,ab.y + y * 9.f + 9 }, cc);
                }
                dl->AddRectFilledMultiColor(ab, { ab.x + sv_sz,ab.y + abh },
                    IM_COL32(color->r, color->g, color->b, 0), IM_COL32(color->r, color->g, color->b, 255),
                    IM_COL32(color->r, color->g, color->b, 255), IM_COL32(color->r, color->g, color->b, 0));
                dl->AddRect(ab, { ab.x + sv_sz,ab.y + abh }, IM_COL32(15, 15, 15, 200));
                float aix = ab.x + sv_sz * state.alpha;
                dl->AddLine({ aix,ab.y - 2 }, { aix,ab.y + abh + 2 }, IM_COL32(0, 0, 0, 255), 3.f);
                dl->AddLine({ aix,ab.y - 2 }, { aix,ab.y + abh + 2 }, IM_COL32(255, 255, 255, 255), 1.5f);
                if (mdn && mouse.x >= ab.x && mouse.x <= ab.x + sv_sz && mouse.y >= ab.y && mouse.y <= ab.y + abh)
                {
                    state.alpha = ImSaturate((mouse.x - ab.x) / sv_sz); value_changed = true;
                }
            }

            bool dragged = false;
            if (mdn && mouse.x >= svp.x && mouse.x <= svp.x + sv_sz && mouse.y >= svp.y && mouse.y <= svp.y + sv_sz)
            {
                state.sv_cursor = { ImClamp(mouse.x - svp.x,0.f,sv_sz),ImClamp(mouse.y - svp.y,0.f,sv_sz) }; value_changed = dragged = true;
            }
            else if (mdn && mouse.x >= hp.x && mouse.x <= hp.x + hw && mouse.y >= hp.y && mouse.y <= hp.y + sv_sz)
            {
                state.hue = ImClamp((mouse.y - hp.y) / sv_sz, 0.f, 1.f); value_changed = dragged = true;
            }

            float hxy = svp.y + sv_sz + (show_alpha ? abh + sp : 0.f) + sp;
            ImVec2 ho(svp.x, hxy);
            _DrawChecker(dl, ho, 14, 14);
            dl->AddRectFilled(ho, { ho.x + 14,ho.y + 14 }, IM_COL32(color->r, color->g, color->b, 255), 2.f);
            dl->AddRect(ho, { ho.x + 14,ho.y + 14 }, IM_COL32(15, 15, 15, 200), 2.f);

            ImGui::SetCursorScreenPos({ ho.x + 18,ho.y });
            ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(25, 25, 25, 255));
            ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, IM_COL32(35, 35, 35, 255));
            ImGui::PushStyleColor(ImGuiCol_FrameBgActive, IM_COL32(40, 40, 40, 255));
            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(170, 170, 170, 255));
            ImGui::SetNextItemWidth(sv_sz - 20.f);

            bool hex_ed = ImGui::InputText("##hex_in", state.hex_buf, sizeof(state.hex_buf),
                ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_CharsUppercase | ImGuiInputTextFlags_AutoSelectAll);
            if (hex_ed && strlen(state.hex_buf) == 6) {
                auto hx = [](char hi, char lo)->uint8_t {
                    auto d = [](char c)->int {return(c >= '0' && c <= '9') ? c - '0' : (c >= 'A' && c <= 'F') ? c - 'A' + 10 : (c >= 'a' && c <= 'f') ? c - 'a' + 10 : 0; };
                    return(uint8_t)(d(hi) * 16 + d(lo)); };
                color->r = hx(state.hex_buf[0], state.hex_buf[1]);
                color->g = hx(state.hex_buf[2], state.hex_buf[3]);
                color->b = hx(state.hex_buf[4], state.hex_buf[5]);
                HSV hsv = rgb_to_hsv({ color->r / 255.,color->g / 255.,color->b / 255. });
                state.hue = (float)(hsv.h / 360.);
                state.sv_cursor = { (float)(hsv.s * sv_sz),(float)((1. - hsv.v) * sv_sz) };
                value_changed = true;
            }
            ImGui::PopStyleColor(4);

            if (value_changed) {
                float sat = state.sv_cursor.x / sv_sz, val = 1.f - state.sv_cursor.y / sv_sz;
                RGB nc = hsv_to_rgb({ state.hue * 360.,sat,val });
                color->r = (uint8_t)(nc.r * 255); color->g = (uint8_t)(nc.g * 255);
                color->b = (uint8_t)(nc.b * 255); color->a = (uint8_t)(state.alpha * 255);
                if (dragged && ImGui::GetActiveID() != ImGui::GetID("##hex_in"))
                    snprintf(state.hex_buf, sizeof(state.hex_buf), "%02X%02X%02X", color->r, color->g, color->b);
            }
            ImGui::EndPopup();
        }
        ImGui::PopStyleColor(2); ImGui::PopStyleVar();
        ImGui::SetCursorScreenPos({ pos.x + box_width + 2,pos.y });
        return value_changed;
    }

    // ─── CHECKBOX_COLOR ───────────────────────────────────────────────────────
    bool checkbox_color(const char* label, bool& value, Color_t* color, bool show_alpha = false, float row_width = 350.f) {
        ImGuiContext* ctx = ImGui::GetCurrentContext();
        if (!ctx) return false;
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (window->SkipItems) return false;

        ImDrawList* draw = ImGui::GetWindowDrawList();
        ImVec2 pos = ImGui::GetCursorScreenPos();
        constexpr float boy = 3.f, tox = 20.f;
        const ImVec2 bsz(14.f, 14.f);

        ImVec2 ts = ImGui::CalcTextSize(label);
        ImVec2 cbsz(row_width, ImMax(bsz.y + boy, ts.y));

        ImGuiID id = ImGui::GetID(label);
        ImRect cbb(pos, { pos.x + cbsz.x,pos.y + cbsz.y });
        ImGui::ItemSize(cbb); if (!ImGui::ItemAdd(cbb, id)) return false;

        ImVec2 cp_tl(pos.x + row_width - bsz.x - 4.f, pos.y + 2.f);
        ImVec2 cp_br(cp_tl.x + bsz.x, cp_tl.y + bsz.y);
        bool hovered_cp = ImGui::IsMouseHoveringRect(cp_tl, cp_br);

        bool hovered = ImGui::IsMouseHoveringRect(cbb.Min, cbb.Max);
        if (hovered && !hovered_cp && ImGui::IsMouseClicked(0)) value = !value;
        if (hovered) draw->AddRectFilled(pos, { pos.x + row_width,pos.y + cbsz.y }, GetAccentColorFaded(12), 2.f);

        ImGuiStorage* stor = ImGui::GetStateStorage();
        float prog = stor->GetFloat(id, value ? 1.f : 0.f);
        prog += ((value ? 1.f : 0.f) - prog) * .15f; stor->SetFloat(id, prog);

        ImVec2 bp(pos.x, pos.y + boy), be(bp.x + bsz.x, bp.y + bsz.y);
        draw->AddRectFilledMultiColor(bp, be,
            IM_COL32(52, 52, 52, 255), IM_COL32(52, 52, 52, 255),
            IM_COL32(41, 41, 41, 255), IM_COL32(41, 41, 41, 255));
        if (prog > .01f) {
            draw->AddRectFilledMultiColor(bp, be,
                GetAccentColor((int)(255 * prog)), GetAccentColor((int)(255 * prog)),
                IM_COL32(25, 25, 25, (int)(255 * prog)), IM_COL32(25, 25, 25, (int)(255 * prog)));
            draw->AddRect(bp, be, GetAccentColor((int)(70 * prog)));
            int a = (int)(255 * prog);
            draw->AddLine({ bp.x + 2.5f,bp.y + 7.f }, { bp.x + 5.5f,bp.y + 10.5f }, IM_COL32(255, 255, 255, a), 1.5f);
            draw->AddLine({ bp.x + 5.5f,bp.y + 10.5f }, { bp.x + 11.5f,bp.y + 3.5f }, IM_COL32(255, 255, 255, a), 1.5f);
        }
        draw->AddRect(bp, be, IM_COL32(15, 15, 15, 150));
        ImU32 tc = IM_COL32((int)(120 + 80 * prog), (int)(120 + 80 * prog), (int)(120 + 80 * prog), 255);
        draw->AddText(gui::font_menu(), 0.f, { pos.x + tox,pos.y + 2.f }, tc, label);

        ImGui::SetCursorScreenPos(cp_tl);
        char pid[64]; snprintf(pid, sizeof(pid), "##color_%s", label);
        gui::color_picker(pid, color, show_alpha);

        ImGui::SetCursorScreenPos({ pos.x,pos.y + cbsz.y + ctx->Style.ItemSpacing.y });
        return value;
    }

    // ─── BUTTON ──────────────────────────────────────────────────────────────
    bool button(const char* label, ImVec2 size = { 360,32 }) {
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (window->SkipItems) return false;
        ImGuiContext* ctx = ImGui::GetCurrentContext();
        const ImGuiStyle& style = ctx->Style;
        ImGuiID id = window->GetID(label);
        ImVec2 pos = ImGui::GetCursorScreenPos();
        ImRect bb(pos, { pos.x + size.x,pos.y + size.y });
        ImGui::ItemSize(bb, style.FramePadding.y);
        if (!ImGui::ItemAdd(bb, id)) return false;

        bool hovered, held; bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held);
        ImDrawList* draw = window->DrawList;

        if (held) {
            draw->AddRectFilled(bb.Min, bb.Max, IM_COL32(38, 38, 38, 255), 3.f);
            draw->AddRect(bb.Min, bb.Max, IM_COL32(15, 15, 15, 155), 3.f);
            draw->AddRectFilled({ bb.Min.x + 4,bb.Max.y - 2 }, { bb.Max.x - 4,bb.Max.y }, GetAccentColor(100), 1.f);
        }
        else {
            ImU32 ct = hovered ? IM_COL32(85, 85, 85, 255) : IM_COL32(72, 72, 72, 255);
            ImU32 cb = hovered ? IM_COL32(95, 95, 95, 255) : IM_COL32(107, 107, 107, 255);
            draw->AddRectFilledMultiColor(bb.Min, bb.Max, cb, cb, ct, ct);
            draw->AddRect(bb.Min, bb.Max, IM_COL32(15, 15, 15, 155), 3.f);
        }

        ImVec2 ts = ImGui::CalcTextSize(label);
        ImVec2 tp(pos.x + (size.x - ts.x) * .5f, pos.y + (size.y - ts.y) * .5f);
        float sh = held ? 0.f : 1.f;
        draw->AddText(gui::font_menu(), 0.f, { tp.x + 1,tp.y + sh }, IM_COL32(10, 10, 10, 235), label);
        ImU32 tc = held ? IM_COL32(130, 130, 130, 255) : hovered ? IM_COL32(220, 220, 220, 255) : IM_COL32(180, 180, 180, 255);
        draw->AddText(gui::font_menu(), 0.f, tp, tc, label);
        return pressed;
    }

} // namespace gui