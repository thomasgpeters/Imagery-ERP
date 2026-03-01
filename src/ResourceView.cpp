// =============================================================================
// ResourceView.cpp — Roles & rates management with markup setting
// =============================================================================

#include "ResourceView.h"
#include <Wt/WText.h>
#include <Wt/WTable.h>
#include <Wt/WLineEdit.h>
#include <Wt/WDoubleSpinBox.h>
#include <Wt/WPushButton.h>

ResourceView::ResourceView(ppc::ProjectData& data)
    : data_(data)
{
    setStyleClass("view-panel");
    content_ = addWidget(std::make_unique<Wt::WContainerWidget>());
}

void ResourceView::refresh()
{
    content_->clear();

    content_->addWidget(std::make_unique<Wt::WText>(
        "<h2 class=\"view-title\">Roles &amp; Rates</h2>"
        "<p class=\"view-subtitle\">Define resource roles, hourly rates, and project markup</p>"
    ));

    // ---- Markup setting -----------------------------------------------------
    auto markupRow = content_->addWidget(std::make_unique<Wt::WContainerWidget>());
    markupRow->setStyleClass("form-row-inline");

    markupRow->addWidget(std::make_unique<Wt::WText>("<span class=\"field-label\">Mark-up Rate:</span>"));
    auto markupSpin = markupRow->addWidget(std::make_unique<Wt::WDoubleSpinBox>());
    markupSpin->setRange(0, 200);
    markupSpin->setValue(data_.markupPct);
    markupSpin->setDecimals(1);
    markupSpin->setSingleStep(1.0);
    markupSpin->setStyleClass("input-field input-sm");
    markupRow->addWidget(std::make_unique<Wt::WText>("<span class=\"field-unit\">%</span>"));

    markupSpin->valueChanged().connect([this](double val) {
        data_.markupPct = val;
        refresh();
    });

    // ---- Roles table --------------------------------------------------------
    content_->addWidget(std::make_unique<Wt::WText>("<h3 class=\"section-title\">Defined Roles</h3>"));
    auto table = content_->addWidget(std::make_unique<Wt::WTable>());
    table->setStyleClass("data-table");
    table->setHeaderCount(1);

    int col = 0;
    for (auto& h : {"Role Name", "Base Rate", "Overhead", "Fully Loaded", "Sell Rate", "Actions"}) {
        table->elementAt(0, col)->addWidget(std::make_unique<Wt::WText>(h));
        table->elementAt(0, col)->setStyleClass(col >= 1 && col <= 4 ? "cell-right" : "");
        col++;
    }

    for (int r = 0; r < (int)data_.roles.size(); r++) {
        int row = r + 1;
        auto& role = data_.roles[r];
        double sellRate = role.fullyLoadedRate() * (1.0 + data_.markupPct / 100.0);

        // Editable name
        auto nameEdit = table->elementAt(row, 0)->addWidget(std::make_unique<Wt::WLineEdit>(role.name));
        nameEdit->setStyleClass("input-field");
        int roleIdx = r;
        nameEdit->changed().connect([this, roleIdx, nameEdit]() {
            data_.roles[roleIdx].name = nameEdit->text().toUTF8();
        });

        // Editable base rate
        auto baseEdit = table->elementAt(row, 1)->addWidget(std::make_unique<Wt::WDoubleSpinBox>());
        baseEdit->setRange(0, 500);
        baseEdit->setValue(role.baseRate);
        baseEdit->setDecimals(2);
        baseEdit->setStyleClass("input-field input-sm");
        table->elementAt(row, 1)->setStyleClass("cell-right");
        baseEdit->valueChanged().connect([this, roleIdx](double val) {
            data_.roles[roleIdx].baseRate = val;
            refresh();
        });

        // Editable overhead rate
        auto ovEdit = table->elementAt(row, 2)->addWidget(std::make_unique<Wt::WDoubleSpinBox>());
        ovEdit->setRange(0, 200);
        ovEdit->setValue(role.overheadRate);
        ovEdit->setDecimals(2);
        ovEdit->setStyleClass("input-field input-sm");
        table->elementAt(row, 2)->setStyleClass("cell-right");
        ovEdit->valueChanged().connect([this, roleIdx](double val) {
            data_.roles[roleIdx].overheadRate = val;
            refresh();
        });

        // Computed fully loaded
        table->elementAt(row, 3)->addWidget(std::make_unique<Wt::WText>(ppc::formatCurrency(role.fullyLoadedRate())));
        table->elementAt(row, 3)->setStyleClass("cell-right cell-bold");

        // Computed sell rate
        table->elementAt(row, 4)->addWidget(std::make_unique<Wt::WText>(ppc::formatCurrency(sellRate)));
        table->elementAt(row, 4)->setStyleClass("cell-right");

        // Delete button
        auto delBtn = table->elementAt(row, 5)->addWidget(std::make_unique<Wt::WPushButton>("Delete"));
        delBtn->setStyleClass("btn btn-danger btn-sm");
        delBtn->clicked().connect([this, roleIdx]() {
            data_.roles.erase(data_.roles.begin() + roleIdx);
            refresh();
        });
    }

    // Blended rate row
    int blendRow = (int)data_.roles.size() + 1;
    table->elementAt(blendRow, 0)->addWidget(std::make_unique<Wt::WText>("Blended Rate"));
    table->elementAt(blendRow, 1)->addWidget(std::make_unique<Wt::WText>(""));
    table->elementAt(blendRow, 2)->addWidget(std::make_unique<Wt::WText>(""));
    table->elementAt(blendRow, 3)->addWidget(std::make_unique<Wt::WText>(ppc::formatCurrency(data_.getBlendedCostRate())));
    table->elementAt(blendRow, 3)->setStyleClass("cell-right cell-bold");
    table->elementAt(blendRow, 4)->addWidget(std::make_unique<Wt::WText>(ppc::formatCurrency(data_.getBlendedSellRate())));
    table->elementAt(blendRow, 4)->setStyleClass("cell-right cell-bold");
    table->elementAt(blendRow, 5)->addWidget(std::make_unique<Wt::WText>(""));
    for (int i = 0; i < 6; i++)
        table->elementAt(blendRow, i)->addStyleClass("total-row");

    // ---- Add role form ------------------------------------------------------
    content_->addWidget(std::make_unique<Wt::WText>("<h3 class=\"section-title\">Add New Role</h3>"));
    auto addRow = content_->addWidget(std::make_unique<Wt::WContainerWidget>());
    addRow->setStyleClass("form-row-inline");

    addRow->addWidget(std::make_unique<Wt::WText>("<span class=\"field-label\">Name:</span>"));
    auto nameInput = addRow->addWidget(std::make_unique<Wt::WLineEdit>());
    nameInput->setPlaceholderText("Role name");
    nameInput->setStyleClass("input-field");

    addRow->addWidget(std::make_unique<Wt::WText>("<span class=\"field-label\">Base Rate: $</span>"));
    auto rateInput = addRow->addWidget(std::make_unique<Wt::WDoubleSpinBox>());
    rateInput->setRange(0, 500);
    rateInput->setValue(50.0);
    rateInput->setDecimals(2);
    rateInput->setStyleClass("input-field input-sm");

    addRow->addWidget(std::make_unique<Wt::WText>("<span class=\"field-label\">Overhead: $</span>"));
    auto overInput = addRow->addWidget(std::make_unique<Wt::WDoubleSpinBox>());
    overInput->setRange(0, 200);
    overInput->setValue(10.0);
    overInput->setDecimals(2);
    overInput->setStyleClass("input-field input-sm");

    auto addBtn = addRow->addWidget(std::make_unique<Wt::WPushButton>("Add Role"));
    addBtn->setStyleClass("btn btn-primary");
    addBtn->clicked().connect([this, nameInput, rateInput, overInput]() {
        auto name = nameInput->text().toUTF8();
        if (!name.empty()) {
            ppc::Role nr;
            nr.id = data_.genId();
            nr.name = name;
            nr.baseRate = rateInput->value();
            nr.overheadRate = overInput->value();
            nr.sortOrder = (int)data_.roles.size() + 1;
            data_.roles.push_back(nr);
            refresh();
        }
    });
}
