// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
#pragma once
#include "copperfin/runtime/prg_engine.h"
#include <algorithm>
#include <charconv>
#include <cmath>
#include <optional>
#include <string_view>
namespace copperfin::runtime {
// RQ-CF-PRG-NUMERIC-RUNTIME-DIVISION-001: metadata only, never payload
// rounding, coercion, reevaluation or allocation. Native pre-code contract:
// tests/fixtures/vfp9-numeric-division-context/runtime-contract.md.
inline std::optional<PrgNumericMetadata> runtime_numeric_division_metadata(
    const PrgValue& left,const PrgValue& right,double quotient,
    std::string_view decimals_text,bool contains_runtime_primary) noexcept {
    if(!contains_runtime_primary||!std::isfinite(quotient)||std::abs(quotient)>=10000000000.0||right.number_value==0.0)
        return std::nullopt;
    const auto eligible=[](const PrgValue& value){
        if(value.is_null||value.kind!=PrgValueKind::number||!value.numeric_metadata||!std::isfinite(value.number_value))return false;
        const auto& m=*value.numeric_metadata;
        if(m.display_width<1U||m.display_width>40U)return false;
        if(m.storage==PrgNumericStorage::integer)return std::abs(value.number_value)<=2147483647.0&&std::trunc(value.number_value)==value.number_value;
        return m.storage==PrgNumericStorage::real&&m.decimal_places<=18U&&m.display_width>m.decimal_places&&std::abs(value.number_value)<10000000000.0;
    };
    if(!eligible(left)||!eligible(right)||decimals_text.empty()||(decimals_text.size()>1&&decimals_text.front()=='0'))return std::nullopt;
    unsigned setting=0;const auto parsed=std::from_chars(decimals_text.data(),decimals_text.data()+decimals_text.size(),setting);
    if(parsed.ec!=std::errc{}||parsed.ptr!=decimals_text.data()+decimals_text.size()||setting>18U)return std::nullopt;
    const auto& l=*left.numeric_metadata;const auto& r=*right.numeric_metadata;
    // Native Integer's decimal slot is unused; ignore even a nonzero placeholder.
    const auto dl=l.storage==PrgNumericStorage::integer?0U:l.decimal_places;
    const auto dr=r.storage==PrgNumericStorage::integer?0U:r.decimal_places;
    const auto d=std::min(18U,std::max({setting+2U,dl,dr}));
    // Eligibility bounds each subtraction and caps the pre-clamp sum at98.
    const auto w=std::min(40U,(l.display_width-dl)+(r.display_width-dr)+d-(dl>0U||dr>0U?1U:0U));
    return PrgNumericMetadata{w,d,PrgNumericStorage::real};
}
} // namespace copperfin::runtime
