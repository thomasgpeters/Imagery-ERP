#pragma once
// =============================================================================
// PdfExporter.h — PDF generation for estimates and quotes
//
// Uses libharu (HPDF) when available (compile with -DHAVE_HPDF).
// Falls back to HTML document generation when libharu is not present.
// =============================================================================

#include <string>
#include "DataModels.h"

namespace ppc {

class PdfExporter {
public:
    // Generate a PDF (or HTML fallback) for the given estimate.
    // Returns the file path of the generated document.
    static std::string exportEstimate(const ProjectData& data, const Estimate& estimate,
                                      const std::string& outputDir = "/tmp");

    // Generate an HTML version of the estimate for online viewing.
    static std::string generateHtml(const ProjectData& data, const Estimate& estimate);
};

} // namespace ppc
