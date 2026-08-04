// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Diagnostics;
using System.Drawing;
using System.Globalization;
using System.IO;
using System.IO.Compression;
using System.Linq;
using System.Reflection;
using System.Threading;
using System.Windows.Forms;

namespace Copperfin.VisualStudio;

internal static partial class Program
{
    private static int failures;

    private static void CloseHarnessForms()
    {
        var openForms = Application.OpenForms.Cast<Form>().ToArray();
        if (openForms.Length == 0)
        {
            return;
        }

        var closedForms = new HashSet<Form>();
        foreach (var form in openForms.Where(form => form.Owner is null))
        {
            CloseHarnessFormTree(form, closedForms);
        }

        // A malformed owner relationship should not leave a window behind.
        foreach (var form in openForms)
        {
            CloseHarnessFormTree(form, closedForms);
        }

        Application.DoEvents();
    }

    private static void CloseHarnessFormTree(Form form, ISet<Form> closedForms)
    {
        if (!closedForms.Add(form))
        {
            return;
        }

        foreach (var ownedForm in form.OwnedForms.ToArray())
        {
            CloseHarnessFormTree(ownedForm, closedForms);
        }

        try
        {
            if (!form.IsDisposed)
            {
                form.Hide();
                form.Close();
            }
        }
        catch (Exception exception)
        {
            Console.Error.WriteLine(
                $"WARN: smoke harness could not close form '{form.Text}': {exception.Message}");
        }
        finally
        {
            try
            {
                if (!form.IsDisposed)
                {
                    form.Dispose();
                }
            }
            catch (Exception exception)
            {
                Console.Error.WriteLine(
                    $"WARN: smoke harness could not dispose form '{form.Text}': {exception.Message}");
            }
        }
    }

    private sealed class ExpectedSectionGroupingMetadata
    {
        public string GroupRole { get; set; } = string.Empty;
        public string? GroupRoleDisplay { get; set; }
        public string SectionExpression { get; set; } = string.Empty;
        public int GroupingIndex { get; set; }
        public int GroupingNestingDepth { get; set; }
        public string GroupingExpression { get; set; } = string.Empty;
        public int? GroupingExpressionFieldIndex { get; set; }
        public int GroupingExpressionMemoBlockNumber { get; set; }
        public string GroupPartnerSectionId { get; set; } = string.Empty;
        public int GroupPartnerRecordIndex { get; set; }
        public bool GroupPartnerDeleted { get; set; }
        public string? GroupPartnerStateDisplay { get; set; }
    }

    private sealed class ExpectedSectionContainedObjectGeometry
    {
        public int RecordIndex { get; set; }
        public int Top { get; set; }
        public int SectionRelativeTop { get; set; }
        public int Bottom { get; set; }
        public int SectionRelativeBottom { get; set; }
    }

    private sealed class ExpectedUntouchedSectionSnapshot
    {
        public int RecordIndex { get; set; }
        public string Title { get; set; } = string.Empty;
        public int Top { get; set; }
        public int Height { get; set; }
        public int ObjectCount { get; set; }
        public ExpectedSectionGroupingMetadata? Grouping { get; set; }
    }

    private sealed class ExpectedPreviewBoundsGeometry
    {
        public int Left { get; set; }
        public int Top { get; set; }
        public int Right { get; set; }
        public int Bottom { get; set; }
        public int Width { get; set; }
        public int Height { get; set; }
    }

    private static ExpectedSectionGroupingMetadata CreateBandedmGroupHeaderGrouping()
    {
        return new ExpectedSectionGroupingMetadata
        {
            GroupRole = "header",
            GroupRoleDisplay = "Header",
            SectionExpression = "OneToMany",
            GroupingIndex = 0,
            GroupingNestingDepth = 0,
            GroupingExpression = "OneToMany",
            GroupingExpressionFieldIndex = 6,
            GroupingExpressionMemoBlockNumber = 25,
            GroupPartnerSectionId = "_RME0ORXFY",
            GroupPartnerRecordIndex = 7,
            GroupPartnerDeleted = false,
            GroupPartnerStateDisplay = "Live"
        };
    }

