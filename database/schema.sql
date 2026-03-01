-- =============================================================================
-- Project Planning, Costing & Estimating Engine
-- PostgreSQL Schema
--
-- Three-Tier Architecture:
--   Tier 1 (Presentation): Wt (Witty) C++ Web Application
--   Tier 2 (Middleware):    ApiLogicServer (auto-generated REST API + business logic)
--   Tier 3 (Persistence):  PostgreSQL
--
-- Run:  psql -U postgres -f schema.sql
-- =============================================================================

-- Drop existing database objects if recreating
DROP SCHEMA IF EXISTS ppc CASCADE;
CREATE SCHEMA ppc;
SET search_path TO ppc, public;

-- =============================================================================
-- EXTENSIONS
-- =============================================================================
CREATE EXTENSION IF NOT EXISTS "uuid-ossp";
CREATE EXTENSION IF NOT EXISTS "pgcrypto";

-- =============================================================================
-- 1. COMPANY / ORGANIZATION  (branding, multi-tenant ready)
-- =============================================================================
CREATE TABLE company (
    id              SERIAL PRIMARY KEY,
    name            VARCHAR(200) NOT NULL,
    legal_name      VARCHAR(300),
    logo_url        VARCHAR(500),
    website         VARCHAR(300),
    address_line1   VARCHAR(200),
    address_line2   VARCHAR(200),
    city            VARCHAR(100),
    state_province  VARCHAR(100),
    postal_code     VARCHAR(20),
    country         VARCHAR(100) DEFAULT 'US',
    phone           VARCHAR(30),
    email           VARCHAR(200),
    tax_id          VARCHAR(50),
    default_markup_pct  NUMERIC(6,2) DEFAULT 30.00,
    default_sprint_weeks INTEGER DEFAULT 2,
    created_at      TIMESTAMP WITH TIME ZONE DEFAULT NOW(),
    updated_at      TIMESTAMP WITH TIME ZONE DEFAULT NOW()
);

COMMENT ON TABLE company IS 'Organization / tenant for branding and defaults';

-- =============================================================================
-- 2. USERS / TEAM MEMBERS
-- =============================================================================
CREATE TABLE app_user (
    id              SERIAL PRIMARY KEY,
    company_id      INTEGER NOT NULL REFERENCES company(id) ON DELETE CASCADE,
    email           VARCHAR(200) NOT NULL UNIQUE,
    display_name    VARCHAR(150) NOT NULL,
    password_hash   VARCHAR(200),
    avatar_url      VARCHAR(500),
    title           VARCHAR(100),
    is_active       BOOLEAN DEFAULT TRUE,
    is_admin        BOOLEAN DEFAULT FALSE,
    created_at      TIMESTAMP WITH TIME ZONE DEFAULT NOW(),
    updated_at      TIMESTAMP WITH TIME ZONE DEFAULT NOW()
);

CREATE INDEX idx_app_user_company ON app_user(company_id);
CREATE INDEX idx_app_user_email   ON app_user(email);

COMMENT ON TABLE app_user IS 'Application users who can create/manage estimates and projects';

-- =============================================================================
-- 3. ROLES  (resource types with hourly rates)
-- =============================================================================
CREATE TABLE role (
    id              SERIAL PRIMARY KEY,
    company_id      INTEGER NOT NULL REFERENCES company(id) ON DELETE CASCADE,
    name            VARCHAR(100) NOT NULL,
    description     TEXT,
    base_rate       NUMERIC(10,2) NOT NULL DEFAULT 0.00,
    overhead_rate   NUMERIC(10,2) NOT NULL DEFAULT 0.00,
    is_active       BOOLEAN DEFAULT TRUE,
    sort_order      INTEGER DEFAULT 0,
    created_at      TIMESTAMP WITH TIME ZONE DEFAULT NOW(),
    updated_at      TIMESTAMP WITH TIME ZONE DEFAULT NOW(),
    UNIQUE(company_id, name)
);

CREATE INDEX idx_role_company ON role(company_id);

COMMENT ON TABLE role IS 'Resource roles (Architect, Developer, Tester, etc.) with base and overhead rates';
COMMENT ON COLUMN role.base_rate IS 'Base hourly pay rate for this role';
COMMENT ON COLUMN role.overhead_rate IS 'Overhead per hour (benefits, insurance, facilities, etc.)';

-- Computed fully_loaded_rate = base_rate + overhead_rate (handled by view or API)

-- =============================================================================
-- 4. RESOURCES  (individual people assigned to roles)
-- =============================================================================
CREATE TABLE resource (
    id              SERIAL PRIMARY KEY,
    company_id      INTEGER NOT NULL REFERENCES company(id) ON DELETE CASCADE,
    role_id         INTEGER NOT NULL REFERENCES role(id) ON DELETE RESTRICT,
    first_name      VARCHAR(100) NOT NULL,
    last_name       VARCHAR(100) NOT NULL,
    email           VARCHAR(200),
    phone           VARCHAR(30),
    rate_override   NUMERIC(10,2),  -- NULL = use role rate
    availability_pct NUMERIC(5,2) DEFAULT 100.00,
    is_active       BOOLEAN DEFAULT TRUE,
    created_at      TIMESTAMP WITH TIME ZONE DEFAULT NOW(),
    updated_at      TIMESTAMP WITH TIME ZONE DEFAULT NOW()
);

CREATE INDEX idx_resource_company ON resource(company_id);
CREATE INDEX idx_resource_role    ON resource(role_id);

COMMENT ON TABLE resource IS 'Individual team members assigned to roles';
COMMENT ON COLUMN resource.rate_override IS 'Per-person rate override; NULL means use the role default rate';
COMMENT ON COLUMN resource.availability_pct IS 'Percentage of full-time availability (100 = full time)';

