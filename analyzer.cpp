#include "analyzer.h"
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <string>
#include <cctype>

// We cannot modify analyzer.h, so we store state using a static map keyed by 'this'
struct AnalyzerState {
    std::unordered_map<std::string, long long> zoneCounts;
    std::unordered_map<std::string, std::unordered_map<int, long long>> slotCounts;
};

static std::map<const TripAnalyzer*, AnalyzerState> s_states;

inline AnalyzerState& getState(const TripAnalyzer* self) {
    return s_states[self];
}

void TripAnalyzer::ingestFile(const std::string& csvPath) {
    auto& state = getState(this);
    state.zoneCounts.clear();
    state.slotCounts.clear();

    std::ifstream file(csvPath);
    if (!file.is_open()) {
        return; // A1: missing file → return empty results
    }

    std::string line;
    std::getline(file, line); // skip header

    while (std::getline(file, line)) {
        if (line.empty()) continue;

        std::stringstream ss(line);
        std::string field;
        std::vector<std::string> fields;
        while (std::getline(ss, field, ',')) {
            fields.push_back(field);
        }

        // A2: Skip malformed rows (must have exactly 6 fields)
        if (fields.size() != 6) continue;

        std::string pickupZone = fields[1];
        std::string datetimeStr = fields[3];

        // Skip if PickupZoneID is empty
        if (pickupZone.empty()) continue;

        // Parse hour from PickupDateTime: "YYYY-MM-DD HH:MM"
        size_t spacePos = datetimeStr.find(' ');
        if (spacePos == std::string::npos) continue;

        std::string timePart = datetimeStr.substr(spacePos + 1);
        size_t colonPos = timePart.find(':');
        if (colonPos == std::string::npos || colonPos == 0) continue;

        std::string hourStr = timePart.substr(0, colonPos);
        // Trim whitespace
        auto start = hourStr.find_first_not_of(" \t");
        auto end = hourStr.find_last_not_of(" \t");
        if (start == std::string::npos) continue;
        hourStr = hourStr.substr(start, (end - start + 1));

        int hour = -1;
        try {
            hour = std::stoi(hourStr);
        } catch (...) {
            continue; // invalid hour format
        }

        if (hour < 0 || hour > 23) continue; // A3: only accept 0–23

        // Aggregate counts
        state.zoneCounts[pickupZone]++;
        state.slotCounts[pickupZone][hour]++;
    }
}

std::vector<ZoneCount> TripAnalyzer::topZones(int k) const {
    const auto& state = getState(this);
    std::vector<ZoneCount> result;
    result.reserve(state.zoneCounts.size());
    for (const auto& p : state.zoneCounts) {
        result.push_back({p.first, p.second});
    }

    // B2: sort by count desc, then zone asc
    std::sort(result.begin(), result.end(), [](const ZoneCount& a, const ZoneCount& b) {
        if (a.count != b.count) return a.count > b.count;
        return a.zone < b.zone;
    });

    if (static_cast<int>(result.size()) > k)
        result.resize(k);
    return result;
}

std::vector<SlotCount> TripAnalyzer::topBusySlots(int k) const {
    const auto& state = getState(this);
    std::vector<SlotCount> result;
    result.reserve(state.slotCounts.size() * 24);
    for (const auto& zoneEntry : state.slotCounts) {
        const std::string& zone = zoneEntry.first;
        for (const auto& hourEntry : zoneEntry.second) {
            result.push_back({zone, hourEntry.first, hourEntry.second});
        }
    }

    // C3: sort by count desc, then zone asc, then hour asc
    std::sort(result.begin(), result.end(), [](const SlotCount& a, const SlotCount& b) {
        if (a.count != b.count) return a.count > b.count;
        if (a.zone != b.zone) return a.zone < b.zone;
        return a.hour < b.hour;
    });

    if (static_cast<int>(result.size()) > k)
        result.resize(k);
    return result;
}
