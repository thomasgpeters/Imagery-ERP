// =============================================================================
// MaterialView.cpp — Material resources management (supplies, equipment, travel, etc.)
// =============================================================================

#include "MaterialView.h"
#include <Wt/WText.h>
#include <Wt/WTable.h>
#include <Wt/WLineEdit.h>
#include <Wt/WDoubleSpinBox.h>
#include <Wt/WPushButton.h>
#include <Wt/WComboBox.h>

MaterialView::MaterialView(ppc::ProjectData& data)
    : data_(data)
{
    setStyleClass("view-panel");
    content_ = addWidget(std::make_unique<Wt::WContainerWidget>());
}

void MaterialView::refresh()
{
    content_->clear();

    content_->addWidget(ppc::xhtml(
        "<h2 class=\"view-title\">Materials &amp; Expenses</h2>"
        "<p class=\"view-subtitle\">Manage non-labor costs: supplies, licenses, equipment, travel, and other project expenses</p>"
    ));

    // ---- Total material cost callout ----------------------------------------
    double totalMatCost = data_.getTotalMaterialCost();
    content_->addWidget(ppc::xhtml(
        "<div class=\"material-summary-bar\">"
        "<span class=\"msb-label\">Total Material Costs Across All Components:</span>"
        "<span class=\"msb-value\">" + ppc::formatCurrency(totalMatCost) + "</span>"
        "<span class=\"msb-sell\">Sell: " +
        ppc::formatCurrency(totalMatCost * (1.0 + data_.markupPct / 100.0)) + "</span>"
        "</div>"
    ));

    // ---- Materials catalogue table ------------------------------------------
    content_->addWidget(ppc::xhtml("<h3 class=\"section-title\">Materials Catalogue</h3>"));
    auto table = content_->addWidget(std::make_unique<Wt::WTable>());
    table->setStyleClass("data-table");
    table->setHeaderCount(1);

    int col = 0;
    for (auto& h : {"Material", "Category", "Unit", "Unit Cost", "Usage", "Total", "Actions"}) {
        table->elementAt(0, col)->addWidget(std::make_unique<Wt::WText>(h));
        table->elementAt(0, col)->setStyleClass(col >= 3 && col <= 5 ? "cell-right" : "");
        col++;
    }

    for (int m = 0; m < (int)data_.materials.size(); m++) {
        int row = m + 1;
        auto& mat = data_.materials[m];

        // Calculate total usage across all components
        double totalQty = 0;
        for (auto& comp : data_.components) {
            for (auto& cm : comp.materials) {
                if (cm.materialId == mat.id) totalQty += cm.quantity;
            }
        }
        double totalCost = totalQty * mat.unitCost;

        // Editable name
        auto nameEdit = table->elementAt(row, 0)->addWidget(std::make_unique<Wt::WLineEdit>(mat.name));
        nameEdit->setStyleClass("input-field input-wide");
        int matIdx = m;
        nameEdit->changed().connect([this, matIdx, nameEdit]() {
            data_.materials[matIdx].name = nameEdit->text().toUTF8();
        });

        // Category badge
        std::string catClass = "material";
        if (mat.category == "Travel") catClass = "travel";
        else if (mat.category == "Software/Licenses") catClass = "software";
        else if (mat.category == "Equipment/Tools") catClass = "equipment";
        else if (mat.category == "Office Supplies") catClass = "office";
        table->elementAt(row, 1)->addWidget(ppc::xhtml(
            "<span class=\"badge badge-" + catClass + "\">" + mat.category + "</span>"));

        // Unit
        table->elementAt(row, 2)->addWidget(std::make_unique<Wt::WText>(mat.unit));

        // Editable unit cost
        auto costEdit = table->elementAt(row, 3)->addWidget(std::make_unique<Wt::WDoubleSpinBox>());
        costEdit->setRange(0, 100000);
        costEdit->setValue(mat.unitCost);
        costEdit->setDecimals(2);
        costEdit->setStyleClass("input-field input-sm");
        table->elementAt(row, 3)->setStyleClass("cell-right");
        costEdit->valueChanged().connect([this, matIdx](double val) {
            data_.materials[matIdx].unitCost = val;
            refresh();
        });

        // Usage (read-only aggregate)
        std::string usageStr = totalQty > 0
            ? ppc::formatNumber(totalQty, 1) + " " + mat.unit + (totalQty != 1.0 ? "s" : "")
            : "—";
        table->elementAt(row, 4)->addWidget(std::make_unique<Wt::WText>(usageStr));
        table->elementAt(row, 4)->setStyleClass("cell-right");

        // Total cost
        table->elementAt(row, 5)->addWidget(std::make_unique<Wt::WText>(
            totalQty > 0 ? ppc::formatCurrency(totalCost) : "—"));
        table->elementAt(row, 5)->setStyleClass("cell-right" + std::string(totalQty > 0 ? " cell-bold" : ""));

        // Delete
        auto delBtn = table->elementAt(row, 6)->addWidget(std::make_unique<Wt::WPushButton>("Delete"));
        delBtn->setStyleClass("btn btn-danger btn-sm");
        delBtn->clicked().connect([this, matIdx]() {
            int matId = data_.materials[matIdx].id;
            // Remove from all components
            for (auto& comp : data_.components) {
                comp.materials.erase(
                    std::remove_if(comp.materials.begin(), comp.materials.end(),
                        [matId](const ppc::ComponentMaterial& cm) { return cm.materialId == matId; }),
                    comp.materials.end());
            }
            data_.materials.erase(data_.materials.begin() + matIdx);
            refresh();
        });
    }

    // Totals row
    int trow = (int)data_.materials.size() + 1;
    table->elementAt(trow, 0)->addWidget(std::make_unique<Wt::WText>("Total"));
    for (int i = 1; i <= 4; i++)
        table->elementAt(trow, i)->addWidget(std::make_unique<Wt::WText>(""));
    table->elementAt(trow, 5)->addWidget(std::make_unique<Wt::WText>(ppc::formatCurrency(totalMatCost)));
    table->elementAt(trow, 5)->setStyleClass("cell-right cell-bold");
    table->elementAt(trow, 6)->addWidget(std::make_unique<Wt::WText>(""));
    for (int i = 0; i <= 6; i++)
        table->elementAt(trow, i)->addStyleClass("total-row");

    // ---- Add new material form ----------------------------------------------
    content_->addWidget(ppc::xhtml("<h3 class=\"section-title\">Add New Material</h3>"));
    auto addForm = content_->addWidget(std::make_unique<Wt::WContainerWidget>());
    addForm->setStyleClass("form-row-inline");

    addForm->addWidget(ppc::xhtml("<span class=\"field-label\">Name:</span>"));
    auto nameIn = addForm->addWidget(std::make_unique<Wt::WLineEdit>());
    nameIn->setPlaceholderText("Material / expense name");
    nameIn->setStyleClass("input-field");

    addForm->addWidget(ppc::xhtml("<span class=\"field-label\">Category:</span>"));
    auto catCombo = addForm->addWidget(std::make_unique<Wt::WComboBox>());
    catCombo->setStyleClass("input-field");
    catCombo->addItem("Office Supplies");
    catCombo->addItem("Construction");
    catCombo->addItem("Equipment/Tools");
    catCombo->addItem("Travel");
    catCombo->addItem("Software/Licenses");
    catCombo->addItem("Other");

    addForm->addWidget(ppc::xhtml("<span class=\"field-label\">Unit:</span>"));
    auto unitCombo = addForm->addWidget(std::make_unique<Wt::WComboBox>());
    unitCombo->setStyleClass("input-field input-sm");
    unitCombo->addItem("unit");
    unitCombo->addItem("month");
    unitCombo->addItem("day");
    unitCombo->addItem("trip");
    unitCombo->addItem("ton");
    unitCombo->addItem("bag");
    unitCombo->addItem("lot");
    unitCombo->addItem("license");
    unitCombo->addItem("mile");

    addForm->addWidget(ppc::xhtml("<span class=\"field-label\">Unit Cost: $</span>"));
    auto costIn = addForm->addWidget(std::make_unique<Wt::WDoubleSpinBox>());
    costIn->setRange(0, 100000);
    costIn->setValue(100.0);
    costIn->setDecimals(2);
    costIn->setStyleClass("input-field input-sm");

    auto addBtn = addForm->addWidget(std::make_unique<Wt::WPushButton>("Add Material"));
    addBtn->setStyleClass("btn btn-primary");
    addBtn->clicked().connect([this, nameIn, catCombo, unitCombo, costIn]() {
        auto name = nameIn->text().toUTF8();
        if (!name.empty()) {
            ppc::Material nm;
            nm.id = data_.genId();
            nm.name = name;
            nm.category = catCombo->currentText().toUTF8();
            nm.unit = unitCombo->currentText().toUTF8();
            nm.unitCost = costIn->value();
            nm.sortOrder = (int)data_.materials.size() + 1;
            data_.materials.push_back(nm);
            refresh();
        }
    });

    // ---- Per-component material usage breakdown -----------------------------
    content_->addWidget(ppc::xhtml("<h3 class=\"section-title\">Material Usage by Component</h3>"));

    bool anyMaterials = false;
    for (auto& comp : data_.components) {
        if (comp.materials.empty()) continue;
        anyMaterials = true;

        double compMatTotal = data_.componentMaterialCost(comp);

        content_->addWidget(ppc::xhtml(
            "<div class=\"material-comp-header\">"
            "<span class=\"mch-name\">" + comp.name + "</span>"
            "<span class=\"mch-total\">" + ppc::formatCurrency(compMatTotal) + "</span>"
            "</div>"
        ));

        auto mTable = content_->addWidget(std::make_unique<Wt::WTable>());
        mTable->setStyleClass("data-table data-table-compact");
        mTable->setHeaderCount(1);

        mTable->elementAt(0, 0)->addWidget(std::make_unique<Wt::WText>("Material"));
        mTable->elementAt(0, 1)->addWidget(std::make_unique<Wt::WText>("Qty"));
        mTable->elementAt(0, 1)->setStyleClass("cell-right");
        mTable->elementAt(0, 2)->addWidget(std::make_unique<Wt::WText>("Unit Cost"));
        mTable->elementAt(0, 2)->setStyleClass("cell-right");
        mTable->elementAt(0, 3)->addWidget(std::make_unique<Wt::WText>("Line Total"));
        mTable->elementAt(0, 3)->setStyleClass("cell-right");

        for (int mi = 0; mi < (int)comp.materials.size(); mi++) {
            auto& cm = comp.materials[mi];
            auto* mat = data_.findMaterial(cm.materialId);
            std::string matName = mat ? mat->name : "Unknown";
            double uc = mat ? mat->unitCost : 0.0;
            int mrow = mi + 1;

            mTable->elementAt(mrow, 0)->addWidget(std::make_unique<Wt::WText>(matName));
            mTable->elementAt(mrow, 1)->addWidget(std::make_unique<Wt::WText>(ppc::formatNumber(cm.quantity, 1)));
            mTable->elementAt(mrow, 1)->setStyleClass("cell-right");
            mTable->elementAt(mrow, 2)->addWidget(std::make_unique<Wt::WText>(ppc::formatCurrency(uc)));
            mTable->elementAt(mrow, 2)->setStyleClass("cell-right");
            mTable->elementAt(mrow, 3)->addWidget(std::make_unique<Wt::WText>(ppc::formatCurrency(cm.quantity * uc)));
            mTable->elementAt(mrow, 3)->setStyleClass("cell-right");
        }
    }

    if (!anyMaterials) {
        content_->addWidget(ppc::xhtml(
            "<div class=\"info-callout\">No materials have been assigned to components yet. "
            "Use the Components &amp; SoW view to assign materials to individual components.</div>"));
    }
}
