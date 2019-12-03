// win_handle_getter.cpp : Defines the entry point for the application.
//

#include "win_handle_getter.h"

#include <Windows.h>

#include <ProcessSnapshot.h>

#include <memory>
#include <string>
#include <type_traits>
#include <vector>

template <typename F> class at_exit {
    F _closure;

public:
    explicit at_exit(F closure)
        : _closure(closure)
    {
        static_assert(std::is_nothrow_invocable<F>::value,
            "at_exit functions must be noexcept");
    }

    ~at_exit() noexcept { _closure(); }
};

static std::wstring handle_to_path(HANDLE hdl)
{
    // IMPORTANT: We use GetFileInformationByHandleEx instead of
    // GetFinalPathNameByHandleW because the latter can hang indefinitely in
    // some cases.

    DWORD ftype = GetFileType(hdl);
    // Named pipes may sometimes hang
    if (ftype == 3) {
        return L"PIPE "
               + std::to_wstring(reinterpret_cast<std::uintptr_t>(hdl));
    }
    // Attempt to get info about the handle--note that we expect this to fail.
    // Unless the path is empty, there isn't enough space for the file name in
    // the struct.
    FILE_NAME_INFO info { 0 };
    auto first_attempt_info_res
        = GetFileInformationByHandleEx(hdl, FileNameInfo, &info, sizeof(info));
    if (first_attempt_info_res) {
        // This seems to occur sometimes when running the tests via the VS test
        // runner. No idea what the handle really is, but it seems to make sense
        // to handle it as an edge case regardless.
        return std::wstring(
            info.FileName, info.FileNameLength / sizeof(wchar_t));
    }

    // Some other errors, like permission issues, can occur. Only
    // ERROR_MORE_DATA indicates that our initial attempt failed due to lack of
    // space in the struct.
    auto first_attempt_err = GetLastError();
    if (first_attempt_err != ERROR_MORE_DATA) {
        return L"UNKNOWN";
    }

    size_t buf_sz = sizeof(FILE_NAME_INFO) + info.FileNameLength;

    // Allocate a new buffer to actually put the filename in.
    // Note the extra byte allocated for NUL-termination, even though
    // technically we don't need it.
    auto file_info_buf_uniq = std::make_unique<char[]>(buf_sz + 1);
    memset(file_info_buf_uniq.get(), 0, buf_sz + 1);

    // Pretend that the buffer is just a massive FILE_NAME_INFO.
    auto file_info_buf
        = reinterpret_cast<FILE_NAME_INFO *>(file_info_buf_uniq.get());

    auto real_info_res = GetFileInformationByHandleEx(
        hdl, FileNameInfo, file_info_buf, (DWORD)buf_sz);

    if (!real_info_res) {
        return L"ERROR " + std::to_wstring(GetLastError());
    }

    // Note that FileNameLength is in bytes, but std::wstring cares about
    // characters.
    return std::wstring(file_info_buf->FileName,
        file_info_buf->FileNameLength / sizeof(wchar_t));
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
    at_exit clean_up_snap(
        [&snap]() noexcept { PssFreeSnapshot(GetCurrentProcess(), snap); });

    if (snap_res != ERROR_SUCCESS) {
        throw std::runtime_error(
            "Unexpected exception while capturing process snapshot: "
            + std::to_string(snap_res));
    }

    // Walk marker is like an iterator for traversing the snapshot data
    HPSSWALK walk_marker { 0 };
    DWORD walk_marker_create_res = PssWalkMarkerCreate(nullptr, &walk_marker);
    at_exit clean_up_walk_marker(
        [&walk_marker]() noexcept { PssWalkMarkerFree(walk_marker); });

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
