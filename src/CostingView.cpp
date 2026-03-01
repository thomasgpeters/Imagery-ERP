// =============================================================================
// CostingView.cpp — Hours allocation grid + fully loaded cost grid + phase costs
// =============================================================================

#include "CostingView.h"
#include <Wt/WText.h>
#include <Wt/WTable.h>
#include <Wt/WDoubleSpinBox.h>
#include <Wt/WPushButton.h>
#include <Wt/WLineEdit.h>
#include <Wt/WSpinBox.h>

CostingView::CostingView(ppc::ProjectData& data)
    : data_(data)
{
    setStyleClass("view-panel");
    content_ = addWidget(std::make_unique<Wt::WContainerWidget>());
}

void CostingView::updateTotals()
{
    for (int r = 0; r < (int)rowTotals_.size() && r < (int)data_.roles.size(); r++) {
        double h = data_.getRoleTotalHours(data_.roles[r].id);
        rowTotals_[r]->setText(ppc::formatNumber(h, 0));
    }
    for (int w = 0; w < (int)colTotals_.size() && w < data_.totalWeeks; w++) {
        double h = data_.getWeekTotalHours(w + 1);
        colTotals_[w]->setText(ppc::formatNumber(h, 0));
    }
    if (grandTotal_) {
        grandTotal_->setText(ppc::formatNumber(data_.getTotalHours(), 0));
    }
}

