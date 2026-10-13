// Scaffold verification ONLY against retained first-fixed GNU libraries.
// Not native evidence or the required repeated original suites.
#include "test_prg_engine_runtime_surface_functions_support.h"
#include "test_prg_engine_runtime_surface_functions_tests.h"
int main() {
    copperfin::runtime_surface_tests::test_native_list_controls_removelistitem_stays_coherent();
    std::cout << "Corrected old missing-ID smoke failures: "
              << copperfin::test_support::test_failures() << '\n';
    return copperfin::test_support::test_failures() == 0 ? 0 : 1;
}