-- =============================================================================
-- 5. PROJECT
-- =============================================================================
CREATE TABLE project (
    id              SERIAL PRIMARY KEY,
    company_id      INTEGER NOT NULL REFERENCES company(id) ON DELETE CASCADE,
    name            VARCHAR(200) NOT NULL,
    description     TEXT,
    client_name     VARCHAR(200),
    client_contact  VARCHAR(200),
    client_email    VARCHAR(200),
    client_phone    VARCHAR(30),
    total_weeks     INTEGER NOT NULL DEFAULT 12,
    sprint_length_weeks INTEGER NOT NULL DEFAULT 2,
    markup_pct      NUMERIC(6,2) NOT NULL DEFAULT 30.00,
    status          VARCHAR(30) NOT NULL DEFAULT 'Planning'
                        CHECK (status IN ('Planning','Active','On Hold','Completed','Cancelled')),
    start_date      DATE,
    target_end_date DATE,
    created_by      INTEGER REFERENCES app_user(id),
    created_at      TIMESTAMP WITH TIME ZONE DEFAULT NOW(),
    updated_at      TIMESTAMP WITH TIME ZONE DEFAULT NOW()
);

CREATE INDEX idx_project_company ON project(company_id);
CREATE INDEX idx_project_status  ON project(status);

COMMENT ON TABLE project IS 'Top-level project container';

-- =============================================================================
-- 6. PHASE  (project phases spanning week ranges)
-- =============================================================================
CREATE TABLE phase (
    id              SERIAL PRIMARY KEY,
    project_id      INTEGER NOT NULL REFERENCES project(id) ON DELETE CASCADE,
    name            VARCHAR(150) NOT NULL,
    description     TEXT,
    start_week      INTEGER NOT NULL DEFAULT 1,
    end_week        INTEGER NOT NULL DEFAULT 1,
    sort_order      INTEGER DEFAULT 0,
    created_at      TIMESTAMP WITH TIME ZONE DEFAULT NOW(),
    updated_at      TIMESTAMP WITH TIME ZONE DEFAULT NOW(),
    CONSTRAINT chk_phase_weeks CHECK (end_week >= start_week AND start_week >= 1)
);

CREATE INDEX idx_phase_project ON phase(project_id);

COMMENT ON TABLE phase IS 'Project phases (Discovery, Design, Development, Testing, Deployment, etc.)';

-- =============================================================================
-- 7. COMPONENT  (estimable unit of work with Statement of Work)
-- =============================================================================
CREATE TABLE component (
    id              SERIAL PRIMARY KEY,
    project_id      INTEGER NOT NULL REFERENCES project(id) ON DELETE CASCADE,
    phase_id        INTEGER REFERENCES phase(id) ON DELETE SET NULL,
    name            VARCHAR(200) NOT NULL,
    description     TEXT,
    statement_of_work TEXT,
    complexity      VARCHAR(20) DEFAULT 'Medium'
                        CHECK (complexity IN ('Low','Medium','High','Critical')),
    status          VARCHAR(30) DEFAULT 'Draft'
                        CHECK (status IN ('Draft','Estimated','Approved','In Progress','Done')),
    sort_order      INTEGER DEFAULT 0,
    created_at      TIMESTAMP WITH TIME ZONE DEFAULT NOW(),
    updated_at      TIMESTAMP WITH TIME ZONE DEFAULT NOW()
);

CREATE INDEX idx_component_project ON component(project_id);
CREATE INDEX idx_component_phase   ON component(phase_id);

COMMENT ON TABLE component IS 'Estimable unit of work with its own Statement of Work, resource needs, and man-hours';
COMMENT ON COLUMN component.statement_of_work IS 'Full Statement of Work text for this component';

-- =============================================================================
-- 8. COMPONENT_RESOURCE  (man-hours needed per role for a component)
-- =============================================================================
CREATE TABLE component_resource (
    id              SERIAL PRIMARY KEY,
    component_id    INTEGER NOT NULL REFERENCES component(id) ON DELETE CASCADE,
    role_id         INTEGER NOT NULL REFERENCES role(id) ON DELETE RESTRICT,
    estimated_hours NUMERIC(8,2) NOT NULL DEFAULT 0.00,
    notes           TEXT,
    created_at      TIMESTAMP WITH TIME ZONE DEFAULT NOW(),
    updated_at      TIMESTAMP WITH TIME ZONE DEFAULT NOW(),
    UNIQUE(component_id, role_id)
);

CREATE INDEX idx_comp_resource_component ON component_resource(component_id);
CREATE INDEX idx_comp_resource_role      ON component_resource(role_id);

COMMENT ON TABLE component_resource IS 'Resource requirements per component — hours needed per role';

-- =============================================================================
-- 8b. MATERIAL  (materials, expenses, and non-labor cost items)
-- =============================================================================
CREATE TABLE material (
    id              SERIAL PRIMARY KEY,
    company_id      INTEGER NOT NULL REFERENCES company(id) ON DELETE CASCADE,
    name            VARCHAR(200) NOT NULL,
    description     TEXT,
    category        VARCHAR(50) DEFAULT 'Other'
                        CHECK (category IN ('Office Supplies','Construction','Equipment/Tools',
                                            'Travel','Software/Licenses','Other')),
    unit            VARCHAR(30) DEFAULT 'unit',
    unit_cost       NUMERIC(12,2) NOT NULL DEFAULT 0.00,
    is_active       BOOLEAN DEFAULT TRUE,
    sort_order      INTEGER DEFAULT 0,
    created_at      TIMESTAMP WITH TIME ZONE DEFAULT NOW(),
    updated_at      TIMESTAMP WITH TIME ZONE DEFAULT NOW()
);

CREATE INDEX idx_material_company ON material(company_id);

COMMENT ON TABLE material IS 'Materials, equipment, travel, licenses, and other non-labor cost items';
COMMENT ON COLUMN material.category IS 'Cost category: Office Supplies, Construction, Equipment/Tools, Travel, Software/Licenses, Other';
COMMENT ON COLUMN material.unit IS 'Unit of measure: unit, month, trip, lot, license, etc.';

