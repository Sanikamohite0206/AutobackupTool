#ifndef BACKUP_MANAGER_H
#define BACKUP_MANAGER_H

#include <string>
#include <filesystem>

class BackupManager {
private:
    std::string sourcePath;
    std::string backupPath;

public:
    BackupManager(const std::string& src, const std::string& dest);
    void startMonitoring();     // Watches for file changes
    void performBackup();       // Copies updated files
    void logActivity(const std::string& message);  // Writes to log
    void restoreFile(const std::string& filename); // 🔹 New function
};

#endif
