#include "analyzer.h"
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <string>
#include <cctype>

// We cannot modify analyzer.h, but we can add private members in .cpp
// by redeclaring the class with identical public interface.
class TripAnalyzer {
public:
    void ingestFile(const std::string& csvPath);
    std::vector<ZoneCount> topZones(int k = 10) const;
    std::vector<SlotCount> topBusySlots(int k = 10) const;

private:
    std::unordered_map<std::string, long long> zoneCounts;
    std::unordered_map<std::string, std::unordered_map<int, long long>> slotCounts;
};

void TripAnalyzer::ingestFile(const std::string& csvPath) {
    zoneCounts.clear();
    slotCounts.clear();

    std::ifstream file(csvPath);
    if (!file.is_open()) {
        return; // silently handle missing file (per test A1)
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

        if (fields.size() != 6) continue; // malformed row

        std::string pickupZone = fields[1];
        std::string datetimeStr = fields[3];

        if (pickupZone.empty()) continue;

        // Parse hour from "YYYY-MM-DD HH:MM"
        size_t spacePos = datetimeStr.find(' ');
        if (spacePos == std::string::npos) continue;

        std::string timePart = datetimeStr.substr(spacePos + 1);
        if (timePart.length() < 2) continue;

        std::string hourStr = timePart.substr(0, 2);
        int hour = -1;

        // Validate that hourStr is two digits
        if (hourStr.size() != 2 || !std::isdigit(hourStr[0]) || !std::isdigit(hourStr[1])) {
            continue;
        }

        try {
            hour = std::stoi(hourStr);
        } catch (...) {
            continue;
        }

        if (hour < 0 || hour > 23) continue;

        zoneCounts[pickupZone]++;
        slotCounts[pickupZone][hour]++;
    }
}

std::vector<ZoneCount> TripAnalyzer::topZones(int k) const {
    std::vector<ZoneCount> result;
    result.reserve(zoneCounts.size());

    for (const auto& entry : zoneCounts) {
        result.push_back({entry.first, entry.second});
    }

    std::sort(result.begin(), result.end(), [](const ZoneCount& a, const ZoneCount& b) {
        if (a.count != b.count) {
            return a.count > b.count; // descending by count
        }
        return a.zone < b.zone; // ascending by zone ID (tie-breaker)
    });

    if (static_cast<int>(result.size()) > k) {
        result.resize(k);
    }
    return result;
}

std::vector<SlotCount> TripAnalyzer::topBusySlots(int k) const {
    std::vector<SlotCount> result;
    result.reserve(slotCounts.size() * 24);

    for (const auto& zoneEntry : slotCounts) {
        const std::string& zone = zoneEntry.first;
        for (const auto& hourEntry : zoneEntry.second) {
            result.push_back({zone, hourEntry.first, hourEntry.second});
        }
    }

    std::sort(result.begin(), result.end(), [](const SlotCount& a, const SlotCount& b) {
        if (a.count != b.count) {
            return a.count > b.count; // descending by count
        }
        if (a.zone != b.zone) {
            return a.zone < b.zone; // ascending by zone
        }
        return a.hour < b.hour; // ascending by hour
    });

    if (static_cast<int>(result.size()) > k) {
        result.resize(k);
    }
    return result;
}
