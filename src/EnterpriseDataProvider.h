#pragma once
// =============================================================================
// EnterpriseDataProvider.h — REST API data provider (enterprise mode)
//
// Fetches and persists data via the ApiLogicServer REST API backed by
// PostgreSQL. This is the provider used for multi-user, multi-tenant
// production deployments.
//
// Activated when app_config.yaml has: architecture_mode: enterprise
//
// NOTE: The HTTP client calls are stubbed out in this initial implementation.
// They will be wired to Wt::Http::Client or libcurl when the ApiLogicServer
// backend is deployed.
// =============================================================================

#include "IDataProvider.h"
#include "AppConfig.h"
#include <iostream>

namespace ppc {

class EnterpriseDataProvider : public IDataProvider {
public:
    explicit EnterpriseDataProvider(const EnterpriseConfig& config)
        : config_(config) {}

    bool initialize() override {
        std::cerr << "[Enterprise] Connecting to API at " << config_.apiBaseUrl
                  << " (timeout: " << config_.requestTimeoutSeconds << "s, "
                  << "pool: " << config_.connectionPoolSize << ")"
                  << std::endl;

        // Future: validate connectivity by calling GET /api/health or similar
        // For now, assume the endpoint is reachable.
        connected_ = true;

        std::cerr << "[Enterprise] Initialized — ready for API calls."
                  << std::endl;
        return true;
    }

    std::string backendDescription() const override {
        return "Enterprise API (" + config_.apiBaseUrl + ")";
    }

    void loadProjectData(ProjectData& data) override {
        if (!connected_) {
            std::cerr << "[Enterprise] ERROR: Not connected. "
                      << "Falling back to sample data." << std::endl;
            data.loadSampleData();
            return;
        }

        // =====================================================================
        // TODO: Replace with actual REST API calls when backend is available.
        //
        // The call sequence will be:
        //   GET /api/company/1              → data.company
        //   GET /api/role?company_id=1      → data.roles
        //   GET /api/resource?company_id=1  → data.resources
        //   GET /api/material?company_id=1  → data.materials
        //   GET /api/project/1              → data.projectName, clientName, etc.
        //   GET /api/phase?project_id=1     → data.phases
        //   GET /api/component?project_id=1 → data.components (with nested resources)
        //   GET /api/agile_ceremony         → data.ceremonies
        //   GET /api/sprint?project_id=1    → data.sprints
        //   GET /api/week_allocation?project_id=1 → data.allocations
        //   GET /api/estimate?project_id=1  → data.estimates
        //   GET /api/change_order?project_id=1 → data.changeOrders
        //
        // Each response is JSON, parsed into the corresponding data struct.
        // =====================================================================

        std::cerr << "[Enterprise] REST API calls not yet implemented. "
                  << "Loading sample data as placeholder." << std::endl;
        data.loadSampleData();
    }

    bool saveProjectData(const ProjectData& data) override {
        if (!connected_) {
            std::cerr << "[Enterprise] ERROR: Not connected — cannot save."
                      << std::endl;
            return false;
        }

        // Future: PATCH/PUT calls to the REST API
        (void)data;
        std::cerr << "[Enterprise] saveProjectData — API persistence "
                  << "not yet implemented." << std::endl;
        return true;
    }

    // --- Granular CRUD (will call individual REST endpoints) ---

    std::vector<Role> getRoles() override {
        // GET /api/role
        return {};
    }

    bool saveRole(const Role& role) override {
        // POST/PATCH /api/role/{id}
        (void)role;
        return false;
    }

    bool deleteRole(int id) override {
        // DELETE /api/role/{id}
        (void)id;
        return false;
    }

    std::vector<Component> getComponents() override {
        // GET /api/component?project_id=1
        return {};
    }

    bool saveComponent(const Component& component) override {
        // POST/PATCH /api/component/{id}
        (void)component;
        return false;
    }

    bool deleteComponent(int id) override {
        // DELETE /api/component/{id}
        (void)id;
        return false;
    }

    std::vector<Estimate> getEstimates() override {
        // GET /api/estimate?project_id=1
        return {};
    }

    bool saveEstimate(const Estimate& estimate) override {
        // POST/PATCH /api/estimate/{id}
        (void)estimate;
        return false;
    }

    std::vector<ChangeOrder> getChangeOrders() override {
        // GET /api/change_order?project_id=1
        return {};
    }

    bool saveChangeOrder(const ChangeOrder& co) override {
        // POST/PATCH /api/change_order/{id}
        (void)co;
        return false;
    }

private:
    EnterpriseConfig config_;
    bool connected_ = false;
};

} // namespace ppc