-- =============================================================================
-- 8c. COMPONENT_MATERIAL  (materials/expenses assigned to a component)
-- =============================================================================
CREATE TABLE component_material (
    id              SERIAL PRIMARY KEY,
    component_id    INTEGER NOT NULL REFERENCES component(id) ON DELETE CASCADE,
    material_id     INTEGER NOT NULL REFERENCES material(id) ON DELETE RESTRICT,
    quantity        NUMERIC(10,2) NOT NULL DEFAULT 0.00,
    notes           TEXT,
    created_at      TIMESTAMP WITH TIME ZONE DEFAULT NOW(),
    updated_at      TIMESTAMP WITH TIME ZONE DEFAULT NOW(),
    UNIQUE(component_id, material_id)
);

CREATE INDEX idx_comp_material_component ON component_material(component_id);
CREATE INDEX idx_comp_material_material  ON component_material(material_id);

COMMENT ON TABLE component_material IS 'Materials and expenses assigned to a component with quantities';

-- =============================================================================
-- 9. AGILE CEREMONY DEFINITIONS
-- =============================================================================
CREATE TABLE agile_ceremony (
    id                      SERIAL PRIMARY KEY,
    company_id              INTEGER NOT NULL REFERENCES company(id) ON DELETE CASCADE,
    name                    VARCHAR(100) NOT NULL,
    description             TEXT,
    duration_hours          NUMERIC(5,2) NOT NULL DEFAULT 1.00,
    occurrences_per_sprint  INTEGER NOT NULL DEFAULT 1,
    is_active               BOOLEAN DEFAULT TRUE,
    sort_order              INTEGER DEFAULT 0,
    created_at              TIMESTAMP WITH TIME ZONE DEFAULT NOW(),
    updated_at              TIMESTAMP WITH TIME ZONE DEFAULT NOW(),
    UNIQUE(company_id, name)
);

CREATE INDEX idx_ceremony_company ON agile_ceremony(company_id);

COMMENT ON TABLE agile_ceremony IS 'Standard Agile ceremony definitions with default durations';
COMMENT ON COLUMN agile_ceremony.duration_hours IS 'Duration in hours per occurrence';
COMMENT ON COLUMN agile_ceremony.occurrences_per_sprint IS 'Number of times this ceremony occurs each sprint';

-- =============================================================================
-- 10. SPRINT  (2-week iteration)
-- =============================================================================
CREATE TABLE sprint (
    id              SERIAL PRIMARY KEY,
    project_id      INTEGER NOT NULL REFERENCES project(id) ON DELETE CASCADE,
    sprint_number   INTEGER NOT NULL,
    name            VARCHAR(150),
    goal            TEXT,
    start_week      INTEGER NOT NULL,
    end_week        INTEGER NOT NULL,
    start_date      DATE,
    end_date        DATE,
    status          VARCHAR(20) NOT NULL DEFAULT 'Planned'
                        CHECK (status IN ('Planned','Active','Completed','Cancelled')),
    velocity_planned NUMERIC(8,2) DEFAULT 0,
    velocity_actual  NUMERIC(8,2) DEFAULT 0,
    created_at      TIMESTAMP WITH TIME ZONE DEFAULT NOW(),
    updated_at      TIMESTAMP WITH TIME ZONE DEFAULT NOW(),
    UNIQUE(project_id, sprint_number),
    CONSTRAINT chk_sprint_weeks CHECK (end_week >= start_week AND start_week >= 1)
);

CREATE INDEX idx_sprint_project ON sprint(project_id);

COMMENT ON TABLE sprint IS 'Sprint / iteration (default 2-week intervals)';

-- =============================================================================
-- 11. SPRINT_COMPONENT  (components/stories assigned to a sprint)
-- =============================================================================
CREATE TABLE sprint_component (
    id              SERIAL PRIMARY KEY,
    sprint_id       INTEGER NOT NULL REFERENCES sprint(id) ON DELETE CASCADE,
    component_id    INTEGER NOT NULL REFERENCES component(id) ON DELETE CASCADE,
    story_points    NUMERIC(5,1),
    priority        INTEGER DEFAULT 0,
    status          VARCHAR(30) DEFAULT 'To Do'
                        CHECK (status IN ('To Do','In Progress','In Review','Done','Blocked')),
    created_at      TIMESTAMP WITH TIME ZONE DEFAULT NOW(),
    UNIQUE(sprint_id, component_id)
);

CREATE INDEX idx_sprint_comp_sprint    ON sprint_component(sprint_id);
CREATE INDEX idx_sprint_comp_component ON sprint_component(component_id);

COMMENT ON TABLE sprint_component IS 'Many-to-many: components assigned to sprints (sprint backlog)';

-- =============================================================================
-- 12. SPRINT_CEREMONY  (ceremony instances per sprint with actual tracking)
-- =============================================================================
CREATE TABLE sprint_ceremony (
    id              SERIAL PRIMARY KEY,
    sprint_id       INTEGER NOT NULL REFERENCES sprint(id) ON DELETE CASCADE,
    ceremony_id     INTEGER NOT NULL REFERENCES agile_ceremony(id) ON DELETE CASCADE,
    scheduled_date  DATE,
    actual_duration_hours NUMERIC(5,2),
    notes           TEXT,
    status          VARCHAR(20) DEFAULT 'Scheduled'
                        CHECK (status IN ('Scheduled','Completed','Skipped')),
    created_at      TIMESTAMP WITH TIME ZONE DEFAULT NOW(),
    UNIQUE(sprint_id, ceremony_id)
);

CREATE INDEX idx_sprint_ceremony_sprint ON sprint_ceremony(sprint_id);

COMMENT ON TABLE sprint_ceremony IS 'Ceremony instances per sprint with scheduling and actual duration tracking';

-- =============================================================================
-- 13. WEEK_ALLOCATION  (hours allocated per role per week — the planning grid)
-- =============================================================================
CREATE TABLE week_allocation (
    id              SERIAL PRIMARY KEY,
    project_id      INTEGER NOT NULL REFERENCES project(id) ON DELETE CASCADE,
    role_id         INTEGER NOT NULL REFERENCES role(id) ON DELETE RESTRICT,
    resource_id     INTEGER REFERENCES resource(id) ON DELETE SET NULL,
    week_number     INTEGER NOT NULL,
    allocated_hours NUMERIC(6,2) NOT NULL DEFAULT 0.00,
    actual_hours    NUMERIC(6,2),
    notes           TEXT,
    created_at      TIMESTAMP WITH TIME ZONE DEFAULT NOW(),
    updated_at      TIMESTAMP WITH TIME ZONE DEFAULT NOW(),
    UNIQUE(project_id, role_id, week_number, resource_id),
    CONSTRAINT chk_week_number CHECK (week_number >= 1)
);

