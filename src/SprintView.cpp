// =============================================================================
// SprintView.cpp — Sprint planning with Agile ceremonies (2-week intervals)
// =============================================================================

#include "SprintView.h"
#include <Wt/WText.h>
#include <Wt/WTable.h>
#include <Wt/WLineEdit.h>
#include <Wt/WTextArea.h>
#include <Wt/WDoubleSpinBox.h>
#include <Wt/WSpinBox.h>
#include <Wt/WPushButton.h>

SprintView::SprintView(ppc::ProjectData& data)
    : data_(data)
{
    setStyleClass("view-panel");
    content_ = addWidget(std::make_unique<Wt::WContainerWidget>());
}

void SprintView::refresh()
{
    content_->clear();

    content_->addWidget(std::make_unique<Wt::WText>(
        "<h2 class=\"view-title\">Sprints &amp; Agile Ceremonies</h2>"
        "<p class=\"view-subtitle\">Manage sprints (" + std::to_string(data_.sprintLengthWeeks) +
        "-week intervals) and Agile ceremony definitions</p>"
    ));

    // ---- Sprint settings ----------------------------------------------------
    auto settingsRow = content_->addWidget(std::make_unique<Wt::WContainerWidget>());
    settingsRow->setStyleClass("form-row-inline");

    settingsRow->addWidget(std::make_unique<Wt::WText>("<span class=\"field-label\">Total Weeks:</span>"));
    auto weeksSpin = settingsRow->addWidget(std::make_unique<Wt::WSpinBox>());
    weeksSpin->setRange(2, 104);
    weeksSpin->setValue(data_.totalWeeks);
    weeksSpin->setStyleClass("input-field input-sm");

    settingsRow->addWidget(std::make_unique<Wt::WText>("<span class=\"field-label\">Sprint Length:</span>"));
    auto sprintLenSpin = settingsRow->addWidget(std::make_unique<Wt::WSpinBox>());
    sprintLenSpin->setRange(1, 4);
    sprintLenSpin->setValue(data_.sprintLengthWeeks);
    sprintLenSpin->setStyleClass("input-field input-sm");
    settingsRow->addWidget(std::make_unique<Wt::WText>("<span class=\"field-unit\">weeks</span>"));

    auto regenBtn = settingsRow->addWidget(std::make_unique<Wt::WPushButton>("Regenerate Sprints"));
    regenBtn->setStyleClass("btn btn-primary");
    regenBtn->clicked().connect([this, weeksSpin, sprintLenSpin]() {
        data_.totalWeeks = weeksSpin->value();
        data_.sprintLengthWeeks = sprintLenSpin->value();
        data_.generateSprints();
        refresh();
    });

    // ---- Agile Ceremonies ---------------------------------------------------
    content_->addWidget(std::make_unique<Wt::WText>("<h3 class=\"section-title\">Agile Ceremonies</h3>"));
    content_->addWidget(std::make_unique<Wt::WText>(
        "<p class=\"view-subtitle\">These ceremonies occur every sprint and their hours are factored into project overhead.</p>"
    ));

    auto cerTable = content_->addWidget(std::make_unique<Wt::WTable>());
    cerTable->setStyleClass("data-table");
    cerTable->setHeaderCount(1);

    int c = 0;
    for (auto& h : {"Ceremony", "Description", "Duration (hrs)", "Per Sprint", "Total/Sprint"}) {
        cerTable->elementAt(0, c)->addWidget(std::make_unique<Wt::WText>(h));
        cerTable->elementAt(0, c)->setStyleClass(c >= 2 ? "cell-right" : "");
        c++;
    }

    double totalCerHrs = 0;
    for (int i = 0; i < (int)data_.ceremonies.size(); i++) {
        auto& cer = data_.ceremonies[i];
        int row = i + 1;
        totalCerHrs += cer.totalHoursPerSprint();

        cerTable->elementAt(row, 0)->addWidget(std::make_unique<Wt::WText>(
            "<strong>" + cer.name + "</strong>"));
        cerTable->elementAt(row, 1)->addWidget(std::make_unique<Wt::WText>(
            "<span class=\"text-muted\">" + cer.description + "</span>"));
        cerTable->elementAt(row, 1)->setStyleClass("cell-desc");

        auto durSpin = cerTable->elementAt(row, 2)->addWidget(std::make_unique<Wt::WDoubleSpinBox>());
        durSpin->setRange(0.25, 8);
        durSpin->setValue(cer.durationHours);
        durSpin->setDecimals(2);
        durSpin->setSingleStep(0.25);
        durSpin->setStyleClass("input-field input-sm");
        cerTable->elementAt(row, 2)->setStyleClass("cell-right");
        int cerIdx = i;
        durSpin->valueChanged().connect([this, cerIdx](double val) {
            data_.ceremonies[cerIdx].durationHours = val;
            refresh();
        });

        auto occSpin = cerTable->elementAt(row, 3)->addWidget(std::make_unique<Wt::WSpinBox>());
        occSpin->setRange(1, 20);
        occSpin->setValue(cer.occurrencesPerSprint);
        occSpin->setStyleClass("input-field input-sm");
        cerTable->elementAt(row, 3)->setStyleClass("cell-right");
        occSpin->changed().connect([this, cerIdx, occSpin]() {
            data_.ceremonies[cerIdx].occurrencesPerSprint = occSpin->value();
            refresh();
        });

        cerTable->elementAt(row, 4)->addWidget(std::make_unique<Wt::WText>(
            ppc::formatNumber(cer.totalHoursPerSprint(), 2) + " hrs"));
        cerTable->elementAt(row, 4)->setStyleClass("cell-right");
    }

    int trow = (int)data_.ceremonies.size() + 1;
    cerTable->elementAt(trow, 0)->addWidget(std::make_unique<Wt::WText>("Total per Sprint (per person)"));
    cerTable->elementAt(trow, 1)->addWidget(std::make_unique<Wt::WText>(""));
    cerTable->elementAt(trow, 2)->addWidget(std::make_unique<Wt::WText>(""));
    cerTable->elementAt(trow, 3)->addWidget(std::make_unique<Wt::WText>(""));
    cerTable->elementAt(trow, 4)->addWidget(std::make_unique<Wt::WText>(
        ppc::formatNumber(totalCerHrs, 2) + " hrs"));
    cerTable->elementAt(trow, 4)->setStyleClass("cell-right cell-bold");
    for (int i2 = 0; i2 < 5; i2++)
        cerTable->elementAt(trow, i2)->addStyleClass("total-row");

    double totalCerPerProject = totalCerHrs * (double)data_.sprints.size();
    content_->addWidget(std::make_unique<Wt::WText>(
        "<div class=\"info-callout\">"
        "<strong>Project Ceremony Overhead:</strong> " +
        ppc::formatNumber(totalCerHrs, 1) + " hrs/sprint &times; " +
        std::to_string(data_.sprints.size()) + " sprints = <strong>" +
        ppc::formatNumber(totalCerPerProject, 0) + " hrs</strong> per team member"
        "</div>"
    ));

    // ---- Sprint board -------------------------------------------------------
    content_->addWidget(std::make_unique<Wt::WText>("<h3 class=\"section-title\">Sprint Board</h3>"));

    for (int si = 0; si < (int)data_.sprints.size(); si++) {
        auto& sprint = data_.sprints[si];

        auto sprintCard = content_->addWidget(std::make_unique<Wt::WContainerWidget>());
        sprintCard->setStyleClass("sprint-card");

        auto statusBadge = "<span class=\"badge badge-" +
            std::string(sprint.status == "Active" ? "active" :
                        sprint.status == "Completed" ? "approved" : "draft") +
            "\">" + sprint.status + "</span>";

        sprintCard->addWidget(std::make_unique<Wt::WText>(
            "<div class=\"sprint-header\">"
            "<div class=\"sprint-name\">" + sprint.name + " " + statusBadge + "</div>"
            "<div class=\"sprint-weeks\">Weeks " + std::to_string(sprint.startWeek) +
            " &ndash; " + std::to_string(sprint.endWeek) + "</div>"
            "</div>"
        ));

        if (!sprint.goal.empty()) {
            sprintCard->addWidget(std::make_unique<Wt::WText>(
                "<div class=\"sprint-goal\"><strong>Goal:</strong> " + sprint.goal + "</div>"
            ));
        }

        // Components in this sprint
        if (!sprint.componentIds.empty()) {
            auto compList = sprintCard->addWidget(std::make_unique<Wt::WContainerWidget>());
            compList->setStyleClass("sprint-components");

            for (int cid : sprint.componentIds) {
                for (auto& comp : data_.components) {
                    if (comp.id == cid) {
                        double hrs = comp.totalHours();
                        double cost = data_.componentCost(comp);
                        compList->addWidget(std::make_unique<Wt::WText>(
                            "<div class=\"sprint-comp-item\">"
                            "<span class=\"comp-name\">" + comp.name + "</span>"
                            "<span class=\"comp-hours\">" + ppc::formatNumber(hrs, 0) + " hrs</span>"
                            "<span class=\"comp-cost\">" + ppc::formatCurrency(cost) + "</span>"
                            "</div>"
                        ));
                        break;
                    }
                }
            }
        }

        // Ceremony summary for this sprint
        sprintCard->addWidget(std::make_unique<Wt::WText>(
            "<div class=\"sprint-ceremony-summary\">"
            "<span class=\"cer-label\">Ceremony overhead:</span> "
            "<span class=\"cer-hours\">" + ppc::formatNumber(totalCerHrs, 1) + " hrs/person</span>"
            "</div>"
        ));
    }
}
