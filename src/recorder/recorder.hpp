#pragma once

#ifdef _WIN32
    #define REC_POPEN _popen
    #define REC_PCLOSE _pclose
    #define REC_MODE "wb"
#else
    #define REC_POPEN popen
    #define REC_PCLOSE pclose
    #define REC_MODE "w"
#endif

struct recorder {
    public:

    private:

};
