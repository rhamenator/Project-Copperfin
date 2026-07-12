// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#include "license_classifier.h"

#include <limits>

namespace copperfin::licensing {

namespace {

std::string get_string(const PayloadFields& fields, const std::string& key, const std::string& fallback) {
    const auto it = fields.find(key);
    if (it == fields.end() || it->second.kind != PayloadValue::Kind::string_value) {
        return fallback;
    }
    return it->second.as_string;
}

bool get_string_required(const PayloadFields& fields, const std::string& key, std::string& out) {
    const auto it = fields.find(key);
    if (it == fields.end() || it->second.kind != PayloadValue::Kind::string_value) {
        return false;
    }
    out = it->second.as_string;
    return true;
}

bool get_integer_as_int(const PayloadFields& fields, const std::string& key, int fallback, int& out) {
    const auto it = fields.find(key);
    if (it == fields.end() || it->second.kind != PayloadValue::Kind::integer_value) {
        out = fallback;
        return true;
    }

    const long long value = it->second.as_integer;
    if (value < static_cast<long long>(std::numeric_limits<int>::min()) ||
        value > static_cast<long long>(std::numeric_limits<int>::max())) {
        return false;
    }

    out = static_cast<int>(value);
    return true;
}

}  // namespace

LicenseStatus classify_verified_payload(
    const PayloadFields& fields,
    int current_major_version,
    const std::string& current_date_iso8601) {
    LicenseStatus status;

    std::string license_type;
    if (!get_string_required(fields, "license_type", license_type)) {
        status.state = LicenseState::malformed;
        status.diagnostic = "missing or invalid license_type";
        return status;
    }

    status.license_type = license_type;
    status.license_id = get_string(fields, "license_id", "");
    status.pricing_model = get_string(fields, "pricing_model", "");
    status.licensee_name = get_string(fields, "licensee_name", "");
    status.licensee_email = get_string(fields, "licensee_email", "");
    status.issued_date = get_string(fields, "issued_date", "");
    status.subscription_expires = get_string(fields, "subscription_expires", "");

    int seats = 0;
    int perpetual_max_major_version = 0;
    if (!get_integer_as_int(fields, "seats", 0, seats)) {
        status.state = LicenseState::malformed;
        status.diagnostic = "integer field out of range: seats";
        return status;
    }
    if (!get_integer_as_int(fields, "perpetual_max_major_version", 0, perpetual_max_major_version)) {
        status.state = LicenseState::malformed;
        status.diagnostic = "integer field out of range: perpetual_max_major_version";
        return status;
    }
    status.seats = seats;
    status.perpetual_max_major_version = perpetual_max_major_version;

    if (license_type == "perpetual") {
        if (status.perpetual_max_major_version <= 0) {
            status.state = LicenseState::malformed;
            status.diagnostic = "perpetual license missing perpetual_max_major_version";
            return status;
        }
        // Being below the current major version never restricts anything --
        // see LicenseState::perpetual_out_of_version's doc comment. This
        // classification is purely informational.
        status.state = (status.perpetual_max_major_version >= current_major_version)
            ? LicenseState::perpetual_current
            : LicenseState::perpetual_out_of_version;
        return status;
    }

    if (license_type == "subscription") {
        if (status.subscription_expires.empty()) {
            status.state = LicenseState::malformed;
            status.diagnostic = "subscription license missing subscription_expires";
            return status;
        }
        // ISO-8601 "YYYY-MM-DD" strings compare correctly with plain
        // lexicographic string comparison.
        status.state = (status.subscription_expires >= current_date_iso8601)
            ? LicenseState::subscription_active
            : LicenseState::subscription_expired;
        return status;
    }

    status.state = LicenseState::malformed;
    status.diagnostic = "unrecognized license_type: " + license_type;
    return status;
}

}  // namespace copperfin::licensing
