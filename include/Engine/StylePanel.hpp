// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

// WHAT'S THIS
// Extension to the imgui library v.1.21 wip (https://github.com/ocornut/imgui)
// to support saving/loading imgui styles.

// USAGE:
/*
1) Compile this cpp file
2) In your main.cpp just add (without including any additional file: there's no header!):
extern bool ImGuiSaveStyle(const char* filename,const ImGuiStyle& style);
extern bool ImGuiLoadStyle(const char* filename,ImGuiStyle& style);
3) Use them together with ImGui::GetStyle() to save/load the current style.
   ImGui::GetStyle() returns a reference of the current style that can be set/get.
Please note that other style options are not globally serializable because they are "window flags",
that must be set on a per-window basis (for example border,titlebar,scrollbar,resizable,movable,per-window alpha).
To edit and save a style, you can use the default ImGui example and append to the "Debug" window the following code:
            ImGui::Text("\n");
            ImGui::Text("Please modify the current style in:");
            ImGui::Text("ImGui Test->Window Options->Style Editor");
            static bool loadCurrentStyle = false;
            static bool saveCurrentStyle = false;
            static bool resetCurrentStyle = false;
            loadCurrentStyle = ImGui::Button("Load Saved Style");
            saveCurrentStyle = ImGui::Button("Save Current Style");
            resetCurrentStyle = ImGui::Button("Reset Current Style");
            if (loadCurrentStyle)   {
                if (!ImGuiLoadStyle("./myimgui.style",ImGui::GetStyle()))   {
                    fprintf(stderr,"Warning: \"./myimgui.style\" not present.\n");
                }
            }
            if (saveCurrentStyle)   {
                if (!ImGuiSaveStyle("./myimgui.style",ImGui::GetStyle()))   {
                    fprintf(stderr,"Warning: \"./myimgui.style\" cannot be saved.\n");
                }
            }
            if (resetCurrentStyle)  ImGui::GetStyle() = ImGuiStyle();
*/

#include "Engine/Log.hpp"
#include "Engine/FileExplorer.hpp"

#include "backends/imgui_impl_opengl3.h"
#include <imgui.h>
#include <vector>
#include <math.h> // sqrtf

#ifdef _WIN32
#define IM_NEWLINE "\r\n"
#else
#define IM_NEWLINE "\n"
#endif

// From <imgui.cpp>:--------------------------------------------------------
#include <stdio.h> // vsnprintf
#define IM_ARRAYSIZE(_ARR) ((int)(sizeof(_ARR) / sizeof(*_ARR)))
#include <string.h>

#define PATH_UI_STYLE RESOURCE_PATH "setting/UIStyle.ini"

static size_t ImFormatString(char* buf, size_t buf_size, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    int w = vsnprintf(buf, buf_size, fmt, args);
    va_end(args);
    buf[buf_size - 1] = 0;
    return (w == -1) ? buf_size : (size_t)w;
}
//---------------------------------------------------------------------------

void ImGuiSaveStyle(const char* filename, const ImGuiStyle& style, const char* currentFont)
{
    // Write .style file
    FILE* f = nullptr;
    if (fopen_s(&f, filename, "wt"))
    {
        logf("The file \"%s\" was not opened to write\n", filename);
        return;
    }

    fprintf(f, "[Alpha]\n%1.3f\n", style.Alpha);
    fprintf(f, "[WindowPadding]\n%1.3f %1.3f\n", style.WindowPadding.x, style.WindowPadding.y);
    fprintf(f, "[WindowMinSize]\n%1.3f %1.3f\n", style.WindowMinSize.x, style.WindowMinSize.y);
    fprintf(f, "[FramePadding]\n%1.3f %1.3f\n", style.FramePadding.x, style.FramePadding.y);
    fprintf(f, "[FrameRounding]\n%1.3f\n", style.FrameRounding);
    fprintf(f, "[ItemSpacing]\n%1.3f %1.3f\n", style.ItemSpacing.x, style.ItemSpacing.y);
    fprintf(f, "[ItemInnerSpacing]\n%1.3f %1.3f\n", style.ItemInnerSpacing.x, style.ItemInnerSpacing.y);
    fprintf(f, "[TouchExtraPadding]\n%1.3f %1.3f\n", style.TouchExtraPadding.x, style.TouchExtraPadding.y);
    fprintf(f, "[WindowRounding]\n%1.3f\n", style.WindowRounding);
    fprintf(f, "[ColumnsMinSpacing]\n%1.3f\n", style.ColumnsMinSpacing);
    if (strcmp(currentFont, "default") == 0)
        fprintf(f, "[Font]\n%s\n", currentFont);
    else
        fprintf(f, "[Font]\n%s %f\n", currentFont, ImGui::GetFont()->ConfigData->SizePixels);

    for (ImGuiCol i = 0; i != ImGuiCol_COUNT; i++)
    {
        const ImVec4& c = style.Colors[i];
        fprintf(f, "[%s]\n", ImGui::GetStyleColorName(i)); // ImGuiColNames[i]);
        fprintf(f, "%1.3f %1.3f %1.3f %1.3f\n", c.x, c.y, c.z, c.w);
    }

    fprintf(f, "\n");
    fclose(f);

    logf("Style exported\n");
}

