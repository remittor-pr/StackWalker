#include "stackwalker.h"
#include <stdio.h>
#include <stdexcept>

#define THROW_CPP_EXCEPTION  1
#define CATCH_CPP_EXCEPTION  1

#define NOINLINE __declspec(noinline)

NOINLINE int bar(int * pi = nullptr)
{
#if THROW_CPP_EXCEPTION == 1
    throw(std::exception("Bar error"));   /* line 13 */
#else
    *pi = 0xBADDEAD;   /* line 15 */
#endif
    return 0;
}

NOINLINE void foo()
{
    int x = bar(nullptr);  /* line 22 */
    printf("bar = %d \n", x);
}

extern "C" void** __cdecl __current_exception_context();

NOINLINE void test()
{
    try {
        foo();  /* line 31 */
    }
    catch(...) {
#if CATCH_CPP_EXCEPTION == 1
        PCONTEXT ctx = *(PCONTEXT *)__current_exception_context();
        StackWalker sw(StackWalker::RetrieveVerbose);
        sw.ShowCallstack(GetCurrentThread(), ctx);
#endif
        throw;
    }
}

LONG WINAPI ExpFilter(EXCEPTION_POINTERS * pExp, DWORD dwExpCode)
{
    StackWalker sw(StackWalker::RetrieveVerbose);
    sw.ShowCallstack(GetCurrentThread(), pExp->ContextRecord);
    return EXCEPTION_EXECUTE_HANDLER;
}

int main()
{
    __try {
        test();   /* line 53 */
    }
    __except( ExpFilter(GetExceptionInformation(), GetExceptionCode()) ) {
        // nothing
    }
    return 0;
}
