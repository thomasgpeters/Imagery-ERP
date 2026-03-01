// =============================================================================
// App.cpp — Main application with branded sidebar, topnav, workarea, theme toggle
//           and role-based navigation (Planning vs Execution)
// =============================================================================

#include "App.h"
#include <Wt/WBootstrapTheme.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WText.h>
#include <Wt/WPushButton.h>
#include <Wt/WTemplate.h>
#include <Wt/WAnchor.h>
#include <Wt/WImage.h>
#include <Wt/WDialog.h>
#include <Wt/WLineEdit.h>
#include <Wt/WLabel.h>

#include "DashboardView.h"
#include "ResourceView.h"
#include "ComponentView.h"
#include "SprintView.h"
#include "CostingView.h"
#include "QuoteView.h"
#include "ChangeOrderView.h"
#include "MaterialView.h"

App::App(const Wt::WEnvironment& env)
    : WApplication(env)
{
    setTitle("Project Planning & Costing — Imagery Solutions");

    // Load custom CSS
    useStyleSheet("style.css");

    // Load sample data
    data_.loadSampleData();

    // Build the UI
    buildLayout();

    // Show dashboard by default
    showView(0);
}

void App::buildLayout()
{
    // Ensure Wt's root container fills the viewport
    root()->setStyleClass("wt-root");

    // App shell: row flex — sidebar (full height) | right column (topbar + workarea)
    appShell_ = root()->addWidget(std::make_unique<Wt::WContainerWidget>());
    appShell_->setStyleClass("app-shell theme-light");
    // Belt-and-suspenders: set flex row via inline style so Wt can't override it
    appShell_->setAttributeValue("style",
        "display:flex !important;flex-direction:row !important;"
        "height:100vh !important;width:100% !important;overflow:hidden !important;");

    // Sidebar — full height, left edge
    sidebar_ = appShell_->addWidget(std::make_unique<Wt::WContainerWidget>());
    buildSidebar();

    // Right column: topbar + workarea
    auto mainColumn = appShell_->addWidget(std::make_unique<Wt::WContainerWidget>());
    mainColumn->setStyleClass("app-main");
    mainColumn->setAttributeValue("style",
        "display:flex !important;flex-direction:column !important;"
        "flex:1 1 0% !important;min-width:0;overflow:hidden;height:100%;");

    topbar_ = mainColumn->addWidget(std::make_unique<Wt::WContainerWidget>());
    buildTopbar();

    workarea_ = mainColumn->addWidget(std::make_unique<Wt::WContainerWidget>());
    workarea_->setStyleClass("app-workarea");

    // Create all views (hidden by default — showView reveals one at a time)
    // Fixed order: 0=Dashboard, 1=Roles, 2=Components, 3=Sprints, 4=Costing, 5=Quote, 6=ChangeOrders, 7=Materials
    auto dv  = std::make_unique<DashboardView>(data_);   dashboardView_   = dv.get();
    auto rv  = std::make_unique<ResourceView>(data_);    resourceView_    = rv.get();
    auto cv  = std::make_unique<ComponentView>(data_);   componentView_   = cv.get();
    auto sv  = std::make_unique<SprintView>(data_);      sprintView_      = sv.get();
    auto csv = std::make_unique<CostingView>(data_);     costingView_     = csv.get();
    auto qv  = std::make_unique<QuoteView>(data_);       quoteView_       = qv.get();
    auto cov = std::make_unique<ChangeOrderView>(data_); changeOrderView_ = cov.get();
    auto mv  = std::make_unique<MaterialView>(data_);    materialView_    = mv.get();

    workarea_->addWidget(std::move(dv));
    workarea_->addWidget(std::move(rv));
    workarea_->addWidget(std::move(cv));
    workarea_->addWidget(std::move(sv));
    workarea_->addWidget(std::move(csv));
    workarea_->addWidget(std::move(qv));
    workarea_->addWidget(std::move(cov));
    workarea_->addWidget(std::move(mv));
}

