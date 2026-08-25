// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "prg_engine_internal.h"
#include "prg_engine_helpers.h"
#include "copperfin/platform/path.h"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <limits>
#include <locale>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

namespace copperfin::runtime {
PrgValue canonicalize_native_olecontrol_doverb_argument(const PrgValue &verb)
{
    if (verb.kind == PrgValueKind::string)
    {
        const std::string normalized_verb = normalize_identifier(trim_copy(value_as_string(verb)));
        if (normalized_verb == "edit")
        {
            return make_number_value(-1.0);
        }
        if (normalized_verb == "open")
        {
            return make_number_value(-2.0);
        }
    }

    return verb;
}

std::optional<PrgValue> read_native_olecontrol_objectverb_by_index(
    const RuntimeOleObjectState &runtime_object,
    const std::vector<PrgValue> &arguments)
{
    const bool is_olecontrol =
        normalize_identifier(runtime_object.base_class_name) == "olecontrol" ||
        normalize_identifier(runtime_object.prog_id) == "olecontrol";
    if (!is_olecontrol)
    {
        return std::nullopt;
    }

    if (arguments.empty())
    {
        return make_empty_value();
    }

    const long long index = std::llround(value_as_number(arguments.front()));
    if (index == 0)
    {
        return make_string_value("edit");
    }
    if (index == 1)
    {
        return make_string_value("open");
    }

    return make_empty_value();
}

bool runtime_object_member_matches(
    const std::vector<std::string> &members,
    const std::string &normalized_member_name)
{
    return std::any_of(members.begin(), members.end(), [&](const std::string &member_name)
    {
        return normalize_identifier(member_name) == normalized_member_name;
    });
}

bool runtime_object_method_ends_with_suffix(
    const std::string &method_name,
    const std::string &suffix,
    std::string *stem)
{
    const std::string normalized_method = normalize_identifier(method_name);
    if (normalized_method.size() <= suffix.size() ||
        normalized_method.compare(normalized_method.size() - suffix.size(), suffix.size(), suffix) != 0)
    {
        return false;
    }

    if (normalized_method[normalized_method.size() - suffix.size() - 1U] != '_')
    {
        return false;
    }

    if (stem != nullptr)
    {
        *stem = normalized_method.substr(0U, normalized_method.size() - suffix.size() - 1U);
    }
    return true;
}

bool runtime_object_has_accessor_property(
    const RuntimeOleObjectState &runtime_object,
    const std::string &normalized_property_name)
{
    return std::any_of(runtime_object.methods.begin(), runtime_object.methods.end(), [&](const std::string &method_name)
    {
        std::string stem;
        return runtime_object_method_ends_with_suffix(method_name, "access", &stem) &&
               stem == normalized_property_name;
    });
}

bool runtime_object_has_assigner_property(
    const RuntimeOleObjectState &runtime_object,
    const std::string &normalized_property_name)
{
    return std::any_of(runtime_object.methods.begin(), runtime_object.methods.end(), [&](const std::string &method_name)
    {
        std::string stem;
        return runtime_object_method_ends_with_suffix(method_name, "assign", &stem) &&
               stem == normalized_property_name;
    });
}

std::string serialize_runtime_expression_text(const PrgValue &value)
{
    switch (value.kind)
    {
    case PrgValueKind::boolean:
        return value.boolean_value ? ".T." : ".F.";
    case PrgValueKind::number:
    case PrgValueKind::int64:
    case PrgValueKind::uint64:
        return value_as_string(value);
    case PrgValueKind::currency:
        return "VAL(\"$" + value_as_string(value) + "\")";
    case PrgValueKind::string:
    {
        std::string quoted = value.string_value;
        std::string escaped;
        escaped.reserve(quoted.size());
        for (const char ch : quoted)
        {
            if (ch == '"')
            {
                escaped += "\"\"";
            }
            else
            {
                escaped.push_back(ch);
            }
        }
        return "\"" + escaped + "\"";
    }
    case PrgValueKind::empty:
    default:
        return value_as_string(value);
    }
}

std::string serialize_insert_value_expression(const PrgValue &value)
{
    if (value.is_null)
    {
        return ".NULL.";
    }
    if (value.kind == PrgValueKind::empty)
    {
        return "\"\"";
    }
    if (value.kind == PrgValueKind::number)
    {
        std::ostringstream stream;
        stream.imbue(std::locale::classic());
        stream << std::setprecision(std::numeric_limits<double>::max_digits10)
               << value.number_value;
        return stream.str();
    }
    return serialize_runtime_expression_text(value);
}

std::string serialize_insert_row_expression_list(const std::vector<PrgValue> &row)
{
    std::string result;
    for (std::size_t index = 0U; index < row.size(); ++index)
    {
        if (index != 0U)
        {
            result.push_back(',');
        }
        result += serialize_insert_value_expression(row[index]);
    }
    return result;
}

std::string make_native_method_override_key(
    const std::string &program_path,
    const std::string &qualified_method_name)
{
    return normalize_path(program_path) + ":" + normalize_identifier(qualified_method_name);
}
} // namespace copperfin::runtime
