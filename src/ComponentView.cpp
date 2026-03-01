// =============================================================================
// ComponentView.cpp — Component estimating with SoW, man hours, resources
// =============================================================================

#include "ComponentView.h"
#include <Wt/WText.h>
#include <Wt/WTable.h>
#include <Wt/WLineEdit.h>
#include <Wt/WTextArea.h>
#include <Wt/WDoubleSpinBox.h>
#include <Wt/WSpinBox.h>
#include <Wt/WPushButton.h>
#include <Wt/WComboBox.h>

ComponentView::ComponentView(ppc::ProjectData& data)
    : data_(data)
{
    setStyleClass("view-panel");
    content_ = addWidget(std::make_unique<Wt::WContainerWidget>());
}

void ComponentView::refresh()
{
    content_->clear();

    content_->addWidget(ppc::xhtml(
        "<h2 class=\"view-title\">Components &amp; Statement of Work</h2>"
        "<p class=\"view-subtitle\">Define estimable units of work with resource needs and SoW</p>"
    ));

    // ---- Component summary table --------------------------------------------
    content_->addWidget(ppc::xhtml("<h3 class=\"section-title\">Component Estimates</h3>"));
    auto table = content_->addWidget(std::make_unique<Wt::WTable>());
    table->setStyleClass("data-table");
    table->setHeaderCount(1);

    int col = 0;
    for (auto& h : {"#", "Component", "Phase", "Complexity", "Hours", "Cost", "Sell", "Status", ""}) {
        table->elementAt(0, col)->addWidget(std::make_unique<Wt::WText>(h));
        table->elementAt(0, col)->setStyleClass(col >= 4 && col <= 6 ? "cell-right" : "");
        col++;
    }

    double totalH = 0, totalC = 0, totalS = 0;
    for (int i = 0; i < (int)data_.components.size(); i++) {
        auto& comp = data_.components[i];
        int row = i + 1;
        double hrs = comp.totalHours();
        double cost = data_.componentCost(comp);
        double sell = data_.componentSell(comp);
        totalH += hrs; totalC += cost; totalS += sell;

        // Find phase name
        std::string phaseName = "—";
        for (auto& ph : data_.phases) {
            if (ph.id == comp.phaseId) { phaseName = ph.name; break; }
        }

        col = 0;
        table->elementAt(row, col++)->addWidget(std::make_unique<Wt::WText>(std::to_string(i + 1)));

        // Component name with expand toggle
        auto nameBtn = table->elementAt(row, col++)->addWidget(std::make_unique<Wt::WPushButton>(comp.name));
        nameBtn->setStyleClass("btn-link");
        int compIdx = i;
        nameBtn->clicked().connect([this, compIdx]() {
            expandedComponent_ = (expandedComponent_ == compIdx) ? -1 : compIdx;
            refresh();
        });

        table->elementAt(row, col++)->addWidget(std::make_unique<Wt::WText>(phaseName));

        auto complexSpan = "<span class=\"badge badge-" +
            std::string(comp.complexity == "Critical" ? "critical" :
                        comp.complexity == "High" ? "high" :
                        comp.complexity == "Medium" ? "medium" : "low") +
            "\">" + comp.complexity + "</span>";
        table->elementAt(row, col++)->addWidget(ppc::xhtml(complexSpan));

        table->elementAt(row, col)->addWidget(std::make_unique<Wt::WText>(ppc::formatNumber(hrs, 0)));
        table->elementAt(row, col++)->setStyleClass("cell-right");
        table->elementAt(row, col)->addWidget(std::make_unique<Wt::WText>(ppc::formatCurrency(cost)));
        table->elementAt(row, col++)->setStyleClass("cell-right");
        table->elementAt(row, col)->addWidget(std::make_unique<Wt::WText>(ppc::formatCurrency(sell)));
        table->elementAt(row, col++)->setStyleClass("cell-right");

        auto statusSpan = "<span class=\"badge badge-" +
            std::string(comp.status == "Approved" ? "approved" :
                        comp.status == "Estimated" ? "estimated" : "draft") +
            "\">" + comp.status + "</span>";
        table->elementAt(row, col++)->addWidget(ppc::xhtml(statusSpan));

        auto delBtn = table->elementAt(row, col)->addWidget(std::make_unique<Wt::WPushButton>("Delete"));
        delBtn->setStyleClass("btn btn-danger btn-xs");
        delBtn->clicked().connect([this, compIdx]() {
            data_.components.erase(data_.components.begin() + compIdx);
            if (expandedComponent_ == compIdx) expandedComponent_ = -1;
            else if (expandedComponent_ > compIdx) expandedComponent_--;
            refresh();
        });

        // ---- Expanded SoW + resource detail ---------------------------------
        if (expandedComponent_ == i) {
            int detailRow = row + 1;
            // We'll use a large merged-looking row
            auto detailCell = table->elementAt(detailRow, 0);
            // Wt tables don't merge cells easily, so we use a full-width container approach
            // Instead, show expanded detail below the table
        }
    }

    // Totals
    int trow = (int)data_.components.size() + 1;
    table->elementAt(trow, 0)->addWidget(std::make_unique<Wt::WText>(""));
    table->elementAt(trow, 1)->addWidget(std::make_unique<Wt::WText>("Total"));
    table->elementAt(trow, 2)->addWidget(std::make_unique<Wt::WText>(""));
    table->elementAt(trow, 3)->addWidget(std::make_unique<Wt::WText>(""));
    table->elementAt(trow, 4)->addWidget(std::make_unique<Wt::WText>(ppc::formatNumber(totalH, 0)));
    table->elementAt(trow, 4)->setStyleClass("cell-right cell-bold");
    table->elementAt(trow, 5)->addWidget(std::make_unique<Wt::WText>(ppc::formatCurrency(totalC)));
    table->elementAt(trow, 5)->setStyleClass("cell-right cell-bold");
    table->elementAt(trow, 6)->addWidget(std::make_unique<Wt::WText>(ppc::formatCurrency(totalS)));
    table->elementAt(trow, 6)->setStyleClass("cell-right cell-bold");
    table->elementAt(trow, 7)->addWidget(std::make_unique<Wt::WText>(""));
    table->elementAt(trow, 8)->addWidget(std::make_unique<Wt::WText>(""));
    for (int c2 = 0; c2 <= 8; c2++)
        table->elementAt(trow, c2)->addStyleClass("total-row");

    // ---- Expanded component detail (shown below table) ----------------------
    if (expandedComponent_ >= 0 && expandedComponent_ < (int)data_.components.size()) {
        auto& comp = data_.components[expandedComponent_];

        auto detail = content_->addWidget(std::make_unique<Wt::WContainerWidget>());
        detail->setStyleClass("component-detail-panel");

        detail->addWidget(ppc::xhtml(
            "<h3 class=\"detail-title\">" + comp.name + "</h3>"
            "<p class=\"detail-desc\">" + comp.description + "</p>"
        ));

        // SoW
        detail->addWidget(ppc::xhtml("<h4 class=\"detail-section\">Statement of Work</h4>"));
        auto sowArea = detail->addWidget(std::make_unique<Wt::WTextArea>(comp.statementOfWork));
        sowArea->setStyleClass("input-textarea");
        sowArea->setRows(6);
        int compIdx = expandedComponent_;
        sowArea->changed().connect([this, compIdx, sowArea]() {
            data_.components[compIdx].statementOfWork = sowArea->text().toUTF8();
        });

        // Resource needs table
        detail->addWidget(ppc::xhtml("<h4 class=\"detail-section\">Resource Requirements</h4>"));
        auto resTable = detail->addWidget(std::make_unique<Wt::WTable>());
        resTable->setStyleClass("data-table data-table-compact");
        resTable->setHeaderCount(1);

        resTable->elementAt(0, 0)->addWidget(std::make_unique<Wt::WText>("Role"));
        resTable->elementAt(0, 1)->addWidget(std::make_unique<Wt::WText>("Rate"));
        resTable->elementAt(0, 1)->setStyleClass("cell-right");
        resTable->elementAt(0, 2)->addWidget(std::make_unique<Wt::WText>("Est. Hours"));
        resTable->elementAt(0, 2)->setStyleClass("cell-right");
        resTable->elementAt(0, 3)->addWidget(std::make_unique<Wt::WText>("Cost"));
        resTable->elementAt(0, 3)->setStyleClass("cell-right");
        resTable->elementAt(0, 4)->addWidget(std::make_unique<Wt::WText>(""));

        for (int ri = 0; ri < (int)comp.resources.size(); ri++) {
            auto& cr = comp.resources[ri];
            auto* role = data_.findRole(cr.roleId);
            std::string roleName = role ? role->name : "Unknown";
            double rate = role ? role->fullyLoadedRate() : 0.0;
            int rrow = ri + 1;

            resTable->elementAt(rrow, 0)->addWidget(std::make_unique<Wt::WText>(roleName));
            resTable->elementAt(rrow, 1)->addWidget(std::make_unique<Wt::WText>(ppc::formatCurrency(rate) + "/hr"));
            resTable->elementAt(rrow, 1)->setStyleClass("cell-right");

            auto hrsSpin = resTable->elementAt(rrow, 2)->addWidget(std::make_unique<Wt::WDoubleSpinBox>());
            hrsSpin->setRange(0, 1000);
            hrsSpin->setValue(cr.estimatedHours);
            hrsSpin->setDecimals(1);
            hrsSpin->setStyleClass("input-field input-sm");
            resTable->elementAt(rrow, 2)->setStyleClass("cell-right");
            int resIdx = ri;
            hrsSpin->valueChanged().connect([this, compIdx, resIdx](double val) {
                data_.components[compIdx].resources[resIdx].estimatedHours = val;
                refresh();
            });

            resTable->elementAt(rrow, 3)->addWidget(std::make_unique<Wt::WText>(
                ppc::formatCurrency(cr.estimatedHours * rate)));
            resTable->elementAt(rrow, 3)->setStyleClass("cell-right");

            auto delResBtn = resTable->elementAt(rrow, 4)->addWidget(std::make_unique<Wt::WPushButton>("Remove"));
            delResBtn->setStyleClass("btn btn-danger btn-xs");
            delResBtn->clicked().connect([this, compIdx, resIdx]() {
                auto& res = data_.components[compIdx].resources;
                res.erase(res.begin() + resIdx);
                refresh();
            });
        }

        // Add resource to component
        auto addResRow = detail->addWidget(std::make_unique<Wt::WContainerWidget>());
        addResRow->setStyleClass("form-row-inline mt-1");
        addResRow->addWidget(ppc::xhtml("<span class=\"field-label\">Add Role:</span>"));

        auto roleCombo = addResRow->addWidget(std::make_unique<Wt::WComboBox>());
        roleCombo->setStyleClass("input-field");
        for (auto& r : data_.roles) roleCombo->addItem(r.name);

        addResRow->addWidget(ppc::xhtml("<span class=\"field-label\">Hours:</span>"));
        auto hrsInput = addResRow->addWidget(std::make_unique<Wt::WDoubleSpinBox>());
        hrsInput->setRange(0, 1000);
        hrsInput->setValue(20);
        hrsInput->setStyleClass("input-field input-sm");

        auto addResBtn = addResRow->addWidget(std::make_unique<Wt::WPushButton>("Add"));
        addResBtn->setStyleClass("btn btn-primary btn-sm");
        addResBtn->clicked().connect([this, compIdx, roleCombo, hrsInput]() {
            int selIdx = roleCombo->currentIndex();
            if (selIdx >= 0 && selIdx < (int)data_.roles.size()) {
                ppc::ComponentResource cr;
                cr.roleId = data_.roles[selIdx].id;
                cr.estimatedHours = hrsInput->value();
                data_.components[compIdx].resources.push_back(cr);
                refresh();
            }
        });

        // ---- Material requirements ------------------------------------------
        detail->addWidget(ppc::xhtml("<h4 class=\"detail-section\">Material Requirements</h4>"));

        if (!comp.materials.empty()) {
            auto matTable = detail->addWidget(std::make_unique<Wt::WTable>());
            matTable->setStyleClass("data-table data-table-compact");
            matTable->setHeaderCount(1);

            matTable->elementAt(0, 0)->addWidget(std::make_unique<Wt::WText>("Material"));
            matTable->elementAt(0, 1)->addWidget(std::make_unique<Wt::WText>("Unit Cost"));
            matTable->elementAt(0, 1)->setStyleClass("cell-right");
            matTable->elementAt(0, 2)->addWidget(std::make_unique<Wt::WText>("Qty"));
            matTable->elementAt(0, 2)->setStyleClass("cell-right");
            matTable->elementAt(0, 3)->addWidget(std::make_unique<Wt::WText>("Cost"));
            matTable->elementAt(0, 3)->setStyleClass("cell-right");
            matTable->elementAt(0, 4)->addWidget(std::make_unique<Wt::WText>(""));

            for (int mi = 0; mi < (int)comp.materials.size(); mi++) {
                auto& cm = comp.materials[mi];
                auto* mat = data_.findMaterial(cm.materialId);
                std::string matName = mat ? mat->name : "Unknown";
                double uc = mat ? mat->unitCost : 0.0;
                int mrow = mi + 1;

                matTable->elementAt(mrow, 0)->addWidget(std::make_unique<Wt::WText>(matName));
                matTable->elementAt(mrow, 1)->addWidget(std::make_unique<Wt::WText>(ppc::formatCurrency(uc)));
                matTable->elementAt(mrow, 1)->setStyleClass("cell-right");

                auto qtySpin = matTable->elementAt(mrow, 2)->addWidget(std::make_unique<Wt::WDoubleSpinBox>());
                qtySpin->setRange(0, 10000);
                qtySpin->setValue(cm.quantity);
                qtySpin->setDecimals(1);
                qtySpin->setStyleClass("input-field input-sm");
                matTable->elementAt(mrow, 2)->setStyleClass("cell-right");
                int matIdx = mi;
                qtySpin->valueChanged().connect([this, compIdx, matIdx](double val) {
                    data_.components[compIdx].materials[matIdx].quantity = val;
                    refresh();
                });

                matTable->elementAt(mrow, 3)->addWidget(std::make_unique<Wt::WText>(
                    ppc::formatCurrency(cm.quantity * uc)));
                matTable->elementAt(mrow, 3)->setStyleClass("cell-right");

                auto delMatBtn = matTable->elementAt(mrow, 4)->addWidget(std::make_unique<Wt::WPushButton>("Remove"));
                delMatBtn->setStyleClass("btn btn-danger btn-xs");
                delMatBtn->clicked().connect([this, compIdx, matIdx]() {
                    auto& mats = data_.components[compIdx].materials;
                    mats.erase(mats.begin() + matIdx);
                    refresh();
                });
            }
        }

        // Add material to component
        if (!data_.materials.empty()) {
            auto addMatRow = detail->addWidget(std::make_unique<Wt::WContainerWidget>());
            addMatRow->setStyleClass("form-row-inline mt-1");
            addMatRow->addWidget(ppc::xhtml("<span class=\"field-label\">Add Material:</span>"));

            auto matCombo = addMatRow->addWidget(std::make_unique<Wt::WComboBox>());
            matCombo->setStyleClass("input-field");
            for (auto& m : data_.materials) matCombo->addItem(m.name);

            addMatRow->addWidget(ppc::xhtml("<span class=\"field-label\">Qty:</span>"));
            auto qtyInput = addMatRow->addWidget(std::make_unique<Wt::WDoubleSpinBox>());
            qtyInput->setRange(0, 10000);
            qtyInput->setValue(1);
            qtyInput->setDecimals(1);
            qtyInput->setStyleClass("input-field input-sm");

            auto addMatBtn = addMatRow->addWidget(std::make_unique<Wt::WPushButton>("Add"));
            addMatBtn->setStyleClass("btn btn-primary btn-sm");
            addMatBtn->clicked().connect([this, compIdx, matCombo, qtyInput]() {
                int selIdx = matCombo->currentIndex();
                if (selIdx >= 0 && selIdx < (int)data_.materials.size()) {
                    ppc::ComponentMaterial cm;
                    cm.materialId = data_.materials[selIdx].id;
                    cm.quantity = qtyInput->value();
                    data_.components[compIdx].materials.push_back(cm);
                    refresh();
                }
            });
        } else {
            detail->addWidget(ppc::xhtml(
                "<p class=\"text-muted\">No materials defined yet. Add materials in the Materials &amp; Expenses view.</p>"));
        }
    }

    // ---- Add new component form ---------------------------------------------
    content_->addWidget(ppc::xhtml("<h3 class=\"section-title\">Add New Component</h3>"));
    auto addForm = content_->addWidget(std::make_unique<Wt::WContainerWidget>());
    addForm->setStyleClass("add-form");

    auto row1 = addForm->addWidget(std::make_unique<Wt::WContainerWidget>());
    row1->setStyleClass("form-row-inline");
    row1->addWidget(ppc::xhtml("<span class=\"field-label\">Name:</span>"));
    auto nameIn = row1->addWidget(std::make_unique<Wt::WLineEdit>());
    nameIn->setPlaceholderText("Component name");
    nameIn->setStyleClass("input-field");

    row1->addWidget(ppc::xhtml("<span class=\"field-label\">Phase:</span>"));
    auto phaseCombo = row1->addWidget(std::make_unique<Wt::WComboBox>());
    phaseCombo->setStyleClass("input-field");
    phaseCombo->addItem("— None —");
    for (auto& ph : data_.phases) phaseCombo->addItem(ph.name);

    auto row2 = addForm->addWidget(std::make_unique<Wt::WContainerWidget>());
    row2->setStyleClass("form-row-inline");
    row2->addWidget(ppc::xhtml("<span class=\"field-label\">Description:</span>"));
    auto descIn = row2->addWidget(std::make_unique<Wt::WLineEdit>());
    descIn->setPlaceholderText("Brief description");
    descIn->setStyleClass("input-field input-wide");

    auto row3 = addForm->addWidget(std::make_unique<Wt::WContainerWidget>());
    row3->setStyleClass("form-row-inline");
    row3->addWidget(ppc::xhtml("<span class=\"field-label\">Statement of Work:</span>"));
    auto sowIn = row3->addWidget(std::make_unique<Wt::WTextArea>());
    sowIn->setPlaceholderText("Detailed statement of work for this component...");
    sowIn->setRows(4);
    sowIn->setStyleClass("input-textarea");

    auto addCompBtn = addForm->addWidget(std::make_unique<Wt::WPushButton>("Add Component"));
    addCompBtn->setStyleClass("btn btn-primary mt-1");
    addCompBtn->clicked().connect([this, nameIn, descIn, sowIn, phaseCombo]() {
        auto name = nameIn->text().toUTF8();
        if (!name.empty()) {
            ppc::Component nc;
            nc.id = data_.genId();
            nc.projectId = 1;
            nc.name = name;
            nc.description = descIn->text().toUTF8();
            nc.statementOfWork = sowIn->text().toUTF8();
            nc.sortOrder = (int)data_.components.size() + 1;
            int phIdx = phaseCombo->currentIndex() - 1;
            if (phIdx >= 0 && phIdx < (int)data_.phases.size())
                nc.phaseId = data_.phases[phIdx].id;
            data_.components.push_back(nc);
            refresh();
        }
    });
}