void CostingView::refresh()
{
    content_->clear();
    rowTotals_.clear();
    colTotals_.clear();
    grandTotal_ = nullptr;

    content_->addWidget(std::make_unique<Wt::WText>(
        "<h2 class=\"view-title\">Cost Analysis</h2>"
        "<p class=\"view-subtitle\">Resource time allocation and fully loaded cost breakdown</p>"
    ));

    // ---- Project Hours Grid -------------------------------------------------
    content_->addWidget(std::make_unique<Wt::WText>("<h3 class=\"section-title\">Project Hours</h3>"));

    auto scrollBox = content_->addWidget(std::make_unique<Wt::WContainerWidget>());
    scrollBox->setStyleClass("grid-scroll");

    auto hrsTable = scrollBox->addWidget(std::make_unique<Wt::WTable>());
    hrsTable->setStyleClass("data-table grid-table");
    hrsTable->setHeaderCount(1);

    // Header: Role | W1..WN | Total
    hrsTable->elementAt(0, 0)->addWidget(std::make_unique<Wt::WText>("Role"));
    hrsTable->elementAt(0, 0)->setStyleClass("cell-sticky");
    for (int w = 0; w < data_.totalWeeks; w++) {
        hrsTable->elementAt(0, w + 1)->addWidget(std::make_unique<Wt::WText>("W" + std::to_string(w + 1)));
        hrsTable->elementAt(0, w + 1)->setStyleClass("cell-center cell-week");
    }
    hrsTable->elementAt(0, data_.totalWeeks + 1)->addWidget(std::make_unique<Wt::WText>("Total"));
    hrsTable->elementAt(0, data_.totalWeeks + 1)->setStyleClass("cell-right cell-bold");

    // Data rows
    for (int r = 0; r < (int)data_.roles.size(); r++) {
        int row = r + 1;
        auto& role = data_.roles[r];

        hrsTable->elementAt(row, 0)->addWidget(std::make_unique<Wt::WText>(
            role.name + " (" + ppc::formatCurrency(role.fullyLoadedRate()) + "/hr)"));
        hrsTable->elementAt(row, 0)->setStyleClass("cell-sticky cell-role");

        for (int w = 0; w < data_.totalWeeks; w++) {
            double hrs = data_.getAllocatedHours(role.id, w + 1);
            auto spin = hrsTable->elementAt(row, w + 1)->addWidget(
                std::make_unique<Wt::WDoubleSpinBox>());
            spin->setRange(0, 80);
            spin->setValue(hrs);
            spin->setDecimals(0);
            spin->setSingleStep(4);
            spin->setStyleClass("input-grid");
            hrsTable->elementAt(row, w + 1)->setStyleClass("cell-grid");

            int roleId = role.id;
            int weekNum = w + 1;
            spin->valueChanged().connect([this, roleId, weekNum](double val) {
                data_.setAllocatedHours(roleId, weekNum, val);
                updateTotals();
            });
        }

        // Row total
        auto rowT = hrsTable->elementAt(row, data_.totalWeeks + 1)->addWidget(
            std::make_unique<Wt::WText>(ppc::formatNumber(data_.getRoleTotalHours(role.id), 0)));
        hrsTable->elementAt(row, data_.totalWeeks + 1)->setStyleClass("cell-right cell-bold");
        rowTotals_.push_back(rowT);
    }

    // Column totals row
    int trow = (int)data_.roles.size() + 1;
    hrsTable->elementAt(trow, 0)->addWidget(std::make_unique<Wt::WText>("Total"));
    hrsTable->elementAt(trow, 0)->setStyleClass("cell-sticky total-row");
    for (int w = 0; w < data_.totalWeeks; w++) {
        auto colT = hrsTable->elementAt(trow, w + 1)->addWidget(
            std::make_unique<Wt::WText>(ppc::formatNumber(data_.getWeekTotalHours(w + 1), 0)));
        hrsTable->elementAt(trow, w + 1)->setStyleClass("cell-right total-row");
        colTotals_.push_back(colT);
    }
    grandTotal_ = hrsTable->elementAt(trow, data_.totalWeeks + 1)->addWidget(
        std::make_unique<Wt::WText>(ppc::formatNumber(data_.getTotalHours(), 0)));
    hrsTable->elementAt(trow, data_.totalWeeks + 1)->setStyleClass("cell-right cell-bold total-row");

    // ---- Fully Loaded Cost Grid (read-only) ---------------------------------
    content_->addWidget(std::make_unique<Wt::WText>("<h3 class=\"section-title\">Weekly — Fully Loaded Costs</h3>"));

    auto costScroll = content_->addWidget(std::make_unique<Wt::WContainerWidget>());
    costScroll->setStyleClass("grid-scroll");

    auto costTable = costScroll->addWidget(std::make_unique<Wt::WTable>());
    costTable->setStyleClass("data-table grid-table");
    costTable->setHeaderCount(1);

    costTable->elementAt(0, 0)->addWidget(std::make_unique<Wt::WText>("Role"));
    costTable->elementAt(0, 0)->setStyleClass("cell-sticky");
    for (int w = 0; w < data_.totalWeeks; w++) {
        costTable->elementAt(0, w + 1)->addWidget(std::make_unique<Wt::WText>("W" + std::to_string(w + 1)));
        costTable->elementAt(0, w + 1)->setStyleClass("cell-center cell-week");
    }
    costTable->elementAt(0, data_.totalWeeks + 1)->addWidget(std::make_unique<Wt::WText>("Total"));
    costTable->elementAt(0, data_.totalWeeks + 1)->setStyleClass("cell-right cell-bold");

    for (int r = 0; r < (int)data_.roles.size(); r++) {
        int row = r + 1;
        auto& role = data_.roles[r];

        costTable->elementAt(row, 0)->addWidget(std::make_unique<Wt::WText>(role.name));
        costTable->elementAt(row, 0)->setStyleClass("cell-sticky cell-role");

        for (int w = 0; w < data_.totalWeeks; w++) {
            double hrs = data_.getAllocatedHours(role.id, w + 1);
            double cost = hrs * role.fullyLoadedRate();
            std::string cls = cost > 0 ? "cell-right cell-cost" : "cell-right cell-cost-zero";
            costTable->elementAt(row, w + 1)->addWidget(
                std::make_unique<Wt::WText>(cost > 0 ? ppc::formatCurrency(cost) : "—"));
            costTable->elementAt(row, w + 1)->setStyleClass(cls);
        }

        costTable->elementAt(row, data_.totalWeeks + 1)->addWidget(
            std::make_unique<Wt::WText>(ppc::formatCurrency(data_.getRoleTotalCost(role.id))));
        costTable->elementAt(row, data_.totalWeeks + 1)->setStyleClass("cell-right cell-bold");
    }

    trow = (int)data_.roles.size() + 1;
    costTable->elementAt(trow, 0)->addWidget(std::make_unique<Wt::WText>("Total"));
    costTable->elementAt(trow, 0)->setStyleClass("cell-sticky total-row");
    for (int w = 0; w < data_.totalWeeks; w++) {
        costTable->elementAt(trow, w + 1)->addWidget(
            std::make_unique<Wt::WText>(ppc::formatCurrency(data_.getWeekTotalCost(w + 1))));
        costTable->elementAt(trow, w + 1)->setStyleClass("cell-right total-row");
    }
    costTable->elementAt(trow, data_.totalWeeks + 1)->addWidget(
        std::make_unique<Wt::WText>(ppc::formatCurrency(data_.getTotalCost())));
    costTable->elementAt(trow, data_.totalWeeks + 1)->setStyleClass("cell-right cell-bold total-row");

    // ---- Phased Pricing (Cost + Markup) -------------------------------------
    content_->addWidget(std::make_unique<Wt::WText>(
        "<h3 class=\"section-title\">Phased Pricing (Cost + " +
        ppc::formatPercent(data_.markupPct) + ")</h3>"));

    auto phaseTable = content_->addWidget(std::make_unique<Wt::WTable>());
    phaseTable->setStyleClass("data-table");
    phaseTable->setHeaderCount(1);

    int c = 0;
    for (auto& h : {"Phase", "Weeks", "Hours", "Cost", "Sell Price", "Margin"}) {
        phaseTable->elementAt(0, c)->addWidget(std::make_unique<Wt::WText>(h));
        phaseTable->elementAt(0, c)->setStyleClass(c >= 2 ? "cell-right" : "");
        c++;
    }

    double tph = 0, tpc = 0, tps = 0;
    for (int p = 0; p < (int)data_.phases.size(); p++) {
        int row = p + 1;
        auto& ph = data_.phases[p];
        double hrs = data_.getPhaseHours(p);
        double cost = data_.getPhaseCost(p);
        double sell = data_.getPhaseSell(p);
        tph += hrs; tpc += cost; tps += sell;

        c = 0;
        phaseTable->elementAt(row, c++)->addWidget(std::make_unique<Wt::WText>(ph.name));
        phaseTable->elementAt(row, c++)->addWidget(std::make_unique<Wt::WText>(
            "W" + std::to_string(ph.startWeek) + " – W" + std::to_string(ph.endWeek)));
        phaseTable->elementAt(row, c)->addWidget(std::make_unique<Wt::WText>(ppc::formatNumber(hrs, 0)));
        phaseTable->elementAt(row, c++)->setStyleClass("cell-right");
        phaseTable->elementAt(row, c)->addWidget(std::make_unique<Wt::WText>(ppc::formatCurrency(cost)));
        phaseTable->elementAt(row, c++)->setStyleClass("cell-right");
        phaseTable->elementAt(row, c)->addWidget(std::make_unique<Wt::WText>(ppc::formatCurrency(sell)));
        phaseTable->elementAt(row, c++)->setStyleClass("cell-right");
        phaseTable->elementAt(row, c)->addWidget(std::make_unique<Wt::WText>(ppc::formatCurrency(sell - cost)));
        phaseTable->elementAt(row, c)->setStyleClass("cell-right");
    }

    trow = (int)data_.phases.size() + 1;
    phaseTable->elementAt(trow, 0)->addWidget(std::make_unique<Wt::WText>("Total"));
    phaseTable->elementAt(trow, 1)->addWidget(std::make_unique<Wt::WText>(""));
    phaseTable->elementAt(trow, 2)->addWidget(std::make_unique<Wt::WText>(ppc::formatNumber(tph, 0)));
    phaseTable->elementAt(trow, 2)->setStyleClass("cell-right cell-bold");
    phaseTable->elementAt(trow, 3)->addWidget(std::make_unique<Wt::WText>(ppc::formatCurrency(tpc)));
    phaseTable->elementAt(trow, 3)->setStyleClass("cell-right cell-bold");
    phaseTable->elementAt(trow, 4)->addWidget(std::make_unique<Wt::WText>(ppc::formatCurrency(tps)));
    phaseTable->elementAt(trow, 4)->setStyleClass("cell-right cell-bold");
    phaseTable->elementAt(trow, 5)->addWidget(std::make_unique<Wt::WText>(ppc::formatCurrency(tps - tpc)));
    phaseTable->elementAt(trow, 5)->setStyleClass("cell-right cell-bold");
    for (int i = 0; i < 6; i++)
        phaseTable->elementAt(trow, i)->addStyleClass("total-row");

    // Recalculate button
    auto recalcBtn = content_->addWidget(std::make_unique<Wt::WPushButton>("Recalculate All"));
    recalcBtn->setStyleClass("btn btn-secondary mt-1");
    recalcBtn->clicked().connect([this]() { refresh(); });
}