CREATE INDEX idx_week_alloc_project  ON week_allocation(project_id);
CREATE INDEX idx_week_alloc_role     ON week_allocation(role_id);
CREATE INDEX idx_week_alloc_resource ON week_allocation(resource_id);
CREATE INDEX idx_week_alloc_week     ON week_allocation(week_number);

COMMENT ON TABLE week_allocation IS 'Resource time allocation grid — hours per role per week per project';
COMMENT ON COLUMN week_allocation.allocated_hours IS 'Planned/estimated hours for this role in this week';
COMMENT ON COLUMN week_allocation.actual_hours IS 'Actual hours tracked (NULL until reported)';

-- =============================================================================
-- 14. ESTIMATE  (assembled project estimate / proposal)
-- =============================================================================
CREATE TABLE estimate (
    id              SERIAL PRIMARY KEY,
    project_id      INTEGER NOT NULL REFERENCES project(id) ON DELETE CASCADE,
    estimate_number VARCHAR(50) NOT NULL,
    name            VARCHAR(200) NOT NULL,
    description     TEXT,
    client_name     VARCHAR(200),
    client_email    VARCHAR(200),
    client_company  VARCHAR(200),
    markup_pct      NUMERIC(6,2) NOT NULL DEFAULT 30.00,
    total_hours     NUMERIC(10,2) DEFAULT 0.00,
    total_cost      NUMERIC(12,2) DEFAULT 0.00,
    total_sell      NUMERIC(12,2) DEFAULT 0.00,
    margin_amount   NUMERIC(12,2) DEFAULT 0.00,
    blended_cost_rate NUMERIC(10,2) DEFAULT 0.00,
    blended_sell_rate NUMERIC(10,2) DEFAULT 0.00,
    version         INTEGER NOT NULL DEFAULT 1,
    status          VARCHAR(30) NOT NULL DEFAULT 'Draft'
                        CHECK (status IN ('Draft','Pending Review','Sent','Viewed',
                                          'Signed','Approved','Rejected','Expired')),
    valid_until     DATE,
    terms_and_conditions TEXT,
    notes           TEXT,
    shared_link_token VARCHAR(100) UNIQUE,
    created_by      INTEGER REFERENCES app_user(id),
    created_at      TIMESTAMP WITH TIME ZONE DEFAULT NOW(),
    updated_at      TIMESTAMP WITH TIME ZONE DEFAULT NOW()
);

CREATE INDEX idx_estimate_project ON estimate(project_id);
CREATE INDEX idx_estimate_status  ON estimate(status);
CREATE INDEX idx_estimate_token   ON estimate(shared_link_token);

COMMENT ON TABLE estimate IS 'Full project estimate / proposal that can be sent, signed, and approved';
COMMENT ON COLUMN estimate.shared_link_token IS 'Unique token for online estimate viewing link (emailed to client)';
COMMENT ON COLUMN estimate.estimate_number IS 'Human-readable estimate number (e.g., EST-2026-0001)';

-- =============================================================================
-- 15. ESTIMATE_COMPONENT  (components included in an estimate)
-- =============================================================================
CREATE TABLE estimate_component (
    id              SERIAL PRIMARY KEY,
    estimate_id     INTEGER NOT NULL REFERENCES estimate(id) ON DELETE CASCADE,
    component_id    INTEGER NOT NULL REFERENCES component(id) ON DELETE RESTRICT,
    included        BOOLEAN DEFAULT TRUE,
    override_hours  NUMERIC(8,2),     -- NULL = use component default
    override_cost   NUMERIC(12,2),    -- NULL = calculated
    sort_order      INTEGER DEFAULT 0,
    created_at      TIMESTAMP WITH TIME ZONE DEFAULT NOW(),
    UNIQUE(estimate_id, component_id)
);

CREATE INDEX idx_est_comp_estimate  ON estimate_component(estimate_id);
CREATE INDEX idx_est_comp_component ON estimate_component(component_id);

COMMENT ON TABLE estimate_component IS 'Components included in a specific estimate with optional overrides';

-- =============================================================================
-- 16. ESTIMATE_PHASE_SUMMARY  (phase-level pricing in the estimate)
-- =============================================================================
CREATE TABLE estimate_phase_summary (
    id              SERIAL PRIMARY KEY,
    estimate_id     INTEGER NOT NULL REFERENCES estimate(id) ON DELETE CASCADE,
    phase_id        INTEGER NOT NULL REFERENCES phase(id) ON DELETE CASCADE,
    phase_hours     NUMERIC(10,2) DEFAULT 0.00,
    phase_cost      NUMERIC(12,2) DEFAULT 0.00,
    phase_sell      NUMERIC(12,2) DEFAULT 0.00,
    sort_order      INTEGER DEFAULT 0,
    created_at      TIMESTAMP WITH TIME ZONE DEFAULT NOW(),
    UNIQUE(estimate_id, phase_id)
);

CREATE INDEX idx_est_phase_estimate ON estimate_phase_summary(estimate_id);

COMMENT ON TABLE estimate_phase_summary IS 'Cached phase-level cost and sell price summaries for an estimate';

-- =============================================================================
-- 17. ESTIMATE_ROLE_SUMMARY  (role-level pricing in the estimate)
-- =============================================================================
CREATE TABLE estimate_role_summary (
    id              SERIAL PRIMARY KEY,
    estimate_id     INTEGER NOT NULL REFERENCES estimate(id) ON DELETE CASCADE,
    role_id         INTEGER NOT NULL REFERENCES role(id) ON DELETE RESTRICT,
    total_hours     NUMERIC(10,2) DEFAULT 0.00,
    cost_rate       NUMERIC(10,2) DEFAULT 0.00,
    sell_rate       NUMERIC(10,2) DEFAULT 0.00,
    total_cost      NUMERIC(12,2) DEFAULT 0.00,
    total_sell      NUMERIC(12,2) DEFAULT 0.00,
    created_at      TIMESTAMP WITH TIME ZONE DEFAULT NOW(),
    UNIQUE(estimate_id, role_id)
);

