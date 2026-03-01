# Development Log

Chronological record of development sessions, decisions made, and work completed on the Imagery ERP — Project Planning, Costing & Estimating Engine.

---

## Session 1 — Project Foundation & Full Application Build

**Date:** 2026-03-01
**Branch:** `claude/project-planning-costing-app-Z04yA`

### Objectives

Build a complete project planning, costing, and estimating application from scratch with a three-tier architecture (Wt frontend, ApiLogicServer middleware, PostgreSQL persistence).

### Work Completed

#### 1. Data Model Design

Designed the full relational data model covering the complete project planning lifecycle:

- **Company & Users** — Multi-tenant organization with branded identity, application users with roles and admin flags
- **Roles & Resources** — Resource types (Architect, Developer, Tester, etc.) with base rate + overhead = fully loaded rate; individual people assigned to roles with optional rate overrides
- **Components** — Estimable units of work, each with a detailed Statement of Work, complexity rating, status, and resource requirements (man-hours per role)
- **Phases** — Project phases spanning week ranges (Discovery, Design, Development, Testing, Deployment)
- **Sprints** — 2-week iterations with sprint goals, status tracking, and assigned components
- **Agile Ceremonies** — Five standard ceremonies (Sprint Planning, Daily Standup, Sprint Review, Retrospective, Backlog Refinement) with configurable durations and occurrences per sprint
- **Week Allocations** — The core planning grid: hours per role per week, enabling the fully loaded cost calculation
- **Estimates** — Assembled proposals with component breakdown, markup, terms & conditions, versioning, shareable links, and status workflow (Draft > Sent > Viewed > Signed > Approved)
- **E-Signatures** — Electronic signature capture (typed, drawn, or uploaded) with signer details, IP address, and timestamp
- **Change Orders** — Scope modifications with line-item detail (Add/Modify/Remove), financial impact, schedule impact, approval workflow, and signatures
- **Email & Audit Logs** — Track sent emails and all entity changes for compliance

#### 2. PostgreSQL Schema (`database/schema.sql`)

**898 lines** — Comprehensive PostgreSQL schema including:

- **23 tables** with proper primary keys, foreign keys, NOT NULL constraints, CHECK constraints, and UNIQUE constraints
- **Indexes** on all foreign keys and commonly queried columns
- **8 computed views** for reporting:
  - `v_role_rates` — Roles with computed fully loaded rate
  - `v_component_cost` — Component-level cost summary
  - `v_weekly_cost_grid` — Weekly fully loaded cost grid
  - `v_project_role_summary` — Per-role hours and cost totals
  - `v_project_summary` — Project-level totals with blended rates
  - `v_phase_cost` — Phase-level cost and sell summaries
  - `v_sprint_ceremony_hours` — Ceremony hours breakdown per sprint
  - `v_sprint_summary` — Sprint-level summary with component and ceremony hours
- **4 stored functions**:
  - `generate_sprints()` — Auto-generate sprints from project weeks/sprint length
  - `recalculate_estimate()` — Recalculate estimate totals from included components
  - `generate_estimate_number()` — Sequential estimate numbering (EST-YYYY-NNNN)
  - `generate_co_number()` — Sequential change order numbering (CO-NNN)
- **13 triggers** — Auto-update `updated_at` timestamps on all mutable tables
- **2 extensions** — `uuid-ossp` and `pgcrypto`
- Schema namespace: `ppc` (project planning & costing)

#### 3. Seed Data (`database/seed_data.sql`)

**491 lines** — Complete sample dataset:

- 1 company (Imagery Solutions)
- 3 application users
- 8 roles with base and overhead rates (Architect $60+$15, Developer 1 $60+$12, Developer 2 $100+$20, Tester $40+$10, PM $55+$14, UX $65+$13, DevOps $80+$16, BA $50+$12)
- 10 individual resources assigned to roles
- 5 Agile ceremony definitions with durations and occurrences
- 1 sample project ("Enterprise Portal Modernization" for Meridian Financial Group)
- 5 project phases spanning 12 weeks
- 15 components with full Statement of Work text and resource requirements
- 6 sprints (2-week intervals) with component assignments and ceremony instances
- 96 week-allocation records (8 roles x 12 weeks)
- 1 sample estimate with all 15 components included
- 1 sample change order ("Add Mobile Responsive Views") with 5 line items

