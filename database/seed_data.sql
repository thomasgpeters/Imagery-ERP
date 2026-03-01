-- =============================================================================
-- Project Planning, Costing & Estimating Engine
-- Seed Data
--
-- Run after schema.sql:
--   psql -U postgres -f seed_data.sql
-- =============================================================================

SET search_path TO ppc, public;

-- =============================================================================
-- 1. COMPANY
-- =============================================================================
INSERT INTO company (id, name, legal_name, website, address_line1, city, state_province, postal_code, country, phone, email, default_markup_pct, default_sprint_weeks)
VALUES (1, 'Imagery Solutions', 'Imagery Solutions LLC', 'https://imagerysolutions.com',
        '100 Innovation Drive', 'Austin', 'TX', '78701', 'US',
        '(512) 555-0100', 'info@imagerysolutions.com', 30.00, 2);

SELECT setval('company_id_seq', 1);

-- =============================================================================
-- 2. USERS
-- =============================================================================
INSERT INTO app_user (id, company_id, email, display_name, title, is_admin) VALUES
(1, 1, 'admin@imagerysolutions.com',   'Admin User',     'System Administrator', TRUE),
(2, 1, 'pm@imagerysolutions.com',      'Sarah Chen',     'Project Manager',      FALSE),
(3, 1, 'lead@imagerysolutions.com',    'James Rodriguez','Technical Lead',        FALSE);

SELECT setval('app_user_id_seq', 3);

-- =============================================================================
-- 3. ROLES  (matching the spreadsheet: Architect $60, Dev1 $60, Dev2 $100, Tester $40)
-- =============================================================================
INSERT INTO role (id, company_id, name, description, base_rate, overhead_rate, sort_order) VALUES
(1, 1, 'Architect',    'Solutions Architect — system design, technical leadership',         60.00, 15.00, 1),
(2, 1, 'Developer 1',  'Mid-level Developer — core feature implementation',                 60.00, 12.00, 2),
(3, 1, 'Developer 2',  'Senior Developer — complex modules, integrations, mentoring',      100.00, 20.00, 3),
(4, 1, 'Tester 1',     'QA Engineer — test planning, execution, automation',                40.00, 10.00, 4),
(5, 1, 'Project Manager', 'Project Manager — planning, coordination, stakeholder comms',    55.00, 14.00, 5),
(6, 1, 'UX Designer',  'UX/UI Designer — wireframes, prototypes, user research',            65.00, 13.00, 6),
(7, 1, 'DevOps Engineer', 'DevOps — CI/CD, infrastructure, deployment automation',          80.00, 16.00, 7),
(8, 1, 'Business Analyst', 'BA — requirements gathering, process mapping, documentation',   50.00, 12.00, 8);

SELECT setval('role_id_seq', 8);

-- =============================================================================
-- 4. RESOURCES  (individual people)
-- =============================================================================
INSERT INTO resource (id, company_id, role_id, first_name, last_name, email) VALUES
(1,  1, 1, 'Maria',    'Thompson',  'maria.thompson@imagerysolutions.com'),
(2,  1, 2, 'David',    'Kim',       'david.kim@imagerysolutions.com'),
(3,  1, 3, 'Alex',     'Patel',     'alex.patel@imagerysolutions.com'),
(4,  1, 4, 'Lisa',     'Nguyen',    'lisa.nguyen@imagerysolutions.com'),
(5,  1, 5, 'Sarah',    'Chen',      'sarah.chen@imagerysolutions.com'),
(6,  1, 6, 'Jordan',   'Rivera',    'jordan.rivera@imagerysolutions.com'),
(7,  1, 7, 'Chris',    'Mueller',   'chris.mueller@imagerysolutions.com'),
(8,  1, 8, 'Priya',    'Sharma',    'priya.sharma@imagerysolutions.com'),
(9,  1, 2, 'Tyler',    'Brooks',    'tyler.brooks@imagerysolutions.com'),
(10, 1, 4, 'Emma',     'Watson',    'emma.watson@imagerysolutions.com');

SELECT setval('resource_id_seq', 10);

