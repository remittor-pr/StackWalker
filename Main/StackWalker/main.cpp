#include "stackwalker.h"
#include <tchar.h>
#include <stdio.h>
#include <stdexcept>

#define THROW_CPP_EXP  1
#define CATCH_CPP_EXP  2

#define NOINLINE __declspec(noinline)

struct Foo
{
    ~Foo()
    {
        OutputDebugStringA("----- Foo destructor -----");
    }
};

NOINLINE int bar(int * pi = nullptr)
{
    OutputDebugStringA("----- bar -----");
    if (THROW_CPP_EXP) throw std::exception("Bar error"); else *pi = 0xDEAD;  /* line 22 */
    OutputDebugStringA("+++++ bar +++++");
    return 0;
}

NOINLINE int foo()
{
    Foo obj;
    return bar(nullptr);  /* line 30 */
}

extern "C" void** __cdecl __current_exception();
extern "C" void** __cdecl __current_exception_context();

NOINLINE int test(int argc)
{
#if CATCH_CPP_EXP >= 1
    try {
#endif
        return argc + foo();  /* line 41 */
#if CATCH_CPP_EXP >= 1
    }
    catch(...) {
        OutputDebugStringA("===== test.catch =====");
        PCONTEXT ctx = nullptr;
#if CATCH_CPP_EXP == 2
        ctx = *(PCONTEXT *)__current_exception_context();
#endif
        StackWalker sw(StackWalker::RetrieveVerbose);
        sw.ShowCallstack(GetCurrentThread(), ctx);
        throw;
    }
#endif
}

LONG WINAPI ExpFilter(EXCEPTION_POINTERS * pExp, DWORD dwExpCode)
{
    OutputDebugStringA("===== main.__except =====");
    StackWalker sw(StackWalker::RetrieveVerbose);
    sw.ShowCallstack(GetCurrentThread(), pExp->ContextRecord);
    return EXCEPTION_EXECUTE_HANDLER;
}

int main(int argc, _TCHAR * argv[])
{
    char txt[80];
    sprintf_s(txt, "THROW_CPP_EXP = %d, CATCH_CPP_EXP = %d \n", THROW_CPP_EXP, CATCH_CPP_EXP);
    OutputDebugStringA(txt);
    __try {
        test(argc);   /* line 71 */
    }
    __except( ExpFilter(GetExceptionInformation(), GetExceptionCode()) ) {
        // nothing
    }
    OutputDebugStringA("***** END *****");
    return 0;
}
