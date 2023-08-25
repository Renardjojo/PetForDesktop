#pragma once

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#include <shobjidl.h>

#include <filesystem>

/**
 * @brief
 * @param title
 * @param filter :          COMDLG_FILTERSPEC rgSpec[] = {
                            {szJPG, L"*.jpg;*.jpeg"},
                            {szBMP, L"*.bmp"},
                            {szAll, L"*.*"}};
                            see :
 https://docs.microsoft.com/en-us/windows/win32/api/shobjidl_core/nf-shobjidl_core-ifiledialog-setfiletypes
 * @return
*/
std::filesystem::path openFileExplorerAndGetAbsoluePath(LPCWSTR                        title  = L"Open",
                                                        std::vector<COMDLG_FILTERSPEC> filter = {{L"All", L"*.*"}});

std::filesystem::path openFileExplorerAndGetRelativePath(LPCWSTR                        title  = L"Open",
                                                         std::vector<COMDLG_FILTERSPEC> filter = {{L"All", L"*.*"}});

std::filesystem::path openFolderExplorerAndGetAbsoluePath(LPCWSTR title = L"Open");

std::filesystem::path openFolderExplorerAndGetRelativePath(LPCWSTR title = L"Open");

void recycleFileOrDirectory(const std::filesystem::path& path);

#endif
