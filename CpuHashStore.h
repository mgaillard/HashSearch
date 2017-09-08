#ifndef CPUHASHSTORE_H
#define CPUHASHSTORE_H

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>

#include "HashStore.h"

using namespace std;

template<typename H, int (*HashDistance)(const H&, const H&)>
class CpuHashStore: public HashStore<H> {
public:
    typedef H Hash;
    typedef typename HashStore<H>::Result Result;
    typedef typename HashStore<H>::ResultEntry ResultEntry;

    void Load(const string &filename) {
        ifstream file(filename, ios::in);

        if (file) {
            Hash file_hash;

            while (file >> file_hash) {
                entries_.push_back(file_hash);
            }

            sort(entries_.begin(), entries_.end());

            file.close();
        } else {
            cout << "Impossible to load the index file." << endl;
        }
    }

    vector<Hash> Entries() const {
        return entries_;
    }

    Result Search(const Hash &file_hash, const int threshold) {
        Result result(file_hash);

        #pragma omp parallel for shared(result)
        for (unsigned int i = 0; i < entries_.size(); i++) {
            int dist = HashDistance(file_hash, entries_[i]);
            if (dist <= threshold) {
                #pragma omp critical
                {
                    result.entries.push_back(ResultEntry(dist, entries_[i]));
                }
            }
        }

        sort(result.entries.begin(), result.entries.end());

        return result;
    }

    vector<Result> BatchSearch(const vector<Hash> &queries, const int threshold) {
        vector<Result> results;

        for (typename vector<Hash>::const_iterator itHash = queries.begin(); itHash != queries.end(); ++itHash) {
            results.push_back(Search(*itHash, threshold));
        }

        sort(results.begin(), results.end());

        return results;
    }

private:
    vector<Hash> entries_;
};

#endif //CPUHASHSTORE_H
