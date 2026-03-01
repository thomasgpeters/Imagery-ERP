// =============================================================================
// DashboardView.cpp — Project dashboard with key metrics and summary tables
// =============================================================================

#include "DashboardView.h"
#include <Wt/WText.h>
#include <Wt/WTable.h>

DashboardView::DashboardView(ppc::ProjectData& data)
    : data_(data)
{
    setStyleClass("view-panel");
    content_ = addWidget(std::make_unique<Wt::WContainerWidget>());
}

void DashboardView::refresh()
{
    content_->clear();

    // View title
    content_->addWidget(std::make_unique<Wt::WText>(
        "<h2 class=\"view-title\">Dashboard</h2>"
        "<p class=\"view-subtitle\">" + data_.projectName + " &mdash; " + data_.clientName + "</p>"
    ));

    // ---- Metric cards -------------------------------------------------------
    auto metrics = content_->addWidget(std::make_unique<Wt::WContainerWidget>());
    metrics->setStyleClass("metrics-grid");

    auto addCard = [&](const std::string& label, const std::string& value,
                       const std::string& sub, const std::string& accent) {
        auto card = metrics->addWidget(std::make_unique<Wt::WContainerWidget>());
        card->setStyleClass("metric-card " + accent);
        card->addWidget(std::make_unique<Wt::WText>(
            "<div class=\"mc-label\">" + label + "</div>"
            "<div class=\"mc-value\">" + value + "</div>"
            "<div class=\"mc-sub\">" + sub + "</div>"
        ));
    };

    addCard("Total Hours",       ppc::formatNumber(data_.getTotalHours(), 0), "across all roles", "accent-blue");
    addCard("Total Cost",        ppc::formatCurrency(data_.getTotalCost()),   "fully loaded",      "accent-red");
    addCard("Sell Price",        ppc::formatCurrency(data_.getTotalSell()),   "at " + ppc::formatPercent(data_.markupPct) + " markup", "accent-green");
    addCard("Profit Margin",     ppc::formatCurrency(data_.getMargin()),     ppc::formatPercent(data_.markupPct), "accent-purple");
    addCard("Blended Cost Rate", ppc::formatCurrency(data_.getBlendedCostRate()) + "/hr", "weighted average", "accent-orange");
    addCard("Blended Sell Rate", ppc::formatCurrency(data_.getBlendedSellRate()) + "/hr", "weighted average", "accent-teal");
    addCard("Sprints",           std::to_string(data_.sprints.size()),       std::to_string(data_.sprintLengthWeeks) + "-week intervals", "accent-blue");
    addCard("Components",        std::to_string(data_.components.size()),    "estimable units of work", "accent-orange");

    // ---- Role summary table -------------------------------------------------
    content_->addWidget(std::make_unique<Wt::WText>("<h3 class=\"section-title\">Role Summary</h3>"));
    auto roleTable = content_->addWidget(std::make_unique<Wt::WTable>());
    roleTable->setStyleClass("data-table");
    roleTable->setHeaderCount(1);

    int c = 0;
    for (auto& h : {"Role", "Base Rate", "Overhead", "Fully Loaded", "Total Hours", "Total Cost", "Sell Rate", "Sell Value"}) {
        roleTable->elementAt(0, c)->addWidget(std::make_unique<Wt::WText>(h));
        roleTable->elementAt(0, c)->setStyleClass(c >= 1 ? "cell-right" : "");
        c++;
    }

    for (int r = 0; r < (int)data_.roles.size(); r++) {
        int row = r + 1;
        auto& role = data_.roles[r];
        double sellRate = role.fullyLoadedRate() * (1.0 + data_.markupPct / 100.0);
        double totalHrs = data_.getRoleTotalHours(role.id);
        double totalCost = data_.getRoleTotalCost(role.id);

        c = 0;
        roleTable->elementAt(row, c++)->addWidget(std::make_unique<Wt::WText>(role.name));
        roleTable->elementAt(row, c)->addWidget(std::make_unique<Wt::WText>(ppc::formatCurrency(role.baseRate)));
        roleTable->elementAt(row, c++)->setStyleClass("cell-right");
        roleTable->elementAt(row, c)->addWidget(std::make_unique<Wt::WText>(ppc::formatCurrency(role.overheadRate)));
        roleTable->elementAt(row, c++)->setStyleClass("cell-right");
        roleTable->elementAt(row, c)->addWidget(std::make_unique<Wt::WText>(ppc::formatCurrency(role.fullyLoadedRate())));
        roleTable->elementAt(row, c++)->setStyleClass("cell-right cell-bold");
        roleTable->elementAt(row, c)->addWidget(std::make_unique<Wt::WText>(ppc::formatNumber(totalHrs, 0)));
        roleTable->elementAt(row, c++)->setStyleClass("cell-right");
        roleTable->elementAt(row, c)->addWidget(std::make_unique<Wt::WText>(ppc::formatCurrency(totalCost)));
        roleTable->elementAt(row, c++)->setStyleClass("cell-right");
        roleTable->elementAt(row, c)->addWidget(std::make_unique<Wt::WText>(ppc::formatCurrency(sellRate)));
        roleTable->elementAt(row, c++)->setStyleClass("cell-right");
        roleTable->elementAt(row, c)->addWidget(std::make_unique<Wt::WText>(ppc::formatCurrency(totalCost * (1.0 + data_.markupPct / 100.0))));
        roleTable->elementAt(row, c)->setStyleClass("cell-right");
    }

    // Totals row
    int trow = (int)data_.roles.size() + 1;
    c = 0;
    roleTable->elementAt(trow, c++)->addWidget(std::make_unique<Wt::WText>("Total"));
    roleTable->elementAt(trow, c++)->addWidget(std::make_unique<Wt::WText>(""));
    roleTable->elementAt(trow, c++)->addWidget(std::make_unique<Wt::WText>(""));
    roleTable->elementAt(trow, c)->addWidget(std::make_unique<Wt::WText>(ppc::formatCurrency(data_.getBlendedCostRate())));
    roleTable->elementAt(trow, c++)->setStyleClass("cell-right cell-bold");
    roleTable->elementAt(trow, c)->addWidget(std::make_unique<Wt::WText>(ppc::formatNumber(data_.getTotalHours(), 0)));
    roleTable->elementAt(trow, c++)->setStyleClass("cell-right cell-bold");
    roleTable->elementAt(trow, c)->addWidget(std::make_unique<Wt::WText>(ppc::formatCurrency(data_.getTotalCost())));
    roleTable->elementAt(trow, c++)->setStyleClass("cell-right cell-bold");
    roleTable->elementAt(trow, c)->addWidget(std::make_unique<Wt::WText>(ppc::formatCurrency(data_.getBlendedSellRate())));
    roleTable->elementAt(trow, c++)->setStyleClass("cell-right cell-bold");
    roleTable->elementAt(trow, c)->addWidget(std::make_unique<Wt::WText>(ppc::formatCurrency(data_.getTotalSell())));
    roleTable->elementAt(trow, c)->setStyleClass("cell-right cell-bold");
    for (int i = 0; i < 8; i++)
        roleTable->elementAt(trow, i)->addStyleClass("total-row");

    // ---- Phase summary table ------------------------------------------------
    if (!data_.phases.empty()) {
        content_->addWidget(std::make_unique<Wt::WText>("<h3 class=\"section-title\">Phase Summary</h3>"));
        auto phaseTable = content_->addWidget(std::make_unique<Wt::WTable>());
        phaseTable->setStyleClass("data-table");
        phaseTable->setHeaderCount(1);

        c = 0;
        for (auto& h : {"Phase", "Weeks", "Hours", "Cost", "Sell Price"}) {
            phaseTable->elementAt(0, c)->addWidget(std::make_unique<Wt::WText>(h));
            phaseTable->elementAt(0, c)->setStyleClass(c >= 2 ? "cell-right" : "");
            c++;
        }

        double totalPH = 0, totalPC = 0, totalPS = 0;
        for (int p = 0; p < (int)data_.phases.size(); p++) {
            int row = p + 1;
            auto& ph = data_.phases[p];
            double hrs = data_.getPhaseHours(p);
            double cost = data_.getPhaseCost(p);
            double sell = data_.getPhaseSell(p);
            totalPH += hrs; totalPC += cost; totalPS += sell;

            c = 0;
            phaseTable->elementAt(row, c++)->addWidget(std::make_unique<Wt::WText>(ph.name));
            phaseTable->elementAt(row, c++)->addWidget(std::make_unique<Wt::WText>(
                "W" + std::to_string(ph.startWeek) + " - W" + std::to_string(ph.endWeek)));
            phaseTable->elementAt(row, c)->addWidget(std::make_unique<Wt::WText>(ppc::formatNumber(hrs, 0)));
            phaseTable->elementAt(row, c++)->setStyleClass("cell-right");
            phaseTable->elementAt(row, c)->addWidget(std::make_unique<Wt::WText>(ppc::formatCurrency(cost)));
            phaseTable->elementAt(row, c++)->setStyleClass("cell-right");
            phaseTable->elementAt(row, c)->addWidget(std::make_unique<Wt::WText>(ppc::formatCurrency(sell)));
            phaseTable->elementAt(row, c)->setStyleClass("cell-right");
        }

        trow = (int)data_.phases.size() + 1;
        phaseTable->elementAt(trow, 0)->addWidget(std::make_unique<Wt::WText>("Total"));
        phaseTable->elementAt(trow, 1)->addWidget(std::make_unique<Wt::WText>(
            "W1 - W" + std::to_string(data_.totalWeeks)));
        phaseTable->elementAt(trow, 2)->addWidget(std::make_unique<Wt::WText>(ppc::formatNumber(totalPH, 0)));
        phaseTable->elementAt(trow, 2)->setStyleClass("cell-right cell-bold");
        phaseTable->elementAt(trow, 3)->addWidget(std::make_unique<Wt::WText>(ppc::formatCurrency(totalPC)));
        phaseTable->elementAt(trow, 3)->setStyleClass("cell-right cell-bold");
        phaseTable->elementAt(trow, 4)->addWidget(std::make_unique<Wt::WText>(ppc::formatCurrency(totalPS)));
        phaseTable->elementAt(trow, 4)->setStyleClass("cell-right cell-bold");
        for (int i = 0; i < 5; i++)
            phaseTable->elementAt(trow, i)->addStyleClass("total-row");
    }

    // ---- Agile ceremony overhead per sprint ---------------------------------
    content_->addWidget(std::make_unique<Wt::WText>("<h3 class=\"section-title\">Agile Ceremony Overhead (per Sprint)</h3>"));
    auto cerTable = content_->addWidget(std::make_unique<Wt::WTable>());
    cerTable->setStyleClass("data-table");
    cerTable->setHeaderCount(1);

    c = 0;
    for (auto& h : {"Ceremony", "Duration", "Occurrences/Sprint", "Total Hrs/Sprint"}) {
        cerTable->elementAt(0, c)->addWidget(std::make_unique<Wt::WText>(h));
        cerTable->elementAt(0, c)->setStyleClass(c >= 1 ? "cell-right" : "");
        c++;
    }

    double totalCerHrs = 0;
    for (int i = 0; i < (int)data_.ceremonies.size(); i++) {
        int row = i + 1;
        auto& cer = data_.ceremonies[i];
        totalCerHrs += cer.totalHoursPerSprint();

        c = 0;
        cerTable->elementAt(row, c++)->addWidget(std::make_unique<Wt::WText>(cer.name));
        cerTable->elementAt(row, c)->addWidget(std::make_unique<Wt::WText>(ppc::formatNumber(cer.durationHours, 2) + " hrs"));
        cerTable->elementAt(row, c++)->setStyleClass("cell-right");
        cerTable->elementAt(row, c)->addWidget(std::make_unique<Wt::WText>(std::to_string(cer.occurrencesPerSprint)));
        cerTable->elementAt(row, c++)->setStyleClass("cell-right");
        cerTable->elementAt(row, c)->addWidget(std::make_unique<Wt::WText>(ppc::formatNumber(cer.totalHoursPerSprint(), 2) + " hrs"));
        cerTable->elementAt(row, c)->setStyleClass("cell-right");
    }

    trow = (int)data_.ceremonies.size() + 1;
    cerTable->elementAt(trow, 0)->addWidget(std::make_unique<Wt::WText>("Total per Sprint (per person)"));
    cerTable->elementAt(trow, 1)->addWidget(std::make_unique<Wt::WText>(""));
    cerTable->elementAt(trow, 2)->addWidget(std::make_unique<Wt::WText>(""));
    cerTable->elementAt(trow, 3)->addWidget(std::make_unique<Wt::WText>(ppc::formatNumber(totalCerHrs, 2) + " hrs"));
    cerTable->elementAt(trow, 3)->setStyleClass("cell-right cell-bold");
    for (int i = 0; i < 4; i++)
        cerTable->elementAt(trow, i)->addStyleClass("total-row");
}