CREATE INDEX idx_est_role_estimate ON estimate_role_summary(estimate_id);

COMMENT ON TABLE estimate_role_summary IS 'Role-level cost and sell price summaries for an estimate';

-- =============================================================================
-- 18. E-SIGNATURE
-- =============================================================================
CREATE TABLE e_signature (
    id              SERIAL PRIMARY KEY,
    estimate_id     INTEGER NOT NULL REFERENCES estimate(id) ON DELETE CASCADE,
    signer_name     VARCHAR(200) NOT NULL,
    signer_title    VARCHAR(150),
    signer_email    VARCHAR(200) NOT NULL,
    signer_company  VARCHAR(200),
    signature_data  TEXT,           -- Base64 encoded signature image or typed name
    signature_type  VARCHAR(20) DEFAULT 'typed'
                        CHECK (signature_type IN ('typed','drawn','uploaded')),
    ip_address      VARCHAR(45),
    user_agent      TEXT,
    signed_at       TIMESTAMP WITH TIME ZONE NOT NULL DEFAULT NOW(),
    is_valid        BOOLEAN DEFAULT TRUE,
    created_at      TIMESTAMP WITH TIME ZONE DEFAULT NOW()
);

CREATE INDEX idx_esig_estimate ON e_signature(estimate_id);

COMMENT ON TABLE e_signature IS 'Electronic signatures captured on estimates';
COMMENT ON COLUMN e_signature.signature_data IS 'Base64-encoded signature image (drawn), typed name, or uploaded file reference';

-- =============================================================================
-- 19. CHANGE ORDER
-- =============================================================================
CREATE TABLE change_order (
    id              SERIAL PRIMARY KEY,
    project_id      INTEGER NOT NULL REFERENCES project(id) ON DELETE CASCADE,
    estimate_id     INTEGER REFERENCES estimate(id) ON DELETE SET NULL,
    change_order_number VARCHAR(50) NOT NULL,
    title           VARCHAR(200) NOT NULL,
    description     TEXT,
    reason          TEXT,
    impact_summary  TEXT,
    additional_hours NUMERIC(10,2) DEFAULT 0.00,
    additional_cost  NUMERIC(12,2) DEFAULT 0.00,
    additional_sell  NUMERIC(12,2) DEFAULT 0.00,
    schedule_impact_weeks INTEGER DEFAULT 0,
    status          VARCHAR(30) NOT NULL DEFAULT 'Draft'
                        CHECK (status IN ('Draft','Pending Review','Submitted',
                                          'Approved','Rejected','Withdrawn')),
    requested_by    VARCHAR(200),
    requested_date  DATE DEFAULT CURRENT_DATE,
    reviewed_by     INTEGER REFERENCES app_user(id),
    reviewed_date   DATE,
    approved_by     VARCHAR(200),
    approved_date   DATE,
    version         INTEGER DEFAULT 1,
    created_by      INTEGER REFERENCES app_user(id),
    created_at      TIMESTAMP WITH TIME ZONE DEFAULT NOW(),
    updated_at      TIMESTAMP WITH TIME ZONE DEFAULT NOW()
);

CREATE INDEX idx_co_project  ON change_order(project_id);
CREATE INDEX idx_co_estimate ON change_order(estimate_id);
CREATE INDEX idx_co_status   ON change_order(status);

COMMENT ON TABLE change_order IS 'Change orders that modify scope, cost, or schedule of an approved estimate';

-- =============================================================================
-- 20. CHANGE_ORDER_ITEM  (line items within a change order)
-- =============================================================================
CREATE TABLE change_order_item (
    id              SERIAL PRIMARY KEY,
    change_order_id INTEGER NOT NULL REFERENCES change_order(id) ON DELETE CASCADE,
    action          VARCHAR(20) NOT NULL
                        CHECK (action IN ('Add','Modify','Remove')),
    component_id    INTEGER REFERENCES component(id) ON DELETE SET NULL,
    description     TEXT NOT NULL,
    role_id         INTEGER REFERENCES role(id) ON DELETE SET NULL,
    hours_delta     NUMERIC(8,2) DEFAULT 0.00,
    cost_delta      NUMERIC(12,2) DEFAULT 0.00,
    sort_order      INTEGER DEFAULT 0,
    created_at      TIMESTAMP WITH TIME ZONE DEFAULT NOW()
);

CREATE INDEX idx_co_item_co ON change_order_item(change_order_id);

COMMENT ON TABLE change_order_item IS 'Individual line-item changes within a change order';

-- =============================================================================
-- 21. CHANGE ORDER SIGNATURE  (approvals on change orders)
-- =============================================================================
CREATE TABLE change_order_signature (
    id              SERIAL PRIMARY KEY,
    change_order_id INTEGER NOT NULL REFERENCES change_order(id) ON DELETE CASCADE,
    signer_name     VARCHAR(200) NOT NULL,
    signer_title    VARCHAR(150),
    signer_email    VARCHAR(200) NOT NULL,
    signature_data  TEXT,
    signature_type  VARCHAR(20) DEFAULT 'typed'
                        CHECK (signature_type IN ('typed','drawn','uploaded')),
    ip_address      VARCHAR(45),
    signed_at       TIMESTAMP WITH TIME ZONE NOT NULL DEFAULT NOW(),
    created_at      TIMESTAMP WITH TIME ZONE DEFAULT NOW()
);

CREATE INDEX idx_co_sig_co ON change_order_signature(change_order_id);

COMMENT ON TABLE change_order_signature IS 'E-signatures on change order approvals';

