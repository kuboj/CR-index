#include <string>
#include <vector>
#include <utility>
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
        vector<pair<int, int>> positions;
        string superstring;

        void debug(string msg);
        void info(string msg);
        bool check_read(string r);
        bool check_contig(string c);
        string rev_compl(string r);
};
