#ifndef HASHSTORE_H
#define HASHSTORE_H

#include <string>

using namespace std;

template<typename H>
class HashStore {
public:
    typedef H Hash;

    struct ResultEntry {
        int dist;
        Hash hash;

        ResultEntry(int d, Hash h) : dist(d), hash(h) {}

        bool operator<(const ResultEntry &other) const {
            return dist < other.dist || (dist == other.dist && hash < other.hash);
        }
    };

    struct Result {
        Hash query_hash;
        vector<ResultEntry> entries;

        Result(Hash h) : query_hash(h) {}
        
        Result(Hash h, const vector<ResultEntry>& e) : query_hash(h), entries(e) {}

        bool operator<(const Result &other) const {
            return query_hash < other.query_hash;
        }
    };

    /**
     * Load the hashes from a file.
     * @param filename A path to a file.
     */
    virtual void Load(const string &filename) = 0;

    /**
     * Return a vector of all entries in the HashStore.
     * @return A vector of all entries in the HashStore.
     */
    virtual vector<Hash> Entries() const = 0;

    /**
     * Search in the store for the hashes that are nearer to the query than a threshold.
     * If the distance is equal to the threshold the hash is selected.
     * @param file_hash The query hash.
     * @param threshold The maximum distance between the query and the hashes.
     * @return A vector with all the hashes and their distances to the query in a pair.
     */
    virtual Result Search(const Hash &file_hash, const int threshold) = 0;

    /**
     * Search in the store for the hashes that are nearer to the query than a threshold.
     * If the distance is equal to the threshold the hash is selected.
     * @param file_hash The query hash.
     * @param threshold The maximum distance between the query and the hashes.
     * @return A vector with all the queries and their results : vectors with all the hashes and their distances to the query in a pair.
     */
    virtual vector<Result> BatchSearch(const vector<Hash> &queries, const int threshold) = 0;
};

#endif //HASHSTORE_H
