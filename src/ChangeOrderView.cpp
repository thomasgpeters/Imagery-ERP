// =============================================================================
// ChangeOrderView.cpp — Change order management with approval workflow
// =============================================================================

#include "ChangeOrderView.h"
#include <Wt/WText.h>
#include <Wt/WTable.h>
#include <Wt/WLineEdit.h>
#include <Wt/WTextArea.h>
#include <Wt/WDoubleSpinBox.h>
#include <Wt/WPushButton.h>
#include <Wt/WComboBox.h>
#include <Wt/WDateEdit.h>
#include <Wt/WDate.h>

 ChangeOrderView::ChangeOrderView(ppc::ProjectData& data)
    : data_(data)
{
    setStyleClass("view-panel");
    content_ = addWidget(std::make_unique<Wt::WContainerWidget>());
}

void ChangeOrderView::refresh()
{
    content_->clear();

    content_->addWidget(ppc::xhtml(
        "<h2 class=\"view-title\">Change Orders</h2>"
        "<p class=\"view-subtitle\">Track and manage modifications to approved estimates</p>"
    ));

    // ---- Existing change orders ---------------------------------------------
    for (int ci = 0; ci < (int)data_.changeOrders.size(); ci++) {
        auto& co = data_.changeOrders[ci];

        auto coCard = content_->addWidget(std::make_unique<Wt::WContainerWidget>());
        coCard->setStyleClass("change-order-card");

        auto statusBadge = "<span class=\"badge badge-" +
            std::string(co.status == "Approved" ? "approved" :
                        co.status == "Rejected" ? "critical" :
                        co.status == "Pending Review" ? "estimated" : "draft") +
            "\">" + co.status + "</span>";

        coCard->addWidget(ppc::xhtml(
            "<div class=\"co-header\">"
            "<div class=\"co-number\">" + co.changeOrderNumber + " " + statusBadge + "</div>"
            "<div class=\"co-title\">" + co.title + "</div>"
            "</div>"
        ));

        coCard->addWidget(ppc::xhtml(
            "<div class=\"co-meta\">"
            "<span><strong>Requested by:</strong> " + co.requestedBy + "</span>"
            "<span><strong>Date:</strong> " + ppc::formatDate(co.requestedDate) + "</span>"
            "<span><strong>Schedule Impact:</strong> " +
            (co.scheduleImpactWeeks > 0 ? "+" + std::to_string(co.scheduleImpactWeeks) + " weeks" : "None") +
            "</span></div>"
        ));

        // Description and reason
        coCard->addWidget(ppc::xhtml(
            "<div class=\"co-description\">"
            "<p><strong>Description:</strong> " + co.description + "</p>"
            "<p><strong>Reason:</strong> " + co.reason + "</p>"
            "<p><strong>Impact:</strong> " + co.impactSummary + "</p>"
            "</div>"
        ));

        // Financial impact
        auto impactMetrics = coCard->addWidget(std::make_unique<Wt::WContainerWidget>());
        impactMetrics->setStyleClass("co-impact-metrics");

        auto addImpact = [&](const std::string& l, const std::string& v, const std::string& cls) {
            impactMetrics->addWidget(ppc::xhtml(
                "<div class=\"co-impact " + cls + "\"><div class=\"ci-label\">" + l +
                "</div><div class=\"ci-value\">" + v + "</div></div>"));
        };

        addImpact("Additional Hours", "+" + ppc::formatNumber(co.additionalHours, 0), "");
        addImpact("Additional Cost", "+" + ppc::formatCurrency(co.additionalCost), "");
        addImpact("Additional Sell", "+" + ppc::formatCurrency(co.additionalSell), "");

        // Line items
        if (!co.items.empty()) {
            coCard->addWidget(ppc::xhtml("<h4 class=\"detail-section\">Change Items</h4>"));
            auto itemTable = coCard->addWidget(std::make_unique<Wt::WTable>());
            itemTable->setStyleClass("data-table data-table-compact");
            itemTable->setHeaderCount(1);

            int c = 0;
            for (auto& h : {"Action", "Description", "Role", "Hours", "Cost"}) {
                itemTable->elementAt(0, c)->addWidget(std::make_unique<Wt::WText>(h));
                itemTable->elementAt(0, c)->setStyleClass(c >= 3 ? "cell-right" : "");
                c++;
            }

            for (int ii = 0; ii < (int)co.items.size(); ii++) {
                int row = ii + 1;
                auto& item = co.items[ii];
                auto* role = data_.findRole(item.roleId);
                std::string roleName = role ? role->name : "—";

                auto actionBadge = "<span class=\"badge badge-" +
                    std::string(item.action == "Add" ? "approved" :
                                item.action == "Remove" ? "critical" : "estimated") +
                    "\">" + item.action + "</span>";

                itemTable->elementAt(row, 0)->addWidget(ppc::xhtml(actionBadge));
                itemTable->elementAt(row, 1)->addWidget(std::make_unique<Wt::WText>(item.description));
                itemTable->elementAt(row, 2)->addWidget(std::make_unique<Wt::WText>(roleName));
                itemTable->elementAt(row, 3)->addWidget(std::make_unique<Wt::WText>(
                    (item.hoursDelta >= 0 ? "+" : "") + ppc::formatNumber(item.hoursDelta, 0)));
                itemTable->elementAt(row, 3)->setStyleClass("cell-right");
                itemTable->elementAt(row, 4)->addWidget(std::make_unique<Wt::WText>(
                    (item.costDelta >= 0 ? "+" : "") + ppc::formatCurrency(item.costDelta)));
                itemTable->elementAt(row, 4)->setStyleClass("cell-right");
            }
        }

        // E-Signature for change order
        coCard->addWidget(ppc::xhtml("<h4 class=\"detail-section\">Approval Signature</h4>"));

        if (!co.signatures.empty()) {
            for (auto& sig : co.signatures) {
                coCard->addWidget(ppc::xhtml(
                    "<div class=\"signature-block\">"
                    "<div class=\"sig-line\">" + sig.signatureData + "</div>"
                    "<div class=\"sig-details\">" + sig.signerName + ", " + sig.signerTitle +
                    " &mdash; " + ppc::formatTimestamp(sig.signedAt) + "</div></div>"));
            }
        }

        // Action buttons
        auto actions = coCard->addWidget(std::make_unique<Wt::WContainerWidget>());
        actions->setStyleClass("co-actions");

        if (co.status != "Approved" && co.status != "Rejected") {
            auto approveBtn = actions->addWidget(std::make_unique<Wt::WPushButton>("Approve"));
            approveBtn->setStyleClass("btn btn-success");
            int coIdx = ci;
            approveBtn->clicked().connect([this, coIdx]() {
                data_.changeOrders[coIdx].status = "Approved";
                data_.changeOrders[coIdx].approvedBy = "Sarah Chen";
                data_.changeOrders[coIdx].approvedDate = ppc::currentDate();
                refresh();
            });

            auto rejectBtn = actions->addWidget(std::make_unique<Wt::WPushButton>("Reject"));
            rejectBtn->setStyleClass("btn btn-danger");
            rejectBtn->clicked().connect([this, coIdx]() {
                data_.changeOrders[coIdx].status = "Rejected";
                refresh();
            });
        }
    }

    // ---- Create new change order --------------------------------------------
    content_->addWidget(ppc::xhtml("<h3 class=\"section-title\">Submit New Change Order</h3>"));
    auto form = content_->addWidget(std::make_unique<Wt::WContainerWidget>());
    form->setStyleClass("add-form");

    auto r1 = form->addWidget(std::make_unique<Wt::WContainerWidget>());
    r1->setStyleClass("form-row-inline");
    r1->addWidget(ppc::xhtml("<span class=\"field-label\">Title:</span>"));
    auto titleIn = r1->addWidget(std::make_unique<Wt::WLineEdit>());
    titleIn->setPlaceholderText("Change order title");
    titleIn->setStyleClass("input-field input-wide");

    auto r2 = form->addWidget(std::make_unique<Wt::WContainerWidget>());
    r2->setStyleClass("form-row-inline");
    r2->addWidget(ppc::xhtml("<span class=\"field-label\">Requested By:</span>"));
    auto reqByIn = r2->addWidget(std::make_unique<Wt::WLineEdit>());
    reqByIn->setPlaceholderText("Requester name");
    reqByIn->setStyleClass("input-field");

    r2->addWidget(ppc::xhtml("<span class=\"field-label\">Request Date:</span>"));
    auto reqDateEdit = r2->addWidget(std::make_unique<Wt::WDateEdit>());
    reqDateEdit->setDate(Wt::WDate::currentDate());
    reqDateEdit->setFormat("MMM d, yyyy");
    reqDateEdit->setStyleClass("input-field input-date");

    form->addWidget(ppc::xhtml("<span class=\"field-label\">Description:</span>"));
    auto descIn = form->addWidget(std::make_unique<Wt::WTextArea>());
    descIn->setPlaceholderText("Describe what is changing...");
    descIn->setRows(3);
    descIn->setStyleClass("input-textarea");

    form->addWidget(ppc::xhtml("<span class=\"field-label\">Reason for Change:</span>"));
    auto reasonIn = form->addWidget(std::make_unique<Wt::WTextArea>());
    reasonIn->setPlaceholderText("Why is this change needed?");
    reasonIn->setRows(2);
    reasonIn->setStyleClass("input-textarea");

    auto r3 = form->addWidget(std::make_unique<Wt::WContainerWidget>());
    r3->setStyleClass("form-row-inline");
    r3->addWidget(ppc::xhtml("<span class=\"field-label\">Additional Hours:</span>"));
    auto hrsIn = r3->addWidget(std::make_unique<Wt::WDoubleSpinBox>());
    hrsIn->setRange(0, 10000);
    hrsIn->setValue(0);
    hrsIn->setDecimals(0);
    hrsIn->setStyleClass("input-field input-sm");

    r3->addWidget(ppc::xhtml("<span class=\"field-label\">Schedule Impact:</span>"));
    auto schedIn = r3->addWidget(std::make_unique<Wt::WDoubleSpinBox>());
    schedIn->setRange(0, 52);
    schedIn->setValue(0);
    schedIn->setDecimals(0);
    schedIn->setStyleClass("input-field input-sm");
    r3->addWidget(ppc::xhtml("<span class=\"field-unit\">weeks</span>"));

    auto createBtn = form->addWidget(std::make_unique<Wt::WPushButton>("Submit Change Order"));
    createBtn->setStyleClass("btn btn-primary mt-1");
    createBtn->clicked().connect([this, titleIn, reqByIn, reqDateEdit, descIn, reasonIn, hrsIn, schedIn]() {
        auto title = titleIn->text().toUTF8();
        if (!title.empty()) {
            ppc::ChangeOrder co;
            co.id = data_.genId();
            co.projectId = 1;
            co.changeOrderNumber = "CO-" + std::to_string(100 + (int)data_.changeOrders.size());
            co.title = title;
            co.requestedBy = reqByIn->text().toUTF8();
            co.description = descIn->text().toUTF8();
            co.reason = reasonIn->text().toUTF8();
            co.additionalHours = hrsIn->value();
            co.scheduleImpactWeeks = (int)schedIn->value();
            co.additionalCost = co.additionalHours * data_.getBlendedCostRate();
            co.additionalSell = co.additionalCost * (1.0 + data_.markupPct / 100.0);
            co.status = "Pending Review";
            auto reqDate = reqDateEdit->date();
            co.requestedDate = reqDate.isValid()
                ? reqDate.toString("yyyy-MM-dd").toUTF8()
                : ppc::currentDate();
            data_.changeOrders.push_back(co);
            refresh();
        }
    });
}