-- =============================================================================
-- 5. AGILE CEREMONY DEFINITIONS
-- =============================================================================
INSERT INTO agile_ceremony (id, company_id, name, description, duration_hours, occurrences_per_sprint, sort_order) VALUES
(1, 1, 'Sprint Planning',
    'Team reviews and commits to the Sprint Backlog. Product Owner presents prioritized stories. Team estimates and plans the work for the upcoming sprint.',
    4.00, 1, 1),
(2, 1, 'Daily Standup',
    'Brief daily synchronization meeting. Each team member shares: what they did yesterday, what they will do today, and any blockers preventing progress.',
    0.25, 10, 2),
(3, 1, 'Sprint Review / Demo',
    'Team demonstrates completed work to stakeholders. Feedback is gathered and the Product Backlog is updated accordingly. Acceptance criteria are validated.',
    2.00, 1, 3),
(4, 1, 'Sprint Retrospective',
    'Team reflects on the sprint process. Identifies what went well, what could be improved, and commits to actionable improvements for the next sprint.',
    1.50, 1, 4),
(5, 1, 'Backlog Refinement',
    'Team and Product Owner review upcoming backlog items. Stories are clarified, acceptance criteria defined, estimates assigned, and items prioritized for future sprints.',
    2.00, 1, 5);

SELECT setval('agile_ceremony_id_seq', 5);

-- =============================================================================
-- 6. SAMPLE PROJECT
-- =============================================================================
INSERT INTO project (id, company_id, name, description, client_name, client_contact, client_email,
                     total_weeks, sprint_length_weeks, markup_pct, status, start_date, created_by)
VALUES (1, 1,
    'Enterprise Portal Modernization',
    'Full modernization of the client enterprise portal including new UX, microservices backend, CI/CD pipeline, and comprehensive testing.',
    'Meridian Financial Group',
    'Robert Blackwell',
    'r.blackwell@meridianfg.com',
    12, 2, 30.00, 'Planning',
    '2026-03-16', 2);

SELECT setval('project_id_seq', 1);

-- =============================================================================
-- 7. PHASES
-- =============================================================================
INSERT INTO phase (id, project_id, name, description, start_week, end_week, sort_order) VALUES
(1, 1, 'Discovery & Inception',   'Requirements gathering, stakeholder interviews, current-state analysis, and project kickoff.',          1,  2,  1),
(2, 1, 'Design & Architecture',   'System architecture, UX/UI design, data modeling, API specification, and technical prototyping.',        3,  4,  2),
(3, 1, 'Development — Core',      'Core module development, API implementation, database build-out, and integration layer construction.',   5,  8,  3),
(4, 1, 'Testing & Hardening',     'System testing, integration testing, performance testing, security review, and defect resolution.',      9, 10,  4),
(5, 1, 'Deployment & Transition', 'Production deployment, data migration, user training, documentation handoff, and hypercare support.',  11, 12,  5);

SELECT setval('phase_id_seq', 5);

-- =============================================================================
-- 8. COMPONENTS  (with Statement of Work)
-- =============================================================================
INSERT INTO component (id, project_id, phase_id, name, description, statement_of_work, complexity, status, sort_order) VALUES

-- Phase 1: Discovery
(1,  1, 1, 'Stakeholder Interviews',
    'Conduct structured interviews with key stakeholders.',
    'Conduct up to 8 structured interviews (60–90 min each) with executive sponsors, department heads, and end-user representatives. Deliverables include interview summaries, consolidated requirements document, and stakeholder map.',
    'Medium', 'Approved', 1),

(2,  1, 1, 'Current-State Assessment',
    'Analyze existing systems, integrations, and pain points.',
    'Perform a comprehensive audit of the current portal: technology stack, integrations, performance baselines, security posture, and user analytics. Deliver a Current-State Assessment Report with gap analysis and risk register.',
    'High', 'Approved', 2),

(3,  1, 1, 'Requirements Documentation',
    'Produce a formal requirements specification.',
    'Compile functional and non-functional requirements into a structured Requirements Specification Document (RSD). Include user stories, acceptance criteria, data dictionary, and traceability matrix. Requires two rounds of stakeholder review.',
    'Medium', 'Approved', 3),

