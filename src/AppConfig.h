#pragma once
// =============================================================================
// AppConfig.h — Application configuration loaded from app_config.yaml
//
// Reads the architecture_mode switch and connection settings at startup.
// Supports two modes:
//   - standalone:  local SQLite database (default)
//   - enterprise:  REST API backend (ApiLogicServer + PostgreSQL)
// =============================================================================

#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iostream>

namespace ppc {

enum class ArchitectureMode { Standalone, Enterprise };

struct StandaloneConfig {
    std::string databasePath = "data/imagery_erp.sqlite";
    bool seedSampleData = true;
};

struct EnterpriseConfig {
    std::string apiBaseUrl = "http://localhost:5656/api";
    std::string apiToken;
    int requestTimeoutSeconds = 30;
    int connectionPoolSize = 5;
};

class AppConfig {
public:
    ArchitectureMode mode = ArchitectureMode::Standalone;
    StandaloneConfig standalone;
    EnterpriseConfig enterprise;

    // Load configuration from a YAML file. Returns true on success.
    // On failure (file not found, parse error), defaults are used and
    // a warning is logged to stderr.
    bool loadFromFile(const std::string& path) {
        std::ifstream file(path);
        if (!file.is_open()) {
            std::cerr << "[AppConfig] Warning: Could not open " << path
                      << " — using defaults (standalone mode)." << std::endl;
            return false;
        }

        std::string line;
        std::string currentSection;  // "", "standalone", "enterprise"

        while (std::getline(file, line)) {
            // Strip comments
            auto commentPos = line.find('#');
            if (commentPos != std::string::npos)
                line = line.substr(0, commentPos);

            // Skip empty lines
            std::string trimmed = trim(line);
            if (trimmed.empty()) continue;

            // Detect section headers (indented keys ending with ':' and no value)
            if (!isIndented(line) && hasKeyValue(trimmed)) {
                auto kv = splitKeyValue(trimmed);
                if (kv.first == "architecture_mode") {
                    std::string val = toLower(kv.second);
                    if (val == "enterprise") {
                        mode = ArchitectureMode::Enterprise;
                    } else {
                        mode = ArchitectureMode::Standalone;
                    }
                    currentSection = "";
                } else if (kv.second.empty()) {
                    // Section header like "standalone:" or "enterprise:"
                    currentSection = kv.first;
                }
            } else if (isIndented(line) && hasKeyValue(trimmed)) {
                auto kv = splitKeyValue(trimmed);
                if (currentSection == "standalone") {
                    parseStandaloneKey(kv.first, kv.second);
                } else if (currentSection == "enterprise") {
                    parseEnterpriseKey(kv.first, kv.second);
                }
            }
        }

        std::cerr << "[AppConfig] Loaded config from " << path
                  << " — mode: " << (mode == ArchitectureMode::Enterprise ? "enterprise" : "standalone")
                  << std::endl;
        return true;
    }

    // Human-readable mode name
    std::string modeName() const {
        return mode == ArchitectureMode::Enterprise ? "enterprise" : "standalone";
    }

private:
    void parseStandaloneKey(const std::string& key, const std::string& value) {
        if (key == "database_path") {
            standalone.databasePath = value;
        } else if (key == "seed_sample_data") {
            standalone.seedSampleData = toBool(value);
        }
    }

    void parseEnterpriseKey(const std::string& key, const std::string& value) {
        if (key == "api_base_url") {
            enterprise.apiBaseUrl = value;
        } else if (key == "api_token") {
            enterprise.apiToken = value;
        } else if (key == "request_timeout_seconds") {
            enterprise.requestTimeoutSeconds = toInt(value, 30);
        } else if (key == "connection_pool_size") {
            enterprise.connectionPoolSize = toInt(value, 5);
        }
    }

    // --- Simple parsing helpers ---

    static std::string trim(const std::string& s) {
        size_t start = s.find_first_not_of(" \t\r\n");
        if (start == std::string::npos) return "";
        size_t end = s.find_last_not_of(" \t\r\n");
        return s.substr(start, end - start + 1);
    }

    static std::string toLower(const std::string& s) {
        std::string result = s;
        std::transform(result.begin(), result.end(), result.begin(),
                       [](unsigned char c) { return std::tolower(c); });
        return result;
    }

    static bool isIndented(const std::string& line) {
        return !line.empty() && (line[0] == ' ' || line[0] == '\t');
    }

    static bool hasKeyValue(const std::string& trimmed) {
        return trimmed.find(':') != std::string::npos;
    }

    static std::pair<std::string, std::string> splitKeyValue(const std::string& trimmed) {
        auto pos = trimmed.find(':');
        if (pos == std::string::npos) return {trimmed, ""};
        std::string key = trim(trimmed.substr(0, pos));
        std::string val = trim(trimmed.substr(pos + 1));
        // Remove surrounding quotes from values
        if (val.size() >= 2 &&
            ((val.front() == '"' && val.back() == '"') ||
             (val.front() == '\'' && val.back() == '\''))) {
            val = val.substr(1, val.size() - 2);
        }
        return {key, val};
    }

    static bool toBool(const std::string& s) {
        std::string lower = toLower(s);
        return lower == "true" || lower == "yes" || lower == "1";
    }

    static int toInt(const std::string& s, int defaultVal) {
        try { return std::stoi(s); }
        catch (...) { return defaultVal; }
    }
};

} // namespace ppc
