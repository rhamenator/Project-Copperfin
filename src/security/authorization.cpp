#include "copperfin/security/authorization.h"

#include <algorithm>

namespace copperfin::security {

bool role_has_permission(
    const NativeSecurityProfile& profile,
    const std::string& role_id,
    const std::string& permission_id) {
    const auto role_it = std::find_if(profile.roles.begin(), profile.roles.end(), [&](const NativeRole& role) {
        return role.id == role_id;
    });
    if (role_it == profile.roles.end()) {
        return false;
    }

    return std::find(role_it->permission_ids.begin(), role_it->permission_ids.end(), permission_id)
        != role_it->permission_ids.end();
}

}  // namespace copperfin::security
