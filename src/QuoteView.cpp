// =============================================================================
// QuoteView.cpp — Quote builder with e-signature, PDF export, email link
// =============================================================================

#include "QuoteView.h"
#include <Wt/WText.h>
#include <Wt/WTable.h>
#include <Wt/WLineEdit.h>
#include <Wt/WTextArea.h>
#include <Wt/WDoubleSpinBox.h>
#include <Wt/WPushButton.h>
#include <Wt/WComboBox.h>
#include <Wt/WMessageBox.h>

QuoteView::QuoteView(ppc::ProjectData& data)
    : data_(data)
{
    setStyleClass("view-panel");
    content_ = addWidget(std::make_unique<Wt::WContainerWidget>());
}

void QuoteView::refresh()
{
    content_->clear();

    content_->addWidget(std::make_unique<Wt::WText>(
        "<h2 class=\"view-title\">Quote Builder</h2>"
        "<p class=\"view-subtitle\">Assemble estimates, generate quotes, request e-signatures</p>"
    ));

    // ---- Estimate list / selector -------------------------------------------
    if (data_.estimates.empty()) {
        content_->addWidget(std::make_unique<Wt::WText>(
            "<div class=\"info-callout\">No estimates yet. "
            "Create one below to start building a quote.</div>"));
    }

    for (int ei = 0; ei < (int)data_.estimates.size(); ei++) {
        auto& est = data_.estimates[ei];
        data_.recalcEstimate(est);

        auto estCard = content_->addWidget(std::make_unique<Wt::WContainerWidget>());
        estCard->setStyleClass("estimate-card");

        // Header
        auto statusBadge = "<span class=\"badge badge-" +
            std::string(est.status == "Approved" ? "approved" :
                        est.status == "Signed" ? "active" :
                        est.status == "Sent" ? "estimated" : "draft") +
            "\">" + est.status + "</span>";

        estCard->addWidget(std::make_unique<Wt::WText>(
            "<div class=\"est-header\">"
            "<div class=\"est-number\">" + est.estimateNumber + " " + statusBadge + "</div>"
            "<div class=\"est-name\">" + est.name + "</div>"
            "</div>"
        ));

        // Client info
        estCard->addWidget(std::make_unique<Wt::WText>(
            "<div class=\"est-client\">"
            "<span class=\"field-label\">Client:</span> " + est.clientName +
            " (" + est.clientCompany + ")"
            " &mdash; <span class=\"field-label\">Valid Until:</span> " + est.validUntil +
            "</div>"
        ));

        // Summary metrics
        auto metrics = estCard->addWidget(std::make_unique<Wt::WContainerWidget>());
        metrics->setStyleClass("est-metrics");

        auto addM = [&](const std::string& l, const std::string& v) {
            metrics->addWidget(std::make_unique<Wt::WText>(
                "<div class=\"est-metric\"><div class=\"em-label\">" + l +
                "</div><div class=\"em-value\">" + v + "</div></div>"));
        };

        addM("Total Hours", ppc::formatNumber(est.totalHours, 0));
        addM("Total Cost",  ppc::formatCurrency(est.totalCost));
        addM("Markup",      ppc::formatPercent(est.markupPct));
        addM("Sell Price",   ppc::formatCurrency(est.totalSell));
        addM("Margin",      ppc::formatCurrency(est.totalSell - est.totalCost));

        // Component breakdown
        estCard->addWidget(std::make_unique<Wt::WText>("<h4 class=\"detail-section\">Included Components</h4>"));
        auto compTable = estCard->addWidget(std::make_unique<Wt::WTable>());
        compTable->setStyleClass("data-table data-table-compact");
        compTable->setHeaderCount(1);

        int c = 0;
        for (auto& h : {"Component", "Hours", "Cost", "Sell"}) {
            compTable->elementAt(0, c)->addWidget(std::make_unique<Wt::WText>(h));
            compTable->elementAt(0, c)->setStyleClass(c >= 1 ? "cell-right" : "");
            c++;
        }

        int crow = 1;
        for (int cid : est.componentIds) {
            for (auto& comp : data_.components) {
                if (comp.id == cid) {
                    double hrs = comp.totalHours();
                    double cost = data_.componentCost(comp);
                    double sell = data_.componentSell(comp);

                    compTable->elementAt(crow, 0)->addWidget(std::make_unique<Wt::WText>(comp.name));
                    compTable->elementAt(crow, 1)->addWidget(std::make_unique<Wt::WText>(ppc::formatNumber(hrs, 0)));
                    compTable->elementAt(crow, 1)->setStyleClass("cell-right");
                    compTable->elementAt(crow, 2)->addWidget(std::make_unique<Wt::WText>(ppc::formatCurrency(cost)));
                    compTable->elementAt(crow, 2)->setStyleClass("cell-right");
                    compTable->elementAt(crow, 3)->addWidget(std::make_unique<Wt::WText>(ppc::formatCurrency(sell)));
                    compTable->elementAt(crow, 3)->setStyleClass("cell-right");
                    crow++;
                    break;
                }
            }
        }

        // Terms
        if (!est.termsAndConditions.empty()) {
            estCard->addWidget(std::make_unique<Wt::WText>("<h4 class=\"detail-section\">Terms &amp; Conditions</h4>"));
            estCard->addWidget(std::make_unique<Wt::WText>(
                "<pre class=\"terms-text\">" + est.termsAndConditions + "</pre>"));
        }

        // ---- E-Signature section --------------------------------------------
        estCard->addWidget(std::make_unique<Wt::WText>("<h4 class=\"detail-section\">E-Signature</h4>"));

        if (!est.signatures.empty()) {
            for (auto& sig : est.signatures) {
                estCard->addWidget(std::make_unique<Wt::WText>(
                    "<div class=\"signature-block\">"
                    "<div class=\"sig-line\">" + sig.signatureData + "</div>"
                    "<div class=\"sig-details\">" + sig.signerName + ", " + sig.signerTitle +
                    " &mdash; " + sig.signedAt + "</div>"
                    "</div>"));
            }
        } else {
            auto sigForm = estCard->addWidget(std::make_unique<Wt::WContainerWidget>());
            sigForm->setStyleClass("signature-form");

            auto sigRow1 = sigForm->addWidget(std::make_unique<Wt::WContainerWidget>());
            sigRow1->setStyleClass("form-row-inline");

            sigRow1->addWidget(std::make_unique<Wt::WText>("<span class=\"field-label\">Full Name:</span>"));
            auto sigName = sigRow1->addWidget(std::make_unique<Wt::WLineEdit>());
            sigName->setPlaceholderText("Signer full name");
            sigName->setStyleClass("input-field");

            sigRow1->addWidget(std::make_unique<Wt::WText>("<span class=\"field-label\">Title:</span>"));
            auto sigTitle = sigRow1->addWidget(std::make_unique<Wt::WLineEdit>());
            sigTitle->setPlaceholderText("Job title");
            sigTitle->setStyleClass("input-field");

            auto sigRow2 = sigForm->addWidget(std::make_unique<Wt::WContainerWidget>());
            sigRow2->setStyleClass("form-row-inline");

            sigRow2->addWidget(std::make_unique<Wt::WText>("<span class=\"field-label\">Email:</span>"));
            auto sigEmail = sigRow2->addWidget(std::make_unique<Wt::WLineEdit>());
            sigEmail->setPlaceholderText("signer@company.com");
            sigEmail->setStyleClass("input-field");

            sigRow2->addWidget(std::make_unique<Wt::WText>("<span class=\"field-label\">Company:</span>"));
            auto sigCompany = sigRow2->addWidget(std::make_unique<Wt::WLineEdit>());
            sigCompany->setPlaceholderText("Company name");
            sigCompany->setStyleClass("input-field");

            sigForm->addWidget(std::make_unique<Wt::WText>("<span class=\"field-label\">Type your signature:</span>"));
            auto sigInput = sigForm->addWidget(std::make_unique<Wt::WLineEdit>());
            sigInput->setPlaceholderText("Type your full name as signature");
            sigInput->setStyleClass("input-field signature-input");

            auto signBtn = sigForm->addWidget(std::make_unique<Wt::WPushButton>("Sign Estimate"));
            signBtn->setStyleClass("btn btn-primary mt-1");
            int estIdx = ei;
            signBtn->clicked().connect([this, estIdx, sigName, sigTitle, sigEmail, sigCompany, sigInput]() {
                auto name = sigName->text().toUTF8();
                auto sigText = sigInput->text().toUTF8();
                if (!name.empty() && !sigText.empty()) {
                    ppc::ESignature sig;
                    sig.id = data_.genId();
                    sig.estimateId = data_.estimates[estIdx].id;
                    sig.signerName = name;
                    sig.signerTitle = sigTitle->text().toUTF8();
                    sig.signerEmail = sigEmail->text().toUTF8();
                    sig.signerCompany = sigCompany->text().toUTF8();
                    sig.signatureData = sigText;
                    sig.signatureType = "typed";
                    sig.signedAt = ppc::currentTimestamp();
                    data_.estimates[estIdx].signatures.push_back(sig);
                    data_.estimates[estIdx].status = "Signed";
                    refresh();
                }
            });
        }

        // ---- Action buttons -------------------------------------------------
        auto actions = estCard->addWidget(std::make_unique<Wt::WContainerWidget>());
        actions->setStyleClass("est-actions");

        auto pdfBtn = actions->addWidget(std::make_unique<Wt::WPushButton>("Export PDF"));
        pdfBtn->setStyleClass("btn btn-secondary");
        pdfBtn->clicked().connect([this]() {
            auto mb = addChild(std::make_unique<Wt::WMessageBox>(
                "Export PDF",
                "PDF export will generate a downloadable estimate document.\n\n"
                "In production, this uses libharu (HPDF) to create a formatted PDF "
                "with company branding, component breakdown, pricing, terms, and signature blocks.\n\n"
                "The PDF endpoint would be: /api/estimates/{id}/pdf",
                Wt::Icon::Information, Wt::StandardButton::Ok));
            mb->buttonClicked().connect([mb]() { mb->accept(); });
            mb->show();
        });

        auto emailBtn = actions->addWidget(std::make_unique<Wt::WPushButton>("Email Link"));
        emailBtn->setStyleClass("btn btn-secondary");
        int estIdx2 = ei;
        emailBtn->clicked().connect([this, estIdx2]() {
            auto& est2 = data_.estimates[estIdx2];
            std::string link = "/estimate/view/" + est2.sharedLinkToken;
            auto mb = addChild(std::make_unique<Wt::WMessageBox>(
                "Email Estimate Link",
                "An email will be sent to: " + est2.clientEmail + "\n\n"
                "Online estimate link: " + link + "\n\n"
                "The client can view the estimate online and sign electronically.\n"
                "In production, this integrates with SMTP/SendGrid for delivery.",
                Wt::Icon::Information, Wt::StandardButton::Ok));
            mb->buttonClicked().connect([mb]() { mb->accept(); });
            mb->show();
            est2.status = "Sent";
            refresh();
        });

        if (est.status == "Signed") {
            auto approveBtn = actions->addWidget(std::make_unique<Wt::WPushButton>("Approve Estimate"));
            approveBtn->setStyleClass("btn btn-success");
            int estIdx3 = ei;
            approveBtn->clicked().connect([this, estIdx3]() {
                data_.estimates[estIdx3].status = "Approved";
                refresh();
            });
        }
    }

    // ---- Create new estimate ------------------------------------------------
    content_->addWidget(std::make_unique<Wt::WText>("<h3 class=\"section-title\">Create New Estimate</h3>"));
    auto form = content_->addWidget(std::make_unique<Wt::WContainerWidget>());
    form->setStyleClass("add-form");

    auto r1 = form->addWidget(std::make_unique<Wt::WContainerWidget>());
    r1->setStyleClass("form-row-inline");
    r1->addWidget(std::make_unique<Wt::WText>("<span class=\"field-label\">Name:</span>"));
    auto nameIn = r1->addWidget(std::make_unique<Wt::WLineEdit>());
    nameIn->setPlaceholderText("Estimate name");
    nameIn->setStyleClass("input-field input-wide");

    auto r2 = form->addWidget(std::make_unique<Wt::WContainerWidget>());
    r2->setStyleClass("form-row-inline");
    r2->addWidget(std::make_unique<Wt::WText>("<span class=\"field-label\">Client Name:</span>"));
    auto clientIn = r2->addWidget(std::make_unique<Wt::WLineEdit>());
    clientIn->setPlaceholderText("Client contact name");
    clientIn->setStyleClass("input-field");
    r2->addWidget(std::make_unique<Wt::WText>("<span class=\"field-label\">Client Email:</span>"));
    auto emailIn = r2->addWidget(std::make_unique<Wt::WLineEdit>());
    emailIn->setPlaceholderText("client@company.com");
    emailIn->setStyleClass("input-field");

    auto r3 = form->addWidget(std::make_unique<Wt::WContainerWidget>());
    r3->setStyleClass("form-row-inline");
    r3->addWidget(std::make_unique<Wt::WText>("<span class=\"field-label\">Markup:</span>"));
    auto markupIn = r3->addWidget(std::make_unique<Wt::WDoubleSpinBox>());
    markupIn->setRange(0, 200);
    markupIn->setValue(data_.markupPct);
    markupIn->setDecimals(1);
    markupIn->setStyleClass("input-field input-sm");
    r3->addWidget(std::make_unique<Wt::WText>("<span class=\"field-unit\">%</span>"));

    auto createBtn = form->addWidget(std::make_unique<Wt::WPushButton>("Create Estimate (all components)"));
    createBtn->setStyleClass("btn btn-primary mt-1");
    createBtn->clicked().connect([this, nameIn, clientIn, emailIn, markupIn]() {
        auto name = nameIn->text().toUTF8();
        if (!name.empty()) {
            ppc::Estimate est;
            est.id = data_.genId();
            est.projectId = 1;
            est.estimateNumber = "EST-2026-" + std::to_string(1000 + (int)data_.estimates.size());
            est.name = name;
            est.clientName = clientIn->text().toUTF8();
            est.clientEmail = emailIn->text().toUTF8();
            est.markupPct = markupIn->value();
            est.createdAt = ppc::currentDate();
            est.sharedLinkToken = "est_" + std::to_string(data_.genId());
            for (auto& c : data_.components) est.componentIds.push_back(c.id);
            data_.recalcEstimate(est);
            data_.estimates.push_back(est);
            refresh();
        }
    });
}
