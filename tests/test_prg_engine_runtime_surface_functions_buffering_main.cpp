#include "test_prg_engine_runtime_surface_functions_support.h"
#include "test_prg_engine_runtime_surface_functions_tests.h"

int main()
{
    using namespace copperfin::runtime_surface_tests;
    using namespace copperfin::test_support;
    test_local_optimistic_table_buffering();
    test_local_optimistic_table_buffering_append_lifecycle();

    if (test_failures() != 0)
    {
        std::cerr << test_failures() << " test(s) failed.\n";
        return EXIT_FAILURE;
    }

    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}
