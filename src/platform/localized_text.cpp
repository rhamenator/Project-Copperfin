#include "localized_text.h"

namespace copperfin::platform {

std::string platform_text(
    std::string_view key,
    const localization::PlaceholderMap& placeholders) {
    static const localization::LocalizedCatalog catalog =
        localization::load_catalogs(localization::resolve_catalog_root(), localization::select_locale());
    return catalog.translate(key, placeholders);
}

}  // namespace copperfin::platform
