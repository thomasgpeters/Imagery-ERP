#pragma once
#include <Wt/WContainerWidget.h>
#include "DataModels.h"

class SprintView : public Wt::WContainerWidget {
public:
    SprintView(ppc::ProjectData& data);
    void refresh();
private:
    ppc::ProjectData& data_;
    Wt::WContainerWidget* content_ = nullptr;
};
