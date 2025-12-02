#ifndef ENGINE_H
#define ENGINE_H

#include <iostream>   
#include <vector>     
#include <algorithm>
#include "BST.h"      
#include "Record.h"
//add header files as needed

using namespace std;

// Converts a string to lowercase (used for case-insensitive searches)
static inline string toLower(string s) {
    for (char &c : s) c = (char)tolower((unsigned char)c);
    return s;
}

// ================== Index Engine ==================
// Acts like a small "database engine" that manages records and two BST indexes:
// 1) idIndex: maps student_id → record index (unique key)
// 2) lastIndex: maps lowercase(last_name) → list of record indices (non-unique key)
struct Engine {
    vector<Record> heap;                  // the main data store (simulates a heap file)
    BST<int, int> idIndex;                // index by student ID
    BST<string, vector<int>> lastIndex;   // index by last name (can have duplicates)


    int insertRecord(const Record& recIn) {
        int rid = (int)heap.size();
        heap.push_back(recIn);
        idIndex.insert(recIn.id, rid);
        string key = toLower(recIn.last);
        auto vecPtr = lastIndex.find(key);
        if (vecPtr) {
            vecPtr->push_back(rid);
        }
        else {
            lastIndex.insert(key, vector<int>{rid});
        }
        return rid;
    }

    // Deletes a record logically (marks as deleted and updates indexes)
    // Returns true if deletion succeeded.
    bool deleteById(int id) {
        int* ridPtr = idIndex.find(id);
        if (!ridPtr) return false;
        int rid = *ridPtr;
        if (rid < 0 || rid >= (int)heap.size()) return false;
        if (heap[rid].deleted) return false;
        heap[rid].deleted = true;
        idIndex.erase(id);
        string key = toLower(heap[rid].last);
        auto vecPtr = lastIndex.find(key);
        if (vecPtr) {
            auto& vec = *vecPtr;
            vec.erase(
                remove(vec.begin(), vec.end(), rid),
                vec.end()
            );
        }

        return true;
    }

    // Finds a record by student ID.
    // Returns a pointer to the record, or nullptr if not found.
    // Outputs the number of comparisons made in the search.
    const Record* findById(int id, int& cmpOut) {
        idIndex.resetMetrics();
        int* ridPtr = idIndex.find(id);
        cmpOut = idIndex.comparisons;

        if (!ridPtr) return nullptr;
        int rid = *ridPtr;

        if (rid >= 0 && rid < (int)heap.size() && !heap[rid].deleted)
            return &heap[rid];
        return nullptr;
    }

    // Returns all records with ID in the range [lo, hi].
    // Also reports the number of key comparisons performed.
    vector<const Record*> rangeById(int lo, int hi, int& cmpOut) {
        idIndex.resetMetrics();
        vector<const Record*> out;

        idIndex.rangeApply(lo, hi, [&](const int& k, int& rid) {
            if (rid >= 0 && rid < (int)heap.size() && !heap[rid].deleted)
                out.push_back(&heap[rid]);
            });

        cmpOut = idIndex.comparisons;
        return out;
    }

    // Returns all records whose last name begins with a given prefix.
    // Case-insensitive using lowercase comparison.
    vector<const Record*> prefixByLast(const string& prefix, int& cmpOut) {
        lastIndex.resetMetrics();
        vector<const Record*> out;
        string p = toLower(prefix);
        string p_hi = p;
        p_hi.push_back(char(255));
        lastIndex.rangeApply(p, p_hi, [&](const string& key, vector<int>& vec) {
            if (key.rfind(p, 0) != 0) return;

            for (int rid : vec) {
                if (rid >= 0 && rid < (int)heap.size() && !heap[rid].deleted)
                    out.push_back(&heap[rid]);
            }
            });
        cmpOut = lastIndex.comparisons;
        return out;
    }
};

#endif
