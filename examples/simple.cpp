#include <algorithm>
#include <cassert>
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
    constexpr wchar_t *fname = L"file.txt";
    const auto initial_paths = get_cur_proc_handle_paths();

    // Check that file.txt doesn't show up as open
    assert(!std::any_of(initial_paths.cbegin(), initial_paths.cend(),
        [&fname](const auto &path) { return ends_with(path, fname); }));

    // Open a file so we can see its handle in the list
    std::ofstream st(fname);

    const auto final_paths = get_cur_proc_handle_paths();

    // Check that file.txt now shows up as open
    assert(std::any_of(final_paths.cbegin(), final_paths.cend(),
        [&fname](const auto &path) { return ends_with(path, fname); }));

    return 0;
}
