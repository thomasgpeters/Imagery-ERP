#pragma once
// =============================================================================
// DataModels.h — In-memory data structures for the Project Planning & Costing app
//
// These mirror the PostgreSQL schema and are populated via REST calls to
// ApiLogicServer (middleware tier). For the initial build they hold sample data.
// =============================================================================

#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <map>
#include <algorithm>
#include <functional>
#include <numeric>
#include <Wt/WText.h>

namespace ppc {

// ---- XHTML WText helper ---------------------------------------------------
// Wt::WText defaults to PlainText in some builds, so HTML content must
// explicitly opt in to XHTML rendering.
inline std::unique_ptr<Wt::WText> xhtml(const std::string& html) {
    return std::make_unique<Wt::WText>(html, Wt::TextFormat::XHTML);
}

// ---- Utility Formatters ----------------------------------------------------

inline std::string formatCurrency(double v) {
    std::ostringstream s;
    s << "$" << std::fixed << std::setprecision(2) << v;
    return s.str();
}
inline std::string formatNumber(double v, int p = 1) {
    std::ostringstream s;
    s << std::fixed << std::setprecision(p) << v;
    return s.str();
}
inline std::string formatPercent(double v) {
    std::ostringstream s;
    s << std::fixed << std::setprecision(1) << v << "%";
    return s.str();
}
inline std::string currentDate() {
    std::time_t t = std::time(nullptr);
    char buf[32];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d", std::localtime(&t));
    return buf;
}
inline std::string currentTimestamp() {
    std::time_t t = std::time(nullptr);
    char buf[64];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", std::localtime(&t));
    return buf;
}

// ---- Data Structures -------------------------------------------------------

struct Company {
    int id = 0;
    std::string name;
    std::string logoUrl;
    std::string website;
    std::string phone;
    std::string email;
    double defaultMarkupPct = 30.0;
    int defaultSprintWeeks = 2;
};

struct Role {
    int id = 0;
    std::string name;
    std::string description;
    double baseRate = 0.0;
    double overheadRate = 0.0;
    bool isActive = true;
    int sortOrder = 0;
    double fullyLoadedRate() const { return baseRate + overheadRate; }
};

struct Resource {
    int id = 0;
    int roleId = 0;
    std::string firstName;
    std::string lastName;
    std::string email;
    double rateOverride = -1.0; // negative = use role default
    double availabilityPct = 100.0;
    bool isActive = true;
    std::string fullName() const { return firstName + " " + lastName; }
};

struct ComponentResource {
    int roleId = 0;
    double estimatedHours = 0.0;
    std::string notes;
};

struct Component {
    int id = 0;
    int projectId = 0;
    int phaseId = 0;
    std::string name;
    std::string description;
    std::string statementOfWork;
    std::string complexity = "Medium";
    std::string status = "Draft";
    int sortOrder = 0;
    std::vector<ComponentResource> resources;