void ImGuiLoadStyle(const char* filename, ImGuiStyle& style)
{
    // Load file into memory
    FILE* f = nullptr;
    if (fopen_s(&f, filename, "rt"))
    {
        logf("The file \"%s\" was not opened to write\n", filename);
        return;
    }

    if (fseek(f, 0, SEEK_END))
    {
        fclose(f);
        return;
    }
    const long f_size_signed = ftell(f);
    if (f_size_signed == -1)
    {
        fclose(f);
        return;
    }
    size_t f_size = (size_t)f_size_signed;
    if (fseek(f, 0, SEEK_SET))
    {
        fclose(f);
        return;
    }
    char* f_data = (char*)ImGui::MemAlloc(f_size + 1);
    f_size = fread(f_data, 1, f_size, f); // Text conversion alter read size so let's not be fussy about return value
    fclose(f);
    if (f_size == 0)
    {
        ImGui::MemFree(f_data);
        return;
    }
    f_data[f_size] = 0;

    // Parse file in memory
    char name[128];
    name[0]             = '\0';
    const char* buf_end = f_data + f_size;
    for (const char* line_start = f_data; line_start < buf_end;)
    {
        const char* line_end = line_start;
        while (line_end < buf_end && *line_end != '\n' && *line_end != '\r')
            line_end++;

        if (name[0] == '\0' && line_start[0] == '[' && line_end > line_start && line_end[-1] == ']')
        {
            ImFormatString(name, IM_ARRAYSIZE(name), "%.*s", line_end - line_start - 2, line_start + 1);
            // fprintf(stderr,"name: %s\n",name);  // dbg
        }
        else if (name[0] != '\0')
        {

            float* pf[4] = {0, 0, 0, 0};
            int    npf   = 0;

            // parsing 'name' here by filling the fields above
            {
                if (strcmp(name, "Alpha") == 0)
                {
                    npf   = 1;
                    pf[0] = &style.Alpha;
                }
                else if (strcmp(name, "WindowPadding") == 0)
                {
                    npf   = 2;
                    pf[0] = &style.WindowPadding.x;
                    pf[1] = &style.WindowPadding.y;
                }
                else if (strcmp(name, "WindowMinSize") == 0)
                {
                    npf   = 2;
                    pf[0] = &style.WindowMinSize.x;
                    pf[1] = &style.WindowMinSize.y;
                }
                else if (strcmp(name, "FramePadding") == 0)
                {
                    npf   = 2;
                    pf[0] = &style.FramePadding.x;
                    pf[1] = &style.FramePadding.y;
                }
                else if (strcmp(name, "FrameRounding") == 0)
                {
                    npf   = 1;
                    pf[0] = &style.FrameRounding;
                }
                else if (strcmp(name, "ItemSpacing") == 0)
                {
                    npf   = 2;
                    pf[0] = &style.ItemSpacing.x;
                    pf[1] = &style.ItemSpacing.y;
                }
                else if (strcmp(name, "ItemInnerSpacing") == 0)
                {
                    npf   = 2;
                    pf[0] = &style.ItemInnerSpacing.x;
                    pf[1] = &style.ItemInnerSpacing.y;
                }
                else if (strcmp(name, "TouchExtraPadding") == 0)
                {
                    npf   = 2;
                    pf[0] = &style.TouchExtraPadding.x;
                    pf[1] = &style.TouchExtraPadding.y;
                }
                else if (strcmp(name, "WindowRounding") == 0)
                {
                    npf   = 1;
                    pf[0] = &style.WindowRounding;
                }
                else if (strcmp(name, "ColumnsMinSpacing") == 0)
                {
                    npf   = 1;
                    pf[0] = &style.ColumnsMinSpacing;
                }
                else if (strcmp(name, "Font") == 0)
                {
                    npf = 5;
                }
                // all the colors here
                else
                {
                    for (int j = 0; j < ImGuiCol_COUNT; j++)
                    {
                        if (strcmp(name, ImGui::GetStyleColorName(j)) == 0)
                        {
                            npf           = 4;
                            ImVec4& color = style.Colors[j];
                            pf[0]         = &color.x;
                            pf[1]         = &color.y;
                            pf[2]         = &color.z;
                            pf[3]         = &color.w;
                            break;
                        }
                    }
                }
            }

            // fprintf(stderr,"name: %s npf=%d\n",name,npf);  // dbg
            // parsing values here and filling pf[]
            float x, y, z, w;
            switch (npf)
            {
            case 1:
                if (sscanf_s(line_start, "%f", &x) == npf)
                {
                    *pf[0] = x;
                }
                else
                    fprintf(stderr, "Warning in ImGui::LoadStyle(\"%s\"): skipped [%s] (parsing error).\n", filename,
                            name);
                break;
            case 2:
                if (sscanf_s(line_start, "%f %f", &x, &y) == npf)
                {
                    *pf[0] = x;
                    *pf[1] = y;
                }
                else
                    fprintf(stderr, "Warning in ImGui::LoadStyle(\"%s\"): skipped [%s] (parsing error).\n", filename,
                            name);
                break;
            case 3:
                if (sscanf_s(line_start, "%f %f %f", &x, &y, &z) == npf)
                {
                    *pf[0] = x;
                    *pf[1] = y;
                    *pf[2] = z;
                }
                else
                    fprintf(stderr, "Warning in ImGui::LoadStyle(\"%s\"): skipped [%s] (parsing error).\n", filename,
                            name);
                break;
            case 4:
                if (sscanf_s(line_start, "%f %f %f %f", &x, &y, &z, &w) == npf)
                {
                    *pf[0] = x;
                    *pf[1] = y;
                    *pf[2] = z;
                    *pf[3] = w;
                }
                break;
            case 5: {
                const char* line_endProps = line_start;
                char        str[256];
                int         strSize = 0;
                float       v       = 0.f;

                // Read the str
                while (line_endProps < buf_end && *line_endProps != ' ' && *line_endProps != '\n' &&
                       *line_endProps != '\r')
                {
                    str[strSize++] = *line_endProps;
                    line_endProps++;
                }
                line_endProps++;
                str[strSize] = '\0';

                if (strcmp(str, "Default") != 0)
                {
                    // Read the size
                    sscanf_s(line_endProps, "%f", &v);

                    if (!ImGui::GetIO().Fonts->Locked)
                    {
                        if (ImGui::GetIO().Fonts->AddFontFromFileTTF(str, v > 0 ? v : 14))
                        {
                            ImGui_ImplOpenGL3_CreateFontsTexture();
                        }
                    }
                }
            }
            break;
            default:
                fprintf(stderr, "Warning in ImGui::LoadStyle(\"%s\"): skipped [%s] (unknown field).\n", filename, name);
                break;
            }
            /*
            // Same reference code from <imgui.cpp> to help parsing
            float x, y;
            int i;
            if (sscanf_s(line_start, "Pos=%f,%f", &x, &y) == 2)
                settings->Pos = ImVec2(x, y);
            else if (sscanf_s(line_start, "Size=%f,%f", &x, &y) == 2)
                settings->Size = ImMax(ImVec2(x, y), g.Style.WindowMinSize);
            else if (sscanf_s(line_start, "Collapsed=%d", &i) == 1)
                settings->Collapsed = (i != 0);
            */
            //---------------------------------------------------------------------------------
            name[0] = '\0'; // mandatory
        }

        line_start = line_end + 1;
    }

    // Release memory
    ImGui::MemFree(f_data);
    logf("Style imported");
}

