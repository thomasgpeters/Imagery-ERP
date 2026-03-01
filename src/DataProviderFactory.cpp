// =============================================================================
// DataProviderFactory.cpp — Creates the appropriate IDataProvider based on
//                           architecture_mode in app_config.yaml
// =============================================================================

#include "IDataProvider.h"
#include "AppConfig.h"
#include "StandaloneDataProvider.h"
#include "EnterpriseDataProvider.h"

namespace ppc {

std::unique_ptr<IDataProvider> createDataProvider(const std::string& configPath)
{
    AppConfig config;
    config.loadFromFile(configPath);

    std::unique_ptr<IDataProvider> provider;

    if (config.mode == ArchitectureMode::Enterprise) {
        provider = std::make_unique<EnterpriseDataProvider>(config.enterprise);
    } else {
        provider = std::make_unique<StandaloneDataProvider>(config.standalone);
    }

    provider->initialize();
    return provider;
}

} // namespace ppc
