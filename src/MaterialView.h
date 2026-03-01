#pragma once
#include <Wt/WContainerWidget.h>
#include "DataModels.h"

class MaterialView : public Wt::WContainerWidget {
public:
    MaterialView(ppc::ProjectData& data);
    void refresh();
private:
    ppc::ProjectData& data_;
    Wt::WContainerWidget* content_ = nullptr;
};
