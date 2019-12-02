#include <fstream>
#include <iostream>

#include <win_handle_getter.h>

// Adapted from https://stackoverflow.com/a/874160
inline bool ends_with(
    const std::wstring &full_string, const std::wstring &ending)
{
    if (full_string.length() >= ending.length()) {
        const auto comp_res = full_string.compare(
            full_string.size() - ending.size(), ending.size(), ending);
        return comp_res == 0;
    } else {
        return false;
    }
}

int main()
{
    // Note that this should also work with SMB shares! Try it via e.g.
    // constexpr wchar_t fname[] = L"\\\\localhost\\C$\\Users\\colin\\file.txt";
    constexpr wchar_t fname[] = L"file.txt";
    const auto initial_paths = get_cur_proc_handle_paths();

    // file.txt shouldn't show up as open
    std::wcout << L"BEFORE OPENING FILE" << std::endl;
    for (const auto &path : initial_paths) {
        std::wcout << path << std::endl;
    }

    // Open a file so we can see its handle in the list
    std::ofstream st(fname);

    const auto final_paths = get_cur_proc_handle_paths();

    // file.txt should now show up as open
    std::wcout << L"AFTER OPENING FILE" << std::endl;
    for (const auto &path : final_paths) {
        std::wcout << path << std::endl;
    }

    return 0;
}
