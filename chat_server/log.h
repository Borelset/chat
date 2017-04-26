#ifndef LOG_H
#define LOG_H
#include <fstream>

class _log
{
    std::fstream fout;
    int flag = 0;
public:
    _log();
    _log& operator<<(char* input);
    _log &operator<<(int input);
    _log& operator<<(_log& input);
};

#endif // LOG_H
