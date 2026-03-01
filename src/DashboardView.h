#pragma once
#include <Wt/WContainerWidget.h>
#include "DataModels.h"

class DashboardView : public Wt::WContainerWidget {
public:
    DashboardView(ppc::ProjectData& data);
    void refresh();
private:
    ppc::ProjectData& data_;
    Wt::WContainerWidget* content_ = nullptr;
};
