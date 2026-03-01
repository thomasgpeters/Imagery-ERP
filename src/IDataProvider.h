#pragma once
// =============================================================================
// IDataProvider.h — Abstract interface for data access
//
// Decouples the UI layer from the data storage mechanism. Two implementations:
//   - StandaloneDataProvider:  local SQLite / in-memory (standalone mode)
//   - EnterpriseDataProvider:  REST API calls (enterprise mode)
//
// The application creates the appropriate provider at startup based on
// architecture_mode in app_config.yaml, then passes it to views.
// =============================================================================

#include <memory>
#include <string>
#include <vector>
#include <functional>
#include "DataModels.h"

namespace ppc {

class IDataProvider {
public:
    virtual ~IDataProvider() = default;

    // --- Lifecycle ---
    // Initialize the provider (open DB, connect to API, etc.)
    virtual bool initialize() = 0;

    // Human-readable description of the active backend
    virtual std::string backendDescription() const = 0;

    // --- Full data load (fills a ProjectData struct) ---
    // This is the primary method used by the current architecture:
    // load everything into the in-memory ProjectData and let views read it.
    virtual void loadProjectData(ProjectData& data) = 0;

    // --- Persistence (save changes back to storage) ---
    // Save the full ProjectData back to the underlying store.
    // In standalone mode this writes to SQLite; in enterprise mode this
    // pushes changes via REST API.
    virtual bool saveProjectData(const ProjectData& data) = 0;

    // --- Granular CRUD (for future use by enterprise views) ---
    // These provide entity-level operations for when we move away from
    // the monolithic load/save pattern. Default implementations in the
    // base class throw "not implemented" so existing code isn't forced
    // to implement them immediately.

    virtual std::vector<Role> getRoles() {
        return {};
    }
    virtual bool saveRole(const Role& role) {
        (void)role; return false;
    }
    virtual bool deleteRole(int id) {
        (void)id; return false;
    }

    virtual std::vector<Resource> getResources() {
        return {};
    }
    virtual bool saveResource(const Resource& resource) {
        (void)resource; return false;
    }
    virtual bool deleteResource(int id) {
        (void)id; return false;
    }

    virtual std::vector<Component> getComponents() {
        return {};
    }
    virtual bool saveComponent(const Component& component) {
        (void)component; return false;
    }
    virtual bool deleteComponent(int id) {
        (void)id; return false;
    }

    virtual std::vector<Estimate> getEstimates() {
        return {};
    }
    virtual bool saveEstimate(const Estimate& estimate) {
        (void)estimate; return false;
    }

    virtual std::vector<ChangeOrder> getChangeOrders() {
        return {};
    }
    virtual bool saveChangeOrder(const ChangeOrder& co) {
        (void)co; return false;
    }
};

// Factory function — creates the right provider based on config
// Defined in DataProviderFactory.cpp
std::unique_ptr<IDataProvider> createDataProvider(
    const std::string& configPath = "app_config.yaml");

} // namespace ppc
