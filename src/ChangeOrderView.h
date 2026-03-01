#pragma once
#include <Wt/WContainerWidget.h>
#include "DataModels.h"

class ChangeOrderView : public Wt::WContainerWidget {
public:
    ChangeOrderView(ppc::ProjectData& data);
    void refresh();
private:
    ppc::ProjectData& data_;
    Wt::WContainerWidget* content_ = nullptr;
};
