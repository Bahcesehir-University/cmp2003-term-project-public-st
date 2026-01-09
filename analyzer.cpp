#include "analyzer.h"
#include <fstream>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <string>

using namespace std;

static unordered_map<string, long long> zoneCounts;
static unordered_map<string, long long> slotCounts;

// Fast split by comma (assumes well-formed but skips bad rows)
static bool parseLine(const string& line, string& pickupZone, int& hour) {
    size_t start = 0;
    int field = 0;
    string fields[4]; // we only need field[1] and field[3]

    for (size_t i = 0; i <= line.size(); ++i) {
        if (i == line.size() || line[i] == ',') {
            if (field < 4) {
                fields[field] = line.substr(start, i - start);
                field++;
            }
            start = i + 1;
        }
    }

    if (field < 4) return false;

    pickupZone = fields[1];
    const string& datetime = fields[3];

    // Skip trimming if we trust data (or do minimal trim)
    if (pickupZone.empty()) return false;

    // Find last space
    size_t space = datetime.rfind(' ');
    if (space == string::npos || space + 6 > datetime.size()) return false;

    const string& timePart = datetime.substr(space + 1);
    if (timePart.size() < 2 || !isdigit(timePart[0])) return false;

    int h = 0;
    if (timePart[1] == ':') {
        // Single-digit hour: "8:42"
        h = timePart[0] - '0';
    } else if (timePart.size() >= 2 && timePart[2] == ':') {
        // Two-digit hour: "08:42"
        h = (timePart[0] - '0') * 10 + (timePart[1] - '0');
    } else {
        return false;
    }

    if (h < 0 || h > 23) return false;
    hour = h;
    return true;
}

void TripAnalyzer::ingestFile(const string& csvPath) {
    zoneCounts.clear();
    slotCounts.clear();

    ifstream file(csvPath, ios::in);
    if (!file.is_open()) return;

    string line;
    bool first = true;

    // Reserve for large input
    zoneCounts.reserve(100000);
    slotCounts.reserve(2400000);

    while (getline(file, line)) {
        if (line.empty()) continue;
        if (line.back() == '\r') line.pop_back();

        if (first) {
            first = false;
            if (line.find("TripID") != string::npos) continue;
        }

        string zone;
        int hour;
        if (!parseLine(line, zone, hour)) continue;

        zoneCounts[zone]++;
        slotCounts[zone + "#" + to_string(hour)]++;
    }
}

// Keep your heap-based topZones / topBusySlots — they're already optimal for small k
vector<ZoneCount> TripAnalyzer::topZones(int k) const {
    auto cmp = [](const ZoneCount& a, const ZoneCount& b) {
        if (a.count != b.count) return a.count > b.count;
        return a.zone < b.zone;
    };
    vector<ZoneCount> heap;
    heap.reserve(k + 1);

    for (const auto& z : zoneCounts) {
        heap.push_back({z.first, z.second});
        push_heap(heap.begin(), heap.end(), cmp);
        if ((int)heap.size() > k) {
            pop_heap(heap.begin(), heap.end(), cmp);
            heap.pop_back();
        }
    }
    sort_heap(heap.begin(), heap.end(), cmp);
    reverse(heap.begin(), heap.end());
    return heap;
}

vector<SlotCount> TripAnalyzer::topBusySlots(int k) const {
    auto cmp = [](const SlotCount& a, const SlotCount& b) {
        if (a.count != b.count) return a.count > b.count;
        if (a.zone != b.zone) return a.zone < b.zone;
        return a.hour < b.hour;
    };
    vector<SlotCount> heap;
    heap.reserve(k + 1);

    for (const auto& s : slotCounts) {
        size_t pos = s.first.find('#');
        if (pos == string::npos) continue;
        SlotCount sc{ s.first.substr(0, pos), stoi(s.first.substr(pos + 1)), s.second };
        heap.push_back(sc);
        push_heap(heap.begin(), heap.end(), cmp);
        if ((int)heap.size() > k) {
            pop_heap(heap.begin(), heap.end(), cmp);
            heap.pop_back();
        }
    }
    sort_heap(heap.begin(), heap.end(), cmp);
    reverse(heap.begin(), heap.end());
    return heap;
}