-- Phase 2: Design
(4,  1, 2, 'UX Research & Wireframes',
    'User research, personas, journey maps, and wireframes.',
    'Conduct user research (surveys, card sorting, task analysis) with up to 15 participants. Produce 3 user personas, 2 journey maps, and low/mid-fidelity wireframes for all primary workflows. Two rounds of revision included.',
    'High', 'Approved', 4),

(5,  1, 2, 'System Architecture Design',
    'Design the target-state system architecture.',
    'Design microservices architecture including service decomposition, API contracts (OpenAPI 3.0), data model (ERD), event-driven patterns, security architecture, and infrastructure topology. Deliver Architecture Decision Records (ADRs) and Technical Design Document.',
    'Critical', 'Approved', 5),

(6,  1, 2, 'UI Prototype',
    'High-fidelity interactive prototype.',
    'Create high-fidelity interactive prototype in Figma covering the top 5 user workflows. Include design system components, responsive layouts (desktop + tablet), and accessibility compliance review (WCAG 2.1 AA). One round of usability testing included.',
    'High', 'Approved', 6),

-- Phase 3: Development
(7,  1, 3, 'Authentication & Authorization Module',
    'SSO, RBAC, MFA implementation.',
    'Implement OAuth 2.0 / OIDC authentication with SSO integration (Azure AD / Okta). Build role-based access control (RBAC) with configurable permission sets. Implement MFA (TOTP + SMS). Includes session management, token refresh, and audit logging.',
    'Critical', 'Estimated', 7),

(8,  1, 3, 'Dashboard & Analytics Module',
    'Real-time dashboards with configurable widgets.',
    'Build configurable dashboard framework with drag-and-drop widget layout. Implement 8 standard widget types (KPI cards, line charts, bar charts, pie charts, tables, maps, gauges, alerts). Data sourced from analytics API with 30-second refresh intervals.',
    'High', 'Estimated', 8),

(9,  1, 3, 'Document Management Module',
    'Upload, versioning, search, and collaboration.',
    'Implement document management with upload (drag-and-drop, bulk), version control, full-text search (Elasticsearch), metadata tagging, folder hierarchy, sharing/permissions, and inline preview (PDF, images, Office docs). Storage backend: S3-compatible.',
    'High', 'Estimated', 9),

(10, 1, 3, 'API Gateway & Integration Layer',
    'Central API gateway, rate limiting, and third-party integrations.',
    'Deploy API gateway with rate limiting, request/response transformation, API key management, and OpenAPI documentation portal. Build integration adapters for 3 third-party systems (CRM, ERP, email service). Include retry logic, circuit breakers, and dead-letter queues.',
    'Critical', 'Estimated', 10),

(11, 1, 3, 'Notification & Messaging Service',
    'Email, SMS, in-app, and push notifications.',
    'Build a multi-channel notification service supporting email (SMTP/SendGrid), SMS (Twilio), in-app notifications (WebSocket), and push notifications (FCM/APNs). Include notification templates, user preferences, delivery tracking, and retry mechanisms.',
    'Medium', 'Estimated', 11),

-- Phase 4: Testing
(12, 1, 4, 'Integration & System Testing',
    'End-to-end testing of all modules and integrations.',
    'Execute comprehensive test plan covering integration testing (API contracts, data flows), system testing (end-to-end workflows), regression testing, and cross-browser/device testing. Deliver test results report, defect log, and test coverage metrics. Target: 85%+ code coverage.',
    'High', 'Estimated', 12),

(13, 1, 4, 'Performance & Security Testing',
    'Load testing, penetration testing, and security audit.',
    'Conduct performance testing (JMeter/k6): load testing (500 concurrent users), stress testing, endurance testing. Perform security assessment: OWASP Top 10 scan, dependency vulnerability audit, penetration testing (authenticated + unauthenticated). Deliver performance benchmarks and security findings report.',
    'Critical', 'Estimated', 13),

-- Phase 5: Deployment
(14, 1, 5, 'Production Deployment & Migration',
    'Infrastructure provisioning, deployment, and data migration.',
    'Provision production infrastructure (IaC via Terraform). Execute blue-green deployment with automated rollback capability. Perform data migration from legacy system with validation checksums. Deliver runbook, monitoring dashboards (Grafana/Datadog), and alert configuration.',
    'Critical', 'Estimated', 14),