-- =============================================================================
-- 22. EMAIL LOG  (track sent estimate/quote emails)
-- =============================================================================
CREATE TABLE email_log (
    id              SERIAL PRIMARY KEY,
    estimate_id     INTEGER REFERENCES estimate(id) ON DELETE SET NULL,
    change_order_id INTEGER REFERENCES change_order(id) ON DELETE SET NULL,
    recipient_email VARCHAR(200) NOT NULL,
    recipient_name  VARCHAR(200),
    subject         VARCHAR(300) NOT NULL,
    body_preview    TEXT,
    link_url        VARCHAR(500),
    status          VARCHAR(20) DEFAULT 'Queued'
                        CHECK (status IN ('Queued','Sent','Delivered','Opened','Bounced','Failed')),
    sent_at         TIMESTAMP WITH TIME ZONE,
    opened_at       TIMESTAMP WITH TIME ZONE,
    created_at      TIMESTAMP WITH TIME ZONE DEFAULT NOW()
);

CREATE INDEX idx_email_estimate ON email_log(estimate_id);
CREATE INDEX idx_email_co       ON email_log(change_order_id);

COMMENT ON TABLE email_log IS 'Audit log of emails sent for estimates and change orders';

-- =============================================================================
-- 23. AUDIT LOG  (general activity tracking)
-- =============================================================================
CREATE TABLE audit_log (
    id              SERIAL PRIMARY KEY,
    entity_type     VARCHAR(50) NOT NULL,
    entity_id       INTEGER NOT NULL,
    action          VARCHAR(20) NOT NULL
                        CHECK (action IN ('Create','Update','Delete','StatusChange',
                                          'Sign','Send','Approve','Reject')),
    user_id         INTEGER REFERENCES app_user(id),
    old_values      JSONB,
    new_values      JSONB,
    description     TEXT,
    ip_address      VARCHAR(45),
    created_at      TIMESTAMP WITH TIME ZONE DEFAULT NOW()
);

CREATE INDEX idx_audit_entity ON audit_log(entity_type, entity_id);
CREATE INDEX idx_audit_user   ON audit_log(user_id);
CREATE INDEX idx_audit_date   ON audit_log(created_at);

COMMENT ON TABLE audit_log IS 'General audit trail for all entity changes';

-- =============================================================================
-- VIEWS  (computed / summary views for ApiLogicServer and reporting)
-- =============================================================================

-- Role fully loaded rate view
CREATE OR REPLACE VIEW v_role_rates AS
SELECT
    r.id,
    r.company_id,
    r.name,
    r.base_rate,
    r.overhead_rate,
    r.base_rate + r.overhead_rate AS fully_loaded_rate,
    r.is_active
FROM role r;

COMMENT ON VIEW v_role_rates IS 'Roles with computed fully loaded rate (base + overhead)';

-- Component cost summary view (labor + materials)
CREATE OR REPLACE VIEW v_component_cost AS
SELECT
    c.id AS component_id,
    c.project_id,
    c.name AS component_name,
    c.phase_id,
    COALESCE(labor.total_hours, 0) AS total_hours,
    COALESCE(labor.labor_cost, 0) AS labor_cost,
    COALESCE(mat.material_cost, 0) AS material_cost,
    COALESCE(labor.labor_cost, 0) + COALESCE(mat.material_cost, 0) AS total_cost,
    COALESCE(labor.role_count, 0) AS role_count
FROM component c
LEFT JOIN (
    SELECT cr.component_id,
           SUM(cr.estimated_hours) AS total_hours,
           SUM(cr.estimated_hours * (r.base_rate + r.overhead_rate)) AS labor_cost,
           COUNT(DISTINCT cr.role_id) AS role_count
    FROM component_resource cr
    JOIN role r ON r.id = cr.role_id
    GROUP BY cr.component_id
) labor ON labor.component_id = c.id
LEFT JOIN (
    SELECT cm.component_id,
           SUM(cm.quantity * m.unit_cost) AS material_cost
    FROM component_material cm
    JOIN material m ON m.id = cm.material_id
    GROUP BY cm.component_id
) mat ON mat.component_id = c.id;

COMMENT ON VIEW v_component_cost IS 'Component-level cost summary with labor hours, labor cost, material cost, and total cost';

-- Weekly cost grid view (the "Fully Loaded Costs" table)
CREATE OR REPLACE VIEW v_weekly_cost_grid AS
SELECT
    wa.project_id,
    wa.role_id,
    r.name AS role_name,
    wa.week_number,
    wa.allocated_hours,
    r.base_rate + r.overhead_rate AS fully_loaded_rate,
    wa.allocated_hours * (r.base_rate + r.overhead_rate) AS weekly_cost
FROM week_allocation wa
JOIN role r ON r.id = wa.role_id
ORDER BY wa.project_id, wa.role_id, wa.week_number;

COMMENT ON VIEW v_weekly_cost_grid IS 'Weekly fully loaded cost grid (hours x fully loaded rate)';

-- Project cost summary by role
CREATE OR REPLACE VIEW v_project_role_summary AS
SELECT
    wa.project_id,
    wa.role_id,
    r.name AS role_name,
    r.base_rate,
    r.overhead_rate,
    r.base_rate + r.overhead_rate AS fully_loaded_rate,
    SUM(wa.allocated_hours) AS total_hours,
    SUM(wa.allocated_hours * (r.base_rate + r.overhead_rate)) AS total_cost
FROM week_allocation wa
JOIN role r ON r.id = wa.role_id
GROUP BY wa.project_id, wa.role_id, r.name, r.base_rate, r.overhead_rate
ORDER BY wa.project_id, r.name;

COMMENT ON VIEW v_project_role_summary IS 'Per-role hours and cost totals for each project';

