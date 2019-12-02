add_executable (tests
	"catch.hpp"
	"main.cpp"
)
target_link_libraries (tests win_handle_getter)

add_test (NAME basic COMMAND tests)
