#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <algorithm>
#include <fstream>

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
    unordered_map<string, unordered_map<int, long long>> hourly_data;

public:
    // ONLY DIFFERENCE: file input instead of stdin
    void ingestFile(const string& filename) {
        ifstream in(filename);
        if (!in.is_open()) {
            cerr << "Error: Cannot open file\n";
            return;
        }

        string line;
        bool skipHeader = true;

        while (getline(in, line)) {
            if (line.empty()) continue;
            if (skipHeader) {
                skipHeader = false;
                continue;
            }

            vector<string> row;
            size_t pos = 0;
            while ((pos = line.find(',')) != string::npos) {
                row.push_back(line.substr(0, pos));
                line.erase(0, pos + 1);
            }
            row.push_back(line);

            if (row.size() < 6) continue;

            string pZone = row[1];
            string timeStr = row[3];

            size_t space = timeStr.find(' ');
            if (space != string::npos) {
                try {
                    int h = stoi(timeStr.substr(space + 1, 2));
                    zone_counts[pZone]++;
                    hourly_data[pZone][h]++;
                } catch (...) {
                    continue;
                }
            }
        }

        in.close();
    }

    vector<ZoneCount> topZones() {
        vector<ZoneCount> list;
        for (auto const& pair : zone_counts) {
            list.push_back({pair.first, pair.second});
        }

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

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cerr << "Usage: ./trip <input_file.csv>\n";
        return 1;
    }

    TripAnalyzer analyzer;
    analyzer.ingestFile(argv[1]);

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
