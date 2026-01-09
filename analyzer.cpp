#include "analyzer.h"
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <string>
#include <cctype>
#include <map>

// Use a static map to associate each object with its data
static std::map<const TripAnalyzer*, 
    std::pair<
        std::unordered_map<std::string, long long>,
        std::unordered_map<std::string, std::unordered_map<int, long long>>
    >
> g_state;

void TripAnalyzer::ingestFile(const std::string& csvPath) {
    auto& [zoneCounts, slotCounts] = g_state[this];
    zoneCounts.clear();
    slotCounts.clear();

    std::ifstream file(csvPath);
    if (!file.is_open()) {
        return; // A1: missing file → return empty
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

        if (fields.size() != 6) continue; // A2: malformed row

        std::string pickupZone = fields[1];
        std::string datetimeStr = fields[3];

        if (pickupZone.empty()) continue;

        size_t spacePos = datetimeStr.find(' ');
        if (spacePos == std::string::npos) continue;

        std::string timePart = datetimeStr.substr(spacePos + 1);
        size_t colonPos = timePart.find(':');
        if (colonPos == std::string::npos || colonPos == 0) continue;

        std::string hourStr = timePart.substr(0, colonPos);
        // Trim whitespace
        size_t start = hourStr.find_first_not_of(" \t");
        size_t end = hourStr.find_last_not_of(" \t");
        if (start == std::string::npos) continue;
        hourStr = hourStr.substr(start, end - start + 1);

        int hour = -1;
        try {
            hour = std::stoi(hourStr);
        } catch (...) {
            continue;
        }

        if (hour < 0 || hour > 23) continue; // A3: valid hour only

        zoneCounts[pickupZone]++;
        slotCounts[pickupZone][hour]++;
    }
}

std::vector<ZoneCount> TripAnalyzer::topZones(int k) const {
    const auto& [zoneCounts, slotCounts] = g_state.at(this);
    std::vector<ZoneCount> result;
    for (const auto& p : zoneCounts) {
        result.push_back({p.first, p.second});
    }

    std::sort(result.begin(), result.end(), [](const ZoneCount& a, const ZoneCount& b) {
        if (a.count != b.count) return a.count > b.count; // desc count
        return a.zone < b.zone; // asc zone (B2)
    });

    if (static_cast<int>(result.size()) > k)
        result.resize(k);
    return result;
}

std::vector<SlotCount> TripAnalyzer::topBusySlots(int k) const {
    const auto& [zoneCounts, slotCounts] = g_state.at(this);
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
        return a.hour < b.hour; // C3: hour asc tie-break
    });

    if (static_cast<int>(result.size()) > k)
        result.resize(k);
    return result;
}
