#pragma once
#include <Wt/WContainerWidget.h>
#include "DataModels.h"

class ComponentView : public Wt::WContainerWidget {
public:
    ComponentView(ppc::ProjectData& data);
    void refresh();
private:
    ppc::ProjectData& data_;
    Wt::WContainerWidget* content_ = nullptr;
    int expandedComponent_ = -1; // which component SoW is expanded
};