    double totalHours() const {
        double s = 0;
        for (auto& r : resources) s += r.estimatedHours;
        return s;
    }
};

struct AgileCeremony {
    int id = 0;
    std::string name;
    std::string description;
    double durationHours = 1.0;
    int occurrencesPerSprint = 1;
    bool isActive = true;
    int sortOrder = 0;
    double totalHoursPerSprint() const {
        return durationHours * occurrencesPerSprint;
    }
};

struct Phase {
    int id = 0;
    int projectId = 0;
    std::string name;
    std::string description;
    int startWeek = 1;
    int endWeek = 1;
    int sortOrder = 0;
};

struct Sprint {
    int id = 0;
    int projectId = 0;
    int sprintNumber = 1;
    std::string name;
    std::string goal;
    int startWeek = 1;
    int endWeek = 2;
    std::string status = "Planned";
    std::vector<int> componentIds;
};

struct WeekAllocation {
    int projectId = 0;
    int roleId = 0;
    int weekNumber = 1;
    double allocatedHours = 0.0;
    double actualHours = -1.0; // negative = not yet reported
};

struct ESignature {
    int id = 0;
    int estimateId = 0;
    std::string signerName;
    std::string signerTitle;
    std::string signerEmail;
    std::string signerCompany;
    std::string signatureData; // typed name or base64 drawn
    std::string signatureType = "typed";
    std::string signedAt;
    bool isValid = true;
};

struct Estimate {
    int id = 0;
    int projectId = 0;
    std::string estimateNumber;
    std::string name;
    std::string description;
    std::string clientName;
    std::string clientEmail;
    std::string clientCompany;
    double markupPct = 30.0;
    double totalHours = 0.0;
    double totalCost = 0.0;
    double totalSell = 0.0;
    int version = 1;
    std::string status = "Draft";
    std::string validUntil;
    std::string termsAndConditions;
    std::string sharedLinkToken;
    std::string createdAt;
    std::vector<int> componentIds;
    std::vector<ESignature> signatures;
};

struct ChangeOrderItem {
    std::string action; // Add, Modify, Remove
    int componentId = 0;
    std::string description;
    int roleId = 0;
    double hoursDelta = 0.0;
    double costDelta = 0.0;
};

struct ChangeOrder {
    int id = 0;
    int projectId = 0;
    int estimateId = 0;
    std::string changeOrderNumber;
    std::string title;
    std::string description;
    std::string reason;
    std::string impactSummary;
    double additionalHours = 0.0;
    double additionalCost = 0.0;
    double additionalSell = 0.0;
    int scheduleImpactWeeks = 0;
    std::string status = "Draft";
    std::string requestedBy;
    std::string requestedDate;
    std::string approvedBy;
    std::string approvedDate;
    int version = 1;
    std::vector<ChangeOrderItem> items;
    std::vector<ESignature> signatures;
};

// ---- Central Project Data Store --------------------------------------------

class ProjectData {
public:
    Company company;
    std::vector<Role> roles;
    std::vector<Resource> resources;
    std::vector<Component> components;
    std::vector<AgileCeremony> ceremonies;
    std::vector<Phase> phases;
    std::vector<Sprint> sprints;
    std::vector<WeekAllocation> allocations;
    std::vector<Estimate> estimates;
    std::vector<ChangeOrder> changeOrders;

    std::string projectName = "New Project";
    std::string clientName;
    std::string clientEmail;
    int totalWeeks = 12;
    int sprintLengthWeeks = 2;
    double markupPct = 30.0;
    int nextId = 100;

    int genId() { return nextId++; }

    // ---- Role helpers -------------------------------------------------------

    const Role* findRole(int id) const {
        for (auto& r : roles) if (r.id == id) return &r;
        return nullptr;
    }
    double roleRate(int roleId) const {
        auto* r = findRole(roleId);
        return r ? r->fullyLoadedRate() : 0.0;
    }

    // ---- Allocation helpers (hours grid) ------------------------------------

    double getAllocatedHours(int roleId, int week) const {
        for (auto& a : allocations)
            if (a.roleId == roleId && a.weekNumber == week) return a.allocatedHours;
        return 0.0;
    }
    void setAllocatedHours(int roleId, int week, double hrs) {
        for (auto& a : allocations) {
            if (a.roleId == roleId && a.weekNumber == week) {
                a.allocatedHours = hrs;
                return;
            }
        }
        allocations.push_back({1, roleId, week, hrs, -1.0});
    }

    double getRoleTotalHours(int roleId) const {
        double s = 0;
        for (auto& a : allocations)
            if (a.roleId == roleId) s += a.allocatedHours;
        return s;
    }
    double getRoleTotalCost(int roleId) const {
        return getRoleTotalHours(roleId) * roleRate(roleId);
    }
    double getWeekTotalHours(int week) const {
        double s = 0;
        for (auto& a : allocations)
            if (a.weekNumber == week) s += a.allocatedHours;
        return s;
    }
    double getWeekTotalCost(int week) const {
        double s = 0;
        for (auto& a : allocations)
            if (a.weekNumber == week) s += a.allocatedHours * roleRate(a.roleId);
        return s;
    }
    double getTotalHours() const {
        double s = 0;
        for (auto& a : allocations) s += a.allocatedHours;
        return s;
    }
    double getTotalCost() const {
        double s = 0;
        for (auto& a : allocations) s += a.allocatedHours * roleRate(a.roleId);
        return s;
    }
    double getTotalSell() const { return getTotalCost() * (1.0 + markupPct / 100.0); }
    double getMargin()    const { return getTotalSell() - getTotalCost(); }
    double getBlendedCostRate() const {
        double h = getTotalHours();
        return h > 0 ? getTotalCost() / h : 0.0;
    }
    double getBlendedSellRate() const {
        return getBlendedCostRate() * (1.0 + markupPct / 100.0);
    }

