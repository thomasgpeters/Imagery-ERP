# User Guide — Imagery ERP

A complete guide to using the Imagery ERP Project Planning, Costing & Estimating Engine.

---

## Table of Contents

1. [Getting Started](#1-getting-started)
2. [Application Layout](#2-application-layout)
3. [Roles](#3-roles)
4. [Planner Workflow](#4-planner-workflow)
   - [4.1 Dashboard](#41-dashboard)
   - [4.2 Roles & Rates](#42-roles--rates)
   - [4.3 Components & SoW](#43-components--sow)
   - [4.4 Materials & Expenses](#44-materials--expenses)
   - [4.5 Cost Analysis](#45-cost-analysis)
   - [4.6 Quote Builder](#46-quote-builder)
5. [Executor Workflow](#5-executor-workflow)
   - [5.1 Dashboard](#51-dashboard)
   - [5.2 Sprints & Agile](#52-sprints--agile)
   - [5.3 Change Orders](#53-change-orders)
6. [Settings & Preferences](#6-settings--preferences)
7. [Keyboard & Navigation Tips](#7-keyboard--navigation-tips)

---

## 1. Getting Started

Launch the application and open `http://localhost:8080` in your browser. The app loads with sample data for the "Enterprise Portal Modernization" project, so you can explore all features immediately.

The application starts in the **Planner** role with the **Dashboard** view active.

---

## 2. Application Layout

The interface has three main areas:

```
+----------------------------------------------------+
|                    Top Bar                          |
|  Project > Client               Theme | User ▼     |
+----------+-----------------------------------------+
|          |                                         |
| Sidebar  |              Work Area                  |
|          |                                         |
| Logo     |  (Dashboard, Roles & Rates,             |
| Project  |   Components, Cost Analysis,            |
| Nav      |   Quote Builder, etc.)                  |
| Settings |                                         |
| v1.0.0   |                                         |
+----------+-----------------------------------------+
```

- **Top Bar** — Shows the project breadcrumb (left) and theme toggle + user profile (right)
- **Sidebar** — Company logo, project/client info, navigation menu, Settings button, and version
- **Work Area** — The active view; changes when you click a navigation item

---

## 3. Roles

Imagery ERP has two roles that control which navigation items are visible:

| Role | Purpose | Navigation Items |
|------|---------|------------------|
| **Planning** | Project estimation and pricing | Dashboard, Roles & Rates, Components & SoW, Materials & Expenses, Cost Analysis, Quote Builder |
| **Execution** | Sprint delivery and scope management | Dashboard, Sprints & Agile, Change Orders |

**To switch roles:** Click your user profile in the top-right corner, then select "Planning" or "Execution" from the dropdown.

The Planning role is the default. When you switch to Execution, the sidebar navigation updates and the Dashboard loads automatically.

---

## 4. Planner Workflow

The recommended workflow for planning a new project:

1. Define roles and hourly rates
2. Create components with resource requirements and statements of work
3. Set up materials and expenses
4. Allocate hours across weeks in the cost analysis grid
5. Build a quote and send it to the client

### 4.1 Dashboard

The Dashboard provides a read-only overview of the entire project.

**Metric Cards** — Eight cards at the top showing:
- **Total Hours** — Sum of all allocated hours across all roles and weeks
- **Total Cost** — Fully loaded cost (hours x fully loaded rate per role)
- **Sell Price** — Total cost with markup applied
- **Profit Margin** — Sell price minus cost
- **Blended Cost Rate** — Weighted average hourly cost across all roles
- **Blended Sell Rate** — Blended cost rate with markup
- **Material Costs** — Total non-labor expenses
- **Sprints / Components** — Counts of each

**Summary Tables:**
- **Role Summary** — Each role with base rate, overhead, fully loaded rate, total hours, total cost, and sell value
- **Phase Summary** — Five project phases with week ranges, hours, cost, and sell price
- **Material Summary** — All materials with category, unit cost, quantity used, total cost, and sell value
- **Agile Ceremony Overhead** — Five Scrum ceremonies with per-sprint time requirements

### 4.2 Roles & Rates

Manage the resource roles used for project estimation.

**Markup Percentage** — A project-wide spinbox (0–200%) that controls the sell-price multiplier applied across all views. Changing this affects sell prices everywhere.

**Roles Table** — Each row represents a resource role:
- **Name** — Editable text field (e.g., "Architect", "Developer 1")
- **Base Rate** — Hourly pay rate (editable spinbox, $0–$500)
- **Overhead** — Per-hour overhead cost (editable spinbox, $0–$200)
- **Fully Loaded** — Computed: Base + Overhead (read-only)
- **Sell Rate** — Computed: Fully Loaded x (1 + Markup%) (read-only)
- **Delete** — Removes the role (caution: also removes it from component assignments)

**Add Role** — Enter a name, base rate, and overhead rate, then click the Add button.

**Blended Rate** — The bottom row shows the weighted-average cost and sell rates across all roles.

### 4.3 Components & SoW

Break the project into estimable components, each with a detailed Statement of Work.

**Component Summary Table** — Lists all components with:
- Clickable component name (expands the detail panel)
- Phase assignment
- Complexity badge (Critical, High, Medium, Low)
- Total hours, cost, and sell price
- Status badge (Approved, Estimated, Draft)

**Expanding a Component** reveals:
- **Description** — Editable text input
- **Statement of Work** — Multi-line textarea with detailed scope description
- **Resource Requirements** — Table of assigned roles with:
  - Role name and hourly rate
  - Estimated hours (editable spinbox, 0–1,000)
  - Computed cost
  - Remove button
  - Add Role dropdown to assign additional roles
- **Material Requirements** — Table of assigned materials with:
  - Material name and unit cost
  - Quantity (editable spinbox, 0–10,000)
  - Computed line cost
  - Remove button
  - Add Material dropdown

**Add Component** — Form at the bottom to create new components with name, phase, description, and statement of work.

### 4.4 Materials & Expenses

Manage the project's non-labor costs — software licenses, travel, supplies, etc.

**Summary Bar** — Shows total material cost and sell value across all components.

**Materials Catalogue** — The master list of available materials:
- **Name** — Editable text field
- **Category** — Color-coded badge (Office Supplies, Construction, Equipment/Tools, Travel, Software/Licenses, Other)
- **Unit** — Unit of measure (unit, month, day, trip, ton, bag, lot, license, mile)
- **Unit Cost** — Editable spinbox ($0–$100,000)
- **Usage** — Read-only count of total quantity used across all components
- **Total Cost** — Read-only aggregate
- **Delete** — Removes the material and all component assignments

**Material Usage by Component** — Below the catalogue, a per-component breakdown shows which materials are assigned to each component with quantities and line totals.

**Add Material** — Form with name, category dropdown, unit dropdown, and unit cost.

### 4.5 Cost Analysis

The core planning grid where hours are allocated and costs are computed.

**Project Hours Grid** — An editable table with:
- Rows: Each role (with its fully loaded rate shown)
- Columns: Each project week (W1, W2, ... W12)
- Cells: Editable spinboxes (0–80 hours per cell, step of 4)
- Row totals, column totals, and grand total

The role column is "sticky" — it stays visible as you scroll horizontally through the weeks.

**Weekly Fully Loaded Costs Grid** — A read-only mirror of the hours grid, showing cost = hours x fully loaded rate for each cell. Same layout with row/column/grand totals.

**Phased Pricing Table** — Aggregates costs by project phase:
- Phase name and week range
- Total hours, cost, sell price, and margin per phase

**Material & Expense Costs** — Summary table of all material costs by item.

**Combined Project Total** — A callout bar showing:
- Labor cost + Material cost = Total project cost
- Total sell price at the current markup

Click **"Recalculate All"** to refresh all computed values after making changes.

### 4.6 Quote Builder

Assemble project components into client-facing estimates.

**Existing Estimates** — Each estimate is displayed as a card showing:
- Estimate number and status badge (Draft > Sent > Signed > Approved)
- Client name, company, creation date, and expiration date
- Metric pills: Total Hours, Total Cost, Markup %, Sell Price, Margin
- Component breakdown table with hours, labor, materials, total cost, and sell price per component
- Terms & Conditions block

**E-Signature** — For unsigned estimates, a form collects:
- Signer's full name, title, email, and company
- A typed signature (rendered in cursive font)
- Click "Sign Estimate" to record the signature and move status to "Signed"

After signing, an **"Approve Estimate"** button appears to finalize the estimate.

**Actions:**
- **Export PDF** — Generates a downloadable estimate document (placeholder in this build)
- **Email Link** — Creates a shareable link for the client; moves status to "Sent"

**Create New Estimate** — Form with:
- Estimate name
- Client name and email
- Markup percentage (defaults to project markup)
- Valid Until date (calendar picker, defaults to 30 days out)

All current components are automatically included in the new estimate.

---

## 5. Executor Workflow

Switch to the Execution role to manage sprints and change orders during project delivery.

### 5.1 Dashboard

Same as the Planner Dashboard (Section 4.1). Provides a read-only project overview.

### 5.2 Sprints & Agile

Configure sprint parameters and review the sprint board.

**Sprint Settings:**
- **Total Weeks** — Project duration (spinbox, 2–104 weeks)
- **Sprint Length** — Duration per sprint (spinbox, 1–4 weeks)
- **Regenerate Sprints** — Rebuilds the sprint board from the current settings

**Agile Ceremonies Table** — Five standard Scrum ceremonies:

| Ceremony | Default Duration | Default Occurrences |
|----------|-----------------|-------------------|
| Sprint Planning | 4.0 hrs | 1x per sprint |
| Daily Standup | 0.25 hrs | 10x per sprint |
| Sprint Review / Demo | 2.0 hrs | 1x per sprint |
| Sprint Retrospective | 1.5 hrs | 1x per sprint |
| Backlog Refinement | 2.0 hrs | 1x per sprint |

Both duration and occurrences are editable. The Total Hrs/Sprint column updates automatically.

**Sprint Board** — One card per sprint showing:
- Sprint name, status badge, and week range
- Assigned components with Statement of Work text
- Per-component hours and cost
- Ceremony overhead hours per person

### 5.3 Change Orders

Track scope modifications during project execution.

**Existing Change Orders** — Each displayed as a card with:
- Change order number and status badge (Draft, Pending Review, Approved, Rejected)
- Title, metadata (requestor, date, schedule impact)
- Description, reason, and impact summary
- Impact metrics: additional hours, cost, and sell price
- Line items table (if any): action type, description, role, hours delta, cost delta
- Approve / Reject buttons (for pending change orders)

**Submit New Change Order** — Form with:
- Title and Requestor name
- Request date (calendar picker)
- Description and reason for change (textareas)
- Additional hours (spinbox; cost and sell auto-calculated)
- Schedule impact in weeks (spinbox)

Change orders follow the workflow: Draft > Pending Review > Approved or Rejected.

---

## 6. Settings & Preferences

### Settings Dialog

Access via the sidebar gear button or user profile dropdown > "Settings".

- **Business Name** — Change the company name displayed in the sidebar and browser title
- **Logo** — Replace the logo image files in the `resources/images/` directory

### Theme Toggle

Click the moon/sun button in the top bar to switch between Light and Dark themes. The theme affects the work area, topbar, and all content — the sidebar remains dark in both modes.

### About Dialog

Access via user profile dropdown > "About Imagery ERP". Shows version, description, and copyright. Close via the X button, the Close button, clicking outside the dialog, or pressing Escape.

---

## 7. Keyboard & Navigation Tips

- **Escape** — Closes any open dialog (Settings, About, message boxes)
- **Tab** — Navigate between form fields in any view
- **Spinbox arrows** — Use up/down arrows or click the spinbox buttons to increment values
- **Click outside a dialog** — Closes About dialog (backdrop click)
- **Horizontal scroll** — In Cost Analysis grids, scroll horizontally while the role column stays pinned

---

*User Guide version: 1.0 — March 2026*
