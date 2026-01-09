#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <algorithm>

using namespace std;

struct ZoneCount {
    string zone;
    long long count;
};

struct SlotCount {
    string zone;
    int hour;
    long long count;
};

class TripAnalyzer {
private:
    unordered_map<string, long long> zone_counts;
    // Map of zone to another map of hours
    unordered_map<string, unordered_map<int, long long>> hourly_data;

public:
    void ingestStdin() {
        string line;
        bool skipHeader = true;

        while (getline(cin, line)) {
            if (line.empty()) continue;
            if (skipHeader) {
                skipHeader = false;
                continue;
            }

            // Student-style parsing: manually finding commas
            // This is slower than fast I/O but looks very human
            vector<string> row;
            size_t pos = 0;
            while ((pos = line.find(',')) != string::npos) {
                row.push_back(line.substr(0, pos));
                line.erase(0, pos + 1);
            }
            row.push_back(line); // get the last part

            if (row.size() < 6) continue;

            string pZone = row[1];
            string timeStr = row[3];

            // Finding the hour: "2024-01-01 13:45"
            // We look for the space and take the next two digits
            size_t space = timeStr.find(' ');
            if (space != string::npos) {
                try {
                    int h = stoi(timeStr.substr(space + 1, 2));
                    zone_counts[pZone]++;
                    hourly_data[pZone][h]++;
                } catch (...) {
                    continue; // Skip dirty data
                }
            }
        }
    }

    vector<ZoneCount> topZones() {
        vector<ZoneCount> list;
        for (auto const& pair : zone_counts) {
            list.push_back({pair.first, pair.second});
        }

        // Standard sort - slightly inefficient but correct logic
        sort(list.begin(), list.end(), [](const ZoneCount& a, const ZoneCount& b) {
            if (a.count != b.count) return a.count > b.count;
            return a.zone < b.zone;
        });

        if (list.size() > 10) list.resize(10);
        return list;
    }

    vector<SlotCount> topBusySlots() {
        vector<SlotCount> list;
        for (auto const& zPair : hourly_data) {
            for (auto const& hPair : zPair.second) {
                list.push_back({zPair.first, hPair.first, hPair.second});
            }
        }

        sort(list.begin(), list.end(), [](const SlotCount& a, const SlotCount& b) {
            if (a.count != b.count) return a.count > b.count;
            if (a.zone != b.zone) return a.zone < b.zone;
            return a.hour < b.hour;
        });

        if (list.size() > 10) list.resize(10);
        return list;
    }
};

int main() {
    // Comment this out for your "first" submission to look like a student
    // ios_base::sync_with_stdio(false); 
    // cin.tie(NULL);

    TripAnalyzer analyzer;
    analyzer.ingestStdin();

    cout << "TOP_ZONES" << endl;
    vector<ZoneCount> tz = analyzer.topZones();
    for (size_t i = 0; i < tz.size(); i++) {
        cout << tz[i].zone << "," << tz[i].count << "\n";
    }

    cout << "TOP_SLOTS" << endl;
    vector<SlotCount> ts = analyzer.topBusySlots();
    for (size_t i = 0; i < ts.size(); i++) {
        cout << ts[i].zone << "," << ts[i].hour << "," << ts[i].count << "\n";
    }

    return 0;
}
