#pragma once
// =============================================================================
// App.h — Main Wt Application with branded sidebar + topnav + workarea layout
// =============================================================================

#include <Wt/WApplication.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WTabWidget.h>
#include <Wt/WText.h>
#include <Wt/WPushButton.h>
#include "DataModels.h"

class DashboardView;
class ResourceView;
class ComponentView;
class SprintView;
class CostingView;
class QuoteView;
class ChangeOrderView;

class App : public Wt::WApplication {
public:
    App(const Wt::WEnvironment& env);

private:
    ppc::ProjectData data_;

    // Layout containers
    Wt::WContainerWidget* root_;
    Wt::WContainerWidget* sidebar_;
    Wt::WContainerWidget* topbar_;
    Wt::WContainerWidget* workarea_;

    // Navigation buttons in sidebar
    std::vector<Wt::WPushButton*> navButtons_;
    int activeNavIndex_ = 0;

    // Theme state
    bool darkMode_ = false;
    Wt::WPushButton* themeToggle_ = nullptr;
    Wt::WContainerWidget* appShell_ = nullptr;

    // Sidebar brand (updated by Settings)
    Wt::WText* brandNameText_ = nullptr;

    // Views
    DashboardView*   dashboardView_   = nullptr;
    ResourceView*    resourceView_    = nullptr;
    ComponentView*   componentView_   = nullptr;
    SprintView*      sprintView_      = nullptr;
    CostingView*     costingView_     = nullptr;
    QuoteView*       quoteView_       = nullptr;
    ChangeOrderView* changeOrderView_ = nullptr;

    void buildLayout();
    void buildSidebar();
    void buildTopbar();
    void showView(int index);
    void toggleTheme();
    void setNavActive(int index);
    void refreshCurrentView();
    void showSettingsDialog();
};