-- Project total summary
CREATE OR REPLACE VIEW v_project_summary AS
SELECT
    p.id AS project_id,
    p.name AS project_name,
    p.markup_pct,
    COALESCE(SUM(wa.allocated_hours), 0) AS total_hours,
    COALESCE(SUM(wa.allocated_hours * (r.base_rate + r.overhead_rate)), 0) AS total_cost,
    COALESCE(SUM(wa.allocated_hours * (r.base_rate + r.overhead_rate)), 0)
        * (1 + p.markup_pct / 100.0) AS total_sell,
    COALESCE(SUM(wa.allocated_hours * (r.base_rate + r.overhead_rate)), 0)
        * (p.markup_pct / 100.0) AS margin_amount,
    CASE WHEN SUM(wa.allocated_hours) > 0
         THEN SUM(wa.allocated_hours * (r.base_rate + r.overhead_rate)) / SUM(wa.allocated_hours)
         ELSE 0 END AS blended_cost_rate,
    CASE WHEN SUM(wa.allocated_hours) > 0
         THEN (SUM(wa.allocated_hours * (r.base_rate + r.overhead_rate)) / SUM(wa.allocated_hours))
              * (1 + p.markup_pct / 100.0)
         ELSE 0 END AS blended_sell_rate
FROM project p
LEFT JOIN week_allocation wa ON wa.project_id = p.id
LEFT JOIN role r ON r.id = wa.role_id
GROUP BY p.id, p.name, p.markup_pct;

COMMENT ON VIEW v_project_summary IS 'Project-level totals: hours, cost, sell, margin, blended rates';

-- Phase cost summary (for phased pricing)
CREATE OR REPLACE VIEW v_phase_cost AS
SELECT
    ph.id AS phase_id,
    ph.project_id,
    ph.name AS phase_name,
    ph.start_week,
    ph.end_week,
    p.markup_pct,
    COALESCE(SUM(wa.allocated_hours), 0) AS phase_hours,
    COALESCE(SUM(wa.allocated_hours * (r.base_rate + r.overhead_rate)), 0) AS phase_cost,
    COALESCE(SUM(wa.allocated_hours * (r.base_rate + r.overhead_rate)), 0)
        * (1 + p.markup_pct / 100.0) AS phase_sell
FROM phase ph
JOIN project p ON p.id = ph.project_id
LEFT JOIN week_allocation wa ON wa.project_id = ph.project_id
    AND wa.week_number >= ph.start_week
    AND wa.week_number <= ph.end_week
LEFT JOIN role r ON r.id = wa.role_id
GROUP BY ph.id, ph.project_id, ph.name, ph.start_week, ph.end_week, p.markup_pct
ORDER BY ph.project_id, ph.start_week;

COMMENT ON VIEW v_phase_cost IS 'Phase-level cost and sell summary (for phased pricing / quoting)';

-- Sprint ceremony hours view
CREATE OR REPLACE VIEW v_sprint_ceremony_hours AS
SELECT
    s.id AS sprint_id,
    s.project_id,
    s.sprint_number,
    ac.name AS ceremony_name,
    ac.duration_hours,
    ac.occurrences_per_sprint,
    ac.duration_hours * ac.occurrences_per_sprint AS total_ceremony_hours,
    sc.status AS ceremony_status,
    sc.actual_duration_hours
FROM sprint s
JOIN sprint_ceremony sc ON sc.sprint_id = s.id
JOIN agile_ceremony ac ON ac.id = sc.ceremony_id
ORDER BY s.sprint_number, ac.sort_order;

COMMENT ON VIEW v_sprint_ceremony_hours IS 'Ceremony hours breakdown per sprint';

-- Sprint summary view
CREATE OR REPLACE VIEW v_sprint_summary AS
SELECT
    s.id AS sprint_id,
    s.project_id,
    s.sprint_number,
    s.name AS sprint_name,
    s.start_week,
    s.end_week,
    s.status,
    COUNT(DISTINCT sc2.component_id) AS component_count,
    COALESCE(SUM(DISTINCT cr_hours.total_hours), 0) AS story_hours,
    COALESCE(ceremony_totals.total_ceremony_hours, 0) AS ceremony_hours
FROM sprint s
LEFT JOIN sprint_component sc2 ON sc2.sprint_id = s.id
LEFT JOIN (
    SELECT cr.component_id, SUM(cr.estimated_hours) AS total_hours
    FROM component_resource cr
    GROUP BY cr.component_id
) cr_hours ON cr_hours.component_id = sc2.component_id
LEFT JOIN (
    SELECT sc.sprint_id,
           SUM(ac.duration_hours * ac.occurrences_per_sprint) AS total_ceremony_hours
    FROM sprint_ceremony sc
    JOIN agile_ceremony ac ON ac.id = sc.ceremony_id
    GROUP BY sc.sprint_id
) ceremony_totals ON ceremony_totals.sprint_id = s.id
GROUP BY s.id, s.project_id, s.sprint_number, s.name,
         s.start_week, s.end_week, s.status,
         ceremony_totals.total_ceremony_hours;

COMMENT ON VIEW v_sprint_summary IS 'Sprint-level summary with component count, story hours, and ceremony overhead';

-- =============================================================================
-- FUNCTIONS
-- =============================================================================

-- Auto-generate sprints for a project based on total_weeks and sprint_length
CREATE OR REPLACE FUNCTION generate_sprints(p_project_id INTEGER)
RETURNS INTEGER AS $$
DECLARE
    v_total_weeks INTEGER;
    v_sprint_len  INTEGER;
    v_sprint_num  INTEGER := 1;
    v_start       INTEGER := 1;
    v_count       INTEGER := 0;
    v_company_id  INTEGER;
BEGIN
    SELECT total_weeks, sprint_length_weeks, company_id
    INTO v_total_weeks, v_sprint_len, v_company_id
    FROM project WHERE id = p_project_id;

    IF NOT FOUND THEN
        RAISE EXCEPTION 'Project % not found', p_project_id;
    END IF;

    -- Delete existing sprints for this project
    DELETE FROM sprint WHERE project_id = p_project_id;

    -- Create sprints
    WHILE v_start <= v_total_weeks LOOP
        INSERT INTO sprint (project_id, sprint_number, name, start_week, end_week)
        VALUES (
            p_project_id,
            v_sprint_num,
            'Sprint ' || v_sprint_num,
            v_start,
            LEAST(v_start + v_sprint_len - 1, v_total_weeks)
        );

        -- Auto-create ceremony instances for this sprint
        INSERT INTO sprint_ceremony (sprint_id, ceremony_id)
        SELECT currval('sprint_id_seq'), ac.id
        FROM agile_ceremony ac
        WHERE ac.company_id = v_company_id AND ac.is_active = TRUE;

        v_start := v_start + v_sprint_len;
        v_sprint_num := v_sprint_num + 1;
        v_count := v_count + 1;
    END LOOP;

    RETURN v_count;
