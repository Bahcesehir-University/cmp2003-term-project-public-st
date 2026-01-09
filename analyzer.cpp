#include "analyzer.h"

#include <fstream>
#include <algorithm>

void TripAnalyzer::ingestFile(const std::string& csvPath) {
    std::ifstream in(csvPath);
    if (!in.is_open()) return;

    std::string line;
    bool skipHeader = true;

    while (std::getline(in, line)) {
        if (line.empty()) continue;
        if (skipHeader) {
            skipHeader = false;
            continue;
        }

        // Student-style parsing: manually finding commas
        std::vector<std::string> row;
        size_t pos = 0;
        while ((pos = line.find(',')) != std::string::npos) {
            row.push_back(line.substr(0, pos));
            line.erase(0, pos + 1);
        }
        row.push_back(line); // get the last part

        if (row.size() < 6) continue;

        std::string pZone = row[1];
        std::string timeStr = row[3];

        // Finding the hour: "2024-01-01 13:45"
        // We look for the space and take the next two digits
        size_t space = timeStr.find(' ');
        if (space != std::string::npos) {
            try {
                int h = std::stoi(timeStr.substr(space + 1, 2));
                zone_counts[pZone]++;
                hourly_data[pZone][h]++;
            } catch (...) {
                continue; // Skip dirty data
            }
        }
    }
}

std::vector<ZoneCount> TripAnalyzer::topZones(int k) const {
    std::vector<ZoneCount> list;
    for (auto const& pair : zone_counts) {
        list.push_back({pair.first, pair.second});
    }

    std::sort(list.begin(), list.end(), [](const ZoneCount& a, const ZoneCount& b) {
        if (a.count != b.count) return a.count > b.count;
        return a.zone < b.zone;
    });

    if ((int)list.size() > k) list.resize(k);
    return list;
}

std::vector<SlotCount> TripAnalyzer::topBusySlots(int k) const {
    std::vector<SlotCount> list;
    for (auto const& zPair : hourly_data) {
        for (auto const& hPair : zPair.second) {
            list.push_back({zPair.first, hPair.first, hPair.second});
        }
    }

    std::sort(list.begin(), list.end(), [](const SlotCount& a, const SlotCount& b) {
        if (a.count != b.count) return a.count > b.count;
        if (a.zone != b.zone) return a.zone < b.zone;
        return a.hour < b.hour;
    });

    if ((int)list.size() > k) list.resize(k);
    return list;
}
