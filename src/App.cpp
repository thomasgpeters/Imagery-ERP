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
    // App shell wrapping everything (for theme class toggling)
    appShell_ = root()->addWidget(std::make_unique<Wt::WContainerWidget>());
    appShell_->setStyleClass("app-shell theme-light");

    // Top navigation bar
    topbar_ = appShell_->addWidget(std::make_unique<Wt::WContainerWidget>());
    buildTopbar();

    // Body: sidebar + workarea
    auto body = appShell_->addWidget(std::make_unique<Wt::WContainerWidget>());
    body->setStyleClass("app-body");

    sidebar_ = body->addWidget(std::make_unique<Wt::WContainerWidget>());
    buildSidebar();

    workarea_ = body->addWidget(std::make_unique<Wt::WContainerWidget>());
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

    // Left: Brand
    auto brand = topbar_->addWidget(std::make_unique<Wt::WContainerWidget>());
    brand->setStyleClass("topbar-brand");
    brand->addWidget(ppc::xhtml(
        "<span class=\"brand-icon\">&#9672;</span>"
        "<span class=\"brand-name\">Imagery Solutions</span>"
        "<span class=\"brand-divider\">|</span>"
        "<span class=\"brand-app\">Project Planner</span>"
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

    // Sidebar header
    auto header = sidebar_->addWidget(std::make_unique<Wt::WContainerWidget>());
    header->setStyleClass("sidebar-header");
    header->addWidget(ppc::xhtml(
        "<div class=\"sidebar-project-label\">PROJECT</div>"
        "<div class=\"sidebar-project-name\">" + data_.projectName + "</div>"
        "<div class=\"sidebar-client\">" + data_.clientName + "</div>"
    ));

    // Navigation items
    struct NavItem { std::string icon; std::string label; };
    std::vector<NavItem> navItems = {
        {"&#9632;", "Dashboard"},
        {"&#9881;", "Roles &amp; Rates"},
        {"&#9638;", "Components &amp; SoW"},
        {"&#9654;", "Sprints &amp; Agile"},
        {"&#9733;", "Cost Analysis"},
        {"&#9830;", "Quote Builder"},
        {"&#8635;", "Change Orders"},
    };

    auto nav = sidebar_->addWidget(std::make_unique<Wt::WContainerWidget>());
    nav->setStyleClass("sidebar-nav");

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
    }

    // Sidebar footer
    auto footer = sidebar_->addWidget(std::make_unique<Wt::WContainerWidget>());
    footer->setStyleClass("sidebar-footer");
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
