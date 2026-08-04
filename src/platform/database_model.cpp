// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/platform/database_model.h"

#include "copperfin/localization/localization.h"

#include <mutex>

namespace copperfin::platform {

namespace {

localization::LocalizedCatalog database_profile_catalog() {
    struct CatalogCache {
        std::filesystem::path locale_root;
        std::string locale;
        localization::LocalizedCatalog catalog;
    };

    static std::mutex cache_mutex;
    static CatalogCache cache{
        {},
        {},
        localization::load_catalogs(
            localization::resolve_catalog_root(),
            localization::default_locale)};

    const std::filesystem::path locale_root = localization::resolve_catalog_root();
    const std::string locale = localization::select_locale();
    std::lock_guard<std::mutex> lock(cache_mutex);
    if (cache.locale_root != locale_root || cache.locale != locale) {
        cache.locale_root = locale_root;
        cache.locale = locale;
        cache.catalog = localization::load_catalogs(locale_root, locale);
    }
    return cache.catalog;
}

std::string database_text(
    const localization::LocalizedCatalog& catalog,
    std::string_view key) {
    return catalog.translate(key);
}

}  // namespace

const QueryTranslationPath* query_translation_path_by_id(
    const DatabaseFederationProfile& profile,
    const std::string& path_id) {
    for (const auto& path : profile.query_paths) {
        if (path.id == path_id) {
            return &path;
        }
    }
    return nullptr;
}

const DatabaseConnectorProfile* database_connector_by_id(
    const DatabaseFederationProfile& profile,
    const std::string& connector_id) {
    for (const auto& connector : profile.connectors) {
        if (connector.id == connector_id) {
            return &connector;
        }
    }
    return nullptr;
}

DatabaseFederationProfile default_database_federation_profile(const localization::LocalizedCatalog& catalog) {
    DatabaseFederationProfile profile;
    profile.available = true;

    profile.connectors = {
        {"dbf", database_text(catalog, "Platform.Database.Connector.Dbf.Title"), "xbase", database_text(catalog, "Platform.Database.Connector.Dbf.AccessMode"), database_text(catalog, "Platform.Database.Connector.Dbf.SchemaShape"), database_text(catalog, "Platform.Database.Connector.Dbf.TranslationStory"), true, true, false},
        {"sqlite", database_text(catalog, "Platform.Database.Connector.Sqlite.Title"), "relational", database_text(catalog, "Platform.Database.Connector.Sqlite.AccessMode"), database_text(catalog, "Platform.Database.Connector.Sqlite.SchemaShape"), database_text(catalog, "Platform.Database.Connector.Sqlite.TranslationStory"), true, true, false},
        {"postgresql", database_text(catalog, "Platform.Database.Connector.Postgresql.Title"), "relational", database_text(catalog, "Platform.Database.Connector.Postgresql.AccessMode"), database_text(catalog, "Platform.Database.Connector.Postgresql.SchemaShape"), database_text(catalog, "Platform.Database.Connector.Postgresql.TranslationStory"), true, true, false},
        {"sqlserver", database_text(catalog, "Platform.Database.Connector.SqlServer.Title"), "relational", database_text(catalog, "Platform.Database.Connector.SqlServer.AccessMode"), database_text(catalog, "Platform.Database.Connector.SqlServer.SchemaShape"), database_text(catalog, "Platform.Database.Connector.SqlServer.TranslationStory"), true, true, false},
        {"oracle", database_text(catalog, "Platform.Database.Connector.Oracle.Title"), "relational", database_text(catalog, "Platform.Database.Connector.Oracle.AccessMode"), database_text(catalog, "Platform.Database.Connector.Oracle.SchemaShape"), database_text(catalog, "Platform.Database.Connector.Oracle.TranslationStory"), true, true, false},
        {"mongodb", database_text(catalog, "Platform.Database.Connector.Mongodb.Title"), "document", database_text(catalog, "Platform.Database.Connector.Mongodb.AccessMode"), database_text(catalog, "Platform.Database.Connector.Mongodb.SchemaShape"), database_text(catalog, "Platform.Database.Connector.Mongodb.TranslationStory"), false, false, true},
        {"json-api", database_text(catalog, "Platform.Database.Connector.JsonApi.Title"), "document", database_text(catalog, "Platform.Database.Connector.JsonApi.AccessMode"), database_text(catalog, "Platform.Database.Connector.JsonApi.SchemaShape"), database_text(catalog, "Platform.Database.Connector.JsonApi.TranslationStory"), false, false, true},
        {"vector", database_text(catalog, "Platform.Database.Connector.Vector.Title"), "vector", database_text(catalog, "Platform.Database.Connector.Vector.AccessMode"), database_text(catalog, "Platform.Database.Connector.Vector.SchemaShape"), database_text(catalog, "Platform.Database.Connector.Vector.TranslationStory"), false, false, true}
    };

    profile.query_paths = {
        {"foxsql-relational", database_text(catalog, "Platform.Database.QueryPath.FoxSqlRelational.Title"), database_text(catalog, "Platform.Database.QueryPath.FoxSqlRelational.SourceShape"), database_text(catalog, "Platform.Database.QueryPath.FoxSqlRelational.TargetShape"), database_text(catalog, "Platform.Database.QueryPath.FoxSqlRelational.Complexity"), database_text(catalog, "Platform.Database.QueryPath.FoxSqlRelational.Strategy"), true, false},
        {"xbase-browse-document", database_text(catalog, "Platform.Database.QueryPath.XbaseBrowseDocument.Title"), database_text(catalog, "Platform.Database.QueryPath.XbaseBrowseDocument.SourceShape"), database_text(catalog, "Platform.Database.QueryPath.XbaseBrowseDocument.TargetShape"), database_text(catalog, "Platform.Database.QueryPath.XbaseBrowseDocument.Complexity"), database_text(catalog, "Platform.Database.QueryPath.XbaseBrowseDocument.Strategy"), true, true},
        {"foxsql-document", database_text(catalog, "Platform.Database.QueryPath.FoxSqlDocument.Title"), database_text(catalog, "Platform.Database.QueryPath.FoxSqlDocument.SourceShape"), database_text(catalog, "Platform.Database.QueryPath.FoxSqlDocument.TargetShape"), database_text(catalog, "Platform.Database.QueryPath.FoxSqlDocument.Complexity"), database_text(catalog, "Platform.Database.QueryPath.FoxSqlDocument.Strategy"), true, true},
        {"foxsql-vector", database_text(catalog, "Platform.Database.QueryPath.FoxSqlVector.Title"), database_text(catalog, "Platform.Database.QueryPath.FoxSqlVector.SourceShape"), database_text(catalog, "Platform.Database.QueryPath.FoxSqlVector.TargetShape"), database_text(catalog, "Platform.Database.QueryPath.FoxSqlVector.Complexity"), database_text(catalog, "Platform.Database.QueryPath.FoxSqlVector.Strategy"), true, true},
        {"xbase-polyglot", database_text(catalog, "Platform.Database.QueryPath.XbasePolyglot.Title"), database_text(catalog, "Platform.Database.QueryPath.XbasePolyglot.SourceShape"), database_text(catalog, "Platform.Database.QueryPath.XbasePolyglot.TargetShape"), database_text(catalog, "Platform.Database.QueryPath.XbasePolyglot.Complexity"), database_text(catalog, "Platform.Database.QueryPath.XbasePolyglot.Strategy"), true, true}
    };

    profile.guardrails = {
        database_text(catalog, "Platform.Database.Guardrail.DeterministicTranslatorsFirst"),
        database_text(catalog, "Platform.Database.Guardrail.AiPlanningOptional"),
        database_text(catalog, "Platform.Database.Guardrail.BrowseableSchemaHints"),
        database_text(catalog, "Platform.Database.Guardrail.ExplainableConnectorFailures")
    };

    return profile;
}

DatabaseFederationProfile default_database_federation_profile() {
    return default_database_federation_profile(database_profile_catalog());
}

}  // namespace copperfin::platform