static void HelpMarker(const char* desc)
{
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

static void setDefaultTheme()
{
    using namespace ImGui;

    auto& style = GetStyle();
    ImVec4* colors                         = style.Colors;
    colors[ImGuiCol_Text]                  = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    colors[ImGuiCol_TextDisabled]          = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
    colors[ImGuiCol_WindowBg]              = ImVec4(0.06f, 0.06f, 0.06f, 0.94f);
    colors[ImGuiCol_ChildBg]               = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_PopupBg]               = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
    colors[ImGuiCol_Border]                = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
    colors[ImGuiCol_BorderShadow]          = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_FrameBg]               = ImVec4(0.44f, 0.44f, 0.44f, 0.60f);
    colors[ImGuiCol_FrameBgHovered]        = ImVec4(0.57f, 0.57f, 0.57f, 0.70f);
    colors[ImGuiCol_FrameBgActive]         = ImVec4(0.76f, 0.76f, 0.76f, 0.80f);
    colors[ImGuiCol_TitleBg]               = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
    colors[ImGuiCol_TitleBgActive]         = ImVec4(0.16f, 0.16f, 0.16f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed]      = ImVec4(0.00f, 0.00f, 0.00f, 0.60f);
    colors[ImGuiCol_MenuBarBg]             = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    colors[ImGuiCol_ScrollbarBg]           = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
    colors[ImGuiCol_ScrollbarGrab]         = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered]  = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive]   = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
    colors[ImGuiCol_CheckMark]             = ImVec4(0.13f, 0.75f, 0.55f, 0.80f);
    colors[ImGuiCol_SliderGrab]            = ImVec4(0.13f, 0.75f, 0.75f, 0.80f);
    colors[ImGuiCol_SliderGrabActive]      = ImVec4(0.13f, 0.75f, 1.00f, 0.80f);
    colors[ImGuiCol_Button]                = ImVec4(0.13f, 0.75f, 0.55f, 0.40f);
    colors[ImGuiCol_ButtonHovered]         = ImVec4(0.13f, 0.75f, 0.75f, 0.60f);
    colors[ImGuiCol_ButtonActive]          = ImVec4(0.13f, 0.75f, 1.00f, 0.80f);
    colors[ImGuiCol_Header]                = ImVec4(0.13f, 0.75f, 0.55f, 0.40f);
    colors[ImGuiCol_HeaderHovered]         = ImVec4(0.13f, 0.75f, 0.75f, 0.60f);
    colors[ImGuiCol_HeaderActive]          = ImVec4(0.13f, 0.75f, 1.00f, 0.80f);
    colors[ImGuiCol_Separator]             = ImVec4(0.13f, 0.75f, 0.55f, 0.40f);
    colors[ImGuiCol_SeparatorHovered]      = ImVec4(0.13f, 0.75f, 0.75f, 0.60f);
    colors[ImGuiCol_SeparatorActive]       = ImVec4(0.13f, 0.75f, 1.00f, 0.80f);
    colors[ImGuiCol_ResizeGrip]            = ImVec4(0.13f, 0.75f, 0.55f, 0.40f);
    colors[ImGuiCol_ResizeGripHovered]     = ImVec4(0.13f, 0.75f, 0.75f, 0.60f);
    colors[ImGuiCol_ResizeGripActive]      = ImVec4(0.13f, 0.75f, 1.00f, 0.80f);
    colors[ImGuiCol_Tab]                   = ImVec4(0.13f, 0.75f, 0.55f, 0.80f);
    colors[ImGuiCol_TabHovered]            = ImVec4(0.13f, 0.75f, 0.75f, 0.80f);
    colors[ImGuiCol_TabActive]             = ImVec4(0.13f, 0.75f, 1.00f, 0.80f);
    colors[ImGuiCol_TabUnfocused]          = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
    colors[ImGuiCol_TabUnfocusedActive]    = ImVec4(0.36f, 0.36f, 0.36f, 0.54f);
    colors[ImGuiCol_PlotLines]             = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered]      = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
    colors[ImGuiCol_PlotHistogram]         = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered]  = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
    colors[ImGuiCol_TableHeaderBg]         = ImVec4(0.19f, 0.19f, 0.20f, 1.00f);
    colors[ImGuiCol_TableBorderStrong]     = ImVec4(0.31f, 0.31f, 0.35f, 1.00f);
    colors[ImGuiCol_TableBorderLight]      = ImVec4(0.23f, 0.23f, 0.25f, 1.00f);
    colors[ImGuiCol_TableRowBg]            = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_TableRowBgAlt]         = ImVec4(1.00f, 1.00f, 1.00f, 0.07f);
    colors[ImGuiCol_TextSelectedBg]        = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
    colors[ImGuiCol_DragDropTarget]        = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
    colors[ImGuiCol_NavHighlight]          = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
    colors[ImGuiCol_NavWindowingDimBg]     = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
    colors[ImGuiCol_ModalWindowDimBg]      = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);

    style.TabRounding       = 4;
    style.GrabRounding      = 16;
    style.ScrollbarRounding = 9;
    style.WindowRounding    = 4;
    style.GrabRounding      = 3;
    style.FrameRounding     = 3;
    style.PopupRounding     = 4;
    style.ChildRounding     = 4;
}