(15, 1, 5, 'Training & Documentation',
    'User training, admin training, and technical documentation.',
    'Develop training materials: end-user guide, admin guide, and API developer documentation. Conduct 3 training sessions (2 hrs each) for end users, 2 sessions for administrators, and 1 technical handoff session for client IT team. Deliver recorded sessions and knowledge base articles.',
    'Medium', 'Estimated', 15);

SELECT setval('component_id_seq', 15);

-- =============================================================================
-- 9. COMPONENT RESOURCES  (man-hours per role per component)
-- =============================================================================
INSERT INTO component_resource (component_id, role_id, estimated_hours, notes) VALUES

-- Component 1: Stakeholder Interviews
(1, 1, 16, 'Lead 4 interviews, synthesize findings'),
(1, 5, 20, 'Schedule, facilitate all 8 interviews, produce summaries'),
(1, 8, 24, 'Attend all interviews, draft requirements'),

-- Component 2: Current-State Assessment
(2, 1, 24, 'Architecture review, integration mapping'),
(2, 3, 16, 'Code audit, performance baseline analysis'),
(2, 7, 12, 'Infrastructure and security posture review'),
(2, 8, 20, 'Process documentation, gap analysis'),

-- Component 3: Requirements Documentation
(3, 8, 40, 'Primary author of RSD'),
(3, 5, 12, 'Review coordination, stakeholder alignment'),
(3, 1,  8, 'Technical requirements validation'),

-- Component 4: UX Research & Wireframes
(4, 6, 60, 'Research, personas, journey maps, wireframes'),
(4, 8, 16, 'Research support, documentation'),
(4, 5,  8, 'Scheduling, stakeholder coordination'),

-- Component 5: System Architecture Design
(5, 1, 60, 'Lead architecture design, ADRs, tech design doc'),
(5, 3, 24, 'Database design, API contract review'),
(5, 7, 16, 'Infrastructure topology, CI/CD design'),

-- Component 6: UI Prototype
(6, 6, 48, 'High-fidelity prototype, design system components'),
(6, 2,  8, 'Technical feasibility review'),
(6, 4,  8, 'Usability test facilitation'),

-- Component 7: Auth & Authorization Module
(7, 3, 60, 'Core implementation: OAuth, RBAC, MFA'),
(7, 2, 40, 'Supporting implementation, unit tests'),
(7, 1, 12, 'Architecture guidance, security review'),
(7, 4, 20, 'Auth flow testing, security test cases'),

-- Component 8: Dashboard & Analytics Module
(8, 3, 48, 'Dashboard framework, complex widgets'),
(8, 2, 60, 'Standard widgets, data integration'),
(8, 6, 24, 'Dashboard UX, widget design'),
(8, 4, 16, 'Dashboard testing, cross-browser'),

-- Component 9: Document Management Module
(9, 3, 56, 'Core doc management, search, versioning'),
(9, 2, 48, 'Upload, preview, folder hierarchy'),
(9, 7, 12, 'S3 integration, Elasticsearch setup'),
(9, 4, 20, 'Document workflow testing'),

-- Component 10: API Gateway & Integration Layer
(10, 1, 20, 'API strategy, gateway architecture'),
(10, 3, 48, 'Gateway implementation, integration adapters'),
(10, 2, 32, 'Adapter implementation, rate limiting'),
(10, 7, 24, 'Gateway deployment, monitoring'),
(10, 4, 16, 'API contract testing'),

-- Component 11: Notification & Messaging Service
(11, 2, 48, 'Multi-channel notification implementation'),
(11, 3, 16, 'Architecture review, WebSocket implementation'),
(11, 7,  8, 'Infrastructure for message queues'),
(11, 4, 12, 'Notification delivery testing'),

-- Component 12: Integration & System Testing
(12, 4, 80, 'Test planning, execution, defect management'),
(12, 2, 16, 'Test environment support, defect fixes'),
(12, 3, 12, 'Complex defect resolution'),
(12, 5, 12, 'Test coordination, status reporting'),

-- Component 13: Performance & Security Testing
(13, 4, 40, 'Performance test execution, analysis'),
(13, 7, 24, 'Infrastructure tuning, monitoring'),
(13, 3, 16, 'Security remediation'),
(13, 1, 12, 'Security review oversight'),

