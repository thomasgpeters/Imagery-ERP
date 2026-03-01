#pragma once
#include <Wt/WContainerWidget.h>
#include "DataModels.h"

class QuoteView : public Wt::WContainerWidget {
public:
    QuoteView(ppc::ProjectData& data);
    void refresh();
private:
    ppc::ProjectData& data_;
    Wt::WContainerWidget* content_ = nullptr;
};