void App::buildTopbar()
{
    topbar_->setStyleClass("app-topbar");

    // Left: Project context breadcrumb
    auto leftSection = topbar_->addWidget(std::make_unique<Wt::WContainerWidget>());
    leftSection->setStyleClass("topbar-left");
    leftSection->addWidget(ppc::xhtml(
        "<span class=\"topbar-project\">" + data_.projectName + "</span>"
        "<span class=\"topbar-sep\">&#8250;</span>"
        "<span class=\"topbar-client\">" + data_.clientName + "</span>"
    ));

    // Right: Theme toggle + User profile
    auto rightSection = topbar_->addWidget(std::make_unique<Wt::WContainerWidget>());
    rightSection->setStyleClass("topbar-right");

    // Theme toggle
    themeToggle_ = rightSection->addWidget(std::make_unique<Wt::WPushButton>());
    themeToggle_->setStyleClass("topbar-btn theme-toggle-btn");
    themeToggle_->setTextFormat(Wt::TextFormat::XHTML);
    themeToggle_->setText("<span class=\"toggle-icon\">&#9789;</span> Dark");
    themeToggle_->clicked().connect(this, &App::toggleTheme);

    // Separator
    rightSection->addWidget(ppc::xhtml(
        "<span class=\"topbar-sep\">|</span>"
    ));

    // User profile button — opens dropdown with role switcher
    userBtn_ = rightSection->addWidget(std::make_unique<Wt::WPushButton>());
    userBtn_->setTextFormat(Wt::TextFormat::XHTML);
    userBtn_->setStyleClass("topbar-user-btn");
    updateUserButton();

    // Create popup menu
    auto popup = std::make_unique<Wt::WPopupMenu>();
    popup->setStyleClass("user-dropdown");

    // Header label (non-clickable)
    auto headerItem = popup->addItem("SWITCH ROLE");
    headerItem->setDisabled(true);
    headerItem->setStyleClass("dropdown-header");

    // Planning role option
    auto planningItem = popup->addItem("Planning");
    planningItem->triggered().connect([this]() { switchRole(AppRole::Planning); });

    // Execution role option
    auto executionItem = popup->addItem("Execution");
    executionItem->triggered().connect([this]() { switchRole(AppRole::Execution); });

    popup->addSeparator();

    // Settings option
    auto settingsItem = popup->addItem("Settings");
    settingsItem->triggered().connect(this, &App::showSettingsDialog);

    // About option
    auto aboutItem = popup->addItem("About Imagery ERP");
    aboutItem->triggered().connect(this, &App::showAboutDialog);

    userBtn_->setMenu(std::move(popup));
}

void App::buildSidebar()
{
    sidebar_->setStyleClass("app-sidebar");

    // ── Brand area (logo + business name, centered) ──
    auto brand = sidebar_->addWidget(std::make_unique<Wt::WContainerWidget>());
    brand->setStyleClass("sidebar-brand");

    // Logo — white variant used on the dark sidebar background in both themes
    auto logoContainer = brand->addWidget(std::make_unique<Wt::WContainerWidget>());
    logoContainer->setStyleClass("sidebar-logo");
    auto logoImg = logoContainer->addWidget(std::make_unique<Wt::WImage>("images/ImageryBusinessLogo_white.png"));
    logoImg->setStyleClass("sidebar-logo-img");
    logoImg->setAlternateText("Imagery Solutions");

    // Business name (configurable via Settings)
    brandNameText_ = brand->addWidget(std::make_unique<Wt::WText>());
    brandNameText_->setTextFormat(Wt::TextFormat::XHTML);
    brandNameText_->setText("<div class=\"sidebar-business-name\">" + data_.company.name + "</div>");

    brand->addWidget(ppc::xhtml(
        "<div class=\"sidebar-app-name\">Project Planner</div>"
    ));

    // ── Separator ──
    sidebar_->addWidget(ppc::xhtml("<div class=\"sidebar-sep\"></div>"));

    // ── Project info ──
    auto header = sidebar_->addWidget(std::make_unique<Wt::WContainerWidget>());
    header->setStyleClass("sidebar-header");
    header->addWidget(ppc::xhtml(
        "<div class=\"sidebar-project-label\">PROJECT</div>"
        "<div class=\"sidebar-project-name\">" + data_.projectName + "</div>"
        "<div class=\"sidebar-client\">" + data_.clientName + "</div>"
    ));

    // ── Separator ──
    sidebar_->addWidget(ppc::xhtml("<div class=\"sidebar-sep\"></div>"));

    // ── Navigation container (rebuilt on role switch) ──
    navContainer_ = sidebar_->addWidget(std::make_unique<Wt::WContainerWidget>());
    navContainer_->setStyleClass("sidebar-nav");
    buildNavForRole();

    // ── Sidebar footer: Settings + Version ──
    auto footer = sidebar_->addWidget(std::make_unique<Wt::WContainerWidget>());
    footer->setStyleClass("sidebar-footer");

    footer->addWidget(ppc::xhtml("<div class=\"sidebar-sep\"></div>"));

    auto settingsBtn = footer->addWidget(std::make_unique<Wt::WPushButton>());
    settingsBtn->setTextFormat(Wt::TextFormat::XHTML);
    settingsBtn->setText(
        "<span class=\"nav-icon\">&#9881;</span>"
        "<span class=\"nav-label\">Settings</span>"
    );
    settingsBtn->setStyleClass("sidebar-nav-btn sidebar-settings-btn");
    settingsBtn->clicked().connect(this, &App::showSettingsDialog);

    footer->addWidget(ppc::xhtml(
        "<div class=\"sidebar-version\">v1.0.0</div>"
    ));
}

