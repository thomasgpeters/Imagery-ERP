// =============================================================================
// App.cpp — Main application with branded sidebar, topnav, workarea, theme toggle
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
    auto dv  = std::make_unique<DashboardView>(data_);   dashboardView_   = dv.get();
    auto rv  = std::make_unique<ResourceView>(data_);    resourceView_    = rv.get();
    auto cv  = std::make_unique<ComponentView>(data_);   componentView_   = cv.get();
    auto sv  = std::make_unique<SprintView>(data_);      sprintView_      = sv.get();
    auto csv = std::make_unique<CostingView>(data_);     costingView_     = csv.get();
    auto qv  = std::make_unique<QuoteView>(data_);       quoteView_       = qv.get();
    auto cov = std::make_unique<ChangeOrderView>(data_); changeOrderView_ = cov.get();

    workarea_->addWidget(std::move(dv));
    workarea_->addWidget(std::move(rv));
    workarea_->addWidget(std::move(cv));
    workarea_->addWidget(std::move(sv));
    workarea_->addWidget(std::move(csv));
    workarea_->addWidget(std::move(qv));
    workarea_->addWidget(std::move(cov));
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

    // User profile
    auto userBtn = rightSection->addWidget(std::make_unique<Wt::WContainerWidget>());
    userBtn->setStyleClass("topbar-user");
    userBtn->addWidget(ppc::xhtml(
        "<span class=\"user-avatar\">SC</span>"
        "<span class=\"user-name\">Sarah Chen</span>"
        "<span class=\"user-role\">Project Manager</span>"
    ));
}

void App::buildSidebar()
{
    sidebar_->setStyleClass("app-sidebar");

    // ── Brand area (logo + business name, centered) ──
    auto brand = sidebar_->addWidget(std::make_unique<Wt::WContainerWidget>());
    brand->setStyleClass("sidebar-brand");

    // Logo placeholder — shows a fallback icon; user can drop a logo.png in resources
    auto logoContainer = brand->addWidget(std::make_unique<Wt::WContainerWidget>());
    logoContainer->setStyleClass("sidebar-logo");
    auto logoImg = logoContainer->addWidget(std::make_unique<Wt::WImage>("logo.png"));
    logoImg->setStyleClass("sidebar-logo-img");
    logoImg->setAlternateText("");
    // If logo.png doesn't exist, hide image and show fallback icon
    logoImg->setHidden(true);  // hidden by default; shown if file exists
    logoContainer->addWidget(ppc::xhtml(
        "<span class=\"sidebar-logo-fallback\">&#9672;</span>"
    ));

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

    // ── Navigation — Group 1: Overview ──
    auto nav = sidebar_->addWidget(std::make_unique<Wt::WContainerWidget>());
    nav->setStyleClass("sidebar-nav");

    struct NavItem { std::string icon; std::string label; int sepAfter; };
    std::vector<NavItem> navItems = {
        {"&#9632;", "Dashboard",             1},  // separator after Dashboard
        {"&#9881;", "Roles &amp; Rates",     0},
        {"&#9638;", "Components &amp; SoW",  0},
        {"&#9654;", "Sprints &amp; Agile",   1},  // separator after planning group
        {"&#9733;", "Cost Analysis",         0},
        {"&#9830;", "Quote Builder",         0},
        {"&#8635;", "Change Orders",         0},
    };

    for (int i = 0; i < (int)navItems.size(); i++) {
        auto btn = nav->addWidget(std::make_unique<Wt::WPushButton>());
        btn->setTextFormat(Wt::TextFormat::XHTML);
        btn->setText(
            "<span class=\"nav-icon\">" + navItems[i].icon + "</span>"
            "<span class=\"nav-label\">" + navItems[i].label + "</span>"
        );
        btn->setStyleClass("sidebar-nav-btn");
        int idx = i;
        btn->clicked().connect([this, idx]() { showView(idx); });
        navButtons_.push_back(btn);

        // Add separator after this item if flagged
        if (navItems[i].sepAfter) {
            nav->addWidget(ppc::xhtml("<div class=\"sidebar-nav-sep\"></div>"));
        }
    }

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

void App::showView(int index)
{
    if (index < 0 || index > 6) return;

    // Hide all views
    for (int i = 0; i < workarea_->count(); i++) {
        workarea_->widget(i)->setHidden(true);
    }

    // Show selected view
    workarea_->widget(index)->setHidden(false);
    activeNavIndex_ = index;
    setNavActive(index);
    refreshCurrentView();
}

void App::setNavActive(int index)
{
    for (int i = 0; i < (int)navButtons_.size(); i++) {
        if (i == index)
            navButtons_[i]->setStyleClass("sidebar-nav-btn active");
        else
            navButtons_[i]->setStyleClass("sidebar-nav-btn");
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
        "<div class=\"settings-hint\">To change the logo, place a <strong>logo.png</strong> file "
        "in the application's resources directory.</div>"
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

void App::refreshCurrentView()
{
    switch (activeNavIndex_) {
        case 0: dashboardView_->refresh();   break;
        case 1: resourceView_->refresh();    break;
        case 2: componentView_->refresh();   break;
        case 3: sprintView_->refresh();      break;
        case 4: costingView_->refresh();     break;
        case 5: quoteView_->refresh();       break;
        case 6: changeOrderView_->refresh(); break;
    }
}
