#include <iostream>
#include <vector>
#include <cstdint>
#include <cstdlib>
#include <chrono>

#include "Hamming.h"
#include "CpuHashStore.h"
#include "MihHashStore.h"
#include "GpuHashStore.h"

const int THRESHOLD = 8;

int main(int argc, const char *argv[])
{
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(0);

    if (argc != 3) {
        std::cout << "The program takes two parameters:\nFirst, the device: 'cpu'(default), 'gpu' or 'mih'.\nSecond, a path to an index file." << std::endl;
        return 1;
    }

    string device(argv[1]);
    string index_file(argv[2]);
    chrono::time_point<chrono::steady_clock> start_time;
    chrono::time_point<chrono::steady_clock> end_time;

    typedef uint64_t Hash;
    typedef typename HashStore<uint64_t>::Result Result;
    typedef typename HashStore<uint64_t>::ResultEntry ResultEntry;

    HashStore<uint64_t>* store = nullptr;
    if (device.compare("gpu") == 0) {
        store = new GpuHashStore<Hash, CpuHammingDistance, GpuHammingDistance>();
    } else if (device.compare("mih") == 0) {
        // Multi index hashing with 4 hash tables and for 4 nearest neighbors.
        store = new MihHashStore<Hash, CpuHammingDistance>(4, 4);
    } else {
        store = new CpuHashStore<Hash, CpuHammingDistance>();
    }

    // Measure time to load the data.
    start_time = chrono::steady_clock::now();
    store->Load(index_file);
    end_time = chrono::steady_clock::now();
    cout << "Time to load the data: " << chrono::duration<double, milli> (end_time - start_time).count() << " ms" << endl;

    vector<Hash> entries(store->Entries());

    // If the hashstore contains more than 10000 hashes, only 10000 queries are performed.
    if (entries.size() > 10000) {
        entries.resize(10000);
    }

    // Measure time to process the queries.
    start_time = chrono::steady_clock::now();
    
    vector<Result> results = store->BatchSearch(entries, THRESHOLD);
    
    end_time = chrono::steady_clock::now();
    cout << "Time to process the queries: " << chrono::duration<double, milli> (end_time - start_time).count() << " ms" << endl;
    
    for (auto itResult = results.begin(); itResult != results.end(); ++itResult) {
        cout << "Query : " << itResult->query_hash << "\n";
        for (auto it = itResult->entries.begin(); it != itResult->entries.end(); ++it) {
            cout << it->dist << " : " << it->hash << "\n";
        }
        cout << "\n";
    }

    delete store;

    return 0;
}

