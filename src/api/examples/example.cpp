#include <iostream>
#include <cspydr.h>

int main() {
    using namespace std;
    using namespace cspydr;

    Compiler_T* compiler = csp_init_compiler();

    // do something with compiler
    cout << csp_status_str(csp_get_status(compiler)) << endl;

    csp_free_compiler(compiler);
    return 0;
}