#### 4. Wt C++ Application (`src/`)

**2,637 lines across 18 source files** — Full interactive web application:

- **`main.cpp`** — Application entry point using `Wt::WRun`
- **`App.h/.cpp`** — Main application shell with sidebar navigation, top navbar (brand, theme toggle, user profile), work area routing, and light/dark theme toggling
- **`DataModels.h`** — In-memory data structures mirroring the PostgreSQL schema, with business logic methods (cost calculations, blended rates, phase/role summaries, sprint generation, estimate recalculation)
- **`DashboardView`** — 8 metric cards, role summary table, phase summary table, ceremony overhead table
- **`ResourceView`** — Editable roles table (inline name/rate editing), markup % control, add/delete roles, blended rate display
- **`ComponentView`** — Component list with expand/collapse for SoW detail, inline resource editing, add/remove resources, complexity badges, status badges
- **`SprintView`** — Sprint settings (total weeks, sprint length), ceremony definition table with editable durations/occurrences, sprint board with component cards and ceremony overhead
- **`CostingView`** — Editable hours-per-role-per-week grid with live total updates, read-only fully loaded cost grid, phased pricing table with markup
- **`QuoteView`** — Estimate cards with metrics, component breakdown, terms & conditions, e-signature form (typed), PDF export and email link dialogs, create new estimate form
- **`ChangeOrderView`** — Change order cards with status badges, impact metrics, line-item tables, approve/reject workflow, submit new change order form
- **`PdfExporter`** — PDF generation via libharu (when available) with HTML fallback; formatted estimate documents with company branding, component breakdown, phased pricing, terms, and signature blocks

#### 5. Stylesheet (`resources/style.css`)

**964 lines** — Professional, polished design:

- **CSS Custom Properties** for complete theme support (50+ variables)
- **Light theme** — Clean whites, subtle grays, blue accents
- **Dark theme** — Slate backgrounds, blue accent highlights
- Theme toggle via CSS class swap on the app shell
- Sidebar: dark navigation with active state indicator, project header, version footer
- Top navbar: brand + app name, theme toggle button, user avatar with name/role
- Data tables: header styling, alternating rows, hover states, sticky columns, total rows
- Metric cards: colored left border accents, hover lift effect
- Sprint cards, estimate cards, change order cards: consistent card-based layout
- Form elements: inputs, textareas, spinboxes, combo boxes with focus states
- Badges: status badges (Draft, Estimated, Approved, Active, Critical, High, Medium, Low)
- E-signature: cursive font styling for signature input and display
- Grid scroll: horizontal scrolling for the hours/cost grids with sticky role column
- Responsive: sidebar collapses at 768px, grid adapts at 1024px
- Print stylesheet: hides navigation, removes buttons
- Custom scrollbar styling

#### 6. Build System (`CMakeLists.txt`)

- CMake 3.14+ with C++17 standard
- Dual Wt detection: CMake config mode (Wt 4.6+) then fallback library search
- Optional libharu detection with `HAVE_HPDF` compile definition
- Suppresses Wt's internal `sprintf` deprecation warnings on macOS (`-Wno-deprecated-declarations`)
- Copies resources directory to build output

### Commits

| Hash | Message |
|------|---------|
| `a321be0` | Add Project Planning & Costing application (Wt + PostgreSQL + ApiLogicServer) |
| `8764a5e` | Suppress Wt sprintf deprecation warnings in CMakeLists.txt |

### Statistics

| Category | Count |
|----------|-------|
| Source files | 24 |
| Total lines of code | 5,226 |
| PostgreSQL tables | 23 |
| PostgreSQL views | 8 |
| Stored functions | 4 |
| Triggers | 13 |
| CSS variables | 50+ |
| UI views | 7 |
| Seed data roles | 8 |
| Seed data components | 15 |
| Seed data sprints | 6 |

---

## Session 2 — Documentation

**Date:** 2026-03-01
**Branch:** `claude/project-planning-costing-app-Z04yA`

### Objectives

Establish project documentation early: README with executive summary, usage guide, and build instructions; development log; and design decisions document.

### Work Completed

- Created `README.md` with executive summary, "Using Imagery ERP" section (navigation guide, workflow), detailed build/run instructions (macOS, Ubuntu, from-source), database setup, ApiLogicServer setup, project structure
- Created `docs/DEVELOPMENT_LOG.md` (this file) with detailed session history
- Created `docs/DEVELOPMENT_OPTIONS.md` with architecture rationale, library evaluations, design decisions, and techniques

---

## Session 3 — Materials & Expenses Capability + Dashboard Icon

**Date:** 2026-03-01
**Branch:** `claude/project-planning-costing-app-Z04yA`

### Objectives

1. Add a person icon to the Role column in the Dashboard Role Summary table for visual clarity.
2. Expand the project planning model to support **Material resources** alongside human resources — enabling planners to track non-labor costs such as office supplies, construction materials, equipment, travel, and software licenses.

### Work Completed

#### 1. Dashboard Role Icon

Added a person icon (&#128100;) to each role name in the Dashboard Role Summary table, rendered inline via `ppc::xhtml()`.

#### 2. Data Model Expansion (`DataModels.h`)

Added two new structures to the `ppc` namespace:

- **`Material`** — Catalogue item with `id`, `name`, `description`, `category` (Office Supplies, Construction, Equipment/Tools, Travel, Software/Licenses, Other), `unit` (unit, ton, bag, day, mile, license, lot), `unitCost`, `isActive`, `sortOrder`
- **`ComponentMaterial`** — Junction record linking a material to a component with `materialId`, `quantity`, and `notes`

Extended existing structures:
- **`Component`** — Added `std::vector<ComponentMaterial> materials` alongside the existing `resources` vector
- **`ProjectData`** — Added `std::vector<Material> materials` catalogue

New helper methods:
- `componentLaborCost(comp)` — Extracted from original `componentCost()` for labor-only calculation
- `componentMaterialCost(comp)` — Sum of `quantity × unitCost` for all component materials
- `componentCost(comp)` — Now returns `componentLaborCost() + componentMaterialCost()`
- `getTotalMaterialCost()` — Aggregate across all components
- `findMaterial(id)` — Lookup by ID

#### 3. Sample Data

Added **9 sample materials** with realistic unit costs:
- Cloud Hosting ($2,500/month), CI/CD Pipeline ($300/month), SSL Certificates ($200/unit)
- Application Monitoring ($150/month), UX Research Incentives ($75/unit)
- Team Travel ($1,200/trip), Printing & Supplies ($50/lot)
- Penetration Testing ($3,500/unit), Load Testing Tools ($500/month)

Assigned materials to **8 of 15 sample components** with appropriate quantities.

#### 4. MaterialView — New View (`src/MaterialView.h`, `src/MaterialView.cpp`)

Full CRUD management view (~200 lines) with three sections:

- **Summary Bar** — Total material costs with sell price at markup
- **Materials Catalogue Table** — Editable name and unit cost, category badges (color-coded), unit display, aggregated usage across components, total cost, delete button, totals row
- **Add New Material Form** — Name input, category combo (6 options), unit combo (9 options), unit cost spinner
- **Material Usage by Component** — Per-component breakdown tables showing material, quantity, unit cost, and line total

#### 5. View Integration

- **App.h / App.cpp** — Added `MaterialView` at view index 7, forward declaration, member pointer, refresh case, "Materials & Expenses" navigation item with &#9863; icon between Components and Cost Analysis
- **CMakeLists.txt** — Added `src/MaterialView.cpp` to the build

#### 6. Existing View Updates

- **DashboardView** — New "Material Costs" metric card; new Material Summary table with package icon (&#128230;), category, unit cost, quantity used, total cost, sell value, and totals row
- **ComponentView** — Added "Material Requirements" section to expanded component detail with material table (Material, Unit Cost, Qty with editable spinner, Cost, Remove), "Add Material" form, and fallback message
- **CostingView** — Added "Material & Expense Costs" table (Material, Category, Qty, Unit Cost, Total Cost) with totals row; new combined project total callout bar (Labor + Materials = Cost / Sell)
- **QuoteView** — Expanded estimate component breakdown table with Labor, Materials, Total Cost, and Sell columns
- **PdfExporter** — Updated HTML and PDF export with Labor and Materials columns in the component breakdown (7 columns total)

#### 7. CSS Additions

- `.role-icon`, `.material-icon` — Inline icon styling
- `.material-summary-bar` with `.msb-label`, `.msb-value`, `.msb-sell` — Callout bar
- `.material-comp-header` with `.mch-name`, `.mch-total` — Component material header
- Category badges: `.badge-material`, `.badge-travel`, `.badge-software`, `.badge-equipment`, `.badge-office`

### Commits

| Hash | Message |
|------|---------|
| `68d86d1` | Add person icon to Role Summary and full Materials & Expenses capability |

### Statistics

| Category | Count |
|----------|-------|
| Files changed | 12 |
| Lines inserted | ~667 |
| New source files | 2 (MaterialView.h, MaterialView.cpp) |
| New data structures | 2 (Material, ComponentMaterial) |
| New helper methods | 5 |
| Sample materials | 9 |
| UI views (total) | 8 (was 7) |

---

## Session 4 — Currency Formatting, Date Pickers & Friendly Dates

**Date:** 2026-03-01
**Branch:** `claude/project-planning-costing-app-Z04yA`

### Objectives

1. Implement true currency formatting with thousands-separator commas for all monetary values (e.g., `$1,234.56` instead of `$1234.56`).
2. Add date picker widgets (`WDateEdit`) wherever dates are entered.
3. Implement user-friendly date and time formatting throughout the app (e.g., "Mar 1, 2026" instead of "2026-03-01").

### Work Completed

#### 1. Currency Formatting (`DataModels.h`)

Rewrote `formatCurrency()` with manual thousands-separator grouping:
- Uses integer arithmetic (`long long cents`) for exact cent handling — avoids floating-point display errors
- Builds the grouped string right-to-left, inserting commas every 3 digits
- Handles negative values with `-$` prefix
- Result: `$12,345.67`, `$0.00`, `-$1,500.00`

Updated `formatNumber()` with comma grouping when `p=0` (whole numbers) — affects all hours displays across the app (e.g., "1,240" instead of "1240").

#### 2. Date & Time Formatting (`DataModels.h`)

Added two new formatting helpers:

- **`formatDate(isoDate)`** — Converts ISO date strings to human-friendly format
  - `"2026-03-01"` → `"Mar 1, 2026"`
  - Uses month name abbreviation array, strips leading zeros from day
  - Try/catch fallback returns raw string on parse failure

- **`formatTimestamp(isoTimestamp)`** — Converts ISO datetime to friendly format
  - `"2026-03-01 14:30:05"` → `"Mar 1, 2026 at 2:30 PM"`
  - 12-hour clock with AM/PM, strips leading zeros from hour
  - Falls back to `formatDate()` if no time component present

Added utility helpers:
- `dateOffsetDays(int days)` — ISO date string offset from today
- `currentDateISO()` / `currentTimestampISO()` — Aliases for clarity

#### 3. Date Pickers (`WDateEdit`)

- **QuoteView** — "Valid Until" field changed from `WLineEdit` to `WDateEdit` with calendar popup, format `"MMM d, yyyy"`, default 30 days from today
- **ChangeOrderView** — "Request Date" field changed to `WDateEdit` with calendar popup, format `"MMM d, yyyy"`, default today

#### 4. Date Display Updates

All date displays across the app now use the friendly formatters:
- **QuoteView** — Estimate creation date, valid-until date, signature timestamps
- **ChangeOrderView** — Request date, approval date, signature timestamps
- **PdfExporter** — HTML and PDF export dates and timestamps

#### 5. CSS for Date Pickers

- `.input-date` — Date picker input field styling (consistent with other inputs)
- `.Wt-datepicker` — Calendar popup styling with hover states, selected day highlight (blue), border and shadow

### Commits

| Hash | Message |
|------|---------|
| `8e6a54c` | Add proper currency formatting, date pickers, and friendly date display |

### Statistics

| Category | Count |
|----------|-------|
| Files changed | 5 |
| Lines inserted | ~166 |
| Formatting helpers added | 4 (formatDate, formatTimestamp, dateOffsetDays, currentDateISO) |
| Date pickers added | 2 (QuoteView, ChangeOrderView) |

---

*This log is maintained as the project evolves. Each development session adds a new dated entry with objectives, work completed, and relevant details.*
