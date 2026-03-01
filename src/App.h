#pragma once
// =============================================================================
// App.h — Main Wt Application with branded sidebar + topnav + workarea layout
// =============================================================================

#include <Wt/WApplication.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WTabWidget.h>
#include <Wt/WText.h>
#include <Wt/WPushButton.h>
#include <Wt/WPopupMenu.h>
#include "DataModels.h"

class DashboardView;
class ResourceView;
class ComponentView;
class SprintView;
class CostingView;
class QuoteView;
class ChangeOrderView;
class MaterialView;

enum class AppRole { Planning, Execution };

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

    // Navigation
    Wt::WContainerWidget* navContainer_ = nullptr;
    std::vector<Wt::WPushButton*> navButtons_;
    int activeNavIndex_ = 0;

    // Role state
    AppRole activeRole_ = AppRole::Planning;
    Wt::WPushButton* userBtn_ = nullptr;

    // Each nav button maps to a view index in the workarea
    std::vector<int> navViewMap_;

    // Theme state
    bool darkMode_ = false;
    Wt::WPushButton* themeToggle_ = nullptr;
    Wt::WContainerWidget* appShell_ = nullptr;

    // Sidebar brand (updated by Settings)
    Wt::WText* brandNameText_ = nullptr;

    // Views — stored in workarea in fixed order:
    //   0=Dashboard, 1=Roles&Rates, 2=Components, 3=Sprints,
    //   4=CostAnalysis, 5=QuoteBuilder, 6=ChangeOrders, 7=Materials
    DashboardView*   dashboardView_   = nullptr;
    ResourceView*    resourceView_    = nullptr;
    ComponentView*   componentView_   = nullptr;
    SprintView*      sprintView_      = nullptr;
    CostingView*     costingView_     = nullptr;
    QuoteView*       quoteView_       = nullptr;
    ChangeOrderView* changeOrderView_ = nullptr;
    MaterialView*    materialView_    = nullptr;

    void buildLayout();
    void buildSidebar();
    void buildTopbar();
    void buildNavForRole();
    void switchRole(AppRole role);
    void updateUserButton();
    void showView(int viewIndex);
    void toggleTheme();
    void setNavActive(int navIdx);
    void refreshView(int viewIndex);
    void showSettingsDialog();
};
