#include "Engine/FileExplorer.hpp"
#include "Engine/Log.hpp"
#include "Engine/ImGuiTools.hpp"

#include "backends/imgui_impl_opengl3.h"
#include <imgui.h>
#include <math.h> // sqrtf
#include <vector>

#ifdef _WIN32
#define IM_NEWLINE "\r\n"
#else
#define IM_NEWLINE "\n"
#endif

// From <imgui.cpp>:--------------------------------------------------------
#include <stdio.h> // vsnprintf
#define IM_ARRAYSIZE(_ARR) ((int)(sizeof(_ARR) / sizeof(*_ARR)))
#include <string.h>

#define PATH_UI_STYLE RESOURCE_PATH "/styles/UIStyle.style"

inline void ImGuiSaveStyle(const char* filename, const ImGuiStyle& style, const char* currentFont)
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
    fprintf(f, "[GrabRounding]\n%1.3f\n", style.GrabRounding);
    fprintf(f, "[GrabMinSize]\n%1.3f\n", style.GrabMinSize);
    fprintf(f, "[ColumnsMinSpacing]\n%1.3f\n", style.ColumnsMinSpacing);
    fprintf(f, "[WindowBorderSize]\n%1.3f\n", style.WindowBorderSize);
    fprintf(f, "[TabBorderSize]\n%1.3f\n", style.TabBorderSize);
    fprintf(f, "[FrameBorderSize]\n%1.3f\n", style.FrameBorderSize);
    fprintf(f, "[PopupBorderSize]\n%1.3f\n", style.PopupBorderSize);
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

inline void ImGuiLoadStyle(const char* filename, ImGuiStyle& style)
{
    // Load file into memory
    FILE* f = nullptr;
    if (fopen_s(&f, filename, "rt"))
    {
        warning(std::string("Could not find the file ") + filename);
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
                else if (strcmp(name, "GrabRounding") == 0)
                {
                    npf   = 1;
                    pf[0] = &style.GrabRounding;
                }
                else if (strcmp(name, "GrabMinSize") == 0)
                {
                    npf   = 1;
                    pf[0] = &style.GrabMinSize;
                }
                else if (strcmp(name, "ColumnsMinSpacing") == 0)
                {
                    npf   = 1;
                    pf[0] = &style.ColumnsMinSpacing;
                }
                else if (strcmp(name, "WindowBorderSize") == 0)
                {
                    npf   = 1;
                    pf[0] = &style.WindowBorderSize;
                }
                else if (strcmp(name, "TabBorderSize") == 0)
                {
                    npf   = 1;
                    pf[0] = &style.TabBorderSize;
                }
                else if (strcmp(name, "FrameBorderSize") == 0)
                {
                    npf   = 1;
                    pf[0] = &style.FrameBorderSize;
                }
                else if (strcmp(name, "PopupBorderSize") == 0)
                {
                    npf   = 1;
                    pf[0] = &style.PopupBorderSize;
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
    logf("Style imported\n");
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

    auto&   style                          = GetStyle();
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
    style.GrabRounding      = 8;
    style.GrabMinSize       = 12;
    style.ScrollbarRounding = 9;
    style.WindowRounding    = 4;
    style.FrameRounding     = 3;
    style.PopupRounding     = 4;
    style.ChildRounding     = 4;
    style.WindowBorderSize  = 0;
    style.FrameBorderSize   = 0;
}

inline void ShowStyleEditor(ImGuiStyle* ref = nullptr)
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

    // Save/Revert button
    if (ImGui::Button("Export"))
    {
        *ref = ref_saved_style = style;
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

        ImGui::EndTabBar();
    }

    ImGui::PopItemWidth();
}