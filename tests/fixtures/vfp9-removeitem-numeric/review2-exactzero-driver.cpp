#include "test_prg_engine_runtime_surface_functions_support.h"
#include "test_prg_engine_runtime_surface_removeitem_exactzero_rowsourcetype.inl"
int main() {
    copperfin::runtime_surface_tests::test_native_removeitem_exactzero_rowsourcetype_admission();
    std::cout << "Review2 supplemental failures: " << copperfin::test_support::test_failures() << '\n';
    return copperfin::test_support::test_failures() == 0 ? 0 : 1;
}
