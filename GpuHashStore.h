#ifndef GPUHASHSTORE_H
#define GPUHASHSTORE_H

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>

#include <thrust/host_vector.h>
#include <thrust/device_vector.h>
#include <thrust/copy.h>
#include <thrust/sort.h>
#include <thrust/unique.h>

#include "HashStore.h"

template<typename H, int (*CpuHashDistance)(const H&, const H&), int (*GpuHashDistance)(const H&, const H&)>
class GpuHashStore: public HashStore<H> {
public:
    typedef H Hash;
    typedef typename HashStore<H>::Result Result;
    typedef typename HashStore<H>::ResultEntry ResultEntry;

    struct ThresholdFilter
    {
        const Hash target_;
        const int threshold_;

        ThresholdFilter(const Hash &target, int threshold) :
            target_(target),
            threshold_(threshold) {}

        __device__ bool operator()(const Hash hash)
        {
            return GpuHashDistance(target_, hash) <= threshold_;
        }
    };

    void Load(const string &filename) {
        std::ifstream file(filename, std::ios::in);

        if (file) {
            Hash file_hash;
            thrust::host_vector<Hash> host_hashes;

            while (file >> file_hash) {
                entries_.push_back(file_hash);
                host_hashes.push_back(file_hash);
            }

            // Copy the hashes to the GPU.
            device_hashes_.resize(host_hashes.size());
            device_results_.resize(MAX_RESULTS);
            thrust::copy(host_hashes.cbegin(), host_hashes.cend(), device_hashes_.begin());

            // Sort and unique the hashes on the GPU.
            thrust::sort(device_hashes_.begin(), device_hashes_.end());
            typename thrust::device_vector<Hash>::iterator device_hashes_end;
            device_hashes_end = thrust::unique(device_hashes_.begin(), device_hashes_.end());
            device_hashes_.resize(thrust::distance(device_hashes_.begin(), device_hashes_end));

            // Sort the hashes on the CPU.
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

        // Filter the hashes on the GPU.
        typename thrust::device_vector<Hash>::iterator device_results_end;
        device_results_end = thrust::copy_if(
            device_hashes_.begin(), device_hashes_.end(), device_results_.begin(),
            ThresholdFilter(file_hash, threshold)
        );
        thrust::host_vector<Hash> hash_results(device_results_.begin(), device_results_end);

        // Format the results from the GPU.
        for (unsigned int i = 0; i < hash_results.size(); i++) {
            result.entries.push_back(ResultEntry(CpuHashDistance(file_hash, hash_results[i]), hash_results[i]));
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
    const unsigned int MAX_RESULTS = 128;

    std::vector<Hash> entries_;

    thrust::device_vector<Hash> device_hashes_;
    thrust::device_vector<Hash> device_results_;
};

#endif //GPUHASHSTORE_H
