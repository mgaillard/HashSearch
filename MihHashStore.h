#ifndef MIHHASHSTORE_H
#define MIHHASHSTORE_H

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <chrono>

#include "HashStore.h"

#include "types.h"
#include "mihasher.h"
#include "memusage.h"
#include "reorder.h"

using namespace std;

template<typename H, int (*HashDistance)(const H&, const H&)>
class MihHashStore: public HashStore<H> {
public:
    typedef H Hash;
    typedef typename HashStore<H>::Result Result;
    typedef typename HashStore<H>::ResultEntry ResultEntry;

    MihHashStore(int chunks, int k) :
        NUMBER_KNN(k),
        mih_(new mihasher(CODE_SIZE_BITS, chunks)),
        mih_hashes_(nullptr) {
        mih_->setK(k);
    }

    ~MihHashStore() {
        delete mih_;
        delete[] mih_hashes_;
    }

    void Load(const string &filename) {
        Hash file_hash;
        ifstream file(filename, ios::in);

        if (!file) {
            cout << "Impossible to load the index file." << endl;
            return;
        }

        while (file >> file_hash) {
            entries_.push_back(file_hash);
        }

        sort(entries_.begin(), entries_.end());
        populateMih();

        file.close();
    }

    vector<Hash> Entries() const {
        return entries_;
    }

    Result Search(const Hash &file_hash, const int threshold) {
        Result result(file_hash);

        vector<Hash> mih_result(queryMih(file_hash));
        for (typename vector<Hash>::const_iterator itHash = mih_result.begin(); itHash != mih_result.end(); ++itHash) {
            int dist = HashDistance(file_hash, *itHash);
            if (dist <= threshold) {
                result.entries.push_back(ResultEntry(dist, *itHash));
            }
        }

        sort(result.entries.begin(), result.entries.end());

        return result;
    }

    vector<Result> BatchSearch(const vector<Hash> &queries, const int threshold) {
        vector<Result> results;

        vector<pair<Hash, vector<Hash> > > queries_results = batchqueryMih(queries);
        for (typename vector<pair<Hash, vector<Hash> > >::const_iterator itResult = queries_results.begin(); itResult != queries_results.end(); ++itResult) {
            vector<ResultEntry> results_with_distance;

            for (typename vector<Hash>::const_iterator itHash = itResult->second.begin(); itHash != itResult->second.end(); ++itHash) {
                int dist = HashDistance(itResult->first, *itHash);
                if (dist <= threshold) {
                    results_with_distance.push_back(ResultEntry(dist, *itHash));
                }
            }

            sort(results_with_distance.begin(), results_with_distance.end());

            results.push_back(Result(itResult->first, results_with_distance));
        }

        sort(results.begin(), results.end());

        return results;
    }

private:

    void populateMih() {
        const unsigned int N = entries_.size();

        mih_hashes_ = new UINT8[N * CODE_SIZE];

        UINT64 *mih_hashes_64 = reinterpret_cast<UINT64*>(mih_hashes_);

        for (unsigned int i = 0; i < N; i++) {
            mih_hashes_64[i] = entries_[i];
        }

        mih_->populate(mih_hashes_, N, CODE_SIZE);
    }

    vector<Hash> queryMih(const Hash &query_hash) {
        vector<Hash> queries;
        queries.push_back(query_hash);

        vector<pair<Hash, vector<Hash> > > batch_result = batchqueryMih(queries);

        return batch_result.front().second;
    }

    vector<pair<Hash, vector<Hash> > > batchqueryMih(const vector<Hash> &queries) {
        vector<pair<Hash, vector<Hash> > > result;

        const unsigned int Q = queries.size();

        // Array of binary codes.
        UINT8 *codes_query = new UINT8[Q * CODE_SIZE];
        UINT64 *codes_query_64 = reinterpret_cast<UINT64*>(codes_query);
        for (unsigned int i = 0; i < Q; i++) {
            codes_query_64[i] = queries[i];
        }
        
        qstat *stats = new qstat[Q];

        UINT32 **results = new UINT32 *[Q];
        results[0] = new UINT32[Q * NUMBER_KNN];
        for (size_t i = 1; i < Q; i++) {
            results[i] = results[i - 1] + NUMBER_KNN;
        }

        UINT32 **n_results = new UINT32 *[Q];
        n_results[0] = new UINT32[Q * (CODE_SIZE_BITS + 1)];
        for (size_t i = 1; i < Q; i++) {
            n_results[i] = n_results[i - 1] + (CODE_SIZE_BITS + 1);
        }

        mih_->batchquery(results[0], n_results[0], stats, codes_query, Q, CODE_SIZE);

        for (unsigned int i = 0; i < Q; i++) {
            Hash query_hash = queries[i];

            result.push_back(make_pair(query_hash, vector<Hash>()));
            for (unsigned int j = 0; j < NUMBER_KNN; j++) {
                UINT32 index_result = results[i][j];
                result[i].second.push_back(entries_[index_result - 1]);
            }
        }

        delete[] codes_query;
        delete[] stats;
        delete[] results[0];
        delete[] results;
        delete[] n_results[0];
        delete[] n_results;

        return result;
    }

    // Number of bytes per code.
    const int CODE_SIZE = sizeof(Hash);

    // Number of bits per code.
    const int CODE_SIZE_BITS = CODE_SIZE * 8;

    // Number of nearest neighbor to search;
    const int NUMBER_KNN;

    vector<Hash> entries_;

    mihasher *mih_;

    // Array of binary codes for the mihasher.
    UINT8 *mih_hashes_;
};

#endif //MIHHASHSTORE_H