-- Component 14: Production Deployment & Migration
(14, 7, 48, 'IaC, deployment pipeline, monitoring'),
(14, 3, 24, 'Data migration scripts, validation'),
(14, 1, 12, 'Deployment architecture, runbook review'),
(14, 2, 16, 'Migration support, smoke testing'),
(14, 4, 16, 'Post-deployment validation'),

-- Component 15: Training & Documentation
(15, 5, 32, 'Training coordination, session facilitation'),
(15, 8, 40, 'Documentation authoring, knowledge base'),
(15, 2, 12, 'Technical documentation, API docs'),
(15, 6,  8, 'User guide screenshots, visual aids');

-- =============================================================================
-- 9b. MATERIALS  (non-labor cost items)
-- =============================================================================
INSERT INTO material (id, company_id, name, description, category, unit, unit_cost, is_active, sort_order) VALUES
(20, 1, 'Cloud Hosting (Monthly)',      'AWS/Azure compute & storage for dev/staging environments',           'Software/Licenses', 'month',   2400.00, TRUE, 1),
(21, 1, 'CI/CD Pipeline Licenses',      'GitHub Actions / Jenkins enterprise license fees',                   'Software/Licenses', 'month',    350.00, TRUE, 2),
(22, 1, 'SSL Certificates',             'Wildcard SSL certs for staging and production',                      'Software/Licenses', 'unit',     250.00, TRUE, 3),
(23, 1, 'Monitoring & APM Tools',       'Datadog / New Relic APM license for project duration',               'Software/Licenses', 'month',    450.00, TRUE, 4),
(24, 1, 'UX Research Incentives',       'Gift cards / compensation for usability test participants',          'Other',             'unit',      50.00, TRUE, 5),
(25, 1, 'Team Travel — On-site Kick-off', 'Round-trip airfare + hotel for 3-day on-site workshop',           'Travel',            'trip',    1800.00, TRUE, 6),
(26, 1, 'Printing & Documentation',     'Bound project deliverables, training manuals',                      'Office Supplies',   'lot',      180.00, TRUE, 7),
(27, 1, 'Security Pen-Test Service',    'Third-party penetration testing engagement',                         'Other',             'unit',    4500.00, TRUE, 8),
(28, 1, 'Load Testing Platform',        'Blazemeter / k6 cloud for performance testing',                     'Software/Licenses', 'month',    600.00, TRUE, 9);

SELECT setval('material_id_seq', 28);

-- =============================================================================
-- 9c. COMPONENT MATERIALS  (materials/expenses per component)
-- =============================================================================
INSERT INTO component_material (component_id, material_id, quantity, notes) VALUES

-- Component 1: Stakeholder Interviews
(1, 25, 2, 'Kick-off + mid-project on-site visits'),
(1, 26, 1, 'Printed interview guides'),

-- Component 4: UX Research & Wireframes
(4, 24, 15, 'Incentives for 15 usability test participants'),

-- Component 5: System Architecture Design
(5, 20, 3, 'Dev/staging cloud hosting during design phase'),
(5, 21, 3, 'CI/CD pipeline setup'),

-- Component 7: Authentication & Authorization Module
(7, 22, 2, 'Wildcard SSL certs for staging + prod'),
(7, 20, 3, 'Cloud hosting during development'),

-- Component 10: API Gateway & Integration Layer
(10, 23, 4, 'APM monitoring during integration testing'),

-- Component 13: Performance & Security Testing
(13, 27, 1, 'Third-party pen test engagement'),
(13, 28, 2, 'Load testing cloud platform'),

-- Component 14: Production Deployment & Migration
(14, 20, 3, 'Production cloud hosting setup'),
(14, 23, 3, 'Monitoring + APM for production'),

-- Component 15: Training & Documentation
(15, 26, 3, 'Printed training manuals and handoff binders');

-- =============================================================================
-- 10. SPRINTS  (6 sprints x 2 weeks = 12 weeks)
-- =============================================================================
INSERT INTO sprint (id, project_id, sprint_number, name, goal, start_week, end_week, status) VALUES
(1, 1, 1, 'Sprint 1 — Discovery',
    'Complete stakeholder interviews, begin current-state assessment, establish project foundations.',
    1, 2, 'Planned'),
