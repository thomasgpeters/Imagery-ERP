#pragma once
// =============================================================================
// StandaloneDataProvider.h — Local data provider (in-memory / SQLite)
//
// This provider preserves the original standalone behavior of the application.
// Data is loaded into memory at startup (either from embedded sample data or
// from a local SQLite database file). All operations are local — no network
// calls, no external dependencies.
//
// Activated when app_config.yaml has: architecture_mode: standalone
// =============================================================================

#include "IDataProvider.h"
#include "AppConfig.h"
#include <iostream>

namespace ppc {

class StandaloneDataProvider : public IDataProvider {
public:
    explicit StandaloneDataProvider(const StandaloneConfig& config)
        : config_(config) {}

    bool initialize() override {
        // Future: open SQLite database at config_.databasePath
        // For now, we use the built-in sample data mechanism.
        std::cerr << "[Standalone] Initialized — database: "
                  << config_.databasePath
                  << " (seed: " << (config_.seedSampleData ? "yes" : "no") << ")"
                  << std::endl;
        initialized_ = true;
        return true;
    }

    std::string backendDescription() const override {
        return "Standalone (local SQLite: " + config_.databasePath + ")";
    }

    void loadProjectData(ProjectData& data) override {
        // Load built-in sample data — this is the existing behavior.
        // When SQLite support is added, this will read from the database
        // file instead (falling back to sample data if the DB is empty
        // or doesn't exist yet).
        if (config_.seedSampleData) {
            data.loadSampleData();
            std::cerr << "[Standalone] Loaded sample data into memory."
                      << std::endl;
        } else {
            std::cerr << "[Standalone] Sample data seeding disabled. "
                      << "Application will start with empty project data."
                      << std::endl;
        }
    }

    bool saveProjectData(const ProjectData& data) override {
        // Future: persist to SQLite at config_.databasePath
        // For now, data lives only in memory for the session lifetime.
        (void)data;
        std::cerr << "[Standalone] saveProjectData — in-memory only "
                  << "(SQLite persistence not yet implemented)." << std::endl;
        return true;
    }

    // --- Granular CRUD (future SQLite implementation) ---

    std::vector<Role> getRoles() override {
        // Will be implemented when SQLite backend is wired up
        return {};
    }

    bool saveRole(const Role& role) override {
        (void)role;
        return false;
    }

    bool deleteRole(int id) override {
        (void)id;
        return false;
    }

private:
    StandaloneConfig config_;
    bool initialized_ = false;
};

} // namespace ppc
