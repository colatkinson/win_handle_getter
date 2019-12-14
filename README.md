# win\_handle\_getter

[![Azure Pipelines builds](https://img.shields.io/azure-devops/build/colinwilliamatkinson/53ebeb2d-44f5-4717-876b-b31fa2d32f89/2?logo=azure-pipelines&style=flat-square)](https://dev.azure.com/colinwilliamatkinson/win_handle_getter/_build)
![License badge](https://img.shields.io/github/license/colatkinson/win_handle_getter?style=flat-square)

A simple example of how to use the win32 `ProcessSnapshot` APIs to get a list
of open file handles in the current process.

## Motivation

Finding leaked file handles can be extremely challenging on Windows. The best
approaches I'd found online were, roughly speaking, to run [Sysinternals
Handle](https://docs.microsoft.com/en-us/sysinternals/downloads/handle) a bunch
of times.

Especially in high-level languages like Python, the best approach to tracking
down the source of a leaked handle can really be just to see a list of open
handles before and after some action.

I stumbled across this API via a random comment squirreled away on a
StackOverflow post that I'm never going to be able to find again, and whipped
up a quick PoC that it does what I wanted. And that's how this repo came to be.

## API

The one and only important function for consumers is
`get_cur_proc_handle_paths`. This returns a vector of `std::wstring` containing
best-effort paths for the open file handles within the current process.

See [`examples/simple.cpp`](examples/simple.cpp) for a simple usage example.

## Other Notes

This really shouldn't be regarded as a complete library--it's more akin to a
snippet. It can be incorporated into your project through a simple copy/paste.

It should hopefully be fairly simple to create wrappers for any language with a
good FFI. I may add examples of this in the future, too.

This code uses C++17 features, most notable `std::string_view`. If you require
compatibility with an earlier version of C++, removing this should get you most
of the way there.
