#include <vector>
#include <string>
#include <sdsl/suffix_arrays.hpp>

using namespace sdsl;
using namespace std;

//typedef csa_wt<wt_huff<bit_vector, rank_support_v5<>, select_support_scan<>,
//        select_support_scan<>>, 512, 1024,
//        text_order_sa_sampling<sd_vector<>>> fm_index_type;
typedef csa_wt<wt_huff<rrr_vector<127> >, 512, 1024> fm_index_type;

class FMWrapper {
    public:
        FMWrapper(string data);
        vector<int> locate(string query);
        string extract(int start, int length);
        int memory_size();

    private:
        fm_index_type fm_index;
};
