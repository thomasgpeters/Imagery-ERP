#pragma once
#include <Wt/WContainerWidget.h>
#include "DataModels.h"

class CostingView : public Wt::WContainerWidget {
public:
    CostingView(ppc::ProjectData& data);
    void refresh();
private:
    ppc::ProjectData& data_;
    Wt::WContainerWidget* content_ = nullptr;

    // References for dynamic total updates
    std::vector<Wt::WText*> rowTotals_;
    std::vector<Wt::WText*> colTotals_;
    Wt::WText* grandTotal_ = nullptr;
    void updateTotals();
};
