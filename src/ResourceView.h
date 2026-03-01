#pragma once
#include <Wt/WContainerWidget.h>
#include "DataModels.h"

class ResourceView : public Wt::WContainerWidget {
public:
    ResourceView(ppc::ProjectData& data);
    void refresh();
private:
    ppc::ProjectData& data_;
    Wt::WContainerWidget* content_ = nullptr;
};