// Build navigation items based on current role
void App::buildNavForRole()
{
    navContainer_->clear();
    navButtons_.clear();
    navViewMap_.clear();

    struct NavItem { std::string icon; std::string label; int viewIndex; bool sepAfter; };
    std::vector<NavItem> items;

    if (activeRole_ == AppRole::Planning) {
        items = {
            {"&#9632;", "Dashboard",            0, true},
            {"&#9881;", "Roles &amp; Rates",    1, false},
            {"&#9638;", "Components &amp; SoW", 2, false},
            {"&#9863;", "Materials &amp; Expenses", 7, true},
            {"&#9733;", "Cost Analysis",        4, false},
            {"&#9830;", "Quote Builder",        5, false},
        };
    } else {
        items = {
            {"&#9632;", "Dashboard",            0, true},
            {"&#9654;", "Sprints &amp; Agile",  3, false},
            {"&#8635;", "Change Orders",        6, false},
        };
    }

    for (int i = 0; i < (int)items.size(); i++) {
        auto btn = navContainer_->addWidget(std::make_unique<Wt::WPushButton>());
        btn->setTextFormat(Wt::TextFormat::XHTML);
        btn->setText(
            "<span class=\"nav-icon\">" + items[i].icon + "</span>"
            "<span class=\"nav-label\">" + items[i].label + "</span>"
        );
        btn->setStyleClass("sidebar-nav-btn");
        int viewIdx = items[i].viewIndex;
        btn->clicked().connect([this, viewIdx]() { showView(viewIdx); });
        navButtons_.push_back(btn);
        navViewMap_.push_back(viewIdx);

        if (items[i].sepAfter) {
            navContainer_->addWidget(ppc::xhtml("<div class=\"sidebar-nav-sep\"></div>"));
        }
    }
}

void App::switchRole(AppRole role)
{
    if (role == activeRole_) return;
    activeRole_ = role;

    // Update user button text (name, title, role icon)
    updateUserButton();

    // Rebuild navigation for new role
    buildNavForRole();

    // Show Dashboard (always index 0 in workarea)
    showView(0);
}

void App::updateUserButton()
{
    std::string roleIcon, roleClass, jobTitle;
    if (activeRole_ == AppRole::Planning) {
        roleIcon  = "&#9998;";   // pencil / planning icon
        roleClass = "role-icon planning";
        jobTitle  = "Planning Manager";
    } else {
        roleIcon  = "&#9654;";   // play / execution icon
        roleClass = "role-icon execution";
        jobTitle  = "Project Manager";
    }

    userBtn_->setText(
        "<span class=\"" + roleClass + "\">" + roleIcon + "</span>"
        "<span class=\"user-avatar\">SC</span>"
        "<span class=\"user-info\">"
        "<span class=\"user-name\">Sarah Chen</span>"
        "<span class=\"user-title\">" + jobTitle + "</span>"
        "</span>"
        "<span class=\"user-chevron\">&#9662;</span>"
    );
}

void App::showView(int viewIndex)
{
    if (viewIndex < 0 || viewIndex > 7) return;

    // Hide all views
    for (int i = 0; i < workarea_->count(); i++) {
        workarea_->widget(i)->setHidden(true);
    }

    // Show selected view
    workarea_->widget(viewIndex)->setHidden(false);
    activeNavIndex_ = viewIndex;

    // Highlight the nav button that maps to this view
    for (int i = 0; i < (int)navViewMap_.size(); i++) {
        if (navViewMap_[i] == viewIndex) {
            setNavActive(i);
            break;
        }
    }

    refreshView(viewIndex);
}

void App::setNavActive(int navIdx)
{
    for (int i = 0; i < (int)navButtons_.size(); i++) {
        if (i == navIdx)
            navButtons_[i]->setStyleClass("sidebar-nav-btn active");
        else
            navButtons_[i]->setStyleClass("sidebar-nav-btn");
    }
}

void App::refreshView(int viewIndex)
{
    switch (viewIndex) {
        case 0: dashboardView_->refresh();   break;
        case 1: resourceView_->refresh();    break;
        case 2: componentView_->refresh();   break;
        case 3: sprintView_->refresh();      break;
        case 4: costingView_->refresh();     break;
        case 5: quoteView_->refresh();       break;
        case 6: changeOrderView_->refresh(); break;
        case 7: materialView_->refresh();    break;
    }
}

