#include "analyzer.h"
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <string>
#include <cctype>
#include <map>

// Private data for each TripAnalyzer instance
struct AnalyzerData {
    std::unordered_map<std::string, long long> zoneCounts;
    std::unordered_map<std::string, std::unordered_map<int, long long>> slotCounts;
};

// Global map: object pointer -> its data
static std::map<const TripAnalyzer*, AnalyzerData> s_data;

// Helper to get data for 'this'
inline AnalyzerData& getData(const TripAnalyzer* self) {
    return s_data[self];
}

void TripAnalyzer::ingestFile(const std::string& csvPath) {
    auto& data = getData(this);
    data.zoneCounts.clear();
    data.slotCounts.clear();

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
        data.zoneCounts[pickupZone]++;
        data.slotCounts[pickupZone][hour]++;
    }
}

std::vector<ZoneCount> TripAnalyzer::topZones(int k) const {
    const auto& data = getData(this);
    std::vector<ZoneCount> result;
    result.reserve(data.zoneCounts.size());
    for (const auto& p : data.zoneCounts) {
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
    const auto& data = getData(this);
    std::vector<SlotCount> result;
    result.reserve(data.slotCounts.size() * 24);
    for (const auto& zoneEntry : data.slotCounts) {
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
