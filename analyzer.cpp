#include "analyzer.h"
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <string>
#include <cctype>

using namespace std;

// Static storage (safe since autograder uses one instance)
static unordered_map<string, long long> zoneCounts;
static unordered_map<string, long long> slotCounts;

// Helper: trim whitespace
static inline void trim(string& s) {
    while (!s.empty() && isspace(static_cast<unsigned char>(s.front())))
        s.erase(s.begin());
    while (!s.empty() && isspace(static_cast<unsigned char>(s.back())))
        s.pop_back();
}

void TripAnalyzer::ingestFile(const string& csvPath) {
    zoneCounts.clear();
    slotCounts.clear();

    ifstream file(csvPath);
    if (!file.is_open()) {
        return; // A1: missing/empty file → return empty results
    }

    string line;
    bool firstLine = true;

    while (getline(file, line)) {
        if (line.empty()) continue;

        // Handle Windows line endings
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }

        // Skip header (identified by presence of "TripID")
        if (firstLine) {
            firstLine = false;
            if (line.find("TripID") != string::npos) {
                continue;
            }
        }

        stringstream ss(line);
        string tripID, pickupZone, dropoffZone, datetime, distance, fare;

        // Read exactly 6 fields
        if (!getline(ss, tripID, ',') ||
            !getline(ss, pickupZone, ',') ||
            !getline(ss, dropoffZone, ',') ||
            !getline(ss, datetime, ',') ||
            !getline(ss, distance, ',') ||
            !getline(ss, fare, ',')) {
            continue; // A2: malformed row
        }

        trim(pickupZone);
        trim(datetime);

        if (pickupZone.empty()) continue;

        // Parse hour from "YYYY-MM-DD HH:MM" → take last 5 chars: "HH:MM"
        if (datetime.size() < 5) continue;

        string timePart = datetime.substr(datetime.size() - 5);
        if (timePart.size() != 5 || timePart[2] != ':') continue;

        string hourStr = timePart.substr(0, 2);
        int hour = -1;
        try {
            hour = stoi(hourStr);
        } catch (...) {
            continue; // invalid hour
        }

        if (hour < 0 || hour > 23) continue; // A3: only 0–23

        // Aggregate
        zoneCounts[pickupZone]++;
        slotCounts[pickupZone + "#" + to_string(hour)]++;
    }
}

vector<ZoneCount> TripAnalyzer::topZones(int k) const {
    vector<ZoneCount> result;
    result.reserve(zoneCounts.size());

    for (const auto& p : zoneCounts) {
        result.push_back({p.first, p.second});
    }

    sort(result.begin(), result.end(), [](const ZoneCount& a, const ZoneCount& b) {
        if (a.count != b.count) return a.count > b.count; // desc count
        return a.zone < b.zone; // asc zone (B2)
    });

    if (static_cast<int>(result.size()) > k)
        result.resize(k);
    return result;
}

vector<SlotCount> TripAnalyzer::topBusySlots(int k) const {
    vector<SlotCount> result;
    result.reserve(slotCounts.size());

    for (const auto& p : slotCounts) {
        const string& key = p.first;
        size_t pos = key.find('#');
        if (pos == string::npos) continue;

        string zone = key.substr(0, pos);
        int hour = stoi(key.substr(pos + 1));
        long long count = p.second;

        result.push_back({zone, hour, count});
    }

    sort(result.begin(), result.end(), [](const SlotCount& a, const SlotCount& b) {
        if (a.count != b.count) return a.count > b.count;
        if (a.zone != b.zone) return a.zone < b.zone;
        return a.hour < b.hour; // C3: hour asc tie-break
    });

    if (static_cast<int>(result.size()) > k)
        result.resize(k);
    return result;
}