    // ---- Phase helpers ------------------------------------------------------

    double getPhaseHours(int phaseIdx) const {
        if (phaseIdx < 0 || phaseIdx >= (int)phases.size()) return 0;
        auto& ph = phases[phaseIdx];
        double s = 0;
        for (auto& a : allocations)
            if (a.weekNumber >= ph.startWeek && a.weekNumber <= ph.endWeek)
                s += a.allocatedHours;
        return s;
    }
    double getPhaseCost(int phaseIdx) const {
        if (phaseIdx < 0 || phaseIdx >= (int)phases.size()) return 0;
        auto& ph = phases[phaseIdx];
        double s = 0;
        for (auto& a : allocations)
            if (a.weekNumber >= ph.startWeek && a.weekNumber <= ph.endWeek)
                s += a.allocatedHours * roleRate(a.roleId);
        return s;
    }
    double getPhaseSell(int phaseIdx) const {
        return getPhaseCost(phaseIdx) * (1.0 + markupPct / 100.0);
    }

    // ---- Component cost helpers ---------------------------------------------

    double componentCost(const Component& c) const {
        double s = 0;
        for (auto& cr : c.resources) s += cr.estimatedHours * roleRate(cr.roleId);
        return s;
    }
    double componentSell(const Component& c) const {
        return componentCost(c) * (1.0 + markupPct / 100.0);
    }

    // ---- Ceremony overhead per sprint ---------------------------------------

    double ceremonyCostPerSprint() const {
        double totalCeremonyHours = 0;
        for (auto& c : ceremonies)
            if (c.isActive) totalCeremonyHours += c.totalHoursPerSprint();
        // Ceremony cost: blended rate * hours * number of participants (approx)
        return totalCeremonyHours * getBlendedCostRate();
    }
    double totalCeremonyHoursPerSprint() const {
        double s = 0;
        for (auto& c : ceremonies)
            if (c.isActive) s += c.totalHoursPerSprint();
        return s;
    }

    // ---- Sprint generation --------------------------------------------------

    void generateSprints() {
        sprints.clear();
        int num = 1;
        for (int w = 1; w <= totalWeeks; w += sprintLengthWeeks) {
            Sprint s;
            s.id = genId();
            s.projectId = 1;
            s.sprintNumber = num;
            s.name = "Sprint " + std::to_string(num);
            s.startWeek = w;
            s.endWeek = std::min(w + sprintLengthWeeks - 1, totalWeeks);
            s.status = "Planned";
            sprints.push_back(s);
            num++;
        }
    }

    // ---- Estimate helpers ---------------------------------------------------

    void recalcEstimate(Estimate& est) const {
        est.totalHours = 0;
        est.totalCost = 0;
        for (int cid : est.componentIds) {
            for (auto& comp : components) {
                if (comp.id == cid) {
                    est.totalHours += comp.totalHours();
                    est.totalCost += componentCost(comp);
                    break;
                }
            }
        }
        est.totalSell = est.totalCost * (1.0 + est.markupPct / 100.0);
    }

    // ---- Load sample data matching the spreadsheet image --------------------

