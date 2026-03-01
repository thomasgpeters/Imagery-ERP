# Development Options — Architecture, Design Decisions & Technology Choices

This document captures the architectural decisions, library evaluations, design patterns, and implementation techniques used to build the Imagery ERP Project Planning, Costing & Estimating Engine.

---

## Table of Contents

1. [Architecture Overview](#1-architecture-overview)
2. [Tier 1 — Presentation Layer (Wt)](#2-tier-1--presentation-layer-wt)
3. [Tier 2 — Middleware Layer (ApiLogicServer)](#3-tier-2--middleware-layer-apilogicserver)
4. [Tier 3 — Persistence Layer (PostgreSQL)](#4-tier-3--persistence-layer-postgresql)
5. [Build System](#5-build-system)
6. [Theming & UI Design](#6-theming--ui-design)
7. [Data Model Design Decisions](#7-data-model-design-decisions)
8. [Business Logic Placement](#8-business-logic-placement)
9. [Formatting & Display Utilities](#9-formatting--display-utilities)
10. [Security Considerations](#10-security-considerations)
11. [Future Considerations](#11-future-considerations)
12. [Dialog Architecture & CSS Scoping](#12-dialog-architecture--css-scoping)

---

## 1. Architecture Overview

### Decision: Three-Tier Separation

**Choice:** Strict three-tier architecture with independent presentation, middleware, and persistence layers.

**Rationale:**
- **Independent scaling** — The Wt frontend, ApiLogicServer API, and PostgreSQL database can be deployed and scaled independently
- **Technology flexibility** — Each tier can be replaced without affecting the others (e.g., swap the Wt frontend for a React SPA, or replace ApiLogicServer with a custom API)
- **Team specialization** — Frontend, API, and database development can proceed in parallel
- **API-first evolution** — The PostgreSQL schema is the source of truth; ApiLogicServer introspects it automatically, and additional clients (mobile, CLI, integrations) can consume the same API

**Alternatives Considered:**
| Option | Pros | Cons | Decision |
|--------|------|------|----------|
| Monolithic (Wt + embedded SQLite) | Simple deployment, no external dependencies | No API layer, hard to scale, no multi-client support | Rejected |
| Wt + direct PostgreSQL (libpqxx) | Fewer moving parts | No auto-generated API, manual CRUD, no admin UI | Rejected |
| Full Python (Django/Flask + React) | Large ecosystem, easy hiring | Doesn't leverage C++ performance, two languages | Rejected |
| **Wt + ApiLogicServer + PostgreSQL** | C++ presentation, auto-generated API, mature database | More components to deploy | **Selected** |

### Communication Flow

```
Browser <--HTTP/WebSocket--> Wt (C++ HTTP Server, port 8080)
                                |
                                |--- (current) In-memory DataModels.h
                                |--- (planned) REST calls to ApiLogicServer
                                |
                            ApiLogicServer (Python, port 5656)
                                |
                                |--- SQLAlchemy ORM
                                |
                            PostgreSQL (port 5432)
                                |
                                |--- Schema: ppc.*
```

---

## 2. Tier 1 — Presentation Layer (Wt)

### Decision: Wt (Witty) C++ Web Toolkit

**Choice:** Wt 4.x as the server-side web framework.

**What Wt Is:**
Wt is a C++ library for developing web applications. It uses a widget-based API (similar to Qt for desktop) where UI components are defined in C++ and Wt handles the HTML/CSS/JavaScript generation, AJAX communication, and session management automatically. The developer writes pure C++ — no HTML templates, no JavaScript, no frontend build tools.

**Why Wt:**
- **C++ performance** — Sub-millisecond widget rendering, efficient memory usage, no garbage collection pauses
- **Server-side rendering** — All UI logic runs on the server; the browser receives pre-rendered HTML and incremental DOM updates via WebSocket/AJAX
- **Widget abstraction** — `WTable`, `WLineEdit`, `WDoubleSpinBox`, `WComboBox`, `WPushButton`, etc. map directly to interactive HTML elements with built-in event handling
- **Built-in HTTP server** — No need for Apache/Nginx; Wt includes its own HTTP server (`wthttp`) with WebSocket support
- **Session management** — Each browser session gets its own `WApplication` instance with isolated state
- **No JavaScript required** — Wt generates all client-side JavaScript automatically, including AJAX callbacks, form validation, and DOM updates

**Alternatives Evaluated:**
| Framework | Language | Pros | Cons | Decision |
|-----------|----------|------|------|----------|
| **Wt** | C++ | Native performance, widget API, built-in HTTP | Smaller ecosystem than web-native frameworks | **Selected** |
| Qt WebEngine | C++ | Familiar to Qt devs | Embeds Chromium (heavy), desktop-focused | Rejected |
| CppCMS | C++ | Lightweight | Template-based (not widget-based), less mature | Rejected |
| Drogon | C++ | High performance async | REST-focused, no widget abstraction | Rejected for UI tier |
| Flask/Django | Python | Huge ecosystem | Not C++, slower, template-based | Rejected |

### Widget Architecture

The application uses a **single-page application** pattern within Wt:

```
WApplication (App)
├── app-shell (WContainerWidget — theme class host)
│   ├── app-topbar (WContainerWidget)
│   │   ├── topbar-brand (company name + app name)
│   │   └── topbar-right (theme toggle + user profile)
│   └── app-body (WContainerWidget — flex row)
│       ├── app-sidebar (WContainerWidget)
│       │   ├── sidebar-header (project + client)
│       │   ├── sidebar-nav (8 nav buttons)
│       │   └── sidebar-footer (version)
│       └── app-workarea (WContainerWidget)
│           ├── DashboardView (hidden/shown)
│           ├── ResourceView
│           ├── ComponentView
│           ├── SprintView
│           ├── CostingView
│           ├── QuoteView
│           ├── ChangeOrderView
│           └── MaterialView
```

**Navigation:** All 8 views are instantiated at startup and hidden. `showView(index)` hides all and shows the selected one, avoiding re-creation overhead. Each view has a `refresh()` method called when it becomes visible, which clears and rebuilds its content from the current data state.

### Key Wt Features Used

| Feature | Usage |
|---------|-------|
| `WContainerWidget` | Layout containers (sidebar, topbar, cards, form rows) |
| `WTable` | Data grids (roles, components, costs, sprints) |
| `WDoubleSpinBox` / `WSpinBox` | Numeric inputs for rates, hours, weeks |
| `WLineEdit` | Text inputs (names, emails) |
| `WTextArea` | Multi-line text (SoW, descriptions, terms) |
| `WComboBox` | Dropdown selectors (roles, phases, categories) |
| `WDateEdit` | Calendar date pickers (estimate valid-until, change order request date) |
| `WPushButton` | Actions (add, delete, approve, sign) |
| `WText` with XHTML | Rich content (badges, metrics, formatted labels) |
| `WMessageBox` | Modal dialogs (PDF export, email confirmation) |
| Signal/slot (`clicked()`, `valueChanged()`, `changed()`) | Event handling with C++ lambdas |
| `setStyleClass()` / `addStyleClass()` | Dynamic CSS class management (theme toggling, active nav) |
| `useStyleSheet()` | External CSS loading |
| `setTextFormat(TextFormat::XHTML)` | HTML content in buttons and text widgets |

---

## 3. Tier 2 — Middleware Layer (ApiLogicServer)

### Decision: ApiLogicServer

**Choice:** ApiLogicServer for auto-generated REST API with declarative business logic.

**What ApiLogicServer Is:**
ApiLogicServer is a Python-based tool that introspects a database schema and automatically generates:
1. A **REST API** (JSON:API compliant) for all tables with full CRUD, filtering, sorting, pagination, and relationship navigation
2. An **admin UI** (React-based) for data management
3. **Business logic hooks** — Declarative rules (constraints, derivations, formulas) that execute automatically on data changes

**Why ApiLogicServer:**
- **Zero-code API generation** — Point it at the PostgreSQL schema and get a complete REST API immediately
- **Schema-driven** — The database schema is the single source of truth; API evolves as the schema evolves
- **Declarative business logic** — Rules like "estimate.total_sell = estimate.total_cost * (1 + markup_pct / 100)" are declared once and enforced on all API writes
- **Admin UI included** — Free data management interface for internal users
- **Swagger/OpenAPI** — Auto-generated API documentation

**How It Connects:**
```
als create --project-name=imagery_api --db-url=postgresql://localhost/imagery_erp
```

ApiLogicServer reads the `ppc` schema, discovers all 23 tables and their relationships via foreign keys, and generates:
- `/api/role` — CRUD for roles
- `/api/component` — CRUD for components with nested `component_resource` access
- `/api/estimate` — CRUD for estimates with related components and signatures
- etc.

**Business Logic Examples (future):**
```python
# In logic/declare_logic.py
Rule.sum(derive=Estimate.total_hours, as_sum_of=ComponentResource.estimated_hours, where=...)
Rule.formula(derive=Estimate.total_sell, as_expression=lambda row: row.total_cost * (1 + row.markup_pct / 100))
Rule.constraint(validate=Role, as_condition=lambda row: row.base_rate >= 0, error_msg="Base rate cannot be negative")
```

---

## 4. Tier 3 — Persistence Layer (PostgreSQL)

### Decision: PostgreSQL

**Choice:** PostgreSQL 12+ as the relational database.

**Why PostgreSQL:**
- **Robust relational model** — The project planning domain is inherently relational (roles have rates, components have resources, sprints have components, estimates have components, etc.)
- **Computed views** — 10 SQL views (`v_project_summary`, `v_component_cost`, `v_material_summary`, `v_phase_cost`, etc.) push computation to the database where it's most efficient
- **Stored functions** — 5 functions (`generate_sprints()`, `recalculate_estimate()`, etc.) encapsulate complex operations close to the data
- **JSONB support** — The `audit_log.old_values` / `new_values` columns use JSONB for flexible change tracking
- **Extensions** — `uuid-ossp` for UUID generation, `pgcrypto` for secure hashing
- **ApiLogicServer compatibility** — ApiLogicServer's SQLAlchemy integration works best with PostgreSQL

**Alternatives Considered:**
| Database | Pros | Cons | Decision |
|----------|------|------|----------|
| **PostgreSQL** | Views, functions, JSONB, robust, ApiLogicServer support | Requires server process | **Selected** |
| SQLite | Zero-config, embedded | No views, no stored functions, no concurrent writes | Rejected |
| MySQL/MariaDB | Widely available | Weaker view support, less feature-rich | Rejected |
| MongoDB | Flexible schema | Poor fit for relational data, no joins | Rejected |

### Schema Design Decisions

**Schema Namespace (`ppc`):**
All tables live in the `ppc` schema (not `public`) to keep the namespace clean and avoid conflicts with other applications sharing the same database.

**Naming Conventions:**
- Tables: `snake_case` singular (`role`, `component`, `sprint`)
- Junction tables: `parent_child` (`sprint_component`, `component_resource`, `component_material`, `estimate_component`)
- Views: `v_` prefix (`v_project_summary`, `v_role_rates`, `v_material_summary`)
- Functions: descriptive verbs (`generate_sprints`, `recalculate_estimate`)

**Fully Loaded Rate Pattern:**
Rather than storing a `fully_loaded_rate` column (which would create a data consistency risk), the rate is computed:
```sql
-- In the v_role_rates view:
r.base_rate + r.overhead_rate AS fully_loaded_rate
```
This ensures the value is always consistent with its inputs.

**Week Allocation Grid:**
The `week_allocation` table stores hours per role per week as individual rows rather than a pivot table with week columns. This design:
- Supports any number of project weeks without schema changes
- Allows easy aggregation (`SUM`, `GROUP BY`)
- Enables the computed views (`v_weekly_cost_grid`, `v_project_role_summary`)

**Estimate Versioning:**
Estimates have a `version` integer and a status workflow (`Draft > Pending Review > Sent > Viewed > Signed > Approved > Rejected > Expired`). Each version is a separate row, enabling full history tracking.

**Audit Trail:**
The `audit_log` table uses `entity_type` (VARCHAR) + `entity_id` (INTEGER) polymorphic pattern rather than separate audit tables per entity, keeping the schema simpler and the audit query interface uniform.

---

## 5. Build System

### Decision: CMake

**Choice:** CMake 3.14+ as the build system.

**Why CMake:**
- Standard C++ build system — Wt's own build uses CMake
- Cross-platform — Works on macOS (Xcode/Clang), Linux (GCC), Windows (MSVC)
- Find-package integration — `find_package(wt)`, `find_library()`, `find_path()` for dependency detection
- Compile definitions — `add_definitions(-DHAVE_HPDF)` for optional feature flags

**Wt Detection Strategy:**
```cmake
# Try config mode first (Wt 4.6+ installs CMake config files)
find_package(wt CONFIG QUIET)
if(wt_FOUND)
    set(WT_LIBS Wt::Wt Wt::HTTP)
else()
    # Fallback: manual library search
    find_library(WT_LIB NAMES wt ...)
    find_library(WT_HTTP_LIB NAMES wthttp ...)
endif()
```
This dual approach handles both modern Wt installations (with `WtConfig.cmake`) and older/manual installs.

**Optional Dependencies:**
libharu is detected at build time. If found, the compile definition `HAVE_HPDF` is set and `PdfExporter.cpp` uses the libharu API. If not found, it falls back to generating HTML documents.

**macOS-Specific:**
The `-Wno-deprecated-declarations` flag suppresses warnings from Wt's `WLogger.h` which uses `std::sprintf` — deprecated by Apple's SDK in favor of `snprintf`. This is a Wt internal issue, not in our code.

---

## 6. Theming & UI Design

### Decision: CSS Custom Properties for Light/Dark Mode

**Choice:** Full theme support via CSS custom properties (variables) with class-based toggling.

**How It Works:**
```css
.theme-light { --bg-primary: #f8fafc; --text-primary: #0f172a; ... }
.theme-dark  { --bg-primary: #0f172a; --text-primary: #e2e8f0; ... }
```

The app shell container gets a class of either `theme-light` or `theme-dark`. All CSS rules reference variables (`var(--bg-primary)`, `var(--text-primary)`, etc.), so toggling the class switches the entire theme instantly.

**In C++:**
```cpp
void App::toggleTheme() {
    darkMode_ = !darkMode_;
    appShell_->setStyleClass(darkMode_ ? "app-shell theme-dark" : "app-shell theme-light");
}
```

**Why Not a CSS Framework (Bootstrap, Tailwind):**
- Wt generates its own HTML structure — CSS framework assumptions about markup don't apply
- Custom properties give complete control over every visual element
- No JavaScript framework conflicts (Wt manages all JS)
- Smaller payload — only the CSS we need (964 lines vs. 10,000+ for Bootstrap)

### Design Language

| Element | Light Theme | Dark Theme |
|---------|-------------|------------|
| Background | `#f8fafc` (near-white) | `#0f172a` (deep slate) |
| Cards | `#ffffff` with subtle border | `#1e293b` with slate border |
| Sidebar | `#0a1628` (always dark) | `#020617` (deeper dark) |
| Accent | `#2563eb` (blue) | `#3b82f6` (brighter blue) |
| Text | `#0f172a` (near-black) | `#e2e8f0` (light gray) |

**Design Principles:**
- Metric cards with colored left-border accents (blue, green, red, orange, purple, teal) — visual hierarchy without garish backgrounds
- Data tables with header styling, alternating rows, and sticky role columns for horizontal scroll
- Sprint cards, estimate cards, and change order cards — consistent card-based layout with status badges
- Form elements with focus rings (blue glow) for accessibility
- Signature input with cursive font family for visual authenticity

---

## 7. Data Model Design Decisions

### Roles vs. Resources

**Decision:** Separate `role` (type with rates) from `resource` (individual person).

A role like "Developer 1" has a base rate of $60/hr. Multiple people can hold that role. Individual resources can have a `rate_override` if their rate differs from the role default. This separation allows:
- Estimating with roles before specific people are assigned
- Rate changes on the role propagating to all unoverridden resources
- Capacity planning by role type

### Component-Resource Pattern

**Decision:** `component_resource` junction table linking components to roles with estimated hours.

Each component specifies how many hours it needs from each role:
```
Component: "Authentication Module"
├── Developer 2 (Senior): 60 hrs
├── Developer 1 (Mid): 40 hrs
├── Architect: 12 hrs
└── Tester 1: 20 hrs
```

This enables per-component costing (`SUM(hours * role.fully_loaded_rate)`) and roll-up to project totals.

### Component-Material Pattern

**Decision:** `ComponentMaterial` junction linking components to materials with quantity.

Each component can optionally include non-labor costs:
```
Component: "Cloud Infrastructure Setup"
├── Resources (labor):
│   ├── DevOps Engineer: 80 hrs
│   └── Architect: 16 hrs
└── Materials (non-labor):
    ├── Cloud Hosting: 3 months × $2,500 = $7,500
    ├── CI/CD Pipeline: 3 months × $300 = $900
    └── SSL Certificates: 2 units × $200 = $400
```

Materials are defined in a project-level catalogue (`ProjectData::materials`) and referenced by components. The cost hierarchy:
- `componentLaborCost()` — `SUM(hours × role.fullyLoadedRate)`
- `componentMaterialCost()` — `SUM(quantity × material.unitCost)`
- `componentCost()` — `componentLaborCost() + componentMaterialCost()`

Material categories: Office Supplies, Construction, Equipment/Tools, Travel, Software/Licenses, Other. Each material has a unit type (unit, ton, bag, day, mile, license, lot, month, trip) and unit cost.

### Phase-Based vs. Sprint-Based Week Ranges

**Decision:** Phases define week ranges; sprints are generated from project settings.

Phases (`start_week`, `end_week`) are manual groupings for pricing and reporting. Sprints are auto-generated in 2-week intervals. They overlap but serve different purposes:
- **Phases** → client-facing milestones for phased billing
- **Sprints** → internal development cadence for Agile ceremonies and velocity tracking

### Estimate Assembly Pattern

**Decision:** Estimates reference components via `estimate_component`, not by copying data.

When a component's hours change, all estimates that include it automatically reflect the updated cost (via `recalculate_estimate()`). This avoids data staleness. For locked historical estimates, the `version` field and `status = 'Approved'` prevent further recalculation.

---

## 8. Business Logic Placement

### Current: In-Memory (DataModels.h)

For the initial build, all business logic lives in `DataModels.h`:
- Cost calculations (`componentCost()`, `componentLaborCost()`, `componentMaterialCost()`, `getRoleTotalCost()`, `getTotalMaterialCost()`, etc.)
- Blended rate computations (`getBlendedCostRate()`, `getBlendedSellRate()`)
- Phase cost aggregation (`getPhaseHours()`, `getPhaseCost()`)
- Sprint generation (`generateSprints()`)
- Estimate recalculation (`recalcEstimate()`)
- Formatting utilities (`formatCurrency()`, `formatDate()`, `formatTimestamp()`, `formatNumber()`, `formatPercent()`)

### Target: Three-Level Logic

| Layer | Logic Type | Example |
|-------|-----------|---------|
| **PostgreSQL views/functions** | Aggregation, computed values | `v_project_summary`, `recalculate_estimate()` |
| **ApiLogicServer rules** | Constraints, derivations, formulas | "total_sell = total_cost * (1 + markup / 100)" |
| **Wt C++ (DataModels.h)** | UI-specific calculations, transient state | Live total updates in the hours grid |

The goal is to push validation and derived values down to the database and API layers, keeping the presentation layer focused on display and user interaction.

---

## 9. Formatting & Display Utilities

### Decision: Custom Formatting Without Locale Dependencies

**Choice:** Hand-rolled formatting functions in `DataModels.h` rather than `std::locale` or ICU.

**Why:**
- `std::locale` behavior varies across platforms (macOS vs. Linux, different C library implementations)
- ICU would add a heavy dependency for simple formatting needs
- The formatting is straightforward enough that custom code is reliable and portable

### Currency Formatting (`formatCurrency`)

Uses integer arithmetic for exact cent handling:
```cpp
long long cents = static_cast<long long>(v * 100.0 + 0.5);
```
Builds the thousands-grouped string right-to-left, inserting commas every 3 digits. Output: `$12,345.67`, `$0.00`, `-$1,500.00`.

### Number Formatting (`formatNumber`)

When `precision=0`, applies the same comma grouping (e.g., `1,240` hours). When `precision>0`, uses `std::fixed` with `std::setprecision`.

### Date Formatting (`formatDate`, `formatTimestamp`)

Parses ISO date strings (`"2026-03-01"`) via simple `std::stoi` + substring extraction. Maps month numbers to abbreviations (`"Jan"` through `"Dec"`). Try/catch fallback returns the raw string on parse failure.

- `formatDate("2026-03-01")` → `"Mar 1, 2026"`
- `formatTimestamp("2026-03-01 14:30:05")` → `"Mar 1, 2026 at 2:30 PM"` (12-hour clock)

### Date Picker Integration (`WDateEdit`)

Wt's `WDateEdit` provides a calendar popup with the format string `"MMM d, yyyy"`. Used for:
- Estimate "Valid Until" date (default: 30 days from today via `dateOffsetDays(30)`)
- Change Order "Request Date" (default: today)

CSS styling for `.Wt-datepicker` provides hover states, selected-day highlight, and consistent border/shadow treatment.

---

## 10. Security Considerations

### Current State

- **Authentication:** Not yet implemented (single-user prototype). The `app_user` table with `password_hash` is ready for bcrypt/argon2 integration.
- **E-Signatures:** Captures signer name, email, IP address, user agent, and timestamp. Signature data is stored as typed text (future: canvas-drawn base64 images).
- **Shared Estimate Links:** Use a `shared_link_token` (random string) rather than sequential IDs to prevent URL guessing.
- **XHTML Text:** All user-facing text uses `Wt::TextFormat::XHTML` which escapes content by default, preventing XSS.

### Planned

- Session-based authentication with Wt's `WApplication` session isolation
- Role-based access control (admin vs. project manager vs. viewer)
- HTTPS enforcement for production
- CSRF protection (Wt provides this by default via session tokens)
- Rate limiting on the ApiLogicServer API
- Audit log for all sensitive operations

---

## 12. Dialog Architecture & CSS Scoping

### Issue: CSS Variables Unavailable in Dialogs

Wt's `addChild()` method appends dialog widgets to the application root, which is **outside** the `.theme-light` / `.theme-dark` class container (`appShell_`). Since all 50+ CSS custom properties are defined under `.theme-light { ... }` and `.theme-dark { ... }`, they do not cascade into dialogs.

**DOM Structure:**
```
<body>
  <div class="wt-root">
    <div class="app-shell theme-light">     ← CSS variables defined here
      <div class="app-sidebar">...</div>
      <div class="app-main">...</div>
    </div>
  </div>
  <div class="Wt-dialogcover">...</div>
  <div class="settings-dialog Wt-dialog">   ← Outside theme scope!
    ...buttons using var(--accent-blue)...   ← Resolves to nothing
  </div>
</body>
```

**Symptoms:**
- `.btn-primary` buttons invisible (transparent background + white text on white)
- `.btn-secondary` buttons invisible (transparent background + unresolved text color)
- Both buttons become visible on hover because hover styles used hardcoded `#1d4ed8`

**Resolution:**
All CSS rules for `.settings-dialog`, `.about-dialog`, `.settings-footer`, and `.about-footer` were converted from CSS variable references to hardcoded hex color values:

```css
/* Before (broken): */
.settings-footer .btn-primary {
    background: var(--accent-blue);  /* → nothing */
    color: white;                     /* → invisible */
}

/* After (fixed): */
.settings-footer .btn-primary {
    background: #2563eb;
    color: #ffffff;
}
```

**Alternative Approaches Considered:**

| Approach | Pros | Cons | Decision |
|----------|------|------|----------|
| Move CSS variables to `:root` | All elements inherit them | Loses per-theme scoping; dark mode requires JS class on `<html>` | Rejected |
| Clone theme class onto dialog | Variables resolve naturally | Fragile; must sync on every theme toggle | Rejected |
| **Hardcode dialog colors** | Simple, reliable, no sync issues | Must update in two places if palette changes | **Selected** |

This is a Wt-specific architectural constraint. If the application migrates to a React or vanilla HTML frontend in the future, CSS variables can be moved to `:root` and this workaround removed.

---

## 11. Future Considerations

### Phase 1 Status: Complete

The following capabilities are fully implemented and ready for UAT:

| Feature | Status |
|---------|--------|
| Role & rate management | Complete |
| Component estimating with SoW | Complete |
| Material & expense tracking | Complete |
| Weekly hours/cost allocation grid | Complete |
| Sprint planning with Agile ceremonies | Complete |
| Quote builder with e-signature | Complete |
| Change order workflow | Complete |
| Light/dark theme | Complete |
| Role-based navigation (Planner/Executor) | Complete |
| Settings dialog (business name) | Complete |
| About dialog | Complete |
| Currency/date formatting | Complete |
| PDF export (placeholder) | Stubbed |
| Email delivery (placeholder) | Stubbed |

### Near-Term

| Feature | Approach |
|---------|----------|
| **Database integration** | Replace in-memory `DataModels` with REST calls to ApiLogicServer via `Wt::Http::Client` |
| **Authentication** | Wt `WApplication` login with bcrypt password hashing against `app_user` table |
| **Canvas signature** | Use Wt's `WPaintedWidget` for drawn signatures, store as base64 PNG |
| **PDF generation** | Full libharu integration with company logo, formatted tables, page breaks |
| **Email delivery** | SMTP or SendGrid integration for estimate delivery |

### Medium-Term

| Feature | Approach |
|---------|----------|
| **Multi-project support** | Project selector in sidebar, full CRUD on projects |
| **Resource assignment** | Assign individual resources to week allocations (currently role-level only) |
| **Gantt chart** | Wt `WPaintedWidget` for visual timeline of phases and sprints |
| **Time tracking** | Actual hours entry against week allocations for burn-down tracking |
| **Notifications** | WebSocket push via Wt's `WServer::post()` for real-time updates |

### Long-Term

| Feature | Approach |
|---------|----------|
| **Multi-tenant SaaS** | Company-scoped data with tenant isolation at the API layer |
| **Mobile companion** | React Native app consuming the ApiLogicServer REST API |
| **Reporting engine** | Scheduled PDF report generation and email delivery |
| **Integration hub** | Webhooks for CRM, invoicing, and project management tool sync |

---

## Libraries & Technologies Summary

| Technology | Version | Role | License |
|------------|---------|------|---------|
| **C++17** | — | Application language | — |
| **CMake** | 3.14+ | Build system | BSD |
| **Wt (Witty)** | 4.x | Web framework (widgets, HTTP server, sessions) | GPL v2 / commercial |
| **PostgreSQL** | 12+ | Relational database | PostgreSQL License (MIT-like) |
| **ApiLogicServer** | Latest | Auto-generated REST API + business logic | MIT |
| **libharu (HPDF)** | 2.x (optional) | PDF generation | zlib/libpng |
| **SQLAlchemy** | (via ApiLogicServer) | ORM for API layer | MIT |
| **CSS Custom Properties** | — | Light/dark theming | — |

---

*This document is updated as new architectural decisions are made or technologies are adopted.*