    private static ExpectedSectionContainedObjectGeometry[] CreateBandedmGroupHeaderContainedObjects(int topDelta)
    {
        return new[]
        {
            new ExpectedSectionContainedObjectGeometry
            {
                RecordIndex = 28,
                Top = topDelta,
                SectionRelativeTop = 0,
                Bottom = topDelta,
                SectionRelativeBottom = 0
            },
            new ExpectedSectionContainedObjectGeometry
            {
                RecordIndex = 13,
                Top = 833 + topDelta,
                SectionRelativeTop = 833,
                Bottom = 937 + topDelta,
                SectionRelativeBottom = 937
            },
            new ExpectedSectionContainedObjectGeometry
            {
                RecordIndex = 14,
                Top = 1250 + topDelta,
                SectionRelativeTop = 1250,
                Bottom = 1354 + topDelta,
                SectionRelativeBottom = 1354
            },
            new ExpectedSectionContainedObjectGeometry
            {
                RecordIndex = 9,
                Top = 1354 + topDelta,
                SectionRelativeTop = 1354,
                Bottom = 6874 + topDelta,
                SectionRelativeBottom = 6874
            },
            new ExpectedSectionContainedObjectGeometry
            {
                RecordIndex = 15,
                Top = 1875 + topDelta,
                SectionRelativeTop = 1875,
                Bottom = 4270 + topDelta,
                SectionRelativeBottom = 4270
            },
            new ExpectedSectionContainedObjectGeometry
            {
                RecordIndex = 12,
                Top = 4479 + topDelta,
                SectionRelativeTop = 4479,
                Bottom = 6249 + topDelta,
                SectionRelativeBottom = 6249
            },
            new ExpectedSectionContainedObjectGeometry
            {
                RecordIndex = 19,
                Top = 6770 + topDelta,
                SectionRelativeTop = 6770,
                Bottom = 6874 + topDelta,
                SectionRelativeBottom = 6874
            },
            new ExpectedSectionContainedObjectGeometry
            {
                RecordIndex = 20,
                Top = 7187 + topDelta,
                SectionRelativeTop = 7187,
                Bottom = 7291 + topDelta,
                SectionRelativeBottom = 7291
            }
        };
    }

    private static ExpectedSectionContainedObjectGeometry[] CreateStyle3vPageHeaderContainedObjects(int topDelta)
    {
        return new[]
        {
            new ExpectedSectionContainedObjectGeometry
            {
                RecordIndex = 12,
                Top = 2916 + topDelta,
                SectionRelativeTop = 2916,
                Bottom = 3332 + topDelta,
                SectionRelativeBottom = 3332
            },
            new ExpectedSectionContainedObjectGeometry
            {
                RecordIndex = 13,
                Top = 3541 + topDelta,
                SectionRelativeTop = 3541,
                Bottom = 5936 + topDelta,
                SectionRelativeBottom = 5936
            },
            new ExpectedSectionContainedObjectGeometry
            {
                RecordIndex = 11,
                Top = 5937 + topDelta,
                SectionRelativeTop = 5937,
                Bottom = 7812 + topDelta,
                SectionRelativeBottom = 7812
            }
        };
    }

    private static ExpectedUntouchedSectionSnapshot CreateBandedmUntouchedGroupFooterSection()
    {
        return new ExpectedUntouchedSectionSnapshot
        {
            RecordIndex = 7,
            Title = "Group Footer",
            Top = 0,
            Height = 1355,
            ObjectCount = 0,
            Grouping = new ExpectedSectionGroupingMetadata
            {
                GroupRole = "footer",
                GroupRoleDisplay = "Footer",
                SectionExpression = "OneToMany",
                GroupingIndex = 0,
                GroupingNestingDepth = 0,
                GroupingExpression = "OneToMany",
                GroupingExpressionFieldIndex = 6,
                GroupingExpressionMemoBlockNumber = 25,
                GroupPartnerSectionId = "_RME0ORXEA",
                GroupPartnerRecordIndex = 5,
                GroupPartnerDeleted = false,
                GroupPartnerStateDisplay = "Live"
            }
        };
    }

    private static ExpectedUntouchedSectionSnapshot CreateBandedmUpdatedGroupFooterSection()
    {
        return new ExpectedUntouchedSectionSnapshot
        {
            RecordIndex = 7,
            Title = "Group Footer",
            Top = 0,
            Height = 1355,
            ObjectCount = 0,
            Grouping = new ExpectedSectionGroupingMetadata
            {
                GroupRole = "footer",
                GroupRoleDisplay = "Footer",
                SectionExpression = string.Empty,
                GroupingIndex = 0,
                GroupingNestingDepth = 0,
                GroupingExpression = "customer.company",
                GroupingExpressionFieldIndex = 6,
                GroupingExpressionMemoBlockNumber = 156,
                GroupPartnerSectionId = "_RME0ORXEA",
                GroupPartnerRecordIndex = 5,
                GroupPartnerDeleted = false,
                GroupPartnerStateDisplay = "Live"
            }
        };
    }

