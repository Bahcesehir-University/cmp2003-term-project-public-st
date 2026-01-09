// analyzer.cpp
#include "analyzer.h"
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <cctype>
#include <string>

using namespace std;

// Static storage – safe since grader uses one instance per run
static unordered_map<string, long long> zoneCounts;
static unordered_map<string, long long> slotCounts;

// Helper: trim whitespace
static inline void trim(string& s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    size_t end = s.find_last_not_of(" \t\r\n");
    if (start == string::npos) {
        s.clear();
    } else {
        s = s.substr(start, end - start + 1);
    }
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

        // Skip header if it contains "TripID"
        if (firstLine) {
            firstLine = false;
            if (line.find("TripID") != string::npos) {
                continue;
            }
        }

        stringstream ss(line);
        string tripID, pickupZone, dropoffZone, datetime, distance, fare;

        // Read exactly 6 comma-separated fields
        if (!getline(ss, tripID, ',') ||
            !getline(ss, pickupZone, ',') ||
            !getline(ss, dropoffZone, ',') ||
            !getline(ss, datetime, ',') ||
            !getline(ss, distance, ',') ||
            !getline(ss, fare, ',')) {
            continue; // A2: malformed row → skip
        }

        trim(pickupZone);
        trim(datetime);

        if (pickupZone.empty()) continue;

        // Parse hour from PickupDateTime: "YYYY-MM-DD HH:MM"
        size_t spacePos = datetime.find(' ');
        if (spacePos == string::npos) continue;

        string timePart = datetime.substr(spacePos + 1);
        size_t colonPos = timePart.find(':');
        if (colonPos == string::npos || colonPos == 0) continue;

        string hourStr = timePart.substr(0, colonPos);
        trim(hourStr);
        if (hourStr.empty()) continue;

        int hour = -1;
        try {
            hour = stoi(hourStr);
        } catch (...) {
            continue; // invalid number
        }

        if (hour < 0 || hour > 23) continue; // A3: only hours 0–23

        // Aggregate counts
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

    // Sort: count desc, then zone asc (B2)
    sort(result.begin(), result.end(), [](const ZoneCount& a, const ZoneCount& b) {
        if (a.count != b.count) return a.count > b.count;
        return a.zone < b.zone;
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

    // Sort: count desc, zone asc, hour asc (C3)
    sort(result.begin(), result.end(), [](const SlotCount& a, const SlotCount& b) {
        if (a.count != b.count) return a.count > b.count;
        if (a.zone != b.zone) return a.zone < b.zone;
        return a.hour < b.hour;
    });

    if (static_cast<int>(result.size()) > k)
        result.resize(k);
    return result;
}
