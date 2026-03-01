# Imagery ERP — Project Planning, Costing & Estimating Engine

## Executive Summary

Imagery ERP is a **project planning, costing, and estimating application** built for professional services firms, consultancies, and software development organizations. It provides a complete workflow for scoping projects, allocating resources, calculating fully loaded costs, assembling client-facing estimates with electronic signatures, and managing change orders throughout the project lifecycle.

The application follows a **three-tier architecture**:

| Tier | Role | Technology |
|------|------|------------|
| **Presentation** | Interactive web UI with branded navigation, light/dark themes | C++ / Wt (Witty) Web Toolkit |
| **Middleware** | Auto-generated REST API with declarative business logic | ApiLogicServer (Python) |
| **Persistence** | Relational database with computed views and stored functions | PostgreSQL |

### Key Capabilities

- **Role & Rate Management** — Define resource roles (Architect, Developer, Tester, etc.) with base pay, overhead, and computed fully loaded hourly rates
- **Component Estimating** — Break projects into estimable components, each with its own Statement of Work, resource requirements, and man-hour estimates
- **Agile Sprint Planning** — Generate 2-week sprints with all five standard Agile ceremonies (Sprint Planning, Daily Standup, Sprint Review, Retrospective, Backlog Refinement) and configurable durations
- **Cost Analysis** — Interactive hours-per-role-per-week allocation grid with computed fully loaded cost grid, phased pricing, and blended rate calculations
- **Quote Builder** — Assemble estimates from components, apply markup, generate shareable links, and capture electronic signatures
- **Change Orders** — Submit, review, approve, or reject scope modifications with line-item detail, financial impact tracking, and approval signatures
- **PDF Export** — Generate formatted estimate documents via libharu (with HTML fallback)
- **Light & Dark Themes** — Toggle between polished light and dark modes from the top navigation bar

---

## Using Imagery ERP

### Navigation

The application uses a **sidebar + top navigation bar** layout:

- **Top Bar** — Company branding on the left; theme toggle (light/dark) and user profile on the right
- **Sidebar** — Project name, client name, and role-based navigation (6 items for Planning, 3 for Execution):

| Section | Description |
|---------|-------------|
| **Dashboard** | High-level project metrics: total hours, cost, sell price, margin, blended rates, phase summary, ceremony overhead |
| **Roles & Rates** | Add/edit/delete roles with base rate and overhead; set the project-wide markup percentage |
| **Components & SoW** | Create components with Statement of Work text; assign resource roles and estimated hours; view per-component cost |
| **Sprints & Agile** | Configure sprint length and project duration; edit Agile ceremony definitions; view the sprint board with assigned components |
| **Cost Analysis** | Edit the weekly hours allocation grid (roles x weeks); view the fully loaded cost grid (read-only); see phased pricing with markup |
| **Quote Builder** | Create estimates from project components; send estimate links to clients; capture typed e-signatures; export to PDF |
| **Change Orders** | Submit change requests with description, reason, and line items; approve or reject; track financial and schedule impact |

### Workflow

1. **Define Roles & Rates** — Set up the resource roles your organization uses, with base pay and overhead rates. Set the markup percentage.
2. **Create Components** — Break the project into estimable units of work. Write a Statement of Work for each. Assign roles and hours.
3. **Allocate Hours** — Use the Cost Analysis grid to distribute hours per role across weeks. The system calculates fully loaded costs automatically.
4. **Plan Sprints** — Generate sprints and assign components. Review ceremony overhead per sprint.
5. **Build a Quote** — Create an estimate that assembles all components, apply markup, and send to the client for signature.
6. **Manage Changes** — When scope changes, submit a Change Order with line-item detail, review, and approve.

---

## Building and Running

### Prerequisites

| Dependency | Required | Notes |
|------------|----------|-------|
| **CMake** 3.14+ | Yes | Build system |
| **C++17 compiler** | Yes | GCC 7+, Clang 5+, MSVC 2017+ |
| **Wt (Witty)** 4.x | Yes | C++ web toolkit — provides HTTP server, widgets, sessions |
| **PostgreSQL** 12+ | Yes | Database (Tier 3) |
| **ApiLogicServer** | Optional | Python middleware for auto-generated REST API (Tier 2) |
| **libharu (HPDF)** | Optional | PDF generation; falls back to HTML export if not present |

### Installing Dependencies

**macOS (Homebrew):**
```bash
brew install cmake wt postgresql libharu
```

**Ubuntu/Debian:**
```bash
sudo apt-get update
sudo apt-get install cmake g++ libwt-dev libwthttp-dev \
    postgresql postgresql-client libhpdf-dev
```

**From Source (Wt):**
```bash
git clone https://github.com/emweb/wt.git
cd wt && mkdir build && cd build
cmake .. -DCMAKE_INSTALL_PREFIX=/usr/local
make -j$(nproc) && sudo make install
```

### Compiling

```bash
cd Imagery-ERP
mkdir build && cd build
cmake ..
make -j$(nproc)
```

CMake will automatically detect Wt (via config mode or library search) and optionally detect libharu. You will see status messages confirming what was found:

```
-- Found Wt: /usr/local/lib/libwt.dylib
-- Found WtHTTP: /usr/local/lib/libwthttp.dylib
-- libharu not found - PDF export will use HTML fallback
```

### Setting Up the Database