// [Internal] Display details for a single font, called by ShowStyleEditor().
static void NodeFont(ImFont* font)
{
    ImGuiIO&    io                  = ImGui::GetIO();
    ImGuiStyle& style               = ImGui::GetStyle();
    bool        font_details_opened = ImGui::TreeNode(font, "Font: \"%s\"\n%.2f px, %d glyphs, %d file(s)",
                                               font->ConfigData ? font->ConfigData[0].Name : "", font->FontSize,
                                               font->Glyphs.Size, font->ConfigDataCount);
    ImGui::SameLine();
    if (ImGui::SmallButton("Set as default"))
    {
        io.FontDefault = font;
    }
    if (!font_details_opened)
        return;

    ImGui::PushFont(font);
    ImGui::Text("The quick brown fox jumps over the lazy dog");
    ImGui::PopFont();
    ImGui::DragFloat("Font scale", &font->Scale, 0.005f, 0.3f, 2.0f, "%.1f"); // Scale only this font
    ImGui::SameLine();
    HelpMarker("Note than the default embedded font is NOT meant to be scaled.\n\n"
               "Font are currently rendered into bitmaps at a given size at the time of building the atlas. "
               "You may oversample them to get some flexibility with scaling. "
               "You can also render at multiple sizes and select which one to use at runtime.\n\n"
               "(Glimmer of hope: the atlas system will be rewritten in the future to make scaling more flexible.)");
    ImGui::Text("Ascent: %f, Descent: %f, Height: %f", font->Ascent, font->Descent, font->Ascent - font->Descent);
    ImGui::Text("Fallback character: '%c' (U+%04X)", font->FallbackChar, font->FallbackChar);
    ImGui::Text("Ellipsis character: '%c' (U+%04X)", font->EllipsisChar, font->EllipsisChar);
    const int surface_sqrt = (int)sqrtf((float)font->MetricsTotalSurface);
    ImGui::Text("Texture Area: about %d px ~%dx%d px", font->MetricsTotalSurface, surface_sqrt, surface_sqrt);
    for (int config_i = 0; config_i < font->ConfigDataCount; config_i++)
        if (font->ConfigData)
            if (const ImFontConfig* cfg = &font->ConfigData[config_i])
                ImGui::BulletText("Input %d: \'%s\', Oversample: (%d,%d), PixelSnapH: %d, Offset: (%.1f,%.1f)",
                                  config_i, cfg->Name, cfg->OversampleH, cfg->OversampleV, cfg->PixelSnapH,
                                  cfg->GlyphOffset.x, cfg->GlyphOffset.y);
    if (ImGui::TreeNode("Glyphs", "Glyphs (%d)", font->Glyphs.Size))
    {
        // Display all glyphs of the fonts in separate pages of 256 characters
        const ImU32 glyph_col = ImGui::GetColorU32(ImGuiCol_Text);
        for (unsigned int base = 0; base <= IM_UNICODE_CODEPOINT_MAX; base += 256)
        {
            // Skip ahead if a large bunch of glyphs are not present in the font (test in chunks of 4k)
            // This is only a small optimization to reduce the number of iterations when IM_UNICODE_MAX_CODEPOINT
            // is large // (if ImWchar==ImWchar32 we will do at least about 272 queries here)
            if (!(base & 4095) && font->IsGlyphRangeUnused(base, base + 4095))
            {
                base += 4096 - 256;
                continue;
            }

            int count = 0;
            for (unsigned int n = 0; n < 256; n++)
                if (font->FindGlyphNoFallback((ImWchar)(base + n)))
                    count++;
            if (count <= 0)
                continue;
            if (!ImGui::TreeNode((void*)(intptr_t)base, "U+%04X..U+%04X (%d %s)", base, base + 255, count,
                                 count > 1 ? "glyphs" : "glyph"))
                continue;
            float       cell_size    = font->FontSize * 1;
            float       cell_spacing = style.ItemSpacing.y;
            ImVec2      base_pos     = ImGui::GetCursorScreenPos();
            ImDrawList* draw_list    = ImGui::GetWindowDrawList();
            for (unsigned int n = 0; n < 256; n++)
            {
                // We use ImFont::RenderChar as a shortcut because we don't have UTF-8 conversion functions
                // available here and thus cannot easily generate a zero-terminated UTF-8 encoded string.
                ImVec2             cell_p1(base_pos.x + (n % 16) * (cell_size + cell_spacing),
                               base_pos.y + (n / 16) * (cell_size + cell_spacing));
                ImVec2             cell_p2(cell_p1.x + cell_size, cell_p1.y + cell_size);
                const ImFontGlyph* glyph = font->FindGlyphNoFallback((ImWchar)(base + n));
                draw_list->AddRect(cell_p1, cell_p2,
                                   glyph ? IM_COL32(255, 255, 255, 100) : IM_COL32(255, 255, 255, 50));
                if (glyph)
                    font->RenderChar(draw_list, cell_size, cell_p1, glyph_col, (ImWchar)(base + n));
                if (glyph && ImGui::IsMouseHoveringRect(cell_p1, cell_p2))
                {
                    ImGui::BeginTooltip();
                    ImGui::Text("Codepoint: U+%04X", base + n);
                    ImGui::Separator();
                    ImGui::Text("Visible: %d", glyph->Visible);
                    ImGui::Text("AdvanceX: %.1f", glyph->AdvanceX);
                    ImGui::Text("Pos: (%.2f,%.2f)->(%.2f,%.2f)", glyph->X0, glyph->Y0, glyph->X1, glyph->Y1);
                    ImGui::Text("UV: (%.3f,%.3f)->(%.3f,%.3f)", glyph->U0, glyph->V0, glyph->U1, glyph->V1);
                    ImGui::EndTooltip();
                }
            }
            ImGui::Dummy(ImVec2((cell_size + cell_spacing) * 16, (cell_size + cell_spacing) * 16));
            ImGui::TreePop();
        }
        ImGui::TreePop();
    }
    ImGui::TreePop();
}