void App::showSettingsDialog()
{
    auto dialog = addChild(std::make_unique<Wt::WDialog>("Settings"));
    dialog->setStyleClass("settings-dialog");
    dialog->setModal(true);
    dialog->setMovable(true);
    dialog->rejectWhenEscapePressed(true);
    dialog->setWidth(Wt::WLength(420));

    auto content = dialog->contents();
    content->setStyleClass("settings-content");

    // Business Name field
    content->addWidget(ppc::xhtml(
        "<div class=\"settings-section-title\">Company</div>"
    ));

    auto row = content->addWidget(std::make_unique<Wt::WContainerWidget>());
    row->setStyleClass("settings-field");
    auto label = row->addWidget(std::make_unique<Wt::WLabel>("Business Name"));
    label->setStyleClass("field-label");
    auto nameEdit = row->addWidget(std::make_unique<Wt::WLineEdit>(data_.company.name));
    nameEdit->setStyleClass("input-field input-wide");
    label->setBuddy(nameEdit);

    // Logo hint
    content->addWidget(ppc::xhtml(
        "<div class=\"settings-hint\">To change the logo, replace "
        "<strong>ImageryBusinessLogo.png</strong> (light) and "
        "<strong>ImageryBusinessLogo_white.png</strong> (dark) "
        "in the resources/images directory.</div>"
    ));

    // Footer buttons
    auto footer = dialog->footer();
    footer->setStyleClass("settings-footer");

    auto cancelBtn = footer->addWidget(std::make_unique<Wt::WPushButton>("Cancel"));
    cancelBtn->setStyleClass("btn btn-secondary");
    cancelBtn->clicked().connect(dialog, &Wt::WDialog::reject);

    auto saveBtn = footer->addWidget(std::make_unique<Wt::WPushButton>("Save"));
    saveBtn->setStyleClass("btn btn-primary");

    saveBtn->clicked().connect([this, dialog, nameEdit]() {
        auto newName = nameEdit->text().toUTF8();
        if (!newName.empty()) {
            data_.company.name = newName;
            brandNameText_->setText("<div class=\"sidebar-business-name\">" + newName + "</div>");
            setTitle("Project Planning & Costing — " + newName);
        }
        dialog->accept();
    });

    dialog->show();
}

void App::showAboutDialog()
{
    auto dialog = addChild(std::make_unique<Wt::WDialog>("About Imagery ERP"));
    dialog->setStyleClass("settings-dialog about-dialog");
    dialog->setModal(true);
    dialog->setMovable(true);
    dialog->rejectWhenEscapePressed(true);
    dialog->setWidth(Wt::WLength(480));

    // Close X button in titlebar
    auto closeX = dialog->titleBar()->addWidget(std::make_unique<Wt::WText>("&times;"));
    closeX->setStyleClass("about-close-x");
    closeX->clicked().connect(dialog, &Wt::WDialog::accept);

    auto content = dialog->contents();
    content->setStyleClass("about-content");

    // Centered logo
    auto logoImg = content->addWidget(std::make_unique<Wt::WImage>("images/ImageryBusinessLogo.png"));
    logoImg->setStyleClass("about-logo");
    logoImg->setAlternateText("Imagery Solutions");

    // Application description
    content->addWidget(ppc::xhtml(
        "<div class=\"about-description\">"
        "Imagery ERP is a comprehensive project planning and costing platform "
        "built for professional services firms. Plan resource allocations, "
        "manage materials and expenses, build detailed cost analyses, "
        "generate client-ready quotes, and track execution through agile sprints "
        "and change orders &mdash; all in one integrated workspace."
        "</div>"
    ));

    // Version and copyright
    content->addWidget(ppc::xhtml(
        "<div class=\"about-meta\">"
        "<div class=\"about-version\">Version 1.0.0</div>"
        "<div class=\"about-copyright\">&copy; 2026 Imagery Solutions. All rights reserved.</div>"
        "</div>"
    ));

    // Close button
    auto footer = dialog->footer();
    footer->setStyleClass("settings-footer about-footer");
    auto closeBtn = footer->addWidget(std::make_unique<Wt::WPushButton>("Close"));
    closeBtn->setStyleClass("btn btn-primary");
    closeBtn->clicked().connect(dialog, &Wt::WDialog::accept);

    dialog->show();

    // Close dialog when clicking on the backdrop overlay
    doJavaScript(
        "setTimeout(function(){"
        "  var covers = document.querySelectorAll('.Wt-dialogcover');"
        "  var cover = covers[covers.length - 1];"
        "  if(cover) cover.addEventListener('click', function(){"
        "    " + closeBtn->jsRef() + ".click();"
        "  });"
        "}, 0);"
    );
}

void App::toggleTheme()
{
    darkMode_ = !darkMode_;
    if (darkMode_) {
        appShell_->setStyleClass("app-shell theme-dark");
        themeToggle_->setText("<span class=\"toggle-icon\">&#9788;</span> Light");
    } else {
        appShell_->setStyleClass("app-shell theme-light");
        themeToggle_->setText("<span class=\"toggle-icon\">&#9789;</span> Dark");
    }
}
