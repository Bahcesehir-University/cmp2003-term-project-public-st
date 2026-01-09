#include "analyzer.h"
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <string>
#include <cctype>

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
        return;
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

        size_t spacePos = datetimeStr.find(' ');
        if (spacePos == std::string::npos) continue;

        std::string timePart = datetimeStr.substr(spacePos + 1);
        if (timePart.length() < 2) continue;

        // Parse hour from "HH:MM" or "H:MM"
        int hour = -1;
        size_t colonPos = timePart.find(':');
        if (colonPos == std::string::npos || colonPos == 0) continue;

        std::string hourStr = timePart.substr(0, colonPos);
        // Trim whitespace
        auto start = hourStr.find_first_not_of(" \t");
        auto end = hourStr.find_last_not_of(" \t");
        if (start == std::string::npos) continue;
        hourStr = hourStr.substr(start, end - start + 1);

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
    for (const auto& p : zoneCounts) {
        result.push_back({p.first, p.second});
    }

    std::sort(result.begin(), result.end(), [](const ZoneCount& a, const ZoneCount& b) {
        if (a.count != b.count) return a.count > b.count;
        return a.zone < b.zone;
    });

    if (static_cast<int>(result.size()) > k)
        result.resize(k);
    return result;
}

std::vector<SlotCount> TripAnalyzer::topBusySlots(int k) const {
    std::vector<SlotCount> result;
    for (const auto& zoneEntry : slotCounts) {
        const std::string& zone = zoneEntry.first;
        for (const auto& hourEntry : zoneEntry.second) {
            result.push_back({zone, hourEntry.first, hourEntry.second});
        }
    }

    std::sort(result.begin(), result.end(), [](const SlotCount& a, const SlotCount& b) {
        if (a.count != b.count) return a.count > b.count;
        if (a.zone != b.zone) return a.zone < b.zone;
        return a.hour < b.hour;
    });

    if (static_cast<int>(result.size()) > k)
        result.resize(k);
    return result;
}