```bash
# Create the database
createdb imagery_erp

# Run the schema (creates the 'ppc' schema with all tables, views, functions)
psql -d imagery_erp -f database/schema.sql

# Load sample data
psql -d imagery_erp -f database/seed_data.sql
```

To verify the seed data:
```bash
psql -d imagery_erp -c "SET search_path TO ppc; SELECT * FROM v_project_summary;"
```

### Running the Application

```bash
cd build
./project_planner --docroot ../resources --http-address 0.0.0.0 --http-port 8080
```

Then open **http://localhost:8080** in your browser.

#### Command-Line Options

| Option | Description |
|--------|-------------|
| `--docroot <path>` | Path to static resources (CSS, images). Point to the `resources/` directory. |
| `--http-address <addr>` | Bind address. Use `0.0.0.0` for all interfaces or `127.0.0.1` for localhost only. |
| `--http-port <port>` | HTTP port (default: 8080). |
| `--https-port <port>` | HTTPS port (requires SSL certificate configuration). |
| `--accesslog <file>` | Write HTTP access log to file. |

### Setting Up ApiLogicServer (Middleware)

ApiLogicServer introspects the PostgreSQL schema and auto-generates a REST API with an admin UI:

```bash
pip install ApiLogicServer

# Create the project from the database
als create --project-name=imagery_api --db-url=postgresql://localhost/imagery_erp

cd imagery_api
als run
```

The API server runs on port 5656 by default, providing:
- REST endpoints for all 23 tables (`/api/company`, `/api/role`, `/api/component`, etc.)
- Admin UI at `http://localhost:5656/admin`
- Swagger documentation at `http://localhost:5656/api`

---

## Project Structure

```
Imagery-ERP/
├── CMakeLists.txt              # Build configuration
├── README.md                   # This file
├── database/
│   ├── schema.sql              # PostgreSQL schema (23 tables, 8 views, 4 functions)
│   └── seed_data.sql           # Sample data (roles, components, sprints, estimate)
├── docs/
│   ├── DEVELOPMENT_LOG.md      # Development history and session notes
│   ├── DEVELOPMENT_OPTIONS.md  # Architecture decisions, libraries, techniques
│   ├── UAT_TESTING.md          # 120-test UAT guide (Planner + Executor roles)
│   └── USER_GUIDE.md           # Complete end-user guide
├── resources/
│   └── style.css               # Professional stylesheet (light + dark themes)
└── src/
    ├── main.cpp                # Application entry point
    ├── App.h / App.cpp         # Main application shell (sidebar, topbar, routing)
    ├── DataModels.h            # In-memory data structures and business logic
    ├── DashboardView.*         # Project dashboard with metrics and summaries
    ├── ResourceView.*          # Roles & rates management
    ├── ComponentView.*         # Component estimating with SoW
    ├── SprintView.*            # Sprint planning and Agile ceremonies
    ├── CostingView.*           # Hours grid, cost grid, phased pricing
    ├── QuoteView.*             # Estimate assembly, e-signature, export
    ├── ChangeOrderView.*       # Change order workflow
    └── PdfExporter.*           # PDF/HTML document generation
```

## Documentation

| Document | Description |
|----------|-------------|
| [README.md](README.md) | Executive summary, usage guide, build and run instructions, project structure |
| [docs/USER_GUIDE.md](docs/USER_GUIDE.md) | Complete end-user guide — application layout, Planner workflow (Dashboard, Roles & Rates, Components & SoW, Materials, Cost Analysis, Quote Builder), Executor workflow (Sprints & Agile, Change Orders), Settings, theme toggle, keyboard tips |
| [docs/UAT_TESTING.md](docs/UAT_TESTING.md) | User Acceptance Testing guide — 120 test cases across 3 chapters: Planner Role (61 tests), Executor Role (31 tests), Cross-Cutting Features (28 tests) with step-by-step scenarios and pass/fail tracking |
| [docs/DEVELOPMENT_LOG.md](docs/DEVELOPMENT_LOG.md) | Chronological development history — 7 sessions covering data model design, schema creation, seed data, Wt application views, materials capability, formatting, UI polish, and documentation |
| [docs/DEVELOPMENT_OPTIONS.md](docs/DEVELOPMENT_OPTIONS.md) | Architecture and design decisions — three-tier rationale, Wt evaluation, ApiLogicServer choice, PostgreSQL design, CSS theming, data model patterns, dialog CSS scoping issue, Phase 1 completion status, and future roadmap |
| [database/schema.sql](database/schema.sql) | PostgreSQL schema — 25 tables, 10 computed views, 5 stored functions, 15 auto-update triggers, 2 extensions, all within the `ppc` schema namespace |
| [database/seed_data.sql](database/seed_data.sql) | Sample dataset — 1 company, 3 users, 8 roles with rates, 10 resources, 9 materials, 5 Agile ceremonies, 1 project with 5 phases, 15 components with SoW text, 6 sprints, 96 week-allocation records, 1 estimate, 1 change order with 5 line items |
| [CMakeLists.txt](CMakeLists.txt) | Build configuration — CMake 3.14+, C++17, dual Wt detection (config mode + fallback), optional libharu for PDF, macOS warning suppression |
| [resources/style.css](resources/style.css) | Stylesheet — 1,454 lines, 50+ CSS custom properties, full light and dark theme support, sidebar/topbar/workarea layout, data tables, metric cards, sprint/estimate/change-order cards, e-signature styling, responsive breakpoints, print stylesheet |

## License

Copyright 2026 Imagery Solutions. All rights reserved.
