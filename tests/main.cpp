#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <win_handle_getter.h>

#include <string>

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

TEST_CASE("Simple file opening test", "[win_handle_getter]")
{
    constexpr wchar_t fname[] = L"file.txt";
    const auto initial_paths = get_cur_proc_handle_paths();

    // Check that file.txt doesn't show up as open
    REQUIRE_FALSE(std::any_of(initial_paths.cbegin(), initial_paths.cend(),
        [&fname](const auto &path) { return ends_with(path, fname); }));

    // Open a file so we can see its handle in the list
    std::ofstream st(fname);

    const auto file_open_paths = get_cur_proc_handle_paths();

    // Check that file.txt now shows up as open
    REQUIRE(std::any_of(file_open_paths.cbegin(), file_open_paths.cend(),
        [&fname](const auto &path) { return ends_with(path, fname); }));

    // Close the file and check again
    st.close();
    const auto file_closed_paths = get_cur_proc_handle_paths();

    REQUIRE_FALSE(
        std::any_of(file_closed_paths.cbegin(), file_closed_paths.cend(),
            [&fname](const auto &path) { return ends_with(path, fname); }));
}
