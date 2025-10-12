#include "backup_manager.h"
#include <iostream>
#include <fstream>
#include <string>

using namespace std;

// Function to read settings
string getSetting(const string& key) {
    ifstream config("config/settings.txt");
    if (!config.is_open()) {
        cout << "Failed to open config/settings.txt" << endl;
        return "";
    }

    string line;
    while (getline(config, line)) {
        if (line.empty() || line[0] == '#') continue;

        size_t pos = line.find('=');
        if (pos == string::npos) continue;

        string k = line.substr(0, pos);
        string v = line.substr(pos + 1);

        // Trim spaces
        k.erase(0, k.find_first_not_of(" \t"));
        k.erase(k.find_last_not_of(" \t") + 1);
        v.erase(0, v.find_first_not_of(" \t"));
        v.erase(v.find_last_not_of(" \t") + 1);

        // Remove quotes if present
        if (!v.empty() && v.front() == '"' && v.back() == '"') {
            v = v.substr(1, v.size() - 2);
        }

        if (k == key) return v;
    }

    return "";
}

int main() {
    string source = getSetting("SOURCE_PATH");
    string backup = getSetting("DEST_PATH");

    cout << "Source path: " << source << endl;
    cout << "Backup path: " << backup << endl;

    BackupManager manager(source, backup);

    int choice;
    do {
        cout << "\n===== Auto Backup Tool =====\n";
        cout << "1. Start Backup\n";
        cout << "2. View Logs\n";
        cout << "3. Exit\n";
        cout << "Enter your choice: ";
        cin >> choice;

        switch(choice) {
            case 1:
                cout << "Performing initial backup...\n";
                manager.performBackup();
                cout << "Monitoring for changes (Press Ctrl+C to stop)...\n";
                manager.startMonitoring();  // Infinite monitoring loop
                break;

            case 2: {
                ifstream logFile("logs/backup_log.txt");
                if (!logFile.is_open()) {
                    cout << "No logs found.\n";
                    break;
                }
                cout << "\n===== Backup Logs =====\n";
                string line;
                while (getline(logFile, line)) {
                    cout << line << endl;
                }
                break;
            }

            case 3:
                cout << "Exiting program...\n";
                break;

            default:
                cout << "Invalid choice! Please try again.\n";
        }

    } while (choice != 3);

    return 0;
}
