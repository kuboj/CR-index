#include "fm_wrapper.hpp"

using namespace std;

FMWrapper::FMWrapper(string data) {
    fm_index_type fm;
    sdsl::construct_im(fm, data.c_str(), 1);
    this->fm_index = fm;
}

vector<int> FMWrapper::locate(string query) {
    auto retval = sdsl::locate(this->fm_index, query.begin(), query.end());
    return vector<int>(retval.begin(), retval.end());
}

string FMWrapper::extract(int start, int length) {
    return sdsl::extract(this->fm_index, start, start + length - 1);
}

int FMWrapper::memory_size() {
    return sdsl::size_in_bytes(this->fm_index);
}