    void loadSampleData() {
        company.id = 1;
        company.name = "Imagery Solutions";
        company.email = "info@imagerysolutions.com";
        company.phone = "(512) 555-0100";
        company.website = "https://imagerysolutions.com";
        company.defaultMarkupPct = 30.0;
        company.defaultSprintWeeks = 2;

        projectName = "Enterprise Portal Modernization";
        clientName = "Meridian Financial Group";
        clientEmail = "r.blackwell@meridianfg.com";
        totalWeeks = 12;
        sprintLengthWeeks = 2;
        markupPct = 30.0;

        // Roles
        roles = {
            {1, "Architect",          "Solutions Architect",           60.00, 15.00, true, 1},
            {2, "Developer 1",        "Mid-level Developer",           60.00, 12.00, true, 2},
            {3, "Developer 2",        "Senior Developer",             100.00, 20.00, true, 3},
            {4, "Tester 1",           "QA Engineer",                   40.00, 10.00, true, 4},
            {5, "Project Manager",    "Project Manager",               55.00, 14.00, true, 5},
            {6, "UX Designer",        "UX/UI Designer",                65.00, 13.00, true, 6},
            {7, "DevOps Engineer",    "DevOps Engineer",               80.00, 16.00, true, 7},
            {8, "Business Analyst",   "Business Analyst",              50.00, 12.00, true, 8},
        };

        // Agile Ceremonies
        ceremonies = {
            {1, "Sprint Planning",
             "Team reviews and commits to the Sprint Backlog. Product Owner presents prioritized stories. Team estimates and plans the work for the upcoming sprint.",
             4.0, 1, true, 1},
            {2, "Daily Standup",
             "Brief daily synchronization meeting. Each team member shares: what they did yesterday, what they will do today, and any blockers.",
             0.25, 10, true, 2},
            {3, "Sprint Review / Demo",
             "Team demonstrates completed work to stakeholders. Feedback is gathered and the Product Backlog is updated accordingly.",
             2.0, 1, true, 3},
            {4, "Sprint Retrospective",
             "Team reflects on the sprint process. Identifies what went well, what could be improved, and commits to actionable improvements.",
             1.5, 1, true, 4},
            {5, "Backlog Refinement",
             "Team and Product Owner review upcoming backlog items. Stories are clarified, acceptance criteria defined, and prioritized for future sprints.",
             2.0, 1, true, 5},
        };

        // Phases
        phases = {
            {1, 1, "Discovery & Inception",   "Requirements, stakeholder interviews, current-state analysis",  1,  2, 1},
            {2, 1, "Design & Architecture",    "Architecture, UX/UI, data modeling, API specs",                 3,  4, 2},
            {3, 1, "Development — Core",       "Core modules, APIs, integrations",                              5,  8, 3},
            {4, 1, "Testing & Hardening",      "System, integration, performance, security testing",            9, 10, 4},
            {5, 1, "Deployment & Transition",  "Production deploy, migration, training, handoff",              11, 12, 5},
        };

        // Week allocations (hours per role per week)
        int hrs_arch[]  = {20,20,30,30,12,10, 8, 8, 6, 6, 6, 6};
        int hrs_dev1[]  = { 4, 4, 8, 8,36,36,40,40,16,12,14,14};
        int hrs_dev2[]  = { 8, 8,12,12,36,36,40,40,14,14,12,12};
        int hrs_test[]  = { 0, 0, 4, 4,10,10,16,16,40,40, 8, 8};
        int hrs_pm[]    = {16,16,10,10, 8, 8, 8, 8, 8, 8,16,16};
        int hrs_ux[]    = { 4, 4,30,30,16, 8, 4, 4, 0, 0, 4, 4};
        int hrs_devops[]= { 4, 8, 8, 8,12,12, 8, 8,12,12,24,24};
        int hrs_ba[]    = {20,20,16,16, 4, 4, 0, 0, 0, 0,20,20};

        int* allHrs[] = {hrs_arch, hrs_dev1, hrs_dev2, hrs_test, hrs_pm, hrs_ux, hrs_devops, hrs_ba};
        for (int r = 0; r < 8; r++) {
            for (int w = 0; w < 12; w++) {
                if (allHrs[r][w] > 0) {
                    allocations.push_back({1, roles[r].id, w + 1, (double)allHrs[r][w], -1.0});
                }
            }
        }

        // Components with SoW
        components = {
            {1, 1, 1, "Stakeholder Interviews", "Conduct structured interviews with key stakeholders.",
             "Conduct up to 8 structured interviews (60-90 min each) with executive sponsors, department heads, and end-user representatives. Deliverables include interview summaries, consolidated requirements document, and stakeholder map.",
             "Medium", "Approved", 1,
             {{1, 16, ""}, {5, 20, ""}, {8, 24, ""}}},

            {2, 1, 1, "Current-State Assessment", "Analyze existing systems, integrations, and pain points.",
             "Perform a comprehensive audit of the current portal: technology stack, integrations, performance baselines, security posture, and user analytics. Deliver a Current-State Assessment Report with gap analysis and risk register.",
             "High", "Approved", 2,
             {{1, 24, ""}, {3, 16, ""}, {7, 12, ""}, {8, 20, ""}}},

            {3, 1, 1, "Requirements Documentation", "Produce a formal requirements specification.",
             "Compile functional and non-functional requirements into a structured Requirements Specification Document (RSD). Include user stories, acceptance criteria, data dictionary, and traceability matrix. Requires two rounds of stakeholder review.",
             "Medium", "Approved", 3,
             {{8, 40, ""}, {5, 12, ""}, {1, 8, ""}}},

            {4, 1, 2, "UX Research & Wireframes", "User research, personas, journey maps, and wireframes.",
             "Conduct user research (surveys, card sorting, task analysis) with up to 15 participants. Produce 3 user personas, 2 journey maps, and low/mid-fidelity wireframes for all primary workflows. Two rounds of revision included.",
             "High", "Approved", 4,
             {{6, 60, ""}, {8, 16, ""}, {5, 8, ""}}},

            {5, 1, 2, "System Architecture Design", "Design the target-state system architecture.",
             "Design microservices architecture including service decomposition, API contracts (OpenAPI 3.0), data model (ERD), event-driven patterns, security architecture, and infrastructure topology. Deliver ADRs and Technical Design Document.",
             "Critical", "Approved", 5,
             {{1, 60, ""}, {3, 24, ""}, {7, 16, ""}}},

            {6, 1, 2, "UI Prototype", "High-fidelity interactive prototype.",
             "Create high-fidelity interactive prototype covering the top 5 user workflows. Include design system components, responsive layouts (desktop + tablet), and accessibility compliance review (WCAG 2.1 AA). One round of usability testing included.",
             "High", "Approved", 6,
             {{6, 48, ""}, {2, 8, ""}, {4, 8, ""}}},

            {7, 1, 3, "Authentication & Authorization Module", "SSO, RBAC, MFA implementation.",
             "Implement OAuth 2.0/OIDC authentication with SSO integration. Build RBAC with configurable permission sets. Implement MFA (TOTP + SMS). Includes session management, token refresh, and audit logging.",
             "Critical", "Estimated", 7,
             {{3, 60, ""}, {2, 40, ""}, {1, 12, ""}, {4, 20, ""}}},

            {8, 1, 3, "Dashboard & Analytics Module", "Real-time dashboards with configurable widgets.",
             "Build configurable dashboard framework with drag-and-drop widget layout. Implement 8 standard widget types. Data sourced from analytics API with 30-second refresh intervals.",
             "High", "Estimated", 8,
             {{3, 48, ""}, {2, 60, ""}, {6, 24, ""}, {4, 16, ""}}},

            {9, 1, 3, "Document Management Module", "Upload, versioning, search, and collaboration.",
             "Implement document management with upload, version control, full-text search, metadata tagging, folder hierarchy, sharing/permissions, and inline preview. Storage backend: S3-compatible.",
             "High", "Estimated", 9,
             {{3, 56, ""}, {2, 48, ""}, {7, 12, ""}, {4, 20, ""}}},

            {10, 1, 3, "API Gateway & Integration Layer", "Central API gateway and third-party integrations.",
             "Deploy API gateway with rate limiting, request/response transformation, API key management. Build integration adapters for 3 third-party systems. Include retry logic, circuit breakers, and dead-letter queues.",
             "Critical", "Estimated", 10,
             {{1, 20, ""}, {3, 48, ""}, {2, 32, ""}, {7, 24, ""}, {4, 16, ""}}},

            {11, 1, 3, "Notification & Messaging Service", "Email, SMS, in-app, and push notifications.",
             "Build multi-channel notification service supporting email, SMS, in-app, and push notifications. Include templates, user preferences, delivery tracking, and retry mechanisms.",
             "Medium", "Estimated", 11,
             {{2, 48, ""}, {3, 16, ""}, {7, 8, ""}, {4, 12, ""}}},

            {12, 1, 4, "Integration & System Testing", "End-to-end testing of all modules.",
             "Execute comprehensive test plan covering integration, system, regression, and cross-browser/device testing. Deliver test results report, defect log, and coverage metrics. Target: 85%+ code coverage.",
             "High", "Estimated", 12,
             {{4, 80, ""}, {2, 16, ""}, {3, 12, ""}, {5, 12, ""}}},

            {13, 1, 4, "Performance & Security Testing", "Load testing, penetration testing, security audit.",
             "Conduct performance testing (500 concurrent users), stress testing, endurance testing. Perform OWASP Top 10 scan, dependency audit, penetration testing. Deliver performance benchmarks and security findings.",
             "Critical", "Estimated", 13,
             {{4, 40, ""}, {7, 24, ""}, {3, 16, ""}, {1, 12, ""}}},

            {14, 1, 5, "Production Deployment & Migration", "Infrastructure, deployment, data migration.",
             "Provision production infrastructure (IaC via Terraform). Execute blue-green deployment with rollback. Perform data migration with validation. Deliver runbook, monitoring dashboards, and alert configuration.",
             "Critical", "Estimated", 14,
             {{7, 48, ""}, {3, 24, ""}, {1, 12, ""}, {2, 16, ""}, {4, 16, ""}}},

            {15, 1, 5, "Training & Documentation", "User training, admin training, technical documentation.",
             "Develop training materials: end-user guide, admin guide, API documentation. Conduct 3 end-user sessions, 2 admin sessions, 1 technical handoff. Deliver recorded sessions and knowledge base articles.",
             "Medium", "Estimated", 15,
             {{5, 32, ""}, {8, 40, ""}, {2, 12, ""}, {6, 8, ""}}},
        };

        // Generate sprints
        generateSprints();

        // Assign components to sprints
        if (sprints.size() >= 6) {
            sprints[0].componentIds = {1, 2, 3};
            sprints[1].componentIds = {4, 5, 6};
            sprints[2].componentIds = {7, 8, 10};
            sprints[3].componentIds = {9, 11};
            sprints[4].componentIds = {12, 13};
            sprints[5].componentIds = {14, 15};
        }

        // Sample estimate
        Estimate est;
        est.id = genId();
        est.projectId = 1;
        est.estimateNumber = "EST-2026-0001";
        est.name = "Enterprise Portal Modernization — Full Estimate";
        est.description = "Complete estimate covering all phases from Discovery through Deployment.";
        est.clientName = "Robert Blackwell";
        est.clientEmail = "r.blackwell@meridianfg.com";
        est.clientCompany = "Meridian Financial Group";
        est.markupPct = 30.0;
        est.version = 1;
        est.status = "Draft";
        est.validUntil = "2026-04-15";
        est.termsAndConditions =
            "1. This estimate is valid for 30 days from date of issue.\n"
            "2. Payment terms: Net 30 from invoice date.\n"
            "3. Work performed on time-and-materials basis with weekly status reporting.\n"
            "4. Scope changes require a formal Change Order.\n"
            "5. Client will provide timely access to stakeholders, systems, and data.\n"
            "6. All IP transfers to client upon final payment.";
        est.sharedLinkToken = "est_abc123def456";
        est.createdAt = "2026-03-01";
        for (auto& c : components) est.componentIds.push_back(c.id);
        recalcEstimate(est);
        estimates.push_back(est);

        // Sample change order
        ChangeOrder co;
        co.id = genId();
        co.projectId = 1;
        co.estimateId = est.id;
        co.changeOrderNumber = "CO-001";
        co.title = "Add Mobile Responsive Views";
        co.description = "Add fully responsive mobile views for all primary workflows.";
        co.reason = "Client feedback revealed 35% of portal usage on mobile devices.";
        co.impactSummary = "Adds ~120 hours. Extends testing by 1 week. +$9,480 cost (before markup).";
        co.additionalHours = 120;
        co.additionalCost = 9480.00;
        co.additionalSell = 12324.00;
        co.scheduleImpactWeeks = 1;
        co.status = "Pending Review";
        co.requestedBy = "Robert Blackwell";
        co.requestedDate = "2026-04-01";
        co.items = {
            {"Add", 0, "Mobile wireframes and responsive design system tokens", 6, 32, 2496.00},
            {"Add", 0, "Responsive CSS/JS for 5 primary views", 2, 40, 2880.00},
            {"Add", 0, "Mobile-specific interactions and touch optimization", 3, 24, 2880.00},
            {"Modify", 0, "Extended cross-device testing (mobile/tablet matrix)", 4, 24, 1200.00},
            {"Modify", 0, "Update docs with mobile screenshots", 8, 0, 24.00},
        };
        changeOrders.push_back(co);
    }
};

} // namespace ppc
