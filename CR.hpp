#include "FM.h"

using namespace std;

class CR {
    public:
        static bool DEFAULT_VERBOSITY;

        CR(string path, bool verbose = DEFAULT_VERBOSITY);
        ~CR();

    private:
        bool verbose;

        void debug(string msg);
};
