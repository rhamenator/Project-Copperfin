#include "test_prg_engine_control_flow_support.h"

namespace cf_test_prg_engine_control_flow {

void test_select_query_into_array_commands() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_select_query_into_array";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path search_path = temp_root / "searchrows.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> search_fields{
        {.name = "REFTYPE", .type = 'C', .length = 1U},
        {.name = "INACTIVE", .type = 'L', .length = 1U}};
    const auto search_result = copperfin::vfp::create_dbf_table_file(
        search_path.string(), search_fields, {{"S", "F"}, {"S", "T"}, {"P", "F"}});
    expect(search_result.ok, "direct SELECT search DBF fixture should be created");

    const fs::path other_path = temp_root / "otherrows.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> other_fields{
        {.name = "MARKER", .type = 'C', .length = 8U}};
    const auto other_result = copperfin::vfp::create_dbf_table_file(
        other_path.string(), other_fields, {{"before"}});
    expect(other_result.ok, "direct SELECT other DBF fixture should be created");

    const fs::path main_path = temp_root / "select_query.prg";
    write_text(
        main_path,
        "USE '" + search_path.string() + "' ALIAS SearchRows IN 0\n"
        "USE '" + other_path.string() + "' ALIAS OtherRows AGAIN IN 0\n"
        "SELECT OtherRows\n"
        "cBefore = ALIAS()\n"
        "SELECT CNT(*) FROM SearchRows WHERE RefType == 'S' AND !Inactive INTO ARRAY aSearchCnt\n"
        "cAfter = ALIAS()\n"
        "nSearchCount = aSearchCnt[1]\n"
        "nTally = _TALLY\n"
        "oHost = CREATEOBJECT('QueryHost')\n"
        "nObjectCount = oHost.CountSearch()\n"
        "RETURN\n"
        "DEFINE CLASS QueryHost AS Session\n"
        "    RefTable = 'SearchRows'\n"
        "    FUNCTION CountSearch\n"
        "        LOCAL ARRAY aCounts[1]\n"
        "        SELECT CNT(*) FROM (THIS.RefTable) WHERE RefType == 'S' AND !Inactive INTO ARRAY aCounts\n"
        "        RETURN IIF(_TALLY > 0, aCounts[1], -1)\n"
        "    ENDFUNC\n"
        "ENDDEFINE\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(main_path.string(), temp_root.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "direct SELECT INTO ARRAY query script should complete: " + state.message);

    const auto check = [&](const std::string &name, const std::string &expected) {
        const auto found = state.globals.find(name);
        expect(found != state.globals.end(), name + " should be captured");
        if (found != state.globals.end()) {
            expect(copperfin::runtime::format_value(found->second) == expected,
                   name + " expected '" + expected + "' got '" +
                       copperfin::runtime::format_value(found->second) + "'");
        }
    };

    check("cbefore", "OtherRows");
    check("cafter", "OtherRows");
    check("nsearchcount", "1");
    check("ntally", "1");
    check("nobjectcount", "1");

    const auto query_event = std::find_if(
        state.events.begin(),
        state.events.end(),
        [](const auto &event) { return event.category == "runtime.select_query"; });
    expect(query_event != state.events.end(), "direct SELECT INTO ARRAY should emit query metadata");
    if (query_event != state.events.end()) {
        expect(query_event->detail == "aSearchCnt",
               "direct SELECT INTO ARRAY metadata should identify the target array");
    }

    fs::remove_all(temp_root, ignored);
}

}  // namespace cf_test_prg_engine_control_flow