END;
$$ LANGUAGE plpgsql;

COMMENT ON FUNCTION generate_sprints IS 'Auto-generate sprints (2-week intervals) for a project and attach all active ceremonies';

-- Recalculate estimate totals from components
CREATE OR REPLACE FUNCTION recalculate_estimate(p_estimate_id INTEGER)
RETURNS VOID AS $$
DECLARE
    v_markup NUMERIC;
BEGIN
    SELECT markup_pct INTO v_markup FROM estimate WHERE id = p_estimate_id;

    UPDATE estimate SET
        total_hours = sub.total_hours,
        total_cost  = sub.total_cost,
        total_sell  = sub.total_cost * (1 + v_markup / 100.0),
        margin_amount = sub.total_cost * (v_markup / 100.0),
        blended_cost_rate = CASE WHEN sub.total_hours > 0
                                 THEN sub.total_cost / sub.total_hours ELSE 0 END,
        blended_sell_rate = CASE WHEN sub.total_hours > 0
                                 THEN (sub.total_cost / sub.total_hours) * (1 + v_markup / 100.0)
                                 ELSE 0 END,
        updated_at = NOW()
    FROM (
        SELECT
            COALESCE(SUM(cr.estimated_hours), 0) AS total_hours,
            COALESCE(SUM(cr.estimated_hours * (r.base_rate + r.overhead_rate)), 0) AS total_cost
        FROM estimate_component ec
        JOIN component_resource cr ON cr.component_id = ec.component_id
        JOIN role r ON r.id = cr.role_id
        WHERE ec.estimate_id = p_estimate_id AND ec.included = TRUE
    ) sub
    WHERE estimate.id = p_estimate_id;
END;
$$ LANGUAGE plpgsql;

COMMENT ON FUNCTION recalculate_estimate IS 'Recalculate estimate totals from included components and their resource costs';

-- Generate estimate number
CREATE OR REPLACE FUNCTION generate_estimate_number(p_company_id INTEGER)
RETURNS VARCHAR AS $$
DECLARE
    v_seq INTEGER;
BEGIN
    SELECT COALESCE(MAX(
        CAST(NULLIF(regexp_replace(estimate_number, '[^0-9]', '', 'g'), '') AS INTEGER)
    ), 0) + 1
    INTO v_seq
    FROM estimate e
    JOIN project p ON p.id = e.project_id
    WHERE p.company_id = p_company_id;

    RETURN 'EST-' || TO_CHAR(NOW(), 'YYYY') || '-' || LPAD(v_seq::TEXT, 4, '0');
END;
$$ LANGUAGE plpgsql;

-- Generate change order number
CREATE OR REPLACE FUNCTION generate_co_number(p_project_id INTEGER)
RETURNS VARCHAR AS $$
DECLARE
    v_seq INTEGER;
BEGIN
    SELECT COALESCE(COUNT(*), 0) + 1 INTO v_seq
    FROM change_order WHERE project_id = p_project_id;

    RETURN 'CO-' || LPAD(v_seq::TEXT, 3, '0');
END;
$$ LANGUAGE plpgsql;

-- =============================================================================
-- TRIGGERS
-- =============================================================================

-- Auto-update updated_at timestamps
CREATE OR REPLACE FUNCTION update_timestamp()
RETURNS TRIGGER AS $$
BEGIN
    NEW.updated_at = NOW();
    RETURN NEW;
END;
$$ LANGUAGE plpgsql;

CREATE TRIGGER trg_company_updated    BEFORE UPDATE ON company    FOR EACH ROW EXECUTE FUNCTION update_timestamp();
CREATE TRIGGER trg_app_user_updated   BEFORE UPDATE ON app_user   FOR EACH ROW EXECUTE FUNCTION update_timestamp();
CREATE TRIGGER trg_role_updated       BEFORE UPDATE ON role       FOR EACH ROW EXECUTE FUNCTION update_timestamp();
CREATE TRIGGER trg_resource_updated   BEFORE UPDATE ON resource   FOR EACH ROW EXECUTE FUNCTION update_timestamp();
CREATE TRIGGER trg_project_updated    BEFORE UPDATE ON project    FOR EACH ROW EXECUTE FUNCTION update_timestamp();
CREATE TRIGGER trg_phase_updated      BEFORE UPDATE ON phase      FOR EACH ROW EXECUTE FUNCTION update_timestamp();
CREATE TRIGGER trg_component_updated  BEFORE UPDATE ON component  FOR EACH ROW EXECUTE FUNCTION update_timestamp();
CREATE TRIGGER trg_comp_res_updated   BEFORE UPDATE ON component_resource FOR EACH ROW EXECUTE FUNCTION update_timestamp();
CREATE TRIGGER trg_material_updated   BEFORE UPDATE ON material           FOR EACH ROW EXECUTE FUNCTION update_timestamp();
CREATE TRIGGER trg_comp_mat_updated   BEFORE UPDATE ON component_material FOR EACH ROW EXECUTE FUNCTION update_timestamp();
CREATE TRIGGER trg_ceremony_updated   BEFORE UPDATE ON agile_ceremony     FOR EACH ROW EXECUTE FUNCTION update_timestamp();
CREATE TRIGGER trg_sprint_updated     BEFORE UPDATE ON sprint     FOR EACH ROW EXECUTE FUNCTION update_timestamp();
CREATE TRIGGER trg_week_alloc_updated BEFORE UPDATE ON week_allocation    FOR EACH ROW EXECUTE FUNCTION update_timestamp();
CREATE TRIGGER trg_estimate_updated   BEFORE UPDATE ON estimate   FOR EACH ROW EXECUTE FUNCTION update_timestamp();
CREATE TRIGGER trg_co_updated         BEFORE UPDATE ON change_order       FOR EACH ROW EXECUTE FUNCTION update_timestamp();