(2, 1, 2, 'Sprint 2 — Design',
    'Complete architecture design, UX wireframes, and requirements documentation.',
    3, 4, 'Planned'),
(3, 1, 3, 'Sprint 3 — Core Dev I',
    'Implement authentication module and begin dashboard and API gateway development.',
    5, 6, 'Planned'),
(4, 1, 4, 'Sprint 4 — Core Dev II',
    'Complete dashboard, document management, and notification modules.',
    7, 8, 'Planned'),
(5, 1, 5, 'Sprint 5 — Testing',
    'Execute integration, system, performance, and security testing. Resolve defects.',
    9, 10, 'Planned'),
(6, 1, 6, 'Sprint 6 — Deploy & Transition',
    'Production deployment, data migration, user training, and project handoff.',
    11, 12, 'Planned');

SELECT setval('sprint_id_seq', 6);

-- =============================================================================
-- 11. SPRINT COMPONENTS  (assign components to sprints)
-- =============================================================================
INSERT INTO sprint_component (sprint_id, component_id, story_points, priority, status) VALUES
-- Sprint 1: Discovery
(1, 1, 8,  1, 'To Do'),   -- Stakeholder Interviews
(1, 2, 13, 2, 'To Do'),   -- Current-State Assessment
(1, 3, 8,  3, 'To Do'),   -- Requirements Documentation

-- Sprint 2: Design
(2, 4, 13, 1, 'To Do'),   -- UX Research & Wireframes
(2, 5, 21, 2, 'To Do'),   -- System Architecture Design
(2, 6, 13, 3, 'To Do'),   -- UI Prototype

-- Sprint 3: Core Dev I
(3, 7, 21, 1, 'To Do'),   -- Auth & Authorization
(3, 8, 13, 2, 'To Do'),   -- Dashboard (start)
(3, 10, 21, 3, 'To Do'),  -- API Gateway (start)

-- Sprint 4: Core Dev II
(4, 9, 21, 1, 'To Do'),   -- Document Management
(4, 11, 13, 2, 'To Do'),  -- Notification Service

-- Sprint 5: Testing
(5, 12, 13, 1, 'To Do'),  -- Integration & System Testing
(5, 13, 21, 2, 'To Do'),  -- Performance & Security Testing

-- Sprint 6: Deploy
(6, 14, 21, 1, 'To Do'),  -- Production Deployment
(6, 15, 8,  2, 'To Do');  -- Training & Documentation

-- =============================================================================
-- 12. SPRINT CEREMONIES  (all ceremonies for all sprints)
-- =============================================================================
INSERT INTO sprint_ceremony (sprint_id, ceremony_id, status) VALUES
-- Sprint 1
(1, 1, 'Scheduled'), (1, 2, 'Scheduled'), (1, 3, 'Scheduled'), (1, 4, 'Scheduled'), (1, 5, 'Scheduled'),
-- Sprint 2
(2, 1, 'Scheduled'), (2, 2, 'Scheduled'), (2, 3, 'Scheduled'), (2, 4, 'Scheduled'), (2, 5, 'Scheduled'),
-- Sprint 3
(3, 1, 'Scheduled'), (3, 2, 'Scheduled'), (3, 3, 'Scheduled'), (3, 4, 'Scheduled'), (3, 5, 'Scheduled'),
-- Sprint 4
(4, 1, 'Scheduled'), (4, 2, 'Scheduled'), (4, 3, 'Scheduled'), (4, 4, 'Scheduled'), (4, 5, 'Scheduled'),
-- Sprint 5
(5, 1, 'Scheduled'), (5, 2, 'Scheduled'), (5, 3, 'Scheduled'), (5, 4, 'Scheduled'), (5, 5, 'Scheduled'),
-- Sprint 6
(6, 1, 'Scheduled'), (6, 2, 'Scheduled'), (6, 3, 'Scheduled'), (6, 4, 'Scheduled'), (6, 5, 'Scheduled');

-- =============================================================================
-- 13. WEEK ALLOCATIONS  (hours per role per week — the planning grid)
--     Matches spreadsheet layout: Roles × 12 Weeks
-- =============================================================================

