#include "backup_manager.h"
#include <iostream>
#include <fstream>
#include <windows.h>
#include <chrono>
#include <iomanip>
#include <thread>

using namespace std;
namespace fs = std::filesystem;

BackupManager::BackupManager(const string& src, const string& dest)
    : sourcePath(src), backupPath(dest) {
    fs::create_directories(backupPath);
    fs::create_directories("logs");
}

void BackupManager::performBackup() {
    cout << "Checking for changes in: " << sourcePath << endl;

    for (auto& entry : fs::recursive_directory_iterator(sourcePath)) {
        auto relativePath = fs::relative(entry.path(), sourcePath);
        fs::path destFile = fs::path(backupPath) / relativePath;

        try {
            if (fs::is_directory(entry.path())) {
                fs::create_directories(destFile);
            } 
            else if (fs::is_regular_file(entry.path())) {
                string filename = entry.path().filename().string();

                if (filename.substr(0, 2) == "~$" || entry.path().extension() == ".tmp")
                    continue;

                bool copyFileFlag = true;

                if (fs::exists(destFile)) {
                    auto srcTime = fs::last_write_time(entry.path());
                    auto destTime = fs::last_write_time(destFile);
                    if (srcTime <= destTime) {
                        copyFileFlag = false;
                    }
                }

                if (copyFileFlag) {
                    fs::create_directories(destFile.parent_path());
                    fs::copy_file(entry.path(), destFile, fs::copy_options::overwrite_existing);

                    string logMsg = "Copied: " + relativePath.string();
                    logActivity(logMsg);
                    cout << logMsg << endl;
                }
            }
        } 
        catch (fs::filesystem_error &e) {
            cout << "Skipped: " << relativePath.string()
                 << " (" << e.what() << ")" << endl;
        }
    }
}

void BackupManager::logActivity(const std::string& message) {
    std::ofstream logFile("logs/backup_log.txt", std::ios::app);
    auto now = std::chrono::system_clock::now();
    auto t = std::chrono::system_clock::to_time_t(now);

    logFile << std::put_time(std::localtime(&t), "%Y-%m-%d %H:%M:%S")
            << " - " << message << std::endl;
}

void BackupManager::startMonitoring() {
    cout << "Monitoring folder for changes..." << endl;

    HANDLE hDir = FindFirstChangeNotificationA(
        sourcePath.c_str(),
        TRUE,
        FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE
    );

    if (hDir == INVALID_HANDLE_VALUE) {
        cerr << "Error: Could not monitor directory." << endl;
        return;
    }

    while (true) {
        DWORD waitStatus = WaitForSingleObject(hDir, INFINITE);

        if (waitStatus == WAIT_OBJECT_0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(300));
            performBackup();
            FindNextChangeNotification(hDir);
        }
    }
}

// 🔹 New Restore Function
void BackupManager::restoreFile(const std::string& filename) {
    fs::path backupFile = fs::path(backupPath) / filename;
    fs::path restorePath = fs::path(sourcePath) / filename;

    std::ofstream logFile("logs/backup_log.txt", std::ios::app);
    auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

    if (!fs::exists(backupFile)) {
        cout << "❌ File not found in backup folder: " << filename << endl;
        logFile << std::put_time(std::localtime(&now), "%Y-%m-%d %H:%M:%S")
                << " - Restore failed (file not found): " << filename << endl;
        return;
    }

    try {
        fs::copy_file(backupFile, restorePath, fs::copy_options::overwrite_existing);
        cout << "✅ File restored successfully: " << filename << endl;
        logFile << std::put_time(std::localtime(&now), "%Y-%m-%d %H:%M:%S")
                << " - Restored file: " << filename << endl;
    }
    catch (const fs::filesystem_error& e) {
        cerr << "Error restoring file: " << e.what() << endl;
        logFile << std::put_time(std::localtime(&now), "%Y-%m-%d %H:%M:%S")
                << " - Restore error: " << e.what() << endl;
    }
}
