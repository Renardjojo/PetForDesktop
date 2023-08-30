#include "Engine/FileExplorer.hpp"
#include "Engine/Log.hpp"

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)

std::filesystem::path openFolderExplorerAndGetAbsoluePath(LPCWSTR title)
{
    std::filesystem::path src;

    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

    if (SUCCEEDED(hr))
    {
        IFileOpenDialog* pFileOpen = nullptr;

        // Create the FileOpenDialog object.
        hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL, IID_IFileOpenDialog,
                              reinterpret_cast<void**>(&pFileOpen));

        if (SUCCEEDED(hr))
        {
            if (FAILED(pFileOpen->SetOptions(FOS_PICKFOLDERS)))
            {
                logf("Invalid title to init windows file explorer");
                return src;
            }

            if (FAILED(pFileOpen->SetTitle(title)))
            {
                logf("Invalid title to init windows file explorer");
                return src;
            }

            // Show the Open dialog box.
            hr = pFileOpen->Show(NULL);

            // Get the file name from the dialog box.
            if (SUCCEEDED(hr))
            {
                IShellItem* pItem;
                hr = pFileOpen->GetResult(&pItem);
                if (SUCCEEDED(hr))
                {
                    LPWSTR pszFilePath;
                    hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);

                    // Display the file name to the user.
                    if (SUCCEEDED(hr))
                    {
                        src = pszFilePath;
                        CoTaskMemFree(pszFilePath);
                    }
                    pItem->Release();
                }
            }
            pFileOpen->Release();
        }
        CoUninitialize();
    }
    return src;
}

std::filesystem::path openFolderExplorerAndGetRelativePath(LPCWSTR title)
{
    return std::filesystem::relative(openFolderExplorerAndGetAbsoluePath(title));
}

std::filesystem::path openFileExplorerAndGetAbsoluePath(LPCWSTR title, std::vector<COMDLG_FILTERSPEC> filter)
{
    std::filesystem::path src;

    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

    if (SUCCEEDED(hr))
    {
        IFileOpenDialog* pFileOpen = nullptr;

        // Create the FileOpenDialog object.
        hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL, IID_IFileOpenDialog,
                              reinterpret_cast<void**>(&pFileOpen));

        if (SUCCEEDED(hr))
        {
            if (FAILED(pFileOpen->SetTitle(title)))
            {
                logf("Invalid title to init windows file explorer");
                return src;
            }

            if (!filter.empty() && FAILED(pFileOpen->SetFileTypes(static_cast<UINT>(filter.size()), filter.data())))
            {
                logf("Invalid filter to init windows file explorer");
                return src;
            }

            // Show the Open dialog box.
            hr = pFileOpen->Show(NULL);

            // Get the file name from the dialog box.
            if (SUCCEEDED(hr))
            {
                IShellItem* pItem;
                hr = pFileOpen->GetResult(&pItem);
                if (SUCCEEDED(hr))
                {
                    LPWSTR pszFilePath;
                    hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);

                    // Display the file name to the user.
                    if (SUCCEEDED(hr))
                    {
                        src = pszFilePath;
                        CoTaskMemFree(pszFilePath);
                    }
                    pItem->Release();
                }
            }
            pFileOpen->Release();
        }
        CoUninitialize();
    }
    return src;
}

std::filesystem::path openFileExplorerAndGetRelativePath(LPCWSTR title, std::vector<COMDLG_FILTERSPEC> filter)
{
    return std::filesystem::relative(openFileExplorerAndGetAbsoluePath(title, filter));
}

void recycleFileOrDirectory(const std::filesystem::path& path)
{
    SHFILEOPSTRUCT fileOp;
    memset(&fileOp, 0, sizeof(SHFILEOPSTRUCT));
    fileOp.hwnd      = NULL;
    fileOp.wFunc     = FO_DELETE;
    std::string temp = path.string() + '\0' + '\0';
    fileOp.pFrom     = temp.c_str();
    fileOp.pTo       = NULL;
    fileOp.fFlags    = FOF_ALLOWUNDO | FOF_NOERRORUI | FOF_NOCONFIRMATION | FOF_SILENT;

    if (SHFileOperation(&fileOp))
        logf("Failed to move file '%s' to Recycle Bin", path.c_str());
}

#endif