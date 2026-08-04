// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

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

bool is_ascii_digit(char value) {
    return value >= '0' && value <= '9';
}

bool is_leap_year(int year) {
    return (year % 4 == 0) && ((year % 100 != 0) || (year % 400 == 0));
}

bool is_canonical_calendar_date(const std::string& value) {
    if (value.size() != 10U || value[4U] != '-' || value[7U] != '-') {
        return false;
    }

    for (std::size_t index = 0U; index < value.size(); ++index) {
        if (index == 4U || index == 7U) {
            continue;
        }
        if (!is_ascii_digit(value[index])) {
            return false;
        }
    }

    const int year =
        ((value[0U] - '0') * 1000) +
        ((value[1U] - '0') * 100) +
        ((value[2U] - '0') * 10) +
        (value[3U] - '0');
    const int month = ((value[5U] - '0') * 10) + (value[6U] - '0');
    const int day = ((value[8U] - '0') * 10) + (value[9U] - '0');

    if (month < 1 || month > 12 || day < 1) {
        return false;
    }

    constexpr int month_lengths[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    int maximum_day = month_lengths[month - 1];
    if (month == 2 && is_leap_year(year)) {
        maximum_day = 29;
    }
    return day <= maximum_day;
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
        status.diagnostic_key = "Licensing.Error.MissingOrInvalidLicenseType";
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
        status.diagnostic_key = "Licensing.Error.IntegerFieldOutOfRange";
        status.diagnostic_argument = "seats";
        return status;
    }
    if (!get_integer_as_int(fields, "perpetual_max_major_version", 0, perpetual_max_major_version)) {
        status.state = LicenseState::malformed;
        status.diagnostic = "integer field out of range: perpetual_max_major_version";
        status.diagnostic_key = "Licensing.Error.IntegerFieldOutOfRange";
        status.diagnostic_argument = "perpetual_max_major_version";
        return status;
    }
    status.seats = seats;
    status.perpetual_max_major_version = perpetual_max_major_version;

    if (license_type == "perpetual") {
        if (status.perpetual_max_major_version <= 0) {
            status.state = LicenseState::malformed;
            status.diagnostic = "perpetual license missing perpetual_max_major_version";
            status.diagnostic_key = "Licensing.Error.PerpetualVersionMissing";
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
            status.diagnostic_key = "Licensing.Error.SubscriptionExpiryMissing";
            return status;
        }
        if (!is_canonical_calendar_date(status.subscription_expires)) {
            status.state = LicenseState::malformed;
            status.diagnostic = "subscription license has invalid subscription_expires";
            status.diagnostic_key = "Licensing.Error.SubscriptionExpiryInvalid";
            return status;
        }
        if (!is_canonical_calendar_date(current_date_iso8601)) {
            status.state = LicenseState::malformed;
            status.diagnostic = "current date is not canonical YYYY-MM-DD";
            status.diagnostic_key = "Licensing.Error.CurrentDateInvalid";
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
    status.diagnostic_key = "Licensing.Error.UnrecognizedLicenseType";
    status.diagnostic_argument = license_type;
    return status;
}

}  // namespace copperfin::licensing