bool ShowStyleSelector(const char* label)
{
    static int style_idx = -1;
    if (ImGui::Combo(label, &style_idx, "PetForDesktopStyle\0Dark\0Light\0"))
    {
        switch (style_idx)
        {
        case 0:
            setDefaultTheme();
            break;
        case 1:
            ImGui::StyleColorsDark();
            break;
        case 2:
            ImGui::StyleColorsLight();
            break;
        }
        return true;
    }
    return false;
}

void ShowFontSelector(const char* label, int& idCurrentFont)
{
    ImGuiIO& io           = ImGui::GetIO();
    ImFont*  font_current = ImGui::GetFont();
    if (ImGui::BeginCombo(label, font_current->GetDebugName()))
    {
        for (int n = 0; n < io.Fonts->Fonts.Size; n++)
        {
            ImFont* font = io.Fonts->Fonts[n];
            ImGui::PushID((void*)font);
            if (ImGui::Selectable(font->GetDebugName(), font == font_current))
            {
                idCurrentFont  = n - 1;
                io.FontDefault = font;
            }
            ImGui::PopID();
        }
        ImGui::EndCombo();
    }
    ImGui::SameLine();
    HelpMarker("- Load additional fonts with io.Fonts->AddFontFromFileTTF().\n"
               "- The font atlas is built when calling io.Fonts->GetTexDataAsXXXX() or io.Fonts->Build().\n"
               "- Read FAQ and docs/FONTS.md for more details.\n"
               "- If you need to add/remove fonts at runtime (e.g. for DPI change), do it before calling NewFrame().");
}

