#include "analyzer.h"
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <queue>
#include <vector>
#include <cctype>

using namespace std;


static unordered_map<string, long long> zoneCounts;
static unordered_map<string, long long> slotCounts;


static inline void trim(string& s) {
    size_t start = 0;
    while (start < s.size() && isspace(s[start])) start++;

    size_t end = s.size();
    while (end > start && isspace(s[end - 1])) end--;

    s = s.substr(start, end - start);
}


void TripAnalyzer::ingestFile(const string& csvPath) {
    zoneCounts.clear();
    slotCounts.clear();


    zoneCounts.reserve(100000);
    slotCounts.reserve(100000);

    ifstream file(csvPath, ios::in);
    if (!file.is_open())
        return;

    string line;
    bool firstLine = true;

    while (getline(file, line)) {
        if (line.empty())
            continue;

        if (line.back() == '\r')
            line.pop_back();

      
        if (firstLine) {
            firstLine = false;
            if (line.find("TripID") != string::npos)
                continue;
        }

        string tripID, pickupZone, dropoffZone, datetime;
        string distance, fare;

        stringstream ss(line);
        if (!getline(ss, tripID, ',')) continue;
        if (!getline(ss, pickupZone, ',')) continue;
        if (!getline(ss, dropoffZone, ',')) continue;
        if (!getline(ss, datetime, ',')) continue;
        if (!getline(ss, distance, ',')) continue;
        if (!getline(ss, fare, ',')) continue;

        trim(pickupZone);
        trim(datetime);

        if (datetime.size() < 5)
            continue;

        int hour;
        try {
            hour = stoi(datetime.substr(datetime.size() - 5, 2));
        } catch (...) {
            continue;
        }

        if (hour < 0 || hour > 23)
            continue;

        ++zoneCounts[pickupZone];
        ++slotCounts[pickupZone + "#" + to_string(hour)];
    }

    file.close();
}


vector<ZoneCount> TripAnalyzer::topZones(int k) const {
    auto cmp = [](const ZoneCount& a, const ZoneCount& b) {
        if (a.count != b.count)
            return a.count > b.count;   
        return a.zone < b.zone;
    };

    priority_queue<ZoneCount, vector<ZoneCount>, decltype(cmp)> heap(cmp);

    for (const auto& z : zoneCounts) {
        heap.push({z.first, z.second});
        if ((int)heap.size() > k)
            heap.pop();
    }

    vector<ZoneCount> result(heap.size());
    for (int i = (int)heap.size() - 1; i >= 0; --i) {
        result[i] = heap.top();
        heap.pop();
    }

    return result;
}


vector<SlotCount> TripAnalyzer::topBusySlots(int k) const {
    auto cmp = [](const SlotCount& a, const SlotCount& b) {
        if (a.count != b.count)
            return a.count > b.count;
        if (a.zone != b.zone)
            return a.zone < b.zone;
        return a.hour < b.hour;
    };

    priority_queue<SlotCount, vector<SlotCount>, decltype(cmp)> heap(cmp);

    for (const auto& s : slotCounts) {
        size_t pos = s.first.find('#');
        if (pos == string::npos)
            continue;

        SlotCount sc;
        sc.zone = s.first.substr(0, pos);
        sc.hour = stoi(s.first.substr(pos + 1));
        sc.count = s.second;

        heap.push(sc);
        if ((int)heap.size() > k)
            heap.pop();
    }

    vector<SlotCount> result(heap.size());
    for (int i = (int)heap.size() - 1; i >= 0; --i) {
        result[i] = heap.top();
        heap.pop();
    }

    return result;
}
