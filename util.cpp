#include "util.hpp"

using namespace std;

namespace cr_util {
    // TODO: modify this when taking into account reads with uNknowns etc. (R, ... )
    bool check_read(string r) {
        for (int i = 0; i < r.size(); i++) {
            if ((r[i] != 'A') &&
                    (r[i] != 'C') &&
                    (r[i] != 'T') &&
                    (r[i] != 'G')) {
                return false;
            }
        }

        return true;
    }

    // TODO: modify this when taking into account reads with uNknowns etc. (R, ... )
    bool check_contig(string c) {
        for (int i = 0; i < c.size(); i++) {
            if ((c[i] != 'A') &&
                    (c[i] != 'C') &&
                    (c[i] != 'T') &&
                    (c[i] != 'G')) {
                return false;
            }
        }

        return true;
    }

    string rev_compl(string s) {
        string rc = "";
        for (int i = 0; i < s.length(); i++) {
            switch (s[i]) {
            case 'A':
                rc += 'T';
                break;
            case 'T':
                rc += 'A';
                break;
            case 'C':
                rc += 'G';
                break;
            case 'G':
                rc += 'C';
                break;
            default:
                rc += s[i];
                break;
            }
        }
        reverse(rc.begin(), rc.end());
        return rc;
    }

    string load_contigs(string contigs_path) {
        string retval = "";
        ifstream f(contigs_path);

        if (!f) {
            throw runtime_error("Error opening file '" + contigs_path + "'");
        }

        string l = "";
        int i = 0;
        int skipped = 0;
        while (getline(f, l)) {
            if (i++ % 2 != 1) {
                continue;
            }
            boost::algorithm::trim(l);
            if (check_contig(l)) {
                retval += l;
            } else {
                skipped++;
            }
        }

        f.close();

        return retval;
    }
}