    private static ExpectedSectionGroupingMetadata CreateUpdatedBandedmGroupHeaderGrouping()
    {
        return new ExpectedSectionGroupingMetadata
        {
            GroupRole = "header",
            GroupRoleDisplay = "Header",
            SectionExpression = "customer.company",
            GroupingIndex = 0,
            GroupingNestingDepth = 0,
            GroupingExpression = "customer.company",
            GroupingExpressionFieldIndex = 6,
            GroupingExpressionMemoBlockNumber = 156,
            GroupPartnerSectionId = "_RME0ORXFY",
            GroupPartnerRecordIndex = 7,
            GroupPartnerDeleted = false,
            GroupPartnerStateDisplay = "Live"
        };
    }

    private static ExpectedUntouchedSectionSnapshot CreateUndoneBandedmGroupFooterSection()
    {
        return new ExpectedUntouchedSectionSnapshot
        {
            RecordIndex = 7,
            Title = "Group Footer",
            Top = 0,
            Height = 1355,
            ObjectCount = 0,
            Grouping = new ExpectedSectionGroupingMetadata
            {
                GroupRole = "footer",
                GroupRoleDisplay = "Footer",
                SectionExpression = string.Empty,
                GroupingIndex = 0,
                GroupingNestingDepth = 0,
                GroupingExpression = "OneToMany",
                GroupingExpressionFieldIndex = 6,
                GroupingExpressionMemoBlockNumber = 157,
                GroupPartnerSectionId = "_RME0ORXEA",
                GroupPartnerRecordIndex = 5,
                GroupPartnerDeleted = false,
                GroupPartnerStateDisplay = "Live"
            }
        };
    }

    private static ExpectedSectionGroupingMetadata CreateUndoneBandedmGroupHeaderGrouping()
    {
        return new ExpectedSectionGroupingMetadata
        {
            GroupRole = "header",
            GroupRoleDisplay = "Header",
            SectionExpression = "OneToMany",
            GroupingIndex = 0,
            GroupingNestingDepth = 0,
            GroupingExpression = "OneToMany",
            GroupingExpressionFieldIndex = 6,
            GroupingExpressionMemoBlockNumber = 157,
            GroupPartnerSectionId = "_RME0ORXFY",
            GroupPartnerRecordIndex = 7,
            GroupPartnerDeleted = false,
            GroupPartnerStateDisplay = "Live"
        };
    }

    private static ExpectedUntouchedSectionSnapshot CreateStylelblUntouchedColumnFooterSection()
    {
        return new ExpectedUntouchedSectionSnapshot
        {
            RecordIndex = 4,
            Title = "Column Footer",
            Top = 0,
            Height = 0,
            ObjectCount = 0
        };
    }

    private static ExpectedUntouchedSectionSnapshot CreateByAuthorUntouchedGroupHeaderSection()
    {
        return new ExpectedUntouchedSectionSnapshot
        {
            RecordIndex = 3,
            Title = "Group Header",
            Top = 0,
            Height = 2709,
            ObjectCount = 0,
            Grouping = new ExpectedSectionGroupingMetadata
            {
                GroupRole = "header",
                GroupRoleDisplay = "Header",
                SectionExpression = "titles_by_author.author_id",
                GroupingIndex = 0,
                GroupingNestingDepth = 0,
                GroupingExpression = "titles_by_author.author_id",
                GroupingExpressionFieldIndex = 6,
                GroupingExpressionMemoBlockNumber = 18,
                GroupPartnerSectionId = "_RC60MBVB4",
                GroupPartnerRecordIndex = 5,
                GroupPartnerDeleted = false,
                GroupPartnerStateDisplay = "Live"
            }
        };
    }

    private sealed class ExpectedReportGroupingMetadata
    {
        public int GroupingIndex { get; set; }
        public int GroupingNestingDepth { get; set; }
        public string GroupingExpression { get; set; } = string.Empty;
        public int? GroupingExpressionFieldIndex { get; set; }
        public int GroupingExpressionMemoBlockNumber { get; set; }
        public string HeaderSectionId { get; set; } = string.Empty;
        public int? HeaderRecordIndex { get; set; }
        public bool HeaderDeleted { get; set; }
        public string FooterSectionId { get; set; } = string.Empty;
        public int? FooterRecordIndex { get; set; }
        public bool FooterDeleted { get; set; }
        public string? HeaderStateDisplay { get; set; }
        public string? FooterStateDisplay { get; set; }
    }