-- Architect (Role 1): Heavy early, taper off
INSERT INTO week_allocation (project_id, role_id, week_number, allocated_hours) VALUES
(1, 1, 1,  20), (1, 1, 2,  20), (1, 1, 3,  30), (1, 1, 4,  30),
(1, 1, 5,  12), (1, 1, 6,  10), (1, 1, 7,   8), (1, 1, 8,   8),
(1, 1, 9,   6), (1, 1, 10,  6), (1, 1, 11,  6), (1, 1, 12,  6);

-- Developer 1 (Role 2): Ramps up through development
INSERT INTO week_allocation (project_id, role_id, week_number, allocated_hours) VALUES
(1, 2, 1,   4), (1, 2, 2,   4), (1, 2, 3,   8), (1, 2, 4,   8),
(1, 2, 5,  36), (1, 2, 6,  36), (1, 2, 7,  40), (1, 2, 8,  40),
(1, 2, 9,  16), (1, 2, 10, 12), (1, 2, 11, 14), (1, 2, 12, 14);

-- Developer 2 / Senior (Role 3): Peaks during complex development
INSERT INTO week_allocation (project_id, role_id, week_number, allocated_hours) VALUES
(1, 3, 1,   8), (1, 3, 2,   8), (1, 3, 3,  12), (1, 3, 4,  12),
(1, 3, 5,  36), (1, 3, 6,  36), (1, 3, 7,  40), (1, 3, 8,  40),
(1, 3, 9,  14), (1, 3, 10, 14), (1, 3, 11, 12), (1, 3, 12, 12);

-- Tester 1 (Role 4): Light early, heavy during testing
INSERT INTO week_allocation (project_id, role_id, week_number, allocated_hours) VALUES
(1, 4, 1,   0), (1, 4, 2,   0), (1, 4, 3,   4), (1, 4, 4,   4),
(1, 4, 5,  10), (1, 4, 6,  10), (1, 4, 7,  16), (1, 4, 8,  16),
(1, 4, 9,  40), (1, 4, 10, 40), (1, 4, 11,  8), (1, 4, 12,  8);

-- Project Manager (Role 5): Consistent throughout
INSERT INTO week_allocation (project_id, role_id, week_number, allocated_hours) VALUES
(1, 5, 1,  16), (1, 5, 2,  16), (1, 5, 3,  10), (1, 5, 4,  10),
(1, 5, 5,   8), (1, 5, 6,   8), (1, 5, 7,   8), (1, 5, 8,   8),
(1, 5, 9,   8), (1, 5, 10,  8), (1, 5, 11, 16), (1, 5, 12, 16);

-- UX Designer (Role 6): Front-loaded
INSERT INTO week_allocation (project_id, role_id, week_number, allocated_hours) VALUES
(1, 6, 1,   4), (1, 6, 2,   4), (1, 6, 3,  30), (1, 6, 4,  30),
(1, 6, 5,  16), (1, 6, 6,   8), (1, 6, 7,   4), (1, 6, 8,   4),
(1, 6, 9,   0), (1, 6, 10,  0), (1, 6, 11,  4), (1, 6, 12,  4);

-- DevOps (Role 7): Spikes during infra & deployment
INSERT INTO week_allocation (project_id, role_id, week_number, allocated_hours) VALUES
(1, 7, 1,   4), (1, 7, 2,   8), (1, 7, 3,   8), (1, 7, 4,   8),
(1, 7, 5,  12), (1, 7, 6,  12), (1, 7, 7,   8), (1, 7, 8,   8),
(1, 7, 9,  12), (1, 7, 10, 12), (1, 7, 11, 24), (1, 7, 12, 24);

-- Business Analyst (Role 8): Heavy in discovery, tapers
INSERT INTO week_allocation (project_id, role_id, week_number, allocated_hours) VALUES
(1, 8, 1,  20), (1, 8, 2,  20), (1, 8, 3,  16), (1, 8, 4,  16),
(1, 8, 5,   4), (1, 8, 6,   4), (1, 8, 7,   0), (1, 8, 8,   0),
(1, 8, 9,   0), (1, 8, 10,  0), (1, 8, 11, 20), (1, 8, 12, 20);

