// =============================================================================
// PdfExporter.cpp — PDF / HTML document generation for estimates
// =============================================================================

#include "PdfExporter.h"
#include <fstream>
#include <sstream>

#ifdef HAVE_HPDF
#include <hpdf.h>
#endif

namespace ppc {

std::string PdfExporter::generateHtml(const ProjectData& data, const Estimate& estimate)
{
    std::ostringstream html;

    html << "<!DOCTYPE html><html><head><meta charset='utf-8'>"
         << "<title>" << estimate.estimateNumber << " — " << estimate.name << "</title>"
         << "<style>"
         << "body{font-family:'Segoe UI',Arial,sans-serif;margin:40px;color:#1a1a2e;line-height:1.6;}"
         << "h1{color:#0a1628;border-bottom:3px solid #2563eb;padding-bottom:8px;}"
         << "h2{color:#1e3a5f;margin-top:30px;}"
         << "table{width:100%;border-collapse:collapse;margin:16px 0;}"
         << "th,td{border:1px solid #d1d5db;padding:8px 12px;text-align:left;}"
         << "th{background:#f1f5f9;font-weight:600;}"
         << ".right{text-align:right;}"
         << ".total{font-weight:700;background:#e8f0fe;}"
         << ".metric{display:inline-block;margin:8px 16px 8px 0;padding:12px 20px;"
         << "background:#f8fafc;border:1px solid #e2e8f0;border-radius:6px;}"
         << ".metric .label{font-size:12px;color:#64748b;}"
         << ".metric .value{font-size:20px;font-weight:700;color:#1e3a5f;}"
         << ".sig{margin-top:40px;padding-top:20px;border-top:2px solid #1e3a5f;}"
         << ".sig-line{font-family:'Brush Script MT',cursive;font-size:28px;color:#1e3a5f;"
         << "border-bottom:1px solid #000;display:inline-block;min-width:250px;padding:4px;}"
         << ".terms{font-size:11px;color:#64748b;white-space:pre-line;margin-top:30px;"
         << "padding:16px;background:#f8fafc;border:1px solid #e2e8f0;border-radius:4px;}"
         << "@media print{body{margin:20px;}}"
         << "</style></head><body>";

    // Header
    html << "<h1>" << data.company.name << "</h1>";
    html << "<h2>" << estimate.estimateNumber << " — " << estimate.name << "</h2>";
    html << "<p><strong>Client:</strong> " << estimate.clientName
         << " (" << estimate.clientCompany << ")<br>"
         << "<strong>Date:</strong> " << formatDate(estimate.createdAt) << "<br>"
         << "<strong>Valid Until:</strong> " << formatDate(estimate.validUntil) << "<br>"
         << "<strong>Version:</strong> " << estimate.version << "</p>";

    // Summary metrics
    html << "<div>";
    html << "<div class='metric'><div class='label'>Total Hours</div><div class='value'>"
         << formatNumber(estimate.totalHours, 0) << "</div></div>";
    html << "<div class='metric'><div class='label'>Total Cost</div><div class='value'>"
         << formatCurrency(estimate.totalCost) << "</div></div>";
    html << "<div class='metric'><div class='label'>Sell Price</div><div class='value'>"
         << formatCurrency(estimate.totalSell) << "</div></div>";
    html << "<div class='metric'><div class='label'>Markup</div><div class='value'>"
         << formatPercent(estimate.markupPct) << "</div></div>";
    html << "</div>";

    // Component breakdown
    html << "<h2>Component Breakdown</h2>";
    html << "<table><tr><th>Component</th><th>Statement of Work</th>"
         << "<th class='right'>Hours</th><th class='right'>Labor</th>"
         << "<th class='right'>Materials</th>"
         << "<th class='right'>Total Cost</th>"
         << "<th class='right'>Sell Price</th></tr>";

    double totalH = 0, totalC = 0, totalS = 0, totalMat = 0;
    for (int cid : estimate.componentIds) {
        for (auto& comp : data.components) {
            if (comp.id == cid) {
                double h = comp.totalHours();
                double labor = data.componentLaborCost(comp);
                double mat = data.componentMaterialCost(comp);
                double c = data.componentCost(comp);
                double s = c * (1.0 + estimate.markupPct / 100.0);
                totalH += h; totalC += c; totalS += s; totalMat += mat;

                html << "<tr><td><strong>" << comp.name << "</strong><br>"
                     << "<small>" << comp.description << "</small></td>"
                     << "<td><small>" << comp.statementOfWork << "</small></td>"
                     << "<td class='right'>" << formatNumber(h, 0) << "</td>"
                     << "<td class='right'>" << formatCurrency(labor) << "</td>"
                     << "<td class='right'>" << (mat > 0 ? formatCurrency(mat) : "&#8212;") << "</td>"
                     << "<td class='right'>" << formatCurrency(c) << "</td>"
                     << "<td class='right'>" << formatCurrency(s) << "</td></tr>";
                break;
            }
        }
    }

    html << "<tr class='total'><td>Total</td><td></td>"
         << "<td class='right'>" << formatNumber(totalH, 0) << "</td>"
         << "<td class='right'>" << formatCurrency(totalC - totalMat) << "</td>"
         << "<td class='right'>" << formatCurrency(totalMat) << "</td>"
         << "<td class='right'>" << formatCurrency(totalC) << "</td>"
         << "<td class='right'>" << formatCurrency(totalS) << "</td></tr>";
    html << "</table>";

    // Phase breakdown
    if (!data.phases.empty()) {
        html << "<h2>Phased Pricing</h2>";
        html << "<table><tr><th>Phase</th><th>Weeks</th>"
             << "<th class='right'>Hours</th><th class='right'>Cost</th>"
             << "<th class='right'>Sell Price</th></tr>";

        for (int p = 0; p < (int)data.phases.size(); p++) {
            auto& ph = data.phases[p];
            double hrs = data.getPhaseHours(p);
            double cost = data.getPhaseCost(p);
            double sell = data.getPhaseSell(p);

            html << "<tr><td>" << ph.name << "</td>"
                 << "<td>W" << ph.startWeek << " – W" << ph.endWeek << "</td>"
                 << "<td class='right'>" << formatNumber(hrs, 0) << "</td>"
                 << "<td class='right'>" << formatCurrency(cost) << "</td>"
                 << "<td class='right'>" << formatCurrency(sell) << "</td></tr>";
        }
        html << "</table>";
    }

    // Terms
    if (!estimate.termsAndConditions.empty()) {
        html << "<h2>Terms &amp; Conditions</h2>";
        html << "<div class='terms'>" << estimate.termsAndConditions << "</div>";
    }

    // Signature block
    html << "<div class='sig'>";
    if (!estimate.signatures.empty()) {
        for (auto& sig : estimate.signatures) {
            html << "<p><span class='sig-line'>" << sig.signatureData << "</span></p>"
                 << "<p>" << sig.signerName << ", " << sig.signerTitle << "<br>"
                 << sig.signerEmail << "<br>"
                 << "Signed: " << formatTimestamp(sig.signedAt) << "</p>";
        }
    } else {
        html << "<p>Signature: ___________________________</p>"
             << "<p>Name: ___________________________</p>"
             << "<p>Title: ___________________________</p>"
             << "<p>Date: ___________________________</p>";
    }
    html << "</div>";

    html << "</body></html>";
    return html.str();
}

std::string PdfExporter::exportEstimate(const ProjectData& data, const Estimate& estimate,
                                         const std::string& outputDir)
{
    std::string filename = outputDir + "/" + estimate.estimateNumber + ".html";

#ifdef HAVE_HPDF
    // When libharu is available, generate a proper PDF
    filename = outputDir + "/" + estimate.estimateNumber + ".pdf";

    HPDF_Doc pdf = HPDF_New(nullptr, nullptr);
    if (!pdf) return "";

    HPDF_UseUTFEncodings(pdf);
    HPDF_SetCurrentEncoder(pdf, "UTF-8");

    HPDF_Font font = HPDF_GetFont(pdf, "Helvetica", nullptr);
    HPDF_Font fontBold = HPDF_GetFont(pdf, "Helvetica-Bold", nullptr);

    HPDF_Page page = HPDF_AddPage(pdf);
    HPDF_Page_SetSize(page, HPDF_PAGE_SIZE_LETTER, HPDF_PAGE_PORTRAIT);

    float w = HPDF_Page_GetWidth(page);
    float h = HPDF_Page_GetHeight(page);
    float y = h - 50;
    float margin = 50;

    // Title
    HPDF_Page_SetFontAndSize(page, fontBold, 18);
    HPDF_Page_BeginText(page);
    HPDF_Page_TextOut(page, margin, y, data.company.name.c_str());
    HPDF_Page_EndText(page);
    y -= 30;

    HPDF_Page_SetFontAndSize(page, fontBold, 14);
    HPDF_Page_BeginText(page);
    std::string title = estimate.estimateNumber + " - " + estimate.name;
    HPDF_Page_TextOut(page, margin, y, title.c_str());
    HPDF_Page_EndText(page);
    y -= 25;

    // Client info
    HPDF_Page_SetFontAndSize(page, font, 10);
    HPDF_Page_BeginText(page);
    std::string clientLine = "Client: " + estimate.clientName + " (" + estimate.clientCompany + ")";
    HPDF_Page_TextOut(page, margin, y, clientLine.c_str());
    HPDF_Page_EndText(page);
    y -= 15;

    HPDF_Page_BeginText(page);
    std::string dateLine = "Valid Until: " + formatDate(estimate.validUntil);
    HPDF_Page_TextOut(page, margin, y, dateLine.c_str());
    HPDF_Page_EndText(page);
    y -= 25;

    // Summary
    HPDF_Page_SetFontAndSize(page, fontBold, 11);
    HPDF_Page_BeginText(page);
    std::string summary = "Total: " + formatCurrency(estimate.totalSell) +
                          "  |  Hours: " + formatNumber(estimate.totalHours, 0) +
                          "  |  Markup: " + formatPercent(estimate.markupPct);
    HPDF_Page_TextOut(page, margin, y, summary.c_str());
    HPDF_Page_EndText(page);
    y -= 30;

    // Component list
    HPDF_Page_SetFontAndSize(page, fontBold, 12);
    HPDF_Page_BeginText(page);
    HPDF_Page_TextOut(page, margin, y, "Components");
    HPDF_Page_EndText(page);
    y -= 20;

    HPDF_Page_SetFontAndSize(page, font, 9);
    for (int cid : estimate.componentIds) {
        for (auto& comp : data.components) {
            if (comp.id == cid) {
                if (y < 80) {
                    page = HPDF_AddPage(pdf);
                    HPDF_Page_SetSize(page, HPDF_PAGE_SIZE_LETTER, HPDF_PAGE_PORTRAIT);
                    y = h - 50;
                    HPDF_Page_SetFontAndSize(page, font, 9);
                }

                double hrs2 = comp.totalHours();
                double sell2 = data.componentCost(comp) * (1.0 + estimate.markupPct / 100.0);
                std::string line = comp.name + "  —  " +
                                   formatNumber(hrs2, 0) + " hrs  —  " + formatCurrency(sell2);
                HPDF_Page_BeginText(page);
                HPDF_Page_TextOut(page, margin, y, line.c_str());
                HPDF_Page_EndText(page);
                y -= 14;
                break;
            }
        }
    }

    HPDF_SaveToFile(pdf, filename.c_str());
    HPDF_Free(pdf);

#else
    // Fallback: write HTML document
    std::string htmlContent = generateHtml(data, estimate);
    std::ofstream out(filename);
    out << htmlContent;
    out.close();
#endif

    return filename;
}

} // namespace ppc