    private sealed class DesignerSmokeTestRunner
    {
        public DesignerSmokeTestRunner(string[] args)
        {
            ParseArguments(args);
        }

        public bool ShouldInitializeUi => ready_ && !listOnly_;

        public void MarkStarted()
        {
            WriteStatus("started");
        }

        public void Run(string testName, Action smokeTest)
        {
            if (!ready_ || !MatchesFilters(testName))
            {
                return;
            }

            matchedAnyTest_ = true;
            if (listOnly_)
            {
                Console.WriteLine(testName);
                return;
            }

            var stopwatch = Stopwatch.StartNew();
            Console.WriteLine($"START: {testName}");
            try
            {
                smokeTest();
            }
            catch (Exception exception)
            {
                ++failures;
                Console.Error.WriteLine(
                    $"FAIL: {testName} threw {exception.GetType().FullName}: {exception.Message}");
                Console.Error.WriteLine(exception.ToString());
            }
            finally
            {
                Console.WriteLine($"END: {testName} ({stopwatch.Elapsed.TotalSeconds:F1}s)");
            }
        }

        public int Finish()
        {
            CloseHarnessForms();
            if (!ready_)
            {
                WriteStatus("invalid");
                return exitCode_;
            }

            if (!matchedAnyTest_)
            {
                Console.Error.WriteLine("no tests matched the requested selection");
                WriteStatus("completed");
                return 3;
            }

            if (listOnly_)
            {
                WriteStatus("completed");
                return 0;
            }

            if (failures != 0)
            {
                Console.Error.WriteLine($"{failures} UI smoke test(s) failed.");
                WriteStatus("completed");
                return 1;
            }

            Console.WriteLine("All UI smoke tests passed.");
            WriteStatus("completed");
            return 0;
        }

        private static void PrintUsage()
        {
            Console.Error.WriteLine(
                "usage: Copperfin.DesignerSmokeTests [--list-tests] [--filter <substring>] [--exact <name>] [--status-file <path>]\n" +
                "       Copperfin.DesignerSmokeTests --list-tests [--filter <substring>] [--exact <name>] [--status-file <path>]");
        }

        private void FailUsage(string message, int code = 2)
        {
            Console.Error.WriteLine(message);
            PrintUsage();
            ready_ = false;
            exitCode_ = code;
        }

        private void ParseArguments(string[] args)
        {
            for (int index = 0; index < args.Length; ++index)
            {
                string argument = args[index];
                if (argument == "--help")
                {
                    PrintUsage();
                    ready_ = false;
                    exitCode_ = 0;
                    return;
                }
                if (argument == "--list-tests")
                {
                    listOnly_ = true;
                    continue;
                }
                if (argument == "--filter")
                {
                    if (index + 1 >= args.Length)
                    {
                        FailUsage("missing value for --filter");
                        return;
                    }

                    substringFilter_ = args[++index];
                    continue;
                }
                if (argument == "--exact")
                {
                    if (index + 1 >= args.Length)
                    {
                        FailUsage("missing value for --exact");
                        return;
                    }

                    exactFilter_ = args[++index];
                    continue;
                }
                if (argument == "--status-file")
                {
                    if (index + 1 >= args.Length)
                    {
                        FailUsage("missing value for --status-file");
                        return;
                    }

                    statusFile_ = args[++index];
                    continue;
                }
                if (argument.StartsWith("-", StringComparison.Ordinal))
                {
                    FailUsage("unknown option: " + argument);
                    return;
                }

                FailUsage("unexpected positional argument: " + argument);
                return;
            }
        }

        private bool MatchesFilters(string testName)
        {
            if (!string.IsNullOrEmpty(exactFilter_) &&
                !string.Equals(testName, exactFilter_, StringComparison.Ordinal))
            {
                return false;
            }

            if (!string.IsNullOrEmpty(substringFilter_) &&
                testName.IndexOf(substringFilter_, StringComparison.Ordinal) < 0)
            {
                return false;
            }

            return true;
        }

        private bool ready_ = true;
        private int exitCode_;
        private bool listOnly_;
        private bool matchedAnyTest_;
        private string substringFilter_ = string.Empty;
        private string exactFilter_ = string.Empty;
        private string statusFile_ = string.Empty;

        private void WriteStatus(string status)
        {
            if (string.IsNullOrWhiteSpace(statusFile_))
            {
                return;
            }

            try
            {
                File.WriteAllText(statusFile_, status + Environment.NewLine);
            }
            catch
            {
                // Status reporting must never change the smoke result.
            }
        }
    }


}