void ShowStyleEditor(ImGuiStyle* ref = nullptr)
{
    // You can pass in a reference ImGuiStyle structure to compare to, revert to and save to
    // (without a reference style pointer, we will use one compared locally as a reference)
    static std::vector<std::string> fonts;
    static int                      idCurrentFont = -1;
    ImGuiStyle&                     style         = ImGui::GetStyle();
    static ImGuiStyle               ref_saved_style;

    // Default to using internal storage as reference
    static bool init = true;
    if (init && ref == NULL)
        ref_saved_style = style;
    init = false;
    if (ref == NULL)
        ref = &ref_saved_style;

    ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.50f);

    if (ShowStyleSelector("Colors##Selector"))
        ref_saved_style = style;
    ShowFontSelector("Fonts##Selector", idCurrentFont);

    static float fontSize = 14;
    if (ImGui::Button("Add font"))
    {
        std::string path = openFileExplorerAndGetRelativePath(L"Select font", {{L"Font", L"*.ttf"}}).string();
        ImGui::GetIO().Fonts->AddFontFromFileTTF(path.c_str(), fontSize);
        ImGui_ImplOpenGL3_CreateFontsTexture();
        fonts.emplace_back(path);
    }
    ImGui::SameLine();
    ImGui::DragFloat("Size", &fontSize, 0.1f, 0.f, 100.f);

    // Simplified Settings (expose floating-pointer border sizes as boolean representing 0.0f or 1.0f)
    if (ImGui::SliderFloat("FrameRounding", &style.FrameRounding, 0.0f, 12.0f, "%.0f"))
        style.GrabRounding = style.FrameRounding; // Make GrabRounding always the same value as FrameRounding
    {
        bool border = (style.WindowBorderSize > 0.0f);
        if (ImGui::Checkbox("WindowBorder", &border))
        {
            style.WindowBorderSize = border ? 1.0f : 0.0f;
        }
    }
    ImGui::SameLine();
    {
        bool border = (style.FrameBorderSize > 0.0f);
        if (ImGui::Checkbox("FrameBorder", &border))
        {
            style.FrameBorderSize = border ? 1.0f : 0.0f;
        }
    }
    ImGui::SameLine();
    {
        bool border = (style.PopupBorderSize > 0.0f);
        if (ImGui::Checkbox("PopupBorder", &border))
        {
            style.PopupBorderSize = border ? 1.0f : 0.0f;
        }
    }

    // Save/Revert button
    if (ImGui::Button("Export"))
    {
        ImGuiSaveStyle(PATH_UI_STYLE, style,
                       (fonts.empty() || idCurrentFont == -1) ? "Default" : fonts[idCurrentFont].c_str());
    }
    ImGui::SameLine();
    if (ImGui::Button("Import"))
    {
        ImGuiLoadStyle(PATH_UI_STYLE, style);
    }
    ImGui::SameLine();
    if (ImGui::Button("Save Ref"))
        *ref = ref_saved_style = style;
    ImGui::SameLine();
    if (ImGui::Button("Revert Ref"))
        style = *ref;
    ImGui::SameLine();
    HelpMarker("Save/Revert in local non-persistent storage. Default Colors definition are not affected. "
               "Use \"Export\" below to save them somewhere.");

    ImGui::Separator();

    if (ImGui::BeginTabBar("##tabs", ImGuiTabBarFlags_None))
    {
        if (ImGui::BeginTabItem("Sizes"))
        {
            ImGui::Text("Main");
            ImGui::SliderFloat2("WindowPadding", (float*)&style.WindowPadding, 0.0f, 20.0f, "%.0f");
            ImGui::SliderFloat2("FramePadding", (float*)&style.FramePadding, 0.0f, 20.0f, "%.0f");
            ImGui::SliderFloat2("CellPadding", (float*)&style.CellPadding, 0.0f, 20.0f, "%.0f");
            ImGui::SliderFloat2("ItemSpacing", (float*)&style.ItemSpacing, 0.0f, 20.0f, "%.0f");
            ImGui::SliderFloat2("ItemInnerSpacing", (float*)&style.ItemInnerSpacing, 0.0f, 20.0f, "%.0f");
            ImGui::SliderFloat2("TouchExtraPadding", (float*)&style.TouchExtraPadding, 0.0f, 10.0f, "%.0f");
            ImGui::SliderFloat("IndentSpacing", &style.IndentSpacing, 0.0f, 30.0f, "%.0f");
            ImGui::SliderFloat("ScrollbarSize", &style.ScrollbarSize, 1.0f, 20.0f, "%.0f");
            ImGui::SliderFloat("GrabMinSize", &style.GrabMinSize, 1.0f, 20.0f, "%.0f");
            ImGui::Text("Borders");
            ImGui::SliderFloat("WindowBorderSize", &style.WindowBorderSize, 0.0f, 1.0f, "%.0f");
            ImGui::SliderFloat("ChildBorderSize", &style.ChildBorderSize, 0.0f, 1.0f, "%.0f");
            ImGui::SliderFloat("PopupBorderSize", &style.PopupBorderSize, 0.0f, 1.0f, "%.0f");
            ImGui::SliderFloat("FrameBorderSize", &style.FrameBorderSize, 0.0f, 1.0f, "%.0f");
            ImGui::SliderFloat("TabBorderSize", &style.TabBorderSize, 0.0f, 1.0f, "%.0f");
            ImGui::Text("Rounding");
            ImGui::SliderFloat("WindowRounding", &style.WindowRounding, 0.0f, 12.0f, "%.0f");
            ImGui::SliderFloat("ChildRounding", &style.ChildRounding, 0.0f, 12.0f, "%.0f");
            ImGui::SliderFloat("FrameRounding", &style.FrameRounding, 0.0f, 12.0f, "%.0f");
            ImGui::SliderFloat("PopupRounding", &style.PopupRounding, 0.0f, 12.0f, "%.0f");
            ImGui::SliderFloat("ScrollbarRounding", &style.ScrollbarRounding, 0.0f, 12.0f, "%.0f");
            ImGui::SliderFloat("GrabRounding", &style.GrabRounding, 0.0f, 12.0f, "%.0f");
            ImGui::SliderFloat("LogSliderDeadzone", &style.LogSliderDeadzone, 0.0f, 12.0f, "%.0f");
            ImGui::SliderFloat("TabRounding", &style.TabRounding, 0.0f, 12.0f, "%.0f");
            ImGui::Text("Alignment");
            ImGui::SliderFloat2("WindowTitleAlign", (float*)&style.WindowTitleAlign, 0.0f, 1.0f, "%.2f");
            int window_menu_button_position = style.WindowMenuButtonPosition + 1;
            if (ImGui::Combo("WindowMenuButtonPosition", (int*)&window_menu_button_position, "None\0Left\0Right\0"))
                style.WindowMenuButtonPosition = window_menu_button_position - 1;
            ImGui::Combo("ColorButtonPosition", (int*)&style.ColorButtonPosition, "Left\0Right\0");
            ImGui::SliderFloat2("ButtonTextAlign", (float*)&style.ButtonTextAlign, 0.0f, 1.0f, "%.2f");
            ImGui::SameLine();
            HelpMarker("Alignment applies when a button is larger than its text content.");
            ImGui::SliderFloat2("SelectableTextAlign", (float*)&style.SelectableTextAlign, 0.0f, 1.0f, "%.2f");
            ImGui::SameLine();
            HelpMarker("Alignment applies when a selectable is larger than its text content.");
            ImGui::Text("Safe Area Padding");
            ImGui::SameLine();
            HelpMarker("Adjust if you cannot see the edges of your screen (e.g. on a TV where scaling has not been "
                       "configured).");
            ImGui::SliderFloat2("DisplaySafeAreaPadding", (float*)&style.DisplaySafeAreaPadding, 0.0f, 30.0f, "%.0f");
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Colors"))
        {
            static ImGuiTextFilter filter;
            filter.Draw("Filter colors", ImGui::GetFontSize() * 16);

            static ImGuiColorEditFlags alpha_flags = 0;
            if (ImGui::RadioButton("Opaque", alpha_flags == ImGuiColorEditFlags_None))
            {
                alpha_flags = ImGuiColorEditFlags_None;
            }
            ImGui::SameLine();
            if (ImGui::RadioButton("Alpha", alpha_flags == ImGuiColorEditFlags_AlphaPreview))
            {
                alpha_flags = ImGuiColorEditFlags_AlphaPreview;
            }
            ImGui::SameLine();
            if (ImGui::RadioButton("Both", alpha_flags == ImGuiColorEditFlags_AlphaPreviewHalf))
            {
                alpha_flags = ImGuiColorEditFlags_AlphaPreviewHalf;
            }
            ImGui::SameLine();
            HelpMarker("In the color list:\n"
                       "Left-click on color square to open color picker,\n"
                       "Right-click to open edit options menu.");

            ImGui::BeginChild("##colors", ImVec2(0, 0), true,
                              ImGuiWindowFlags_AlwaysVerticalScrollbar | ImGuiWindowFlags_AlwaysHorizontalScrollbar |
                                  ImGuiWindowFlags_NavFlattened);
            ImGui::PushItemWidth(-160);
            for (int i = 0; i < ImGuiCol_COUNT; i++)
            {
                const char* name = ImGui::GetStyleColorName(i);
                if (!filter.PassFilter(name))
                    continue;
                ImGui::PushID(i);
                ImGui::ColorEdit4("##color", (float*)&style.Colors[i], ImGuiColorEditFlags_AlphaBar | alpha_flags);
                if (memcmp(&style.Colors[i], &ref->Colors[i], sizeof(ImVec4)) != 0)
                {
                    // Tips: in a real user application, you may want to merge and use an icon font into the main font,
                    // so instead of "Save"/"Revert" you'd use icons!
                    // Read the FAQ and docs/FONTS.md about using icon fonts. It's really easy and super convenient!
                    ImGui::SameLine(0.0f, style.ItemInnerSpacing.x);
                    if (ImGui::Button("Save"))
                    {
                        ref->Colors[i] = style.Colors[i];
                    }
                    ImGui::SameLine(0.0f, style.ItemInnerSpacing.x);
                    if (ImGui::Button("Revert"))
                    {
                        style.Colors[i] = ref->Colors[i];
                    }
                }
                ImGui::SameLine(0.0f, style.ItemInnerSpacing.x);
                ImGui::TextUnformatted(name);
                ImGui::PopID();
            }
            ImGui::PopItemWidth();
            ImGui::EndChild();

            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Fonts"))
        {
            ImGuiIO&     io    = ImGui::GetIO();
            ImFontAtlas* atlas = io.Fonts;
            HelpMarker("Read FAQ and docs/FONTS.md for details on font loading.");
            ImGui::PushItemWidth(120);
            for (int i = 0; i < atlas->Fonts.Size; i++)
            {
                ImFont* font = atlas->Fonts[i];
                ImGui::PushID(font);
                NodeFont(font);
                ImGui::PopID();
            }
            if (ImGui::TreeNode("Atlas texture", "Atlas texture (%dx%d pixels)", atlas->TexWidth, atlas->TexHeight))
            {
                ImVec4 tint_col   = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
                ImVec4 border_col = ImVec4(1.0f, 1.0f, 1.0f, 0.5f);
                ImGui::Image(atlas->TexID, ImVec2((float)atlas->TexWidth, (float)atlas->TexHeight), ImVec2(0, 0),
                             ImVec2(1, 1), tint_col, border_col);
                ImGui::TreePop();
            }

            // Post-baking font scaling. Note that this is NOT the nice way of scaling fonts, read below.
            // (we enforce hard clamping manually as by default DragFloat/SliderFloat allows CTRL+Click text to get out
            // of bounds).
            const float MIN_SCALE = 0.3f;
            const float MAX_SCALE = 2.0f;
            HelpMarker(
                "Those are old settings provided for convenience.\n"
                "However, the _correct_ way of scaling your UI is currently to reload your font at the designed size, "
                "rebuild the font atlas, and call style.ScaleAllSizes() on a reference ImGuiStyle structure.\n"
                "Using those settings here will give you poor quality results.");
            static float window_scale = 1.0f;
            if (ImGui::DragFloat("window scale", &window_scale, 0.005f, MIN_SCALE, MAX_SCALE, "%.2f",
                                 ImGuiSliderFlags_AlwaysClamp)) // Scale only this window
                ImGui::SetWindowFontScale(window_scale);
            ImGui::DragFloat("global scale", &io.FontGlobalScale, 0.005f, MIN_SCALE, MAX_SCALE, "%.2f",
                             ImGuiSliderFlags_AlwaysClamp); // Scale everything
            ImGui::PopItemWidth();

            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Rendering"))
        {
            ImGui::Checkbox("Anti-aliased lines", &style.AntiAliasedLines);
            ImGui::SameLine();
            HelpMarker(
                "When disabling anti-aliasing lines, you'll probably want to disable borders in your style as well.");

            ImGui::Checkbox("Anti-aliased lines use texture", &style.AntiAliasedLinesUseTex);
            ImGui::SameLine();
            HelpMarker("Faster lines using texture data. Require backend to render with bilinear filtering (not "
                       "point/nearest filtering).");

            ImGui::Checkbox("Anti-aliased fill", &style.AntiAliasedFill);
            ImGui::PushItemWidth(100);
            ImGui::DragFloat("Curve Tessellation Tolerance", &style.CurveTessellationTol, 0.02f, 0.10f, 10.0f, "%.2f");
            if (style.CurveTessellationTol < 0.10f)
                style.CurveTessellationTol = 0.10f;

            ImGui::DragFloat(
                "Global Alpha", &style.Alpha, 0.005f, 0.20f, 1.0f,
                "%.2f"); // Not exposing zero here so user doesn't "lose" the UI (zero alpha clips all widgets). But
                         // application code could have a toggle to switch between zero and non-zero.
            ImGui::PopItemWidth();

            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::PopItemWidth();
}