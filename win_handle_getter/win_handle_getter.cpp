// win_handle_getter.cpp : Defines the entry point for the application.
//

#include "win_handle_getter.h"

#include <Windows.h>

#include <ProcessSnapshot.h>

#include <cassert>
#include <string>
#include <vector>

static std::wstring handle_to_path(HANDLE hdl)
{
    // Get the length of the path--these days, MAX_PATH isn't a reliable
    // assumption Note that this includes the terminating NUL
    DWORD handle_name_len
        = GetFinalPathNameByHandleW(hdl, nullptr, 0, VOLUME_NAME_DOS);

    if (handle_name_len == 0) {
        return L"UNKNOWN";
    }

    // Actually retrieve the path
    std::wstring path(handle_name_len - 1, '\0');
    DWORD handle_name_out_chars = GetFinalPathNameByHandleW(
        hdl, path.data(), static_cast<DWORD>(path.size()) + 1, VOLUME_NAME_DOS);

    // A weird bit of this API is that if the call succeeds, the length it
    // returns doesn't include the NUL. If it doesn't succeed, the reverse is
    // true. Thus, the -1.
    assert(handle_name_out_chars == handle_name_len - 1);

    return path;
}

std::vector<std::wstring> get_cur_proc_handle_paths()
{
    std::vector<std::wstring> paths;

    // Get current proc handle
    HANDLE cur_proc = GetCurrentProcess();

    // Get a snapshot of the process's handles
    HPSS snap { 0 };
    DWORD snap_res
        = PssCaptureSnapshot(cur_proc, PSS_CAPTURE_HANDLES, 0, &snap);
    if (snap_res != ERROR_SUCCESS) {
        throw std::runtime_error(
            "Unexpected exception while capturing process snapshot: "
            + std::to_string(snap_res));
    }

    // Walk marker is like an iterator for traversing the snapshot data
    HPSSWALK walk_marker { 0 };
    DWORD walk_marker_create_res = PssWalkMarkerCreate(nullptr, &walk_marker);
    if (walk_marker_create_res != ERROR_SUCCESS) {
        throw std::runtime_error(
            "Unexpected exception while iterating process snapshot: "
            + std::to_string(walk_marker_create_res));
    }

    PSS_HANDLE_ENTRY buf { 0 };
    while (true) {
        // We continue until we stop getting data
        auto walk_res = PssWalkSnapshot(
            snap, PSS_WALK_HANDLES, walk_marker, &buf, sizeof(buf));
        if (walk_res != ERROR_SUCCESS) {
            break;
        }

        if (buf.Flags == PSS_HANDLE_NONE) {
            continue;
        }

        // We only care about file handles
        std::wstring_view type_name(
            buf.TypeName, buf.TypeNameLength / sizeof(WCHAR) - 1);
        if (type_name != L"File") {
            continue;
        }

        // Extract the handle's path
        paths.emplace_back(handle_to_path(buf.Handle));
    }

    return paths;
}