-- =============================================================================
-- 14. SAMPLE ESTIMATE
-- =============================================================================
INSERT INTO estimate (id, project_id, estimate_number, name, description,
                      client_name, client_email, client_company,
                      markup_pct, version, status, valid_until,
                      terms_and_conditions, created_by,
                      shared_link_token)
VALUES (1, 1, 'EST-2026-0001',
    'Enterprise Portal Modernization — Full Estimate',
    'Complete estimate for the Enterprise Portal Modernization project including all phases from Discovery through Deployment and Transition.',
    'Robert Blackwell', 'r.blackwell@meridianfg.com', 'Meridian Financial Group',
    30.00, 1, 'Draft', '2026-04-15',
    E'1. This estimate is valid for 30 days from date of issue.\n2. Payment terms: Net 30 from invoice date.\n3. Work will be performed on a time-and-materials basis with weekly status reporting.\n4. Any scope changes require a formal Change Order.\n5. Client will provide timely access to stakeholders, systems, and data as needed.\n6. All intellectual property created during the engagement will transfer to the client upon final payment.',
    2,
    'est_abc123def456');

SELECT setval('estimate_id_seq', 1);

-- Include all components in the estimate
INSERT INTO estimate_component (estimate_id, component_id, included, sort_order)
SELECT 1, id, TRUE, sort_order FROM component WHERE project_id = 1 ORDER BY sort_order;

-- =============================================================================
-- 15. SAMPLE CHANGE ORDER
-- =============================================================================
INSERT INTO change_order (id, project_id, estimate_id, change_order_number,
                          title, description, reason, impact_summary,
                          additional_hours, additional_cost, additional_sell,
                          schedule_impact_weeks, status, requested_by, requested_date,
                          created_by)
VALUES (1, 1, 1, 'CO-001',
    'Add Mobile Responsive Views',
    'Add fully responsive mobile views for all primary workflows to support field users accessing the portal from tablets and smartphones.',
    'Client stakeholder feedback from Sprint 1 interviews revealed 35% of portal usage occurs on mobile devices, which was not in the original scope.',
    'Adds approximately 120 hours across UX Design and Development. Extends testing phase by 1 week. Total additional cost of $9,480 (before markup).',
    120, 9480.00, 12324.00, 1, 'Pending Review',
    'Robert Blackwell', '2026-04-01', 2);

SELECT setval('change_order_id_seq', 1);

INSERT INTO change_order_item (change_order_id, action, description, role_id, hours_delta, cost_delta, sort_order) VALUES
(1, 'Add',    'Mobile wireframes and responsive design system tokens',                  6, 32,  2496.00, 1),
(1, 'Add',    'Responsive CSS/JS implementation for 5 primary views',                   2, 40,  2880.00, 2),
(1, 'Add',    'Mobile-specific interactions and touch optimization',                     3, 24,  2880.00, 3),
(1, 'Modify', 'Extended cross-device testing (add mobile/tablet test matrix)',           4, 24,  1200.00, 4),
(1, 'Modify', 'Update user documentation with mobile screenshots and instructions',     8,  0,    24.00, 5);

-- =============================================================================
-- VERIFICATION QUERIES  (run these to validate the seed data)
-- =============================================================================
-- Uncomment to run:

-- SELECT '--- Role Rates ---';
-- SELECT id, name, base_rate, overhead_rate, base_rate + overhead_rate AS fully_loaded FROM role ORDER BY sort_order;

-- SELECT '--- Component Summary ---';
-- SELECT * FROM v_component_cost ORDER BY component_id;

-- SELECT '--- Weekly Cost Grid (Architect) ---';
-- SELECT * FROM v_weekly_cost_grid WHERE role_id = 1 ORDER BY week_number;

-- SELECT '--- Project Summary ---';
-- SELECT * FROM v_project_summary;

-- SELECT '--- Phase Costs ---';
-- SELECT * FROM v_phase_cost ORDER BY start_week;

-- SELECT '--- Sprint Summary ---';
-- SELECT * FROM v_sprint_summary ORDER BY sprint_number;

-- SELECT '--- Ceremony Hours Per Sprint ---';
-- SELECT sprint_number, SUM(total_ceremony_hours) AS total_ceremony_hours_per_sprint
-- FROM v_sprint_ceremony_hours GROUP BY sprint_number ORDER BY sprint_number;
