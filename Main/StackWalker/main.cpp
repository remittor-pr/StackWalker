#include "stackwalker.h"
#include <tchar.h>
#include <stdexcept>

#define THROW_CPP_EXCEPTION  1
#define CATCH_CPP_EXCEPTION  1
#define USE_PTD_FOR_EXP_CTX  1

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
    if (THROW_CPP_EXCEPTION) throw std::exception("Bar error"); else *pi = 0xBADDEAD;  /* line 22 */
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
#if CATCH_CPP_EXCEPTION == 1
    try {
#endif
        return argc + foo();  /* line 41 */
#if CATCH_CPP_EXCEPTION == 1
    }
    catch(...) {
        OutputDebugStringA("===== test.catch =====");
        PCONTEXT ctx = nullptr;
#if USE_PTD_FOR_EXP_CTX == 1
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
    __try {
        test(argc);   /* line 68 */
    }
    __except( ExpFilter(GetExceptionInformation(), GetExceptionCode()) ) {
        // nothing
    }
    OutputDebugStringA("***** END *****");
    return 0;
}
