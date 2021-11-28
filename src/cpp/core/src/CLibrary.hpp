
#if defined(_MSC_VER) || defined(_WIN32) || defined(_WIN64)
    #include "CLibrary_Windows.hpp"
#elif defined(_UNIX) || defined(linux)
    #include "CLibrary_Linux.hpp"
#endif//defined(_MSC_VER) || defined(_WIN32) || defined(_WIN64)