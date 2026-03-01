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

*This log is maintained as the project evolves. Each development session adds a new dated entry with objectives, work completed, and relevant details.*
