#include <string>
#include <vector>
#include <fstream>
#include <stdexcept>
#include <boost/algorithm/string.hpp>

using namespace std;

namespace cr_util {
    bool check_read(string r);
    bool check_contig(string c);
    string rev_compl(string s);
    string load_contigs(string contigs_path);
}
