#include "test_prg_engine_seek_index_support.h"

namespace copperfin::seek_index_tests
{
void test_set_filter_scopes_local_cursor_visibility() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_set_filter";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    write_people_dbf(table_path, {{"ALPHA", 10}, {"BRAVO", 20}, {"CHARLIE", 30}, {"DELTA", 40}});

    const fs::path main_path = temp_root / "set_filter.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "USE '" + table_path.string() + "' ALIAS Other AGAIN IN 0\n"
        "SELECT People\n"
        "SET FILTER TO AGE >= 30\n"
        "GO TOP\n"
        "cTopFiltered = NAME\n"
        "SKIP\n"
        "cNextFiltered = NAME\n"
        "SKIP\n"
        "lFilteredEof = EOF()\n"
        "LOCATE FOR NAME = 'BRAVO'\n"
        "lFilteredFound = FOUND()\n"
        "lFilteredLocateEof = EOF()\n"
        "SELECT Other\n"
        "GO TOP\n"
        "cOtherTop = NAME\n"
        "SELECT People\n"
        "SET FILTER TO\n"
        "GO TOP\n"
        "cTopUnfiltered = NAME\n"
        "LOCATE FOR NAME = 'BRAVO'\n"
        "cLocateUnfiltered = NAME\n"
        "SET FILTER TO AGE >= 30\n"
        "SET FILTER OFF\n"
        "GO TOP\n"
        "cTopAfterOff = NAME\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SET FILTER script should complete");

    const auto top_filtered = state.globals.find("ctopfiltered");
    const auto next_filtered = state.globals.find("cnextfiltered");
    const auto filtered_eof = state.globals.find("lfilteredeof");
    const auto filtered_found = state.globals.find("lfilteredfound");
    const auto filtered_locate_eof = state.globals.find("lfilteredlocateeof");
    const auto other_top = state.globals.find("cothertop");
    const auto top_unfiltered = state.globals.find("ctopunfiltered");
    const auto locate_unfiltered = state.globals.find("clocateunfiltered");
    const auto top_after_off = state.globals.find("ctopafteroff");

    expect(top_filtered != state.globals.end(), "filtered GO TOP should expose the first visible record");
    expect(next_filtered != state.globals.end(), "filtered SKIP should expose the next visible record");
    expect(filtered_eof != state.globals.end(), "filtered SKIP past the last visible row should update EOF()");
    expect(filtered_found != state.globals.end(), "filtered LOCATE should expose FOUND()");
    expect(filtered_locate_eof != state.globals.end(), "filtered LOCATE miss should expose EOF()");
    expect(other_top != state.globals.end(), "filters should not bleed into a second alias/work area");
    expect(top_unfiltered != state.globals.end(), "bare SET FILTER TO should restore unfiltered navigation");
    expect(locate_unfiltered != state.globals.end(), "bare SET FILTER TO should restore unfiltered LOCATE behavior");
    expect(top_after_off != state.globals.end(), "SET FILTER OFF should remain a supported clear-filter form");

    if (top_filtered != state.globals.end()) {
        expect(copperfin::runtime::format_value(top_filtered->second) == "CHARLIE", "GO TOP should land on the first filtered-visible row");
    }
    if (next_filtered != state.globals.end()) {
        expect(copperfin::runtime::format_value(next_filtered->second) == "DELTA", "SKIP should move among filtered-visible rows");
    }
    if (filtered_eof != state.globals.end()) {
        expect(copperfin::runtime::format_value(filtered_eof->second) == "true", "SKIP past the filtered-visible tail should reach EOF");
    }
    if (filtered_found != state.globals.end()) {
        expect(copperfin::runtime::format_value(filtered_found->second) == "false", "LOCATE should not find rows excluded by the active filter");
    }
    if (filtered_locate_eof != state.globals.end()) {
        expect(copperfin::runtime::format_value(filtered_locate_eof->second) == "true", "LOCATE misses within a filtered set should leave the cursor at EOF");
    }
    if (other_top != state.globals.end()) {
        expect(copperfin::runtime::format_value(other_top->second) == "ALPHA", "SET FILTER should remain scoped to the targeted cursor/work area");
    }
    if (top_unfiltered != state.globals.end()) {
        expect(copperfin::runtime::format_value(top_unfiltered->second) == "ALPHA", "bare SET FILTER TO should restore full-table GO TOP semantics");
    }
    if (locate_unfiltered != state.globals.end()) {
        expect(copperfin::runtime::format_value(locate_unfiltered->second) == "BRAVO", "bare SET FILTER TO should restore full-table LOCATE behavior");
    }
    if (top_after_off != state.globals.end()) {
        expect(copperfin::runtime::format_value(top_after_off->second) == "ALPHA", "SET FILTER OFF should continue restoring full-table GO TOP semantics");
    }

    expect(
        std::count_if(state.events.begin(), state.events.end(), [](const auto& event) { return event.category == "runtime.filter" && event.detail.find("AGE >= 30") != std::string::npos; }) >= 2 &&
        std::count_if(state.events.begin(), state.events.end(), [](const auto& event) { return event.category == "runtime.filter" && event.detail == "OFF"; }) >= 2,
        "SET FILTER expressions and both clear forms should emit invariant runtime.filter events");

    fs::remove_all(temp_root, ignored);
}
void test_set_filter_defers_local_cursor_evaluation_until_navigation() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_set_filter_deferred";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    write_people_dbf(table_path, {{"ALPHA", 10}, {"BRAVO", 20}, {"CHARLIE", 30}, {"DELTA", 40}});

    const fs::path main_path = temp_root / "set_filter_deferred.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "USE '" + table_path.string() + "' ALIAS Other AGAIN IN 0\n"
        "SELECT People\n"
        "LOCATE FOR NAME = 'DELTA'\n"
        "SET FILTER TO AGE <= 20\n"
        "nBeforeOnlyRec = RECNO()\n"
        "cBeforeOnlyName = NAME\n"
        "lBeforeOnlyFound = FOUND()\n"
        "lBeforeOnlyBof = BOF()\n"
        "lBeforeOnlyEof = EOF()\n"
        "GO TOP\n"
        "nBeforeOnlyTop = RECNO()\n"
        "SET FILTER TO\n"
        "LOCATE FOR NAME = 'MISSING'\n"
        "GO 1\n"
        "SET FILTER TO AGE >= 30\n"
        "nAfterOnlyRec = RECNO()\n"
        "cAfterOnlyName = NAME\n"
        "lAfterOnlyFound = FOUND()\n"
        "lAfterOnlyBof = BOF()\n"
        "lAfterOnlyEof = EOF()\n"
        "SKIP\n"
        "nAfterOnlySkip = RECNO()\n"
        "cAfterOnlySkipName = NAME\n"
        "SET FILTER TO\n"
        "GO 2\n"
        "SET FILTER TO .F.\n"
        "nEmptyRec = RECNO()\n"
        "cEmptyName = NAME\n"
        "lEmptyBof = BOF()\n"
        "lEmptyEof = EOF()\n"
        "lEmptyFound = FOUND()\n"
        "GO TOP\n"
        "nEmptyTopRec = RECNO()\n"
        "lEmptyTopBof = BOF()\n"
        "lEmptyTopEof = EOF()\n"
        "lEmptyTopFound = FOUND()\n"
        "cEmptyTopName = NAME\n"
        "nEmptyTopAge = AGE\n"
        "GO BOTTOM\n"
        "nEmptyBottomRec = RECNO()\n"
        "lEmptyBottomBof = BOF()\n"
        "lEmptyBottomEof = EOF()\n"
        "lEmptyBottomFound = FOUND()\n"
        "cEmptyBottomName = NAME\n"
        "nEmptyBottomAge = AGE\n"
        "SET FILTER TO\n"
        "GO 3\n"
        "SET FILTER TO AGE >= 30\n"
        "nVisibleRec = RECNO()\n"
        "cVisibleName = NAME\n"
        "lVisibleBof = BOF()\n"
        "lVisibleEof = EOF()\n"
        "SET FILTER TO\n"
        "GO 4 IN People\n"
        "SELECT Other\n"
        "GO 2\n"
        "SET FILTER TO AGE <= 20 IN People\n"
        "cSelectedAfterTarget = ALIAS()\n"
        "nSelectedRecAfterTarget = RECNO()\n"
        "nTargetRecAfterFilter = RECNO('People')\n"
        "cTargetNameAfterFilter = People.NAME\n"
        "GO TOP IN People\n"
        "nTargetTopAfterMove = RECNO('People')\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "deferred local SET FILTER evaluation script should complete");

    const auto expect_value = [&](const std::string &name, const std::string &expected, const std::string &message) {
        const auto found = state.globals.find(name);
        expect(found != state.globals.end(), message + " should be captured");
        if (found != state.globals.end()) {
            expect(copperfin::runtime::format_value(found->second) == expected, message);
        }
    };

    expect_value("nbeforeonlyrec", "4", "SET FILTER with visible rows only before the pointer should preserve RECNO()");
    expect_value("cbeforeonlyname", "DELTA", "field access before navigation should retain the excluded physical row");
    expect_value("lbeforeonlyfound", "true", "SET FILTER installation should preserve FOUND()");
    expect_value("lbeforeonlybof", "false", "SET FILTER installation should preserve BOF()");
    expect_value("lbeforeonlyeof", "false", "SET FILTER installation should preserve EOF()");
    expect_value("nbeforeonlytop", "1", "GO TOP should evaluate the filter and find an earlier visible row");
    expect_value("nafteronlyrec", "1", "SET FILTER with visible rows only after the pointer should preserve RECNO()");
    expect_value("cafteronlyname", "ALPHA", "deferred filtering should preserve immediate field access before forward navigation");
    expect_value("lafteronlyfound", "false", "deferred SET FILTER installation should preserve a false FOUND() state");
    expect_value("lafteronlybof", "false", "deferred SET FILTER installation should not synthesize BOF()");
    expect_value("lafteronlyeof", "false", "deferred SET FILTER installation should not synthesize EOF()");
    expect_value("nafteronlyskip", "3", "SKIP should evaluate the installed filter from the preserved physical row");
    expect_value("cafteronlyskipname", "CHARLIE", "SKIP should expose the next filter-visible row");
    expect_value("nemptyrec", "2", "an empty filter result should not move the pointer until navigation");
    expect_value("cemptyname", "BRAVO", "an empty filter result should not hide the current field before navigation");
    expect_value("lemptybof", "false", "an empty filter result should preserve BOF() before navigation");
    expect_value("lemptyeof", "false", "an empty filter result should preserve EOF() before navigation");
    expect_value("lemptyfound", "false", "an empty filter result should preserve FOUND() before navigation");
    expect_value("nemptytoprec", "5", "GO TOP with no filter-visible rows should move past the physical record set");
    expect_value("lemptytopbof", "false", "GO TOP with no filter-visible rows in a nonempty cursor should leave BOF() false");
    expect_value("lemptytopeof", "true", "GO TOP should report EOF() when no records satisfy the filter");
    expect_value("lemptytopfound", "false", "GO TOP should preserve FOUND() when no records satisfy the filter");
    expect_value("cemptytopname", "", "field access at filtered EOF should return the field's typed blank value");
    expect_value("nemptytopage", "0", "numeric field access at filtered EOF should return zero");
    expect_value("nemptybottomrec", "5", "GO BOTTOM with no filter-visible rows should retain the physical EOF record number");
    expect_value("lemptybottombof", "true", "GO BOTTOM with no filter-visible rows should set BOF()");
    expect_value("lemptybottomeof", "true", "GO BOTTOM with no filter-visible rows should retain EOF()");
    expect_value("lemptybottomfound", "false", "GO BOTTOM should preserve FOUND() when no records satisfy the filter");
    expect_value("cemptybottomname", "", "character field access after empty-result GO BOTTOM should stay blank");
    expect_value("nemptybottomage", "0", "numeric field access after empty-result GO BOTTOM should stay zero");
    expect_value("nvisiblerec", "3", "a current filter-visible row should remain selected");
    expect_value("cvisiblename", "CHARLIE", "a current filter-visible row should retain field access");
    expect_value("lvisiblebof", "false", "a current filter-visible row should retain BOF()");
    expect_value("lvisibleeof", "false", "a current filter-visible row should retain EOF()");
    expect_value("cselectedaftertarget", "Other", "targeted SET FILTER should preserve the selected work area");
    expect_value("nselectedrecaftertarget", "2", "targeted SET FILTER should preserve the selected cursor pointer");
    expect_value("ntargetrecafterfilter", "4", "targeted SET FILTER should preserve the non-selected target pointer");
    expect_value("ctargetnameafterfilter", "DELTA", "targeted SET FILTER should preserve immediate target field access");
    expect_value("ntargettopaftermove", "1", "targeted navigation should later evaluate the installed filter");

    std::vector<std::string> filter_event_details;
    for (const auto &event : state.events) {
        if (event.category == "runtime.filter") {
            filter_event_details.push_back(event.detail);
        }
    }
    expect(
        filter_event_details == std::vector<std::string>{
            "AGE <= 20", "OFF", "AGE >= 30", "OFF", ".F.", "OFF", "AGE >= 30", "OFF", "AGE <= 20"},
        "deferred and targeted SET FILTER changes should emit one invariant event with exact detail per statement");

    fs::remove_all(temp_root, ignored);
}
void test_seek_respects_active_filter_visibility() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_seek_filter";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    const fs::path other_table_path = temp_root / "other.dbf";
    const fs::path cdx_path = temp_root / "people.cdx";
    write_people_dbf(table_path, {{"ALPHA", 10}, {"BRAVO", 20}, {"BRAVO", 30}, {"CHARLIE", 40}});
    write_people_dbf(other_table_path, {{"OTHER", 99}});
    write_synthetic_cdx(cdx_path, "NAME", "UPPER(NAME)");

    const fs::path main_path = temp_root / "seek_filter.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "SET ORDER TO TAG NAME\n"
        "nMinimumAge = 30\n"
        "SET FILTER TO AGE >= nMinimumAge\n"
        "SEEK 'BRAVO'\n"
        "lDuplicateFound = FOUND()\n"
        "nDuplicateRec = RECNO()\n"
        "lDuplicateBof = BOF()\n"
        "lDuplicateEof = EOF()\n"
        "SET NEAR OFF\n"
        "SEEK 'ALPHA'\n"
        "lHiddenFound = FOUND()\n"
        "nHiddenRec = RECNO()\n"
        "lHiddenEof = EOF()\n"
        "SET NEAR ON\n"
        "SEEK 'ALPHA'\n"
        "lNearFound = FOUND()\n"
        "nNearRec = RECNO()\n"
        "lNearEof = EOF()\n"
        "SEEK 'CHARLIE'\n"
        "lVisibleFound = FOUND()\n"
        "nVisibleRec = RECNO()\n"
        "SET NEAR OFF\n"
        "lFunctionHidden = SEEK('ALPHA')\n"
        "nFunctionRec = RECNO()\n"
        "GO 4\n"
        "nIndexBefore = RECNO()\n"
        "lIndexHidden = INDEXSEEK('ALPHA')\n"
        "nIndexAfter = RECNO()\n"
        "SET FILTER TO SEEK(NAME)\n"
        "SEEK 'BRAVO'\n"
        "lReentrantFound = FOUND()\n"
        "nReentrantRec = RECNO()\n"
        "SET FILTER TO SEEK('MISSING') OR AGE >= nMinimumAge\n"
        "SEEK 'BRAVO'\n"
        "lNestedMissFound = FOUND()\n"
        "nNestedMissRec = RECNO()\n"
        "SET FILTER TO IIF(NAME = 'ALPHA', 1 / 0 = 0, AGE >= nMinimumAge)\n"
        "SEEK 'CHARLIE'\n"
        "lLazyFilterFound = FOUND()\n"
        "nLazyFilterRec = RECNO()\n"
        "SET FILTER TO\n"
        "USE '" + other_table_path.string() + "' ALIAS Other IN 0\n"
        "SELECT People\n"
        "SET ORDER TO 0\n"
        "SET FILTER TO EMPTY(ORDER()) AND RECNO() = 3 AND AGE >= nMinimumAge IN People\n"
        "SELECT Other\n"
        "lTargetContextFound = SEEK('BRAVO', 'People', 'NAME')\n"
        "nTargetContextRec = RECNO('People')\n"
        "cSelectedAfterTarget = ALIAS()\n"
        "nSelectedRecAfterTarget = RECNO()\n"
        "SELECT People\n"
        "SET FILTER TO EMPTY(ORDER()) AND AGE >= nMinimumAge\n"
        "SEEK 'BRAVO'\n"
        "lImplicitOrderFound = FOUND()\n"
        "nImplicitOrderRec = RECNO()\n"
        "cImplicitOrderAfter = ORDER()\n"
        "LOCATE FOR NAME = 'BRAVO'\n"
        "lRushmoreFilterFound = FOUND()\n"
        "nRushmoreFilterRec = RECNO()\n"
        "cRushmoreFilterOrder = ORDER()\n"
        "SET FILTER TO\n"
        "LOCATE FOR NAME = 'BRAVO'\n"
        "GO 5\n"
        "SET FIELDS TO NAME\n"
        "cFaultFieldsBefore = SET('FIELDS')\n"
        "SET FILTER TO 1 / 0 = 0\n"
        "SET ORDER TO 0\n"
        "GO 2\n"
        "nFaultRecBefore = RECNO()\n"
        "lFaultFoundBefore = FOUND()\n"
        "lSeekFaultCaught = .F.\n"
        "TRY\n"
        "    SEEK 'BRAVO' TAG NAME\n"
        "CATCH\n"
        "    lSeekFaultCaught = .T.\n"
        "ENDTRY\n"
        "nFaultRecAfter = RECNO()\n"
        "lFaultFoundAfter = FOUND()\n"
        "lFaultBofAfter = BOF()\n"
        "lFaultEofAfter = EOF()\n"
        "cFaultOrderAfter = ORDER()\n"
        "cFaultFieldsAfter = SET('FIELDS')\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SEEK with an active filter should complete");

    const auto expect_value = [&](const std::string &name, const std::string &expected, const std::string &message) {
        const auto found = state.globals.find(name);
        expect(found != state.globals.end(), message + " should be captured");
        if (found != state.globals.end()) {
            expect(copperfin::runtime::format_value(found->second) == expected, message);
        }
    };

    expect_value("lduplicatefound", "true", "SEEK should find a later visible duplicate after skipping the hidden first duplicate");
    expect_value("nduplicaterec", "3", "SEEK should land on the first filter-visible duplicate");
    expect_value("lduplicatebof", "false", "a filter-visible duplicate hit should clear BOF()");
    expect_value("lduplicateeof", "false", "a filter-visible duplicate hit should clear EOF()");
    expect_value("lhiddenfound", "false", "SEEK should not find a key available only on a filtered-out record");
    expect_value("nhiddenrec", "5", "a filtered-out exact key with SET NEAR OFF should move to physical EOF");
    expect_value("lhiddeneof", "true", "a filtered-out exact key with SET NEAR OFF should set EOF()");
    expect_value("lnearfound", "false", "SET NEAR should not turn a filtered-out exact key into a hit");
    expect_value("nnearrec", "3", "SET NEAR should land on the next filter-visible indexed record");
    expect_value("lneareof", "false", "a filter-visible near record should clear EOF()");
    expect_value("lvisiblefound", "true", "SEEK should continue finding filter-visible exact keys");
    expect_value("nvisiblerec", "4", "SEEK should position on the filter-visible exact key");
    expect_value("lfunctionhidden", "false", "SEEK() should share command visibility behavior");
    expect_value("nfunctionrec", "5", "SEEK() with SET NEAR OFF should move to physical EOF on a filtered key");
    expect_value("nindexbefore", "4", "INDEXSEEK() pointer-preservation setup should select a visible record");
    expect_value("lindexhidden", "false", "INDEXSEEK() should not report a filtered-out key");
    expect_value("nindexafter", "4", "INDEXSEEK() without pointer movement should preserve RECNO()");
    expect_value("lreentrantfound", "true", "SEEK inside the active filter expression should not recursively overflow the runtime");
    expect_value("nreentrantrec", "2", "a re-entrant SEEK filter should preserve ordinary duplicate-key order");
    expect_value("lnestedmissfound", "true", "a nested same-cursor SEEK miss should not hide the candidate record from the rest of the filter");
    expect_value("nnestedmissrec", "3", "a nested SEEK miss should still allow the first filter-visible duplicate");
    expect_value("llazyfilterfound", "true", "SEEK should not evaluate filters for unrelated keys before the lower bound");
    expect_value("nlazyfilterrec", "4", "lazy filter evaluation should preserve the visible exact-key position");
    expect_value("ltargetcontextfound", "true", "SEEK should evaluate an active filter in its non-selected target work area");
    expect_value("ntargetcontextrec", "3", "a targeted SEEK should use the target cursor's RECNO() and logical order in its filter");
    expect_value("cselectedaftertarget", "Other", "a targeted SEEK should preserve the selected work area");
    expect_value("nselectedrecaftertarget", "1", "a targeted SEEK should preserve the selected cursor position");
    expect_value("limplicitorderfound", "true", "bare SEEK should evaluate filters against the logical order before implicit index selection");
    expect_value("nimplicitorderrec", "3", "bare SEEK should find the first visible duplicate through an implicitly selected index");
    expect_value("cimplicitorderafter", "", "bare SEEK should restore the logical controlling order after implicit index selection");
    expect_value("lrushmorefilterfound", "true", "Rushmore LOCATE should evaluate the active filter against the logical controlling order");
    expect_value("nrushmorefilterrec", "3", "Rushmore LOCATE should land on the first filter-visible matching record");
    expect_value("crushmorefilterorder", "", "Rushmore LOCATE should not expose its temporary search order");
    expect_value("nfaultrecbefore", "2", "filter-fault restoration should start from the selected physical record");
    expect_value("lfaultfoundbefore", "true", "filter-fault restoration should start with the prior FOUND() state");
    expect_value("cfaultfieldsbefore", "NAME", "filter-fault restoration should start with the configured SET FIELDS list");
    expect_value("lseekfaultcaught", "true", "a filter evaluation fault during SEEK should remain catchable");
    expect_value("nfaultrecafter", "2", "a filter evaluation fault should restore RECNO()");
    expect_value("lfaultfoundafter", "true", "a filter evaluation fault should restore FOUND()");
    expect_value("lfaultbofafter", "false", "a filter evaluation fault should restore BOF()");
    expect_value("lfaulteofafter", "false", "a filter evaluation fault should restore EOF()");
    expect_value("cfaultorderafter", "", "a filter evaluation fault should restore the prior controlling order");
    expect_value("cfaultfieldsafter", "NAME", "a caught filter evaluation fault should restore SET FIELDS state");

    fs::remove_all(temp_root, ignored);
}
void test_set_filter_in_targets_nonselected_alias() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_set_filter_in";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    write_people_dbf(table_path, {{"ALPHA", 10}, {"BRAVO", 20}, {"CHARLIE", 30}, {"DELTA", 40}});

    const fs::path main_path = temp_root / "set_filter_in.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "USE '" + table_path.string() + "' ALIAS Other AGAIN IN 0\n"
        "SELECT People\n"
        "cTargetAlias = 'Other'\n"
        "SET FILTER TO AGE >= 30 IN cTargetAlias\n"
        "GO TOP\n"
        "cPeopleTop = NAME\n"
        "SELECT Other\n"
        "GO TOP\n"
        "cOtherTop = NAME\n"
        "SET FILTER OFF IN cTargetAlias\n"
        "GO TOP\n"
        "cOtherTopUnfiltered = NAME\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SET FILTER ... IN script should complete");

    const auto people_top = state.globals.find("cpeopletop");
    const auto other_top = state.globals.find("cothertop");
    const auto other_top_unfiltered = state.globals.find("cothertopunfiltered");

    expect(people_top != state.globals.end(), "selected alias top row should be captured");
    expect(other_top != state.globals.end(), "targeted alias filtered row should be captured");
    expect(other_top_unfiltered != state.globals.end(), "targeted alias unfiltered row should be captured");

    if (people_top != state.globals.end()) {
        expect(copperfin::runtime::format_value(people_top->second) == "ALPHA", "SET FILTER ... IN cTargetAlias should not affect People");
    }
    if (other_top != state.globals.end()) {
        expect(copperfin::runtime::format_value(other_top->second) == "CHARLIE", "SET FILTER ... IN cTargetAlias should affect the targeted alias");
    }
    if (other_top_unfiltered != state.globals.end()) {
        expect(copperfin::runtime::format_value(other_top_unfiltered->second) == "ALPHA", "SET FILTER OFF IN cTargetAlias should restore unfiltered navigation for the targeted alias");
    }

    expect(
        std::count_if(state.events.begin(), state.events.end(), [](const auto& event) {
            return event.category == "runtime.filter";
        }) >= 2,
        "SET FILTER ... IN and SET FILTER OFF IN should emit runtime.filter events");

    fs::remove_all(temp_root, ignored);
}

}
