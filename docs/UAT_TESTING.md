# UAT Testing Guide — Imagery ERP

User Acceptance Testing checklist for the Imagery ERP Project Planning, Costing & Estimating Engine. Organized by application role with step-by-step test scenarios.

---

## Table of Contents

1. [General / Pre-Test Setup](#1-general--pre-test-setup)
2. [Chapter 1 — Planner Role](#chapter-1--planner-role)
   - [1.1 Role Switching](#11-role-switching)
   - [1.2 Dashboard (Planner)](#12-dashboard-planner)
   - [1.3 Roles & Rates](#13-roles--rates)
   - [1.4 Components & SoW](#14-components--sow)
   - [1.5 Materials & Expenses](#15-materials--expenses)
   - [1.6 Cost Analysis](#16-cost-analysis)
   - [1.7 Quote Builder](#17-quote-builder)
3. [Chapter 2 — Executor Role](#chapter-2--executor-role)
   - [2.1 Role Switching](#21-role-switching)
   - [2.2 Dashboard (Executor)](#22-dashboard-executor)
   - [2.3 Sprints & Agile](#23-sprints--agile)
   - [2.4 Change Orders](#24-change-orders)
4. [Chapter 3 — Cross-Cutting Features](#chapter-3--cross-cutting-features)
   - [3.1 Theme Toggle (Light / Dark)](#31-theme-toggle-light--dark)
   - [3.2 Settings Dialog](#32-settings-dialog)
   - [3.3 About Dialog](#33-about-dialog)
   - [3.4 Responsive Layout](#34-responsive-layout)

---

## 1. General / Pre-Test Setup

| # | Step | Expected |
|---|------|----------|
| 0.1 | Launch application at `http://localhost:8080` | Page loads with sidebar, topbar, and Dashboard view |
| 0.2 | Verify sidebar shows Imagery Solutions logo, business name, "Project Planner" subtitle | All branding elements visible |
| 0.3 | Verify sidebar shows project name "Enterprise Portal Modernization" and client "Meridian Financial Group" | Project info displayed |
| 0.4 | Verify topbar shows project breadcrumb (project > client) on the left | Breadcrumb rendered |
| 0.5 | Verify topbar right side shows theme toggle button and user profile (Sarah Chen, Planning Manager) | Both controls visible |
| 0.6 | Verify sidebar version shows "v1.0.0" | Version displayed at sidebar bottom |

---

## Chapter 1 — Planner Role

The Planner role is the default on startup. It provides access to: Dashboard, Roles & Rates, Components & SoW, Materials & Expenses, Cost Analysis, and Quote Builder.

### 1.1 Role Switching

| # | Step | Expected | Pass/Fail |
|---|------|----------|-----------|
| 1.1.1 | Click user profile button (Sarah Chen) in topbar | Dropdown menu appears with: SWITCH ROLE header, Planning, Execution, separator, Settings, About Imagery ERP |  |
| 1.1.2 | Verify "Planning" and "Execution" are clickable menu items | Both items highlight on hover |  |
| 1.1.3 | Click "Planning" (already active) | No visible change; sidebar nav remains the same |  |
| 1.1.4 | Click user profile > "Execution" | Sidebar nav changes to: Dashboard, Sprints & Agile, Change Orders. User title changes to "Project Manager" |  |
| 1.1.5 | Click user profile > "Planning" to switch back | Sidebar nav restores full Planner menu. User title changes to "Planning Manager" |  |

### 1.2 Dashboard (Planner)

| # | Step | Expected | Pass/Fail |
|---|------|----------|-----------|
| 1.2.1 | Click "Dashboard" in sidebar | Dashboard view loads with metric cards |  |
| 1.2.2 | Verify 8 metric cards are displayed | Total Hours, Total Cost, Sell Price, Profit Margin, Blended Cost Rate, Blended Sell Rate, Material Costs, Sprints, Components |  |
| 1.2.3 | Verify metric values use proper formatting | Currency: `$123,456.78` with commas. Hours: `1,240` with commas. Percentages: `30.0%` |  |
| 1.2.4 | Hover over a metric card | Card lifts slightly (shadow change + translateY) |  |
| 1.2.5 | Verify Role Summary table displays all 8 roles | Architect, Developer 1, Developer 2, Tester 1, Project Manager, UX Designer, DevOps Engineer, Business Analyst |  |
| 1.2.6 | Verify Role Summary has person icon next to each role name | Icon (&#128100;) renders inline |  |
| 1.2.7 | Verify Role Summary columns: Role, Base Rate, Overhead, Fully Loaded, Total Hours, Total Cost, Sell Rate, Sell Value | All columns present with data |  |
| 1.2.8 | Verify Role Summary has a totals row | Bold row at bottom with aggregated values |  |
| 1.2.9 | Verify Phase Summary table | 5 phases (Discovery, Design, Development, Testing, Deployment) with Weeks, Hours, Cost, Sell columns |  |
| 1.2.10 | Verify Material Summary table | 9 materials listed with Category, Unit Cost, Qty Used, Total Cost, Sell Value |  |
| 1.2.11 | Verify Agile Ceremony Overhead table | 5 ceremonies with Duration, Occurrences/Sprint, Total Hrs/Sprint |  |

### 1.3 Roles & Rates

| # | Step | Expected | Pass/Fail |
|---|------|----------|-----------|
| 1.3.1 | Click "Roles & Rates" in sidebar | View loads with markup control and roles table |  |
| 1.3.2 | Verify Markup % spinbox shows 30.0 | Default markup value displayed |  |
| 1.3.3 | Change Markup % to 35.0 | Value accepts; sell rates in the table should update on next refresh |  |
| 1.3.4 | Verify roles table shows all 8 roles | Each row: Name (editable), Base Rate (editable), Overhead (editable), Fully Loaded (computed), Sell Rate (computed), Delete button |  |
| 1.3.5 | Edit a role name (e.g., change "Architect" to "Solutions Architect") | Name updates in the input field |  |
| 1.3.6 | Edit a base rate (e.g., change Architect from $60 to $65) | Spinbox accepts new value |  |
| 1.3.7 | Edit an overhead rate | Spinbox accepts new value |  |
| 1.3.8 | Verify Fully Loaded rate = Base + Overhead | Computed column updates correctly |  |
| 1.3.9 | Verify Sell Rate = Fully Loaded × (1 + Markup%) | Computed column reflects markup |  |
| 1.3.10 | Click Delete on a role | Role row is removed from the table |  |
| 1.3.11 | Add a new role: Name="Data Engineer", Base=70, Overhead=15 | New role appears in the table with correct computed rates |  |
| 1.3.12 | Attempt to add a role with empty name | Role should not be created (validation) |  |
| 1.3.13 | Verify blended rate row at bottom | Shows weighted average cost and sell rates |  |

### 1.4 Components & SoW

| # | Step | Expected | Pass/Fail |
|---|------|----------|-----------|
| 1.4.1 | Click "Components & SoW" in sidebar | View loads with Component Estimates Summary table |  |
| 1.4.2 | Verify 15 components listed | Each row: #, Component (clickable link), Phase, Complexity badge, Hours, Cost, Sell, Status badge |  |
| 1.4.3 | Verify complexity badges are color-coded | Critical=red, High=orange, Medium=yellow, Low=green |  |
| 1.4.4 | Verify status badges display correctly | Approved=green, Estimated=yellow, Draft=gray |  |
| 1.4.5 | Verify totals row at bottom | Aggregated hours, cost, sell across all components |  |
| 1.4.6 | Click a component name (e.g., "Stakeholder Interviews") | Component detail panel expands below the table |  |
| 1.4.7 | Verify expanded detail shows: Description, Statement of Work, Resource Requirements table, Material Requirements (if applicable) | All sections rendered |  |
| 1.4.8 | Verify SoW text is multi-line and readable | Statement of Work displays in a textarea |  |
| 1.4.9 | Edit resource hours (e.g., change Architect hours from 16 to 20) | Spinbox accepts value; cost recalculates |  |
| 1.4.10 | Click Remove on a resource row | Resource is removed from the component |  |
| 1.4.11 | Add a role to a component: select role from dropdown, enter hours, click Add | New resource row appears with correct cost |  |
| 1.4.12 | For a component with materials: verify Material Requirements table | Material, Unit Cost, Qty (editable spinbox), Cost, Remove button |  |
| 1.4.13 | Edit material quantity | Spinbox accepts value; line cost updates |  |
| 1.4.14 | Remove a material from a component | Material row removed |  |
| 1.4.15 | Add a material to a component | Select from dropdown, enter qty, click Add |  |
| 1.4.16 | Click the same component name again | Detail panel collapses |  |
| 1.4.17 | Delete a component using the Delete button | Component removed from list; totals update |  |
| 1.4.18 | Add a new component: Name, Phase (dropdown), Description, SoW | New component appears in the table with "Draft" status |  |
| 1.4.19 | Attempt to add component with empty name | Component should not be created |  |

### 1.5 Materials & Expenses

| # | Step | Expected | Pass/Fail |
|---|------|----------|-----------|
| 1.5.1 | Click "Materials & Expenses" in sidebar | View loads with summary bar and materials catalogue |  |
| 1.5.2 | Verify summary bar shows total material cost and sell value | Formatted currency values |  |
| 1.5.3 | Verify Materials Catalogue table lists 9 materials | Each row: Material (editable), Category badge, Unit, Unit Cost (editable), Usage count, Total Cost, Delete button |  |
| 1.5.4 | Verify category badges are color-coded | Software/Licenses=blue, Travel=yellow, Office Supplies=green, Other=purple |  |
| 1.5.5 | Edit a material name | Text input accepts new value |  |
| 1.5.6 | Edit a unit cost (e.g., change Cloud Hosting from $2,400 to $2,600) | Spinbox accepts; total cost recalculates on refresh |  |
| 1.5.7 | Verify Usage column shows aggregate quantity across components | Number reflects total usage |  |
| 1.5.8 | Delete a material | Material removed from catalogue and all component assignments |  |
| 1.5.9 | Add a new material: Name, Category (dropdown), Unit (dropdown), Unit Cost | New material appears in the table |  |
| 1.5.10 | Attempt to add material with empty name | Material should not be created |  |
| 1.5.11 | Scroll to Material Usage by Component section | Per-component breakdown tables displayed |  |
| 1.5.12 | Verify each component with materials has a header with its total cost | Component name + total material cost |  |
| 1.5.13 | Verify line items within each component breakdown | Material, Qty, Unit Cost, Line Total |  |

### 1.6 Cost Analysis

| # | Step | Expected | Pass/Fail |
|---|------|----------|-----------|
| 1.6.1 | Click "Cost Analysis" in sidebar | View loads with hours grid, cost grid, phased pricing |  |
| 1.6.2 | Verify Project Hours Grid displays | Roles as rows (with fully loaded rate), Weeks as columns (W1–W12), Total column |  |
| 1.6.3 | Verify role column is sticky (stays visible on horizontal scroll) | Scroll right; role names remain pinned |  |
| 1.6.4 | Edit hours in a cell (e.g., Architect Week 1 from 20 to 24) | Spinbox accepts value (0–80, step 4) |  |
| 1.6.5 | Verify row total updates | Sum of hours across all weeks for the role |  |
| 1.6.6 | Verify column total updates | Sum of hours across all roles for the week |  |
| 1.6.7 | Verify grand total (bottom-right corner) | Sum of all hours in the grid |  |
| 1.6.8 | Scroll to Weekly Fully Loaded Costs Grid | Same layout as hours grid but showing cost = hours × fully loaded rate |  |
| 1.6.9 | Verify cost grid is read-only | No editable inputs; values are display-only |  |
| 1.6.10 | Verify cost values match hours × fully loaded rate | Spot-check: Architect W1 hours × $75/hr = expected cost |  |
| 1.6.11 | Verify Phased Pricing table | 5 phases: Phase name, Weeks, Hours, Cost, Sell Price, Margin |  |
| 1.6.12 | Verify Sell Price = Cost × (1 + Markup%) | Spot-check one phase |  |
| 1.6.13 | Verify Material & Expense Costs table (if materials assigned) | Material, Category, Qty, Unit Cost, Total Cost with totals row |  |
| 1.6.14 | Verify Combined Project Total callout | Labor cost + Material cost = Total Cost; Total Sell shown |  |
| 1.6.15 | Click "Recalculate All" button | All computed values refresh |  |

### 1.7 Quote Builder

| # | Step | Expected | Pass/Fail |
|---|------|----------|-----------|
| 1.7.1 | Click "Quote Builder" in sidebar | View loads with existing estimate card and "Create New Estimate" form |  |
| 1.7.2 | Verify sample estimate card: EST-2026-0001 | Estimate number, "Draft" status badge, name, client info, creation/expiry dates |  |
| 1.7.3 | Verify estimate metrics | Total Hours, Total Cost, Markup (30.0%), Sell Price, Margin |  |
| 1.7.4 | Verify Included Components table | 15 components: Component, Hours, Labor, Materials, Total Cost, Sell |  |
| 1.7.5 | Verify Terms & Conditions section | 6 pre-loaded terms displayed in monospace block |  |
| 1.7.6 | Verify E-Signature section | Form with: Full Name, Title, Email, Company, signature input (cursive font), Sign button |  |
| 1.7.7 | Fill in signer details and type a signature | Fields accept input; signature input renders in cursive |  |
| 1.7.8 | Click "Sign Estimate" | Status changes to "Signed"; signature block appears with name, title, timestamp |  |
| 1.7.9 | Verify "Approve Estimate" button appears after signing | Green button visible in actions area |  |
| 1.7.10 | Click "Approve Estimate" | Status changes to "Approved" badge |  |
| 1.7.11 | Click "Export PDF" button | Modal dialog appears explaining PDF generation (placeholder) |  |
| 1.7.12 | Click OK to dismiss PDF dialog | Dialog closes |  |
| 1.7.13 | Click "Email Link" button | Modal dialog shows client email and shareable link; status moves to "Sent" |  |
| 1.7.14 | Scroll to "Create New Estimate" form | Form with: Name, Client Name, Client Email, Markup %, Valid Until (date picker), Create button |  |
| 1.7.15 | Verify Valid Until defaults to 30 days from today | Date picker shows a date ~30 days out |  |
| 1.7.16 | Click the date picker calendar icon | Calendar popup appears with styled dates |  |
| 1.7.17 | Create a new estimate: fill in name and client | New estimate card appears above the form with all components included |  |
| 1.7.18 | Verify new estimate number is sequential (EST-2026-1001) | Auto-generated estimate number |  |
| 1.7.19 | Attempt to create estimate with empty name | Estimate should not be created |  |

---

## Chapter 2 — Executor Role

Switch to Execution role via the user profile dropdown > "Execution". The Executor role provides access to: Dashboard, Sprints & Agile, and Change Orders.

### 2.1 Role Switching

| # | Step | Expected | Pass/Fail |
|---|------|----------|-----------|
| 2.1.1 | Click user profile button in topbar | Dropdown appears |  |
| 2.1.2 | Click "Execution" | Sidebar nav changes to: Dashboard, Sprints & Agile, Change Orders |  |
| 2.1.3 | Verify user title changes to "Project Manager" | Displayed under "Sarah Chen" |  |
| 2.1.4 | Verify role icon changes from pencil (Planning) to play (Execution) | Icon color changes from blue to green |  |
| 2.1.5 | Verify Dashboard loads automatically after role switch | Dashboard is the default view |  |

### 2.2 Dashboard (Executor)

| # | Step | Expected | Pass/Fail |
|---|------|----------|-----------|
| 2.2.1 | Verify Dashboard displays the same data as in Planner role | Same metric cards, Role Summary, Phase Summary, Material Summary, Ceremony Overhead tables |  |
| 2.2.2 | Verify Dashboard is read-only | No editable fields; display only |  |

### 2.3 Sprints & Agile

| # | Step | Expected | Pass/Fail |
|---|------|----------|-----------|
| 2.3.1 | Click "Sprints & Agile" in sidebar | View loads with sprint settings, ceremony table, sprint board |  |
| 2.3.2 | Verify Total Weeks spinbox shows 12 | Default project duration |  |
| 2.3.3 | Verify Sprint Length spinbox shows 2 (weeks) | Default sprint interval |  |
| 2.3.4 | Change Total Weeks to 16 | Spinbox accepts (range: 2–104) |  |
| 2.3.5 | Click "Regenerate Sprints" | Sprint board rebuilds with 8 sprints instead of 6 |  |
| 2.3.6 | Reset Total Weeks to 12 and regenerate | Sprint board returns to 6 sprints |  |
| 2.3.7 | Verify Agile Ceremonies table | 5 ceremonies: Sprint Planning (4hr), Daily Standup (0.25hr×10), Sprint Review (2hr), Retrospective (1.5hr), Backlog Refinement (2hr) |  |
| 2.3.8 | Edit ceremony duration (e.g., change Sprint Planning from 4.0 to 3.0 hours) | Spinbox accepts (range: 0.25–8.0, step 0.25) |  |
| 2.3.9 | Edit ceremony occurrences per sprint | Spinbox accepts (range: 1–20) |  |
| 2.3.10 | Verify Total Hrs/Sprint column updates | Duration × Occurrences |  |
| 2.3.11 | Verify Project Ceremony Overhead callout | Total hours × number of sprints |  |
| 2.3.12 | Verify Sprint Board cards (6 sprints) | Each card: Sprint name, Status badge, Week range, Component table |  |
| 2.3.13 | Verify Sprint 1 contains Components 1, 2, 3 | Stakeholder Interviews, Current-State Assessment, Requirements Documentation |  |
| 2.3.14 | Verify each sprint card shows component SoW text | SoW text displayed below component name |  |
| 2.3.15 | Verify each sprint card shows hours, cost per component | Computed values for each assigned component |  |
| 2.3.16 | Verify sprint ceremony overhead summary at bottom of each card | Per-person ceremony hours for the sprint |  |

### 2.4 Change Orders

| # | Step | Expected | Pass/Fail |
|---|------|----------|-----------|
| 2.4.1 | Click "Change Orders" in sidebar | View loads with existing change order card and submission form |  |
| 2.4.2 | Verify sample change order CO-001 | "Add Mobile Responsive Views", status "Pending Review" |  |
| 2.4.3 | Verify change order metadata | Requested By: Robert Blackwell, Request Date: Apr 1, 2026, Schedule Impact: 1 week |  |
| 2.4.4 | Verify description, reason, impact summary text | All three text blocks displayed |  |
| 2.4.5 | Verify Impact Metrics | Additional Hours: +120, Additional Cost: +$9,480.00, Additional Sell: +$12,324.00 |  |
| 2.4.6 | Verify Change Items table | 5 line items with Action badge (Add/Modify), Description, Role, Hours Delta, Cost Delta |  |
| 2.4.7 | Click "Approve" button on CO-001 | Status changes to "Approved"; approved-by and approved-date recorded |  |
| 2.4.8 | Verify approved change order shows approval info | Approver name and approval date displayed |  |
| 2.4.9 | Scroll to "Submit New Change Order" form | Form with: Title, Requested By, Request Date (date picker), Description, Reason, Additional Hours, Schedule Impact |  |
| 2.4.10 | Verify Request Date defaults to today with calendar picker | Date picker shows current date |  |
| 2.4.11 | Submit a new change order: Title="Add API Rate Limiting", Hours=40, Impact=0 weeks | New CO card appears with auto-generated number (CO-101), status "Pending Review" |  |
| 2.4.12 | Verify additional cost auto-calculated | Hours × blended cost rate |  |
| 2.4.13 | Verify additional sell auto-calculated | Additional cost × (1 + markup%) |  |
| 2.4.14 | Click "Reject" on the new change order | Status changes to "Rejected" badge |  |
| 2.4.15 | Attempt to submit change order with empty title | Change order should not be created |  |

---

## Chapter 3 — Cross-Cutting Features

These features are available regardless of the active role.

### 3.1 Theme Toggle (Light / Dark)

| # | Step | Expected | Pass/Fail |
|---|------|----------|-----------|
| 3.1.1 | Verify app starts in Light theme | White backgrounds, dark text, moon icon with "Dark" label on toggle |  |
| 3.1.2 | Click the "Dark" toggle button in topbar | Entire app switches to dark theme: slate backgrounds, light text |  |
| 3.1.3 | Verify toggle button changes to sun icon with "Light" label | Button text updates |  |
| 3.1.4 | Verify sidebar remains dark in both themes | Sidebar uses its own dark palette always |  |
| 3.1.5 | Verify data tables are readable in dark mode | Alternating row colors, text contrast |  |
| 3.1.6 | Verify metric cards render correctly in dark mode | Card backgrounds, text, and accent borders adapt |  |
| 3.1.7 | Verify form inputs are usable in dark mode | Input backgrounds, text color, focus rings |  |
| 3.1.8 | Click "Light" to toggle back | App returns to light theme |  |

### 3.2 Settings Dialog

| # | Step | Expected | Pass/Fail |
|---|------|----------|-----------|
| 3.2.1 | Click sidebar Settings button (gear icon at bottom) | Settings dialog opens as a modal |  |
| 3.2.2 | Alternatively: user profile dropdown > "Settings" | Same Settings dialog opens |  |
| 3.2.3 | Verify gear icon (⚙) appears next to "Settings" in the dialog titlebar | Gear icon rendered before title text |  |
| 3.2.4 | Verify dialog has white background (not transparent) | Solid white, no see-through |  |
| 3.2.5 | Verify "Company" section title displayed | Uppercase, gray, with letter spacing |  |
| 3.2.6 | Verify "Business Name" field shows "Imagery Solutions" | Pre-populated with current company name |  |
| 3.2.7 | Verify logo hint text below the field | Instructions about replacing logo files |  |
| 3.2.8 | Verify **Save button is visible** (blue background, white text) | Button must be clearly visible, NOT invisible |  |
| 3.2.9 | Verify Cancel button is visible (gray background) | Button clearly visible |  |
| 3.2.10 | Hover over Save button | Button darkens slightly |  |
| 3.2.11 | Change business name to "Imagery Creative" and click Save | Dialog closes; sidebar business name updates to "Imagery Creative"; browser title updates |  |
| 3.2.12 | Reopen Settings and verify name persisted | Field shows "Imagery Creative" |  |
| 3.2.13 | Click Cancel | Dialog closes without saving changes |  |
| 3.2.14 | Press Escape key | Dialog closes (rejectWhenEscapePressed) |  |
| 3.2.15 | Verify dialog is movable | Click and drag the titlebar to reposition |  |

### 3.3 About Dialog

| # | Step | Expected | Pass/Fail |
|---|------|----------|-----------|
| 3.3.1 | Click user profile dropdown > "About Imagery ERP" | About dialog opens as a modal |  |
| 3.3.2 | Verify dialog has white background with thin border | Solid white, 0.25px border |  |
| 3.3.3 | Verify company logo displayed and centered | Imagery Solutions logo, max 160px wide |  |
| 3.3.4 | Verify application description text | Paragraph describing Imagery ERP capabilities |  |
| 3.3.5 | Verify "Version 1.0.0" displayed | Version number centered below description |  |
| 3.3.6 | Verify copyright notice | "© 2026 Imagery Solutions. All rights reserved." |  |
| 3.3.7 | Verify close X button in titlebar | Float right, pointer cursor |  |
| 3.3.8 | Hover over X button | Background changes to light gray, text darkens |  |
| 3.3.9 | Click and hold X button | Press effect: slightly smaller (scale 0.92), darker background |  |
| 3.3.10 | Click X button | Dialog closes |  |
| 3.3.11 | Reopen About dialog | Dialog opens again |  |
| 3.3.12 | Click Close button in footer | Dialog closes |  |
| 3.3.13 | Reopen and click the dark backdrop area outside the dialog | Dialog closes (click-outside-to-close) |  |
| 3.3.14 | Press Escape key while dialog is open | Dialog closes |  |

### 3.4 Responsive Layout

| # | Step | Expected | Pass/Fail |
|---|------|----------|-----------|
| 3.4.1 | Resize browser window to < 1024px width | Sidebar narrows to 210px; view panel padding reduces |  |
| 3.4.2 | Verify metric cards reflow to 2 columns | Grid adapts from 5-column to 2-column layout |  |
| 3.4.3 | Resize to < 768px width | Sidebar collapses to 56px (icons only); nav labels, project info, brand area hide |  |
| 3.4.4 | Verify sidebar nav buttons center their icons | Icons centered, no text labels |  |
| 3.4.5 | Verify Cost Analysis hours grid scrolls horizontally | Horizontal scrollbar appears; role column stays sticky |  |
| 3.4.6 | Return to full-width browser | Layout restores to full sidebar with labels |  |

---

## Test Summary

| Chapter | Area | Total Tests |
|---------|------|-------------|
| 1 | Planner Role | 61 |
| 2 | Executor Role | 31 |
| 3 | Cross-Cutting Features | 28 |
| | **Total** | **120** |

### Sign-Off

| Role | Tester | Date | Result |
|------|--------|------|--------|
| Planner | | | |
| Executor | | | |
| Cross-Cutting | | | |

---

*Document version: 1.0 — March 2026*
