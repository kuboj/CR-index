#include <string>
#include <vector>
#include <utility>
#include "FM.h"

using namespace std;

class CR {
    public:
        static const bool DEFAULT_VERBOSITY;
        static const int DEFAULT_READ_LENGTH;

        CR(string path, int read_length = DEFAULT_READ_LENGTH,
                bool verbose = DEFAULT_VERBOSITY);
        vector<int> locate(string s);
        vector<string> locate2(string s);
        ~CR();

    private:
        bool verbose;
        int read_length;
        vector<pair<int, int>> positions;

        FM* fm_index;
        vector<pair<int, int>> locate_positions(string s);
        void debug(string msg);
        void info(string msg);
        bool check_read(string r);
        bool check_contig(string c);
        string rev_compl(string r);
        string load_contigs(string contigs_path);
        FM* fm_construct(string s);
        vector<int> fm_locate(string p);
        string fm_extract(int start, int length);
};
