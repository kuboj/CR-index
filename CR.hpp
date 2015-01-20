#include <string>
#include <vector>
#include "FM.h"

using namespace std;

class CR {
    public:
        static const bool DEFAULT_VERBOSITY;

        CR(string path, bool verbose = DEFAULT_VERBOSITY);
        ~CR();

    private:
        bool verbose;
        vector<string> reads;

        void debug(string msg);
        bool check_read(string r);
};
