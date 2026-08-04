
// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

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
    private static string BuildBatchUpdateHostResponseJson()
    {
        return """
{"Status":"ok","Document":{"AssetFamily":"report","FieldCount":5,"Objects":[{"RecordIndex":6,"Title":"customer.company","Subtitle":"field","Properties":[{"Name":"HPOS","Value":"1200"},{"Name":"VPOS","Value":"2600"},{"Name":"WIDTH","Value":"4000"},{"Name":"HEIGHT","Value":"500"},{"Name":"EXPR","Value":"customer.company"}]}],"ReportLayout":{"Sections":[{"Id":"detail_1","Title":"Detail","BandKind":"detail","RecordIndex":42,"Top":2000,"Height":5000,"Objects":[{"RecordIndex":6,"ObjectKind":"field","Title":"customer.company","Expression":"customer.company","Left":1200,"Top":2600,"Width":4000,"Height":500}]}],"DeletedSections":[],"UnplacedObjects":[]}}}
""";
    }

    private static string BuildBatchUpdatePreviewRefreshHostResponseJson()
    {
        return """
{"Status":"ok","Document":{"AssetFamily":"report","FieldCount":5,"Objects":[{"RecordIndex":6,"Title":"customer.company","Subtitle":"field","Properties":[{"Name":"HPOS","Value":"1500"},{"Name":"VPOS","Value":"2800"},{"Name":"WIDTH","Value":"4600"},{"Name":"HEIGHT","Value":"3500"},{"Name":"EXPR","Value":"customer.company"}]}],"ReportLayout":{"PreviewBoundsAvailable":true,"PreviewBoundsLeft":1500,"PreviewBoundsTop":2800,"PreviewBoundsRight":6100,"PreviewBoundsBottom":6300,"PreviewBoundsWidth":4600,"PreviewBoundsHeight":3500,"Sections":[{"Id":"detail_1","Title":"Detail","BandKind":"detail","RecordIndex":42,"Top":2000,"Height":5000,"Objects":[{"RecordIndex":6,"ObjectKind":"field","Title":"customer.company","Expression":"customer.company","Left":1500,"Top":2800,"Width":4600,"Height":3500}]}],"DeletedSections":[],"UnplacedObjects":[]}}}
""";
    }

    private static string BuildBatchLabelUpdatePreviewRefreshHostResponseJson()
    {
        return """
{"Status":"ok","Document":{"AssetFamily":"label","FieldCount":5,"Objects":[{"RecordIndex":6,"Title":"customer.company","Subtitle":"label","Properties":[{"Name":"HPOS","Value":"1500"},{"Name":"VPOS","Value":"2800"},{"Name":"WIDTH","Value":"4600"},{"Name":"HEIGHT","Value":"3500"},{"Name":"EXPR","Value":"customer.company"}]}],"ReportLayout":{"IsLabel":true,"PreviewBoundsAvailable":true,"PreviewBoundsLeft":1500,"PreviewBoundsTop":2800,"PreviewBoundsRight":6100,"PreviewBoundsBottom":6300,"PreviewBoundsWidth":4600,"PreviewBoundsHeight":3500,"Sections":[{"Id":"detail_1","Title":"Detail","BandKind":"detail","RecordIndex":42,"Top":2000,"Height":5000,"Objects":[{"RecordIndex":6,"ObjectKind":"label","Title":"customer.company","Expression":"customer.company","Left":1500,"Top":2800,"Width":4600,"Height":3500}]}],"DeletedSections":[],"UnplacedObjects":[]}}}
""";
    }

    private static string BuildSectionUpdateHostResponseJson()
    {
        return """
{"Status":"ok","Document":{"AssetFamily":"report","FieldCount":5,"Objects":[{"RecordIndex":6,"Title":"customer.company","Subtitle":"field","Properties":[{"Name":"HPOS","Value":"1200"},{"Name":"VPOS","Value":"3800"},{"Name":"WIDTH","Value":"4000"},{"Name":"HEIGHT","Value":"500"},{"Name":"EXPR","Value":"customer.company"}]}],"ReportLayout":{"Sections":[{"Id":"detail_1","Title":"Detail","BandKind":"detail","RecordIndex":42,"Top":3200,"Height":5000,"Objects":[{"RecordIndex":6,"ObjectKind":"field","Title":"customer.company","Expression":"customer.company","Left":1200,"Top":3800,"Width":4000,"Height":500}]}],"DeletedSections":[],"UnplacedObjects":[]}}}
""";
    }

    private static string BuildGroupingUpdateHostResponseJson()
    {
        return """
{"Status":"ok","Document":{"AssetFamily":"report","ReportLayout":{"Groupings":[{"GroupingIndex":1,"NestingDepth":2,"Expression":"customer.region","ExpressionFieldIndex":2,"ExpressionMemoBlockNumber":7,"HeaderSectionId":"group_header_7","HeaderRecordIndex":41,"HeaderDeleted":false,"FooterSectionId":"group_footer_7","FooterRecordIndex":47,"FooterDeleted":true}],"Sections":[{"Id":"detail_1","Title":"Detail","BandKind":"detail","RecordIndex":42,"Top":2000,"Height":5000}],"DeletedSections":[],"UnplacedObjects":[]}}}
""";
    }

    private static string BuildSettingsUpdateHostResponseJson()
    {
        return """
{"Status":"ok","Document":{"AssetFamily":"report","FieldCount":5,"Objects":[],"ReportLayout":{"DocumentTitle":"Customer Invoice","PreviewBoundsAvailable":true,"PreviewBoundsLeft":0,"PreviewBoundsTop":2000,"PreviewBoundsRight":5200,"PreviewBoundsBottom":8100,"PreviewBoundsWidth":5200,"PreviewBoundsHeight":6100,"DeletedPreviewBoundsAvailable":true,"DeletedPreviewBoundsLeft":1000,"DeletedPreviewBoundsTop":2600,"DeletedPreviewBoundsRight":2200,"DeletedPreviewBoundsBottom":2900,"DeletedPreviewBoundsWidth":1200,"DeletedPreviewBoundsHeight":300,"Settings":[{"Name":"ORIENTATION","Value":"0","RecordIndex":0,"FieldIndex":2,"SourceLineIndex":0,"MemoBlockNumber":9},{"Name":"COLS","Value":"2","RecordIndex":0,"FieldIndex":2,"SourceLineIndex":1,"MemoBlockNumber":9},{"Name":"COLWIDTH","Value":"4200","RecordIndex":0,"FieldIndex":2,"SourceLineIndex":2,"MemoBlockNumber":9},{"Name":"COLSPACING","Value":"120","RecordIndex":0,"FieldIndex":2,"SourceLineIndex":3,"MemoBlockNumber":9},{"Name":"PAPERLENGTH","Value":"2794","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":4,"MemoBlockNumber":8},{"Name":"PAPERWIDTH","Value":"2159","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":5,"MemoBlockNumber":8},{"Name":"TOPMARGIN","Value":"20","RecordIndex":0,"FieldIndex":3,"MemoBlockNumber":0},{"Name":"LEFTMARGIN","Value":"15","RecordIndex":0,"FieldIndex":10,"MemoBlockNumber":0},{"Name":"RIGHTMARGIN","Value":"25","RecordIndex":0,"FieldIndex":11,"MemoBlockNumber":0},{"Name":"TAG","Value":"customer.country","RecordIndex":0,"FieldIndex":9,"MemoBlockNumber":11},{"Name":"DRIVER","Value":"winspool","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":10,"MemoBlockNumber":8},{"Name":"DEVICE","Value":"FinePrint 2000","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":11,"MemoBlockNumber":8},{"Name":"OUTPUT","Value":"FPR4:","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":12,"MemoBlockNumber":8},{"Name":"DEFAULTSOURCE","Value":"15","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":13,"MemoBlockNumber":8},{"Name":"PRINTQUALITY","Value":"600","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":14,"MemoBlockNumber":8},{"Name":"YRESOLUTION","Value":"600","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":15,"MemoBlockNumber":8},{"Name":"TTOPTION","Value":"3","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":16,"MemoBlockNumber":8},{"Name":"COLOR","Value":"1","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":17,"MemoBlockNumber":8},{"Name":"ASCII","Value":"9","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":18,"MemoBlockNumber":8},{"Name":"COLLATE","Value":"1","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":19,"MemoBlockNumber":8},{"Name":"COPIES","Value":"1","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":20,"MemoBlockNumber":8}],"Sections":[{"Id":"detail_1","Title":"Detail","BandKind":"detail","RecordIndex":42,"Top":2000,"Height":5000,"Objects":[]}],"DeletedSections":[],"UnplacedObjects":[]}}}
""";
    }

    private static string BuildDeletedSettingsUpdateHostResponseJson()
    {
        return """
{"Status":"ok","Document":{"AssetFamily":"report","FieldCount":5,"Objects":[],"ReportLayout":{"DocumentTitle":"Deleted Customer Invoice","DeletedPreviewBoundsAvailable":true,"DeletedPreviewBoundsLeft":1000,"DeletedPreviewBoundsTop":2600,"DeletedPreviewBoundsRight":2200,"DeletedPreviewBoundsBottom":2900,"DeletedPreviewBoundsWidth":1200,"DeletedPreviewBoundsHeight":300,"DeletedSettings":[{"Name":"ORIENTATION","Value":"1","RecordIndex":0,"FieldIndex":2,"SourceLineIndex":0,"MemoBlockNumber":19},{"Name":"COLS","Value":"3","RecordIndex":0,"FieldIndex":2,"SourceLineIndex":1,"MemoBlockNumber":19},{"Name":"COLWIDTH","Value":"2400","RecordIndex":0,"FieldIndex":2,"SourceLineIndex":2,"MemoBlockNumber":19},{"Name":"COLSPACING","Value":"240","RecordIndex":0,"FieldIndex":2,"SourceLineIndex":3,"MemoBlockNumber":19},{"Name":"PAPERLENGTH","Value":"4318","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":4,"MemoBlockNumber":18},{"Name":"PAPERWIDTH","Value":"2794","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":5,"MemoBlockNumber":18},{"Name":"TOPMARGIN","Value":"40","RecordIndex":0,"FieldIndex":3,"MemoBlockNumber":0},{"Name":"LEFTMARGIN","Value":"35","RecordIndex":0,"FieldIndex":10,"MemoBlockNumber":0},{"Name":"RIGHTMARGIN","Value":"45","RecordIndex":0,"FieldIndex":11,"MemoBlockNumber":0},{"Name":"TAG","Value":"deleted.customer.country","RecordIndex":0,"FieldIndex":9,"MemoBlockNumber":21},{"Name":"DRIVER","Value":"deleted.winspool","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":10,"MemoBlockNumber":18},{"Name":"DEVICE","Value":"Deleted Printer","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":11,"MemoBlockNumber":18},{"Name":"OUTPUT","Value":"DPRN:","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":12,"MemoBlockNumber":18},{"Name":"DEFAULTSOURCE","Value":"16","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":13,"MemoBlockNumber":18},{"Name":"PRINTQUALITY","Value":"1200","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":14,"MemoBlockNumber":18},{"Name":"YRESOLUTION","Value":"1200","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":15,"MemoBlockNumber":18},{"Name":"TTOPTION","Value":"2","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":16,"MemoBlockNumber":18},{"Name":"COLOR","Value":"2","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":17,"MemoBlockNumber":18},{"Name":"ASCII","Value":"10","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":18,"MemoBlockNumber":18},{"Name":"COLLATE","Value":"0","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":19,"MemoBlockNumber":18},{"Name":"COPIES","Value":"2","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":20,"MemoBlockNumber":18}],"Sections":[{"Id":"detail_1","Title":"Detail","BandKind":"detail","RecordIndex":42,"Top":2000,"Height":5000,"Objects":[]}],"DeletedSections":[],"UnplacedObjects":[]}}}
""";
    }

    private static string BuildDeletedSettingsMissingOrientationHostResponseJson()
    {
        return """
{"Status":"ok","Document":{"AssetFamily":"report","FieldCount":5,"Objects":[],"ReportLayout":{"DocumentTitle":"Deleted Customer Invoice","DeletedPreviewBoundsAvailable":true,"DeletedPreviewBoundsLeft":1000,"DeletedPreviewBoundsTop":2600,"DeletedPreviewBoundsRight":2200,"DeletedPreviewBoundsBottom":2900,"DeletedPreviewBoundsWidth":1200,"DeletedPreviewBoundsHeight":300,"DeletedSettings":[{"Name":"ORIENTATION","Value":"0","RecordIndex":0,"FieldIndex":2,"SourceLineIndex":0,"MemoBlockNumber":19},{"Name":"COLS","Value":"3","RecordIndex":0,"FieldIndex":2,"SourceLineIndex":1,"MemoBlockNumber":19},{"Name":"COLWIDTH","Value":"2400","RecordIndex":0,"FieldIndex":2,"SourceLineIndex":2,"MemoBlockNumber":19},{"Name":"COLSPACING","Value":"180","RecordIndex":0,"FieldIndex":2,"SourceLineIndex":3,"MemoBlockNumber":19},{"Name":"PAPERLENGTH","Value":"4318","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":4,"MemoBlockNumber":18},{"Name":"PAPERWIDTH","Value":"2794","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":5,"MemoBlockNumber":18},{"Name":"TOPMARGIN","Value":"40","RecordIndex":0,"FieldIndex":3,"MemoBlockNumber":0},{"Name":"LEFTMARGIN","Value":"35","RecordIndex":0,"FieldIndex":10,"MemoBlockNumber":0},{"Name":"RIGHTMARGIN","Value":"45","RecordIndex":0,"FieldIndex":11,"MemoBlockNumber":0},{"Name":"TAG","Value":"deleted.customer.country","RecordIndex":0,"FieldIndex":9,"MemoBlockNumber":21},{"Name":"DRIVER","Value":"deleted.winspool","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":10,"MemoBlockNumber":18},{"Name":"DEVICE","Value":"Deleted Printer","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":11,"MemoBlockNumber":18},{"Name":"OUTPUT","Value":"DPRN:","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":12,"MemoBlockNumber":18},{"Name":"DEFAULTSOURCE","Value":"16","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":13,"MemoBlockNumber":18},{"Name":"PRINTQUALITY","Value":"1200","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":14,"MemoBlockNumber":18},{"Name":"YRESOLUTION","Value":"1200","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":15,"MemoBlockNumber":18},{"Name":"TTOPTION","Value":"2","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":16,"MemoBlockNumber":18},{"Name":"COLOR","Value":"2","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":17,"MemoBlockNumber":18},{"Name":"ASCII","Value":"10","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":18,"MemoBlockNumber":18},{"Name":"COLLATE","Value":"0","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":19,"MemoBlockNumber":18},{"Name":"COPIES","Value":"2","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":20,"MemoBlockNumber":18}],"Sections":[{"Id":"detail_1","Title":"Detail","BandKind":"detail","RecordIndex":42,"Top":2000,"Height":5000,"Objects":[]}],"DeletedSections":[],"UnplacedObjects":[]}}}
""";
    }

    private static string BuildDeletedSettingsMissingPaperSizeHostResponseJson()
    {
        return """
{"Status":"ok","Document":{"AssetFamily":"report","FieldCount":5,"Objects":[],"ReportLayout":{"DocumentTitle":"Deleted Customer Invoice","DeletedPreviewBoundsAvailable":true,"DeletedPreviewBoundsLeft":1000,"DeletedPreviewBoundsTop":2600,"DeletedPreviewBoundsRight":2200,"DeletedPreviewBoundsBottom":2900,"DeletedPreviewBoundsWidth":1200,"DeletedPreviewBoundsHeight":300,"DeletedSettings":[{"Name":"ORIENTATION","Value":"1","RecordIndex":0,"FieldIndex":2,"SourceLineIndex":0,"MemoBlockNumber":19},{"Name":"PAPERSIZE","Value":"9","RecordIndex":0,"FieldIndex":2,"SourceLineIndex":1,"MemoBlockNumber":19},{"Name":"COLS","Value":"3","RecordIndex":0,"FieldIndex":2,"SourceLineIndex":2,"MemoBlockNumber":19},{"Name":"COLWIDTH","Value":"2400","RecordIndex":0,"FieldIndex":2,"SourceLineIndex":3,"MemoBlockNumber":19},{"Name":"COLSPACING","Value":"180","RecordIndex":0,"FieldIndex":2,"SourceLineIndex":4,"MemoBlockNumber":19},{"Name":"PAPERLENGTH","Value":"4318","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":4,"MemoBlockNumber":18},{"Name":"PAPERWIDTH","Value":"2794","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":5,"MemoBlockNumber":18},{"Name":"TOPMARGIN","Value":"40","RecordIndex":0,"FieldIndex":3,"MemoBlockNumber":0},{"Name":"LEFTMARGIN","Value":"35","RecordIndex":0,"FieldIndex":10,"MemoBlockNumber":0},{"Name":"RIGHTMARGIN","Value":"45","RecordIndex":0,"FieldIndex":11,"MemoBlockNumber":0},{"Name":"TAG","Value":"deleted.customer.country","RecordIndex":0,"FieldIndex":9,"MemoBlockNumber":21},{"Name":"DRIVER","Value":"deleted.winspool","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":10,"MemoBlockNumber":18},{"Name":"DEVICE","Value":"Deleted Printer","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":11,"MemoBlockNumber":18},{"Name":"OUTPUT","Value":"DPRN:","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":12,"MemoBlockNumber":18},{"Name":"DEFAULTSOURCE","Value":"16","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":13,"MemoBlockNumber":18},{"Name":"PRINTQUALITY","Value":"1200","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":14,"MemoBlockNumber":18},{"Name":"YRESOLUTION","Value":"1200","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":15,"MemoBlockNumber":18},{"Name":"TTOPTION","Value":"2","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":16,"MemoBlockNumber":18},{"Name":"COLOR","Value":"2","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":17,"MemoBlockNumber":18},{"Name":"ASCII","Value":"10","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":18,"MemoBlockNumber":18},{"Name":"COLLATE","Value":"0","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":19,"MemoBlockNumber":18},{"Name":"COPIES","Value":"2","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":20,"MemoBlockNumber":18}],"Sections":[{"Id":"detail_1","Title":"Detail","BandKind":"detail","RecordIndex":42,"Top":2000,"Height":5000,"Objects":[]}],"DeletedSections":[],"UnplacedObjects":[]}}}
""";
    }

    private static string BuildDeletedSettingsMissingGridVHostResponseJson()
    {
        return """
{"Status":"ok","Document":{"AssetFamily":"report","FieldCount":5,"Objects":[],"ReportLayout":{"DocumentTitle":"Deleted Customer Invoice","DeletedPreviewBoundsAvailable":true,"DeletedPreviewBoundsLeft":1000,"DeletedPreviewBoundsTop":2600,"DeletedPreviewBoundsRight":2200,"DeletedPreviewBoundsBottom":2900,"DeletedPreviewBoundsWidth":1200,"DeletedPreviewBoundsHeight":300,"DeletedSettings":[{"Name":"ORIENTATION","Value":"1","RecordIndex":0,"FieldIndex":2,"SourceLineIndex":0,"MemoBlockNumber":19},{"Name":"PAPERSIZE","Value":"9","RecordIndex":0,"FieldIndex":2,"SourceLineIndex":1,"MemoBlockNumber":19},{"Name":"COLS","Value":"3","RecordIndex":0,"FieldIndex":2,"SourceLineIndex":2,"MemoBlockNumber":19},{"Name":"COLWIDTH","Value":"2400","RecordIndex":0,"FieldIndex":2,"SourceLineIndex":3,"MemoBlockNumber":19},{"Name":"COLSPACING","Value":"180","RecordIndex":0,"FieldIndex":2,"SourceLineIndex":4,"MemoBlockNumber":19},{"Name":"GRIDV","Value":"6","RecordIndex":0,"FieldIndex":2,"SourceLineIndex":5,"MemoBlockNumber":19},{"Name":"GRIDH","Value":"10","RecordIndex":0,"FieldIndex":2,"SourceLineIndex":6,"MemoBlockNumber":19},{"Name":"PAPERLENGTH","Value":"4318","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":4,"MemoBlockNumber":18},{"Name":"PAPERWIDTH","Value":"2794","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":5,"MemoBlockNumber":18},{"Name":"TOPMARGIN","Value":"40","RecordIndex":0,"FieldIndex":3,"MemoBlockNumber":0},{"Name":"LEFTMARGIN","Value":"35","RecordIndex":0,"FieldIndex":10,"MemoBlockNumber":0},{"Name":"RIGHTMARGIN","Value":"45","RecordIndex":0,"FieldIndex":11,"MemoBlockNumber":0},{"Name":"TAG","Value":"deleted.customer.country","RecordIndex":0,"FieldIndex":9,"MemoBlockNumber":21},{"Name":"DRIVER","Value":"deleted.winspool","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":10,"MemoBlockNumber":18},{"Name":"DEVICE","Value":"Deleted Printer","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":11,"MemoBlockNumber":18},{"Name":"OUTPUT","Value":"DPRN:","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":12,"MemoBlockNumber":18},{"Name":"DEFAULTSOURCE","Value":"16","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":13,"MemoBlockNumber":18},{"Name":"PRINTQUALITY","Value":"1200","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":14,"MemoBlockNumber":18},{"Name":"YRESOLUTION","Value":"1200","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":15,"MemoBlockNumber":18},{"Name":"TTOPTION","Value":"2","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":16,"MemoBlockNumber":18},{"Name":"COLOR","Value":"2","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":17,"MemoBlockNumber":18},{"Name":"ASCII","Value":"10","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":18,"MemoBlockNumber":18},{"Name":"COLLATE","Value":"0","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":19,"MemoBlockNumber":18},{"Name":"COPIES","Value":"2","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":20,"MemoBlockNumber":18}],"Sections":[{"Id":"detail_1","Title":"Detail","BandKind":"detail","RecordIndex":42,"Top":2000,"Height":5000,"Objects":[]}],"DeletedSections":[],"UnplacedObjects":[]}}}
""";
    }

    private static string BuildDeletedSettingsMissingGridHHostResponseJson()
    {
        return """
{"Status":"ok","Document":{"AssetFamily":"report","FieldCount":5,"Objects":[],"ReportLayout":{"DocumentTitle":"Deleted Customer Invoice","DeletedPreviewBoundsAvailable":true,"DeletedPreviewBoundsLeft":1000,"DeletedPreviewBoundsTop":2600,"DeletedPreviewBoundsRight":2200,"DeletedPreviewBoundsBottom":2900,"DeletedPreviewBoundsWidth":1200,"DeletedPreviewBoundsHeight":300,"DeletedSettings":[{"Name":"ORIENTATION","Value":"1","RecordIndex":0,"FieldIndex":2,"SourceLineIndex":0,"MemoBlockNumber":19},{"Name":"PAPERSIZE","Value":"9","RecordIndex":0,"FieldIndex":2,"SourceLineIndex":1,"MemoBlockNumber":19},{"Name":"COLS","Value":"3","RecordIndex":0,"FieldIndex":2,"SourceLineIndex":2,"MemoBlockNumber":19},{"Name":"COLWIDTH","Value":"2400","RecordIndex":0,"FieldIndex":2,"SourceLineIndex":3,"MemoBlockNumber":19},{"Name":"COLSPACING","Value":"180","RecordIndex":0,"FieldIndex":2,"SourceLineIndex":4,"MemoBlockNumber":19},{"Name":"GRIDV","Value":"6","RecordIndex":0,"FieldIndex":2,"SourceLineIndex":5,"MemoBlockNumber":19},{"Name":"GRIDH","Value":"10","RecordIndex":0,"FieldIndex":2,"SourceLineIndex":6,"MemoBlockNumber":19},{"Name":"PAPERLENGTH","Value":"4318","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":4,"MemoBlockNumber":18},{"Name":"PAPERWIDTH","Value":"2794","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":5,"MemoBlockNumber":18},{"Name":"TOPMARGIN","Value":"40","RecordIndex":0,"FieldIndex":3,"MemoBlockNumber":0},{"Name":"LEFTMARGIN","Value":"35","RecordIndex":0,"FieldIndex":10,"MemoBlockNumber":0},{"Name":"RIGHTMARGIN","Value":"45","RecordIndex":0,"FieldIndex":11,"MemoBlockNumber":0},{"Name":"TAG","Value":"deleted.customer.country","RecordIndex":0,"FieldIndex":9,"MemoBlockNumber":21},{"Name":"DRIVER","Value":"deleted.winspool","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":10,"MemoBlockNumber":18},{"Name":"DEVICE","Value":"Deleted Printer","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":11,"MemoBlockNumber":18},{"Name":"OUTPUT","Value":"DPRN:","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":12,"MemoBlockNumber":18},{"Name":"DEFAULTSOURCE","Value":"16","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":13,"MemoBlockNumber":18},{"Name":"PRINTQUALITY","Value":"1200","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":14,"MemoBlockNumber":18},{"Name":"YRESOLUTION","Value":"1200","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":15,"MemoBlockNumber":18},{"Name":"TTOPTION","Value":"2","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":16,"MemoBlockNumber":18},{"Name":"COLOR","Value":"2","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":17,"MemoBlockNumber":18},{"Name":"ASCII","Value":"10","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":18,"MemoBlockNumber":18},{"Name":"COLLATE","Value":"0","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":19,"MemoBlockNumber":18},{"Name":"COPIES","Value":"2","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":20,"MemoBlockNumber":18}],"Sections":[{"Id":"detail_1","Title":"Detail","BandKind":"detail","RecordIndex":42,"Top":2000,"Height":5000,"Objects":[]}],"DeletedSections":[],"UnplacedObjects":[]}}}
""";
    }

    private static string BuildDeletedSettingsMissingPaperLengthHostResponseJson()
    {
        return """
{"Status":"ok","Document":{"AssetFamily":"report","FieldCount":5,"Objects":[],"ReportLayout":{"DocumentTitle":"Deleted Customer Invoice","DeletedPreviewBoundsAvailable":true,"DeletedPreviewBoundsLeft":1000,"DeletedPreviewBoundsTop":2600,"DeletedPreviewBoundsRight":2200,"DeletedPreviewBoundsBottom":2900,"DeletedPreviewBoundsWidth":1200,"DeletedPreviewBoundsHeight":300,"DeletedSettings":[{"Name":"ORIENTATION","Value":"1","RecordIndex":0,"FieldIndex":2,"SourceLineIndex":0,"MemoBlockNumber":19},{"Name":"COLS","Value":"3","RecordIndex":0,"FieldIndex":2,"SourceLineIndex":1,"MemoBlockNumber":19},{"Name":"COLWIDTH","Value":"2400","RecordIndex":0,"FieldIndex":2,"SourceLineIndex":2,"MemoBlockNumber":19},{"Name":"COLSPACING","Value":"180","RecordIndex":0,"FieldIndex":2,"SourceLineIndex":3,"MemoBlockNumber":19},{"Name":"PAPERLENGTH","Value":"5588","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":4,"MemoBlockNumber":18},{"Name":"PAPERWIDTH","Value":"2794","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":5,"MemoBlockNumber":18},{"Name":"TOPMARGIN","Value":"40","RecordIndex":0,"FieldIndex":3,"MemoBlockNumber":0},{"Name":"LEFTMARGIN","Value":"35","RecordIndex":0,"FieldIndex":10,"MemoBlockNumber":0},{"Name":"RIGHTMARGIN","Value":"45","RecordIndex":0,"FieldIndex":11,"MemoBlockNumber":0},{"Name":"TAG","Value":"deleted.customer.country","RecordIndex":0,"FieldIndex":9,"MemoBlockNumber":21},{"Name":"DRIVER","Value":"deleted.winspool","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":10,"MemoBlockNumber":18},{"Name":"DEVICE","Value":"Deleted Printer","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":11,"MemoBlockNumber":18},{"Name":"OUTPUT","Value":"DPRN:","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":12,"MemoBlockNumber":18},{"Name":"DEFAULTSOURCE","Value":"16","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":13,"MemoBlockNumber":18},{"Name":"PRINTQUALITY","Value":"1200","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":14,"MemoBlockNumber":18},{"Name":"YRESOLUTION","Value":"1200","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":15,"MemoBlockNumber":18},{"Name":"TTOPTION","Value":"2","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":16,"MemoBlockNumber":18},{"Name":"COLOR","Value":"2","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":17,"MemoBlockNumber":18},{"Name":"ASCII","Value":"10","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":18,"MemoBlockNumber":18},{"Name":"COLLATE","Value":"0","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":19,"MemoBlockNumber":18},{"Name":"COPIES","Value":"2","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":20,"MemoBlockNumber":18}],"Sections":[{"Id":"detail_1","Title":"Detail","BandKind":"detail","RecordIndex":42,"Top":2000,"Height":5000,"Objects":[]}],"DeletedSections":[],"UnplacedObjects":[]}}}
""";
    }

    private static string BuildDeletedSettingsMissingPaperWidthHostResponseJson()
    {
        return """
{"Status":"ok","Document":{"AssetFamily":"report","FieldCount":5,"Objects":[],"ReportLayout":{"DocumentTitle":"Deleted Customer Invoice","DeletedPreviewBoundsAvailable":true,"DeletedPreviewBoundsLeft":1000,"DeletedPreviewBoundsTop":2600,"DeletedPreviewBoundsRight":2200,"DeletedPreviewBoundsBottom":2900,"DeletedPreviewBoundsWidth":1200,"DeletedPreviewBoundsHeight":300,"DeletedSettings":[{"Name":"ORIENTATION","Value":"1","RecordIndex":0,"FieldIndex":2,"SourceLineIndex":0,"MemoBlockNumber":19},{"Name":"COLS","Value":"3","RecordIndex":0,"FieldIndex":2,"SourceLineIndex":1,"MemoBlockNumber":19},{"Name":"COLWIDTH","Value":"2400","RecordIndex":0,"FieldIndex":2,"SourceLineIndex":2,"MemoBlockNumber":19},{"Name":"COLSPACING","Value":"180","RecordIndex":0,"FieldIndex":2,"SourceLineIndex":3,"MemoBlockNumber":19},{"Name":"PAPERLENGTH","Value":"4318","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":4,"MemoBlockNumber":18},{"Name":"PAPERWIDTH","Value":"4318","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":5,"MemoBlockNumber":18},{"Name":"TOPMARGIN","Value":"40","RecordIndex":0,"FieldIndex":3,"MemoBlockNumber":0},{"Name":"LEFTMARGIN","Value":"35","RecordIndex":0,"FieldIndex":10,"MemoBlockNumber":0},{"Name":"RIGHTMARGIN","Value":"45","RecordIndex":0,"FieldIndex":11,"MemoBlockNumber":0},{"Name":"TAG","Value":"deleted.customer.country","RecordIndex":0,"FieldIndex":9,"MemoBlockNumber":21},{"Name":"DRIVER","Value":"deleted.winspool","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":10,"MemoBlockNumber":18},{"Name":"DEVICE","Value":"Deleted Printer","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":11,"MemoBlockNumber":18},{"Name":"OUTPUT","Value":"DPRN:","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":12,"MemoBlockNumber":18},{"Name":"DEFAULTSOURCE","Value":"16","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":13,"MemoBlockNumber":18},{"Name":"PRINTQUALITY","Value":"1200","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":14,"MemoBlockNumber":18},{"Name":"YRESOLUTION","Value":"1200","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":15,"MemoBlockNumber":18},{"Name":"TTOPTION","Value":"2","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":16,"MemoBlockNumber":18},{"Name":"COLOR","Value":"2","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":17,"MemoBlockNumber":18},{"Name":"ASCII","Value":"10","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":18,"MemoBlockNumber":18},{"Name":"COLLATE","Value":"0","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":19,"MemoBlockNumber":18},{"Name":"COPIES","Value":"2","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":20,"MemoBlockNumber":18}],"Sections":[{"Id":"detail_1","Title":"Detail","BandKind":"detail","RecordIndex":42,"Top":2000,"Height":5000,"Objects":[]}],"DeletedSections":[],"UnplacedObjects":[]}}}
""";
    }

    private static string BuildDeletedSettingsMissingLeftMarginHostResponseJson()
    {
        return """
{"Status":"ok","Document":{"AssetFamily":"report","FieldCount":5,"Objects":[],"ReportLayout":{"DocumentTitle":"Deleted Customer Invoice","DeletedPreviewBoundsAvailable":true,"DeletedPreviewBoundsLeft":1000,"DeletedPreviewBoundsTop":2600,"DeletedPreviewBoundsRight":2200,"DeletedPreviewBoundsBottom":2900,"DeletedPreviewBoundsWidth":1200,"DeletedPreviewBoundsHeight":300,"DeletedSettings":[{"Name":"ORIENTATION","Value":"1","RecordIndex":0,"FieldIndex":2,"SourceLineIndex":0,"MemoBlockNumber":19},{"Name":"COLS","Value":"3","RecordIndex":0,"FieldIndex":2,"SourceLineIndex":1,"MemoBlockNumber":19},{"Name":"COLWIDTH","Value":"2400","RecordIndex":0,"FieldIndex":2,"SourceLineIndex":2,"MemoBlockNumber":19},{"Name":"COLSPACING","Value":"180","RecordIndex":0,"FieldIndex":2,"SourceLineIndex":3,"MemoBlockNumber":19},{"Name":"PAPERLENGTH","Value":"4318","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":4,"MemoBlockNumber":18},{"Name":"PAPERWIDTH","Value":"2794","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":5,"MemoBlockNumber":18},{"Name":"TOPMARGIN","Value":"40","RecordIndex":0,"FieldIndex":3,"MemoBlockNumber":0},{"Name":"LEFTMARGIN","Value":"50","RecordIndex":0,"FieldIndex":10,"MemoBlockNumber":0},{"Name":"RIGHTMARGIN","Value":"45","RecordIndex":0,"FieldIndex":11,"MemoBlockNumber":0},{"Name":"TAG","Value":"deleted.customer.country","RecordIndex":0,"FieldIndex":9,"MemoBlockNumber":21},{"Name":"DRIVER","Value":"deleted.winspool","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":10,"MemoBlockNumber":18},{"Name":"DEVICE","Value":"Deleted Printer","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":11,"MemoBlockNumber":18},{"Name":"OUTPUT","Value":"DPRN:","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":12,"MemoBlockNumber":18},{"Name":"DEFAULTSOURCE","Value":"16","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":13,"MemoBlockNumber":18},{"Name":"PRINTQUALITY","Value":"1200","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":14,"MemoBlockNumber":18},{"Name":"YRESOLUTION","Value":"1200","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":15,"MemoBlockNumber":18},{"Name":"TTOPTION","Value":"2","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":16,"MemoBlockNumber":18},{"Name":"COLOR","Value":"2","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":17,"MemoBlockNumber":18},{"Name":"ASCII","Value":"10","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":18,"MemoBlockNumber":18},{"Name":"COLLATE","Value":"0","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":19,"MemoBlockNumber":18},{"Name":"COPIES","Value":"2","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":20,"MemoBlockNumber":18}],"Sections":[{"Id":"detail_1","Title":"Detail","BandKind":"detail","RecordIndex":42,"Top":2000,"Height":5000,"Objects":[]}],"DeletedSections":[],"UnplacedObjects":[]}}}
""";
    }

    private static string BuildDeletedSettingsMissingRightMarginHostResponseJson()
    {
        return """
{"Status":"ok","Document":{"AssetFamily":"report","FieldCount":5,"Objects":[],"ReportLayout":{"DocumentTitle":"Deleted Customer Invoice","DeletedPreviewBoundsAvailable":true,"DeletedPreviewBoundsLeft":1000,"DeletedPreviewBoundsTop":2600,"DeletedPreviewBoundsRight":2200,"DeletedPreviewBoundsBottom":2900,"DeletedPreviewBoundsWidth":1200,"DeletedPreviewBoundsHeight":300,"DeletedSettings":[{"Name":"ORIENTATION","Value":"1","RecordIndex":0,"FieldIndex":2,"SourceLineIndex":0,"MemoBlockNumber":19},{"Name":"COLS","Value":"3","RecordIndex":0,"FieldIndex":2,"SourceLineIndex":1,"MemoBlockNumber":19},{"Name":"COLWIDTH","Value":"2400","RecordIndex":0,"FieldIndex":2,"SourceLineIndex":2,"MemoBlockNumber":19},{"Name":"COLSPACING","Value":"180","RecordIndex":0,"FieldIndex":2,"SourceLineIndex":3,"MemoBlockNumber":19},{"Name":"PAPERLENGTH","Value":"4318","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":4,"MemoBlockNumber":18},{"Name":"PAPERWIDTH","Value":"2794","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":5,"MemoBlockNumber":18},{"Name":"TOPMARGIN","Value":"40","RecordIndex":0,"FieldIndex":3,"MemoBlockNumber":0},{"Name":"LEFTMARGIN","Value":"35","RecordIndex":0,"FieldIndex":10,"MemoBlockNumber":0},{"Name":"RIGHTMARGIN","Value":"60","RecordIndex":0,"FieldIndex":11,"MemoBlockNumber":0},{"Name":"TAG","Value":"deleted.customer.country","RecordIndex":0,"FieldIndex":9,"MemoBlockNumber":21},{"Name":"DRIVER","Value":"deleted.winspool","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":10,"MemoBlockNumber":18},{"Name":"DEVICE","Value":"Deleted Printer","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":11,"MemoBlockNumber":18},{"Name":"OUTPUT","Value":"DPRN:","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":12,"MemoBlockNumber":18},{"Name":"DEFAULTSOURCE","Value":"16","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":13,"MemoBlockNumber":18},{"Name":"PRINTQUALITY","Value":"1200","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":14,"MemoBlockNumber":18},{"Name":"YRESOLUTION","Value":"1200","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":15,"MemoBlockNumber":18},{"Name":"TTOPTION","Value":"2","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":16,"MemoBlockNumber":18},{"Name":"COLOR","Value":"2","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":17,"MemoBlockNumber":18},{"Name":"ASCII","Value":"10","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":18,"MemoBlockNumber":18},{"Name":"COLLATE","Value":"0","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":19,"MemoBlockNumber":18},{"Name":"COPIES","Value":"2","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":20,"MemoBlockNumber":18}],"Sections":[{"Id":"detail_1","Title":"Detail","BandKind":"detail","RecordIndex":42,"Top":2000,"Height":5000,"Objects":[]}],"DeletedSections":[],"UnplacedObjects":[]}}}
""";
    }

    private static string BuildDeletedSettingsMissingTopMarginHostResponseJson()
    {
        return """
{"Status":"ok","Document":{"AssetFamily":"report","FieldCount":5,"Objects":[],"ReportLayout":{"DocumentTitle":"Deleted Customer Invoice","DeletedPreviewBoundsAvailable":true,"DeletedPreviewBoundsLeft":1000,"DeletedPreviewBoundsTop":2600,"DeletedPreviewBoundsRight":2200,"DeletedPreviewBoundsBottom":2900,"DeletedPreviewBoundsWidth":1200,"DeletedPreviewBoundsHeight":300,"DeletedSettings":[{"Name":"ORIENTATION","Value":"1","RecordIndex":0,"FieldIndex":2,"SourceLineIndex":0,"MemoBlockNumber":19},{"Name":"COLS","Value":"3","RecordIndex":0,"FieldIndex":2,"SourceLineIndex":1,"MemoBlockNumber":19},{"Name":"COLWIDTH","Value":"2400","RecordIndex":0,"FieldIndex":2,"SourceLineIndex":2,"MemoBlockNumber":19},{"Name":"COLSPACING","Value":"180","RecordIndex":0,"FieldIndex":2,"SourceLineIndex":3,"MemoBlockNumber":19},{"Name":"PAPERLENGTH","Value":"4318","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":4,"MemoBlockNumber":18},{"Name":"PAPERWIDTH","Value":"2794","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":5,"MemoBlockNumber":18},{"Name":"TOPMARGIN","Value":"40","RecordIndex":0,"FieldIndex":3,"MemoBlockNumber":0},{"Name":"LEFTMARGIN","Value":"35","RecordIndex":0,"FieldIndex":10,"MemoBlockNumber":0},{"Name":"RIGHTMARGIN","Value":"45","RecordIndex":0,"FieldIndex":11,"MemoBlockNumber":0},{"Name":"TAG","Value":"deleted.customer.country","RecordIndex":0,"FieldIndex":9,"MemoBlockNumber":21},{"Name":"DRIVER","Value":"deleted.winspool","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":10,"MemoBlockNumber":18},{"Name":"DEVICE","Value":"Deleted Printer","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":11,"MemoBlockNumber":18},{"Name":"OUTPUT","Value":"DPRN:","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":12,"MemoBlockNumber":18},{"Name":"DEFAULTSOURCE","Value":"16","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":13,"MemoBlockNumber":18},{"Name":"PRINTQUALITY","Value":"1200","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":14,"MemoBlockNumber":18},{"Name":"YRESOLUTION","Value":"1200","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":15,"MemoBlockNumber":18},{"Name":"TTOPTION","Value":"2","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":16,"MemoBlockNumber":18},{"Name":"COLOR","Value":"2","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":17,"MemoBlockNumber":18},{"Name":"ASCII","Value":"10","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":18,"MemoBlockNumber":18},{"Name":"COLLATE","Value":"0","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":19,"MemoBlockNumber":18},{"Name":"COPIES","Value":"2","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":20,"MemoBlockNumber":18}],"Sections":[{"Id":"detail_1","Title":"Detail","BandKind":"detail","RecordIndex":42,"Top":2000,"Height":5000,"Objects":[]}],"DeletedSections":[],"UnplacedObjects":[]}}}
""";
    }

    private static string BuildDeletedSettingsMissingBottomMarginHostResponseJson()
    {
        return """
{"Status":"ok","Document":{"AssetFamily":"report","FieldCount":5,"Objects":[],"ReportLayout":{"DocumentTitle":"Deleted Customer Invoice","DeletedPreviewBoundsAvailable":true,"DeletedPreviewBoundsLeft":1000,"DeletedPreviewBoundsTop":2600,"DeletedPreviewBoundsRight":2200,"DeletedPreviewBoundsBottom":2900,"DeletedPreviewBoundsWidth":1200,"DeletedPreviewBoundsHeight":300,"DeletedSettings":[{"Name":"ORIENTATION","Value":"1","RecordIndex":0,"FieldIndex":2,"SourceLineIndex":0,"MemoBlockNumber":19},{"Name":"COLS","Value":"3","RecordIndex":0,"FieldIndex":2,"SourceLineIndex":1,"MemoBlockNumber":19},{"Name":"COLWIDTH","Value":"2400","RecordIndex":0,"FieldIndex":2,"SourceLineIndex":2,"MemoBlockNumber":19},{"Name":"COLSPACING","Value":"180","RecordIndex":0,"FieldIndex":2,"SourceLineIndex":3,"MemoBlockNumber":19},{"Name":"PAPERLENGTH","Value":"4318","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":4,"MemoBlockNumber":18},{"Name":"PAPERWIDTH","Value":"2794","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":5,"MemoBlockNumber":18},{"Name":"BOTMARGIN","Value":"55","RecordIndex":0,"FieldIndex":3,"MemoBlockNumber":0},{"Name":"LEFTMARGIN","Value":"35","RecordIndex":0,"FieldIndex":10,"MemoBlockNumber":0},{"Name":"RIGHTMARGIN","Value":"45","RecordIndex":0,"FieldIndex":11,"MemoBlockNumber":0},{"Name":"TAG","Value":"deleted.customer.country","RecordIndex":0,"FieldIndex":9,"MemoBlockNumber":21},{"Name":"DRIVER","Value":"deleted.winspool","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":10,"MemoBlockNumber":18},{"Name":"DEVICE","Value":"Deleted Printer","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":11,"MemoBlockNumber":18},{"Name":"OUTPUT","Value":"DPRN:","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":12,"MemoBlockNumber":18},{"Name":"DEFAULTSOURCE","Value":"16","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":13,"MemoBlockNumber":18},{"Name":"PRINTQUALITY","Value":"1200","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":14,"MemoBlockNumber":18},{"Name":"YRESOLUTION","Value":"1200","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":15,"MemoBlockNumber":18},{"Name":"TTOPTION","Value":"2","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":16,"MemoBlockNumber":18},{"Name":"COLOR","Value":"2","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":17,"MemoBlockNumber":18},{"Name":"ASCII","Value":"10","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":18,"MemoBlockNumber":18},{"Name":"COLLATE","Value":"0","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":19,"MemoBlockNumber":18},{"Name":"COPIES","Value":"2","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":20,"MemoBlockNumber":18}],"Sections":[{"Id":"detail_1","Title":"Detail","BandKind":"detail","RecordIndex":42,"Top":2000,"Height":5000,"Objects":[]}],"DeletedSections":[],"UnplacedObjects":[]}}}
""";
    }

    private static string BuildDeletedSettingsMissingTagHostResponseJson()
    {
        return """
{"Status":"ok","Document":{"AssetFamily":"report","FieldCount":5,"Objects":[],"ReportLayout":{"DocumentTitle":"Deleted Customer Invoice","DeletedPreviewBoundsAvailable":true,"DeletedPreviewBoundsLeft":1000,"DeletedPreviewBoundsTop":2600,"DeletedPreviewBoundsRight":2200,"DeletedPreviewBoundsBottom":2900,"DeletedPreviewBoundsWidth":1200,"DeletedPreviewBoundsHeight":300,"DeletedSettings":[{"Name":"ORIENTATION","Value":"1","RecordIndex":0,"FieldIndex":2,"SourceLineIndex":0,"MemoBlockNumber":19},{"Name":"COLS","Value":"3","RecordIndex":0,"FieldIndex":2,"SourceLineIndex":1,"MemoBlockNumber":19},{"Name":"COLWIDTH","Value":"2400","RecordIndex":0,"FieldIndex":2,"SourceLineIndex":2,"MemoBlockNumber":19},{"Name":"COLSPACING","Value":"180","RecordIndex":0,"FieldIndex":2,"SourceLineIndex":3,"MemoBlockNumber":19},{"Name":"PAPERLENGTH","Value":"4318","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":4,"MemoBlockNumber":18},{"Name":"PAPERWIDTH","Value":"2794","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":5,"MemoBlockNumber":18},{"Name":"TOPMARGIN","Value":"40","RecordIndex":0,"FieldIndex":3,"MemoBlockNumber":0},{"Name":"LEFTMARGIN","Value":"35","RecordIndex":0,"FieldIndex":10,"MemoBlockNumber":0},{"Name":"RIGHTMARGIN","Value":"45","RecordIndex":0,"FieldIndex":11,"MemoBlockNumber":0},{"Name":"TAG","Value":"deleted.customer.region","RecordIndex":0,"FieldIndex":9,"MemoBlockNumber":31},{"Name":"DRIVER","Value":"deleted.winspool","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":10,"MemoBlockNumber":18},{"Name":"DEVICE","Value":"Deleted Printer","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":11,"MemoBlockNumber":18},{"Name":"OUTPUT","Value":"DPRN:","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":12,"MemoBlockNumber":18},{"Name":"DEFAULTSOURCE","Value":"16","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":13,"MemoBlockNumber":18},{"Name":"PRINTQUALITY","Value":"1200","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":14,"MemoBlockNumber":18},{"Name":"YRESOLUTION","Value":"1200","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":15,"MemoBlockNumber":18},{"Name":"TTOPTION","Value":"2","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":16,"MemoBlockNumber":18},{"Name":"COLOR","Value":"2","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":17,"MemoBlockNumber":18},{"Name":"ASCII","Value":"10","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":18,"MemoBlockNumber":18},{"Name":"COLLATE","Value":"0","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":19,"MemoBlockNumber":18},{"Name":"COPIES","Value":"2","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":20,"MemoBlockNumber":18}],"Sections":[{"Id":"detail_1","Title":"Detail","BandKind":"detail","RecordIndex":42,"Top":2000,"Height":5000,"Objects":[]}],"DeletedSections":[],"UnplacedObjects":[]}}}
""";
    }

    private static string BuildDeletedSettingsMissingDefaultSourceHostResponseJson()
    {
        return """
{"Status":"ok","Document":{"AssetFamily":"report","FieldCount":5,"Objects":[],"ReportLayout":{"DocumentTitle":"Deleted Customer Invoice","DeletedPreviewBoundsAvailable":true,"DeletedPreviewBoundsLeft":1000,"DeletedPreviewBoundsTop":2600,"DeletedPreviewBoundsRight":2200,"DeletedPreviewBoundsBottom":2900,"DeletedPreviewBoundsWidth":1200,"DeletedPreviewBoundsHeight":300,"DeletedSettings":[{"Name":"ORIENTATION","Value":"1","RecordIndex":0,"FieldIndex":2,"SourceLineIndex":0,"MemoBlockNumber":19},{"Name":"COLS","Value":"3","RecordIndex":0,"FieldIndex":2,"SourceLineIndex":1,"MemoBlockNumber":19},{"Name":"COLWIDTH","Value":"2400","RecordIndex":0,"FieldIndex":2,"SourceLineIndex":2,"MemoBlockNumber":19},{"Name":"COLSPACING","Value":"180","RecordIndex":0,"FieldIndex":2,"SourceLineIndex":3,"MemoBlockNumber":19},{"Name":"PAPERLENGTH","Value":"4318","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":4,"MemoBlockNumber":18},{"Name":"PAPERWIDTH","Value":"2794","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":5,"MemoBlockNumber":18},{"Name":"TOPMARGIN","Value":"40","RecordIndex":0,"FieldIndex":3,"MemoBlockNumber":0},{"Name":"LEFTMARGIN","Value":"35","RecordIndex":0,"FieldIndex":10,"MemoBlockNumber":0},{"Name":"RIGHTMARGIN","Value":"45","RecordIndex":0,"FieldIndex":11,"MemoBlockNumber":0},{"Name":"TAG","Value":"deleted.customer.country","RecordIndex":0,"FieldIndex":9,"MemoBlockNumber":21},{"Name":"DRIVER","Value":"deleted.winspool","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":10,"MemoBlockNumber":18},{"Name":"DEVICE","Value":"Deleted Printer","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":11,"MemoBlockNumber":18},{"Name":"OUTPUT","Value":"DPRN:","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":12,"MemoBlockNumber":18},{"Name":"DEFAULTSOURCE","Value":"17","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":13,"MemoBlockNumber":18},{"Name":"PRINTQUALITY","Value":"1200","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":14,"MemoBlockNumber":18},{"Name":"YRESOLUTION","Value":"1200","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":15,"MemoBlockNumber":18},{"Name":"TTOPTION","Value":"2","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":16,"MemoBlockNumber":18},{"Name":"COLOR","Value":"2","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":17,"MemoBlockNumber":18},{"Name":"ASCII","Value":"10","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":18,"MemoBlockNumber":18},{"Name":"COLLATE","Value":"0","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":19,"MemoBlockNumber":18},{"Name":"COPIES","Value":"2","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":20,"MemoBlockNumber":18}],"Sections":[{"Id":"detail_1","Title":"Detail","BandKind":"detail","RecordIndex":42,"Top":2000,"Height":5000,"Objects":[]}],"DeletedSections":[],"UnplacedObjects":[]}}}
""";
    }

    private static string BuildDeletedSettingsMissingDriverHostResponseJson()
    {
        return """
{"Status":"ok","Document":{"AssetFamily":"report","FieldCount":5,"Objects":[],"ReportLayout":{"DocumentTitle":"Deleted Customer Invoice","DeletedPreviewBoundsAvailable":true,"DeletedPreviewBoundsLeft":1000,"DeletedPreviewBoundsTop":2600,"DeletedPreviewBoundsRight":2200,"DeletedPreviewBoundsBottom":2900,"DeletedPreviewBoundsWidth":1200,"DeletedPreviewBoundsHeight":300,"DeletedSettings":[{"Name":"ORIENTATION","Value":"1","RecordIndex":0,"FieldIndex":2,"SourceLineIndex":0,"MemoBlockNumber":19},{"Name":"COLS","Value":"3","RecordIndex":0,"FieldIndex":2,"SourceLineIndex":1,"MemoBlockNumber":19},{"Name":"COLWIDTH","Value":"2400","RecordIndex":0,"FieldIndex":2,"SourceLineIndex":2,"MemoBlockNumber":19},{"Name":"COLSPACING","Value":"180","RecordIndex":0,"FieldIndex":2,"SourceLineIndex":3,"MemoBlockNumber":19},{"Name":"PAPERLENGTH","Value":"4318","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":4,"MemoBlockNumber":18},{"Name":"PAPERWIDTH","Value":"2794","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":5,"MemoBlockNumber":18},{"Name":"TOPMARGIN","Value":"40","RecordIndex":0,"FieldIndex":3,"MemoBlockNumber":0},{"Name":"LEFTMARGIN","Value":"35","RecordIndex":0,"FieldIndex":10,"MemoBlockNumber":0},{"Name":"RIGHTMARGIN","Value":"45","RecordIndex":0,"FieldIndex":11,"MemoBlockNumber":0},{"Name":"TAG","Value":"deleted.customer.country","RecordIndex":0,"FieldIndex":9,"MemoBlockNumber":21},{"Name":"DRIVER","Value":"deleted.cups","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":10,"MemoBlockNumber":18},{"Name":"DEVICE","Value":"Deleted Printer","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":11,"MemoBlockNumber":18},{"Name":"OUTPUT","Value":"DPRN:","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":12,"MemoBlockNumber":18},{"Name":"DEFAULTSOURCE","Value":"16","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":13,"MemoBlockNumber":18},{"Name":"PRINTQUALITY","Value":"1200","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":14,"MemoBlockNumber":18},{"Name":"YRESOLUTION","Value":"1200","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":15,"MemoBlockNumber":18},{"Name":"TTOPTION","Value":"2","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":16,"MemoBlockNumber":18},{"Name":"COLOR","Value":"2","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":17,"MemoBlockNumber":18},{"Name":"ASCII","Value":"10","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":18,"MemoBlockNumber":18},{"Name":"COLLATE","Value":"0","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":19,"MemoBlockNumber":18},{"Name":"COPIES","Value":"2","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":20,"MemoBlockNumber":18}],"Sections":[{"Id":"detail_1","Title":"Detail","BandKind":"detail","RecordIndex":42,"Top":2000,"Height":5000,"Objects":[]}],"DeletedSections":[],"UnplacedObjects":[]}}}
""";
    }

    private static string BuildDeletedSettingsMissingDeviceHostResponseJson()
    {
        return """
{"Status":"ok","Document":{"AssetFamily":"report","FieldCount":5,"Objects":[],"ReportLayout":{"DocumentTitle":"Deleted Customer Invoice","DeletedPreviewBoundsAvailable":true,"DeletedPreviewBoundsLeft":1000,"DeletedPreviewBoundsTop":2600,"DeletedPreviewBoundsRight":2200,"DeletedPreviewBoundsBottom":2900,"DeletedPreviewBoundsWidth":1200,"DeletedPreviewBoundsHeight":300,"DeletedSettings":[{"Name":"ORIENTATION","Value":"1","RecordIndex":0,"FieldIndex":2,"SourceLineIndex":0,"MemoBlockNumber":19},{"Name":"COLS","Value":"3","RecordIndex":0,"FieldIndex":2,"SourceLineIndex":1,"MemoBlockNumber":19},{"Name":"COLWIDTH","Value":"2400","RecordIndex":0,"FieldIndex":2,"SourceLineIndex":2,"MemoBlockNumber":19},{"Name":"COLSPACING","Value":"180","RecordIndex":0,"FieldIndex":2,"SourceLineIndex":3,"MemoBlockNumber":19},{"Name":"PAPERLENGTH","Value":"4318","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":4,"MemoBlockNumber":18},{"Name":"PAPERWIDTH","Value":"2794","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":5,"MemoBlockNumber":18},{"Name":"TOPMARGIN","Value":"40","RecordIndex":0,"FieldIndex":3,"MemoBlockNumber":0},{"Name":"LEFTMARGIN","Value":"35","RecordIndex":0,"FieldIndex":10,"MemoBlockNumber":0},{"Name":"RIGHTMARGIN","Value":"45","RecordIndex":0,"FieldIndex":11,"MemoBlockNumber":0},{"Name":"TAG","Value":"deleted.customer.country","RecordIndex":0,"FieldIndex":9,"MemoBlockNumber":21},{"Name":"DRIVER","Value":"deleted.winspool","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":10,"MemoBlockNumber":18},{"Name":"DEVICE","Value":"Deleted Queue B","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":11,"MemoBlockNumber":18},{"Name":"OUTPUT","Value":"DPRN:","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":12,"MemoBlockNumber":18},{"Name":"DEFAULTSOURCE","Value":"16","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":13,"MemoBlockNumber":18},{"Name":"PRINTQUALITY","Value":"1200","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":14,"MemoBlockNumber":18},{"Name":"YRESOLUTION","Value":"1200","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":15,"MemoBlockNumber":18},{"Name":"TTOPTION","Value":"2","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":16,"MemoBlockNumber":18},{"Name":"COLOR","Value":"2","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":17,"MemoBlockNumber":18},{"Name":"ASCII","Value":"10","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":18,"MemoBlockNumber":18},{"Name":"COLLATE","Value":"0","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":19,"MemoBlockNumber":18},{"Name":"COPIES","Value":"2","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":20,"MemoBlockNumber":18}],"Sections":[{"Id":"detail_1","Title":"Detail","BandKind":"detail","RecordIndex":42,"Top":2000,"Height":5000,"Objects":[]}],"DeletedSections":[],"UnplacedObjects":[]}}}
""";
    }

    private static string BuildDeletedSettingsMissingOutputHostResponseJson()
    {
        return """
{"Status":"ok","Document":{"AssetFamily":"report","FieldCount":5,"Objects":[],"ReportLayout":{"DocumentTitle":"Deleted Customer Invoice","DeletedPreviewBoundsAvailable":true,"DeletedPreviewBoundsLeft":1000,"DeletedPreviewBoundsTop":2600,"DeletedPreviewBoundsRight":2200,"DeletedPreviewBoundsBottom":2900,"DeletedPreviewBoundsWidth":1200,"DeletedPreviewBoundsHeight":300,"DeletedSettings":[{"Name":"ORIENTATION","Value":"1","RecordIndex":0,"FieldIndex":2,"SourceLineIndex":0,"MemoBlockNumber":19},{"Name":"COLS","Value":"3","RecordIndex":0,"FieldIndex":2,"SourceLineIndex":1,"MemoBlockNumber":19},{"Name":"COLWIDTH","Value":"2400","RecordIndex":0,"FieldIndex":2,"SourceLineIndex":2,"MemoBlockNumber":19},{"Name":"COLSPACING","Value":"180","RecordIndex":0,"FieldIndex":2,"SourceLineIndex":3,"MemoBlockNumber":19},{"Name":"PAPERLENGTH","Value":"4318","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":4,"MemoBlockNumber":18},{"Name":"PAPERWIDTH","Value":"2794","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":5,"MemoBlockNumber":18},{"Name":"TOPMARGIN","Value":"40","RecordIndex":0,"FieldIndex":3,"MemoBlockNumber":0},{"Name":"LEFTMARGIN","Value":"35","RecordIndex":0,"FieldIndex":10,"MemoBlockNumber":0},{"Name":"RIGHTMARGIN","Value":"45","RecordIndex":0,"FieldIndex":11,"MemoBlockNumber":0},{"Name":"TAG","Value":"deleted.customer.country","RecordIndex":0,"FieldIndex":9,"MemoBlockNumber":21},{"Name":"DRIVER","Value":"deleted.winspool","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":10,"MemoBlockNumber":18},{"Name":"DEVICE","Value":"Deleted Printer","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":11,"MemoBlockNumber":18},{"Name":"OUTPUT","Value":"DeletedOutput.prn","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":12,"MemoBlockNumber":18},{"Name":"DEFAULTSOURCE","Value":"16","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":13,"MemoBlockNumber":18},{"Name":"PRINTQUALITY","Value":"1200","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":14,"MemoBlockNumber":18},{"Name":"YRESOLUTION","Value":"1200","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":15,"MemoBlockNumber":18},{"Name":"TTOPTION","Value":"2","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":16,"MemoBlockNumber":18},{"Name":"COLOR","Value":"2","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":17,"MemoBlockNumber":18},{"Name":"ASCII","Value":"10","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":18,"MemoBlockNumber":18},{"Name":"COLLATE","Value":"0","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":19,"MemoBlockNumber":18},{"Name":"COPIES","Value":"2","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":20,"MemoBlockNumber":18}],"Sections":[{"Id":"detail_1","Title":"Detail","BandKind":"detail","RecordIndex":42,"Top":2000,"Height":5000,"Objects":[]}],"DeletedSections":[],"UnplacedObjects":[]}}}
""";
    }

    private static string BuildDeletedSettingsMissingPrintQualityHostResponseJson()
    {
        return """
{"Status":"ok","Document":{"AssetFamily":"report","FieldCount":5,"Objects":[],"ReportLayout":{"DocumentTitle":"Deleted Customer Invoice","DeletedPreviewBoundsAvailable":true,"DeletedPreviewBoundsLeft":1000,"DeletedPreviewBoundsTop":2600,"DeletedPreviewBoundsRight":2200,"DeletedPreviewBoundsBottom":2900,"DeletedPreviewBoundsWidth":1200,"DeletedPreviewBoundsHeight":300,"DeletedSettings":[{"Name":"ORIENTATION","Value":"1","RecordIndex":0,"FieldIndex":2,"SourceLineIndex":0,"MemoBlockNumber":19},{"Name":"COLS","Value":"3","RecordIndex":0,"FieldIndex":2,"SourceLineIndex":1,"MemoBlockNumber":19},{"Name":"COLWIDTH","Value":"2400","RecordIndex":0,"FieldIndex":2,"SourceLineIndex":2,"MemoBlockNumber":19},{"Name":"COLSPACING","Value":"180","RecordIndex":0,"FieldIndex":2,"SourceLineIndex":3,"MemoBlockNumber":19},{"Name":"PAPERLENGTH","Value":"4318","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":4,"MemoBlockNumber":18},{"Name":"PAPERWIDTH","Value":"2794","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":5,"MemoBlockNumber":18},{"Name":"TOPMARGIN","Value":"40","RecordIndex":0,"FieldIndex":3,"MemoBlockNumber":0},{"Name":"LEFTMARGIN","Value":"35","RecordIndex":0,"FieldIndex":10,"MemoBlockNumber":0},{"Name":"RIGHTMARGIN","Value":"45","RecordIndex":0,"FieldIndex":11,"MemoBlockNumber":0},{"Name":"TAG","Value":"deleted.customer.country","RecordIndex":0,"FieldIndex":9,"MemoBlockNumber":21},{"Name":"DRIVER","Value":"deleted.winspool","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":10,"MemoBlockNumber":18},{"Name":"DEVICE","Value":"Deleted Printer","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":11,"MemoBlockNumber":18},{"Name":"OUTPUT","Value":"DPRN:","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":12,"MemoBlockNumber":18},{"Name":"DEFAULTSOURCE","Value":"16","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":13,"MemoBlockNumber":18},{"Name":"PRINTQUALITY","Value":"600","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":14,"MemoBlockNumber":18},{"Name":"YRESOLUTION","Value":"1200","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":15,"MemoBlockNumber":18},{"Name":"TTOPTION","Value":"2","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":16,"MemoBlockNumber":18},{"Name":"COLOR","Value":"2","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":17,"MemoBlockNumber":18},{"Name":"ASCII","Value":"10","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":18,"MemoBlockNumber":18},{"Name":"COLLATE","Value":"0","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":19,"MemoBlockNumber":18},{"Name":"COPIES","Value":"2","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":20,"MemoBlockNumber":18}],"Sections":[{"Id":"detail_1","Title":"Detail","BandKind":"detail","RecordIndex":42,"Top":2000,"Height":5000,"Objects":[]}],"DeletedSections":[],"UnplacedObjects":[]}}}
""";
    }

    private static string BuildDeletedSettingsMissingYResolutionHostResponseJson()
    {
        return """
{"Status":"ok","Document":{"AssetFamily":"report","FieldCount":5,"Objects":[],"ReportLayout":{"DocumentTitle":"Deleted Customer Invoice","DeletedPreviewBoundsAvailable":true,"DeletedPreviewBoundsLeft":1000,"DeletedPreviewBoundsTop":2600,"DeletedPreviewBoundsRight":2200,"DeletedPreviewBoundsBottom":2900,"DeletedPreviewBoundsWidth":1200,"DeletedPreviewBoundsHeight":300,"DeletedSettings":[{"Name":"ORIENTATION","Value":"1","RecordIndex":0,"FieldIndex":2,"SourceLineIndex":0,"MemoBlockNumber":19},{"Name":"COLS","Value":"3","RecordIndex":0,"FieldIndex":2,"SourceLineIndex":1,"MemoBlockNumber":19},{"Name":"COLWIDTH","Value":"2400","RecordIndex":0,"FieldIndex":2,"SourceLineIndex":2,"MemoBlockNumber":19},{"Name":"COLSPACING","Value":"180","RecordIndex":0,"FieldIndex":2,"SourceLineIndex":3,"MemoBlockNumber":19},{"Name":"PAPERLENGTH","Value":"4318","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":4,"MemoBlockNumber":18},{"Name":"PAPERWIDTH","Value":"2794","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":5,"MemoBlockNumber":18},{"Name":"TOPMARGIN","Value":"40","RecordIndex":0,"FieldIndex":3,"MemoBlockNumber":0},{"Name":"LEFTMARGIN","Value":"35","RecordIndex":0,"FieldIndex":10,"MemoBlockNumber":0},{"Name":"RIGHTMARGIN","Value":"45","RecordIndex":0,"FieldIndex":11,"MemoBlockNumber":0},{"Name":"TAG","Value":"deleted.customer.country","RecordIndex":0,"FieldIndex":9,"MemoBlockNumber":21},{"Name":"DRIVER","Value":"deleted.winspool","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":10,"MemoBlockNumber":18},{"Name":"DEVICE","Value":"Deleted Printer","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":11,"MemoBlockNumber":18},{"Name":"OUTPUT","Value":"DPRN:","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":12,"MemoBlockNumber":18},{"Name":"DEFAULTSOURCE","Value":"16","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":13,"MemoBlockNumber":18},{"Name":"PRINTQUALITY","Value":"1200","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":14,"MemoBlockNumber":18},{"Name":"YRESOLUTION","Value":"600","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":15,"MemoBlockNumber":18},{"Name":"TTOPTION","Value":"2","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":16,"MemoBlockNumber":18},{"Name":"COLOR","Value":"2","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":17,"MemoBlockNumber":18},{"Name":"ASCII","Value":"10","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":18,"MemoBlockNumber":18},{"Name":"COLLATE","Value":"0","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":19,"MemoBlockNumber":18},{"Name":"COPIES","Value":"2","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":20,"MemoBlockNumber":18}],"Sections":[{"Id":"detail_1","Title":"Detail","BandKind":"detail","RecordIndex":42,"Top":2000,"Height":5000,"Objects":[]}],"DeletedSections":[],"UnplacedObjects":[]}}}
""";
    }

    private static string BuildDeletedSettingsMissingTTOptionHostResponseJson()
    {
        return """
{"Status":"ok","Document":{"AssetFamily":"report","FieldCount":5,"Objects":[],"ReportLayout":{"DocumentTitle":"Deleted Customer Invoice","DeletedPreviewBoundsAvailable":true,"DeletedPreviewBoundsLeft":1000,"DeletedPreviewBoundsTop":2600,"DeletedPreviewBoundsRight":2200,"DeletedPreviewBoundsBottom":2900,"DeletedPreviewBoundsWidth":1200,"DeletedPreviewBoundsHeight":300,"DeletedSettings":[{"Name":"ORIENTATION","Value":"1","RecordIndex":0,"FieldIndex":2,"SourceLineIndex":0,"MemoBlockNumber":19},{"Name":"COLS","Value":"3","RecordIndex":0,"FieldIndex":2,"SourceLineIndex":1,"MemoBlockNumber":19},{"Name":"COLWIDTH","Value":"2400","RecordIndex":0,"FieldIndex":2,"SourceLineIndex":2,"MemoBlockNumber":19},{"Name":"COLSPACING","Value":"180","RecordIndex":0,"FieldIndex":2,"SourceLineIndex":3,"MemoBlockNumber":19},{"Name":"PAPERLENGTH","Value":"4318","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":4,"MemoBlockNumber":18},{"Name":"PAPERWIDTH","Value":"2794","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":5,"MemoBlockNumber":18},{"Name":"TOPMARGIN","Value":"40","RecordIndex":0,"FieldIndex":3,"MemoBlockNumber":0},{"Name":"LEFTMARGIN","Value":"35","RecordIndex":0,"FieldIndex":10,"MemoBlockNumber":0},{"Name":"RIGHTMARGIN","Value":"45","RecordIndex":0,"FieldIndex":11,"MemoBlockNumber":0},{"Name":"TAG","Value":"deleted.customer.country","RecordIndex":0,"FieldIndex":9,"MemoBlockNumber":21},{"Name":"DRIVER","Value":"deleted.winspool","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":10,"MemoBlockNumber":18},{"Name":"DEVICE","Value":"Deleted Printer","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":11,"MemoBlockNumber":18},{"Name":"OUTPUT","Value":"DPRN:","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":12,"MemoBlockNumber":18},{"Name":"DEFAULTSOURCE","Value":"16","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":13,"MemoBlockNumber":18},{"Name":"PRINTQUALITY","Value":"1200","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":14,"MemoBlockNumber":18},{"Name":"YRESOLUTION","Value":"1200","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":15,"MemoBlockNumber":18},{"Name":"TTOPTION","Value":"1","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":16,"MemoBlockNumber":18},{"Name":"COLOR","Value":"2","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":17,"MemoBlockNumber":18},{"Name":"ASCII","Value":"10","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":18,"MemoBlockNumber":18},{"Name":"COLLATE","Value":"0","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":19,"MemoBlockNumber":18},{"Name":"COPIES","Value":"2","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":20,"MemoBlockNumber":18}],"Sections":[{"Id":"detail_1","Title":"Detail","BandKind":"detail","RecordIndex":42,"Top":2000,"Height":5000,"Objects":[]}],"DeletedSections":[],"UnplacedObjects":[]}}}
""";
    }

    private static string BuildDeletedSettingsMissingAsciiHostResponseJson()
    {
        return """
{"Status":"ok","Document":{"AssetFamily":"report","FieldCount":5,"Objects":[],"ReportLayout":{"DocumentTitle":"Deleted Customer Invoice","DeletedPreviewBoundsAvailable":true,"DeletedPreviewBoundsLeft":1000,"DeletedPreviewBoundsTop":2600,"DeletedPreviewBoundsRight":2200,"DeletedPreviewBoundsBottom":2900,"DeletedPreviewBoundsWidth":1200,"DeletedPreviewBoundsHeight":300,"DeletedSettings":[{"Name":"ORIENTATION","Value":"1","RecordIndex":0,"FieldIndex":2,"SourceLineIndex":0,"MemoBlockNumber":19},{"Name":"COLS","Value":"3","RecordIndex":0,"FieldIndex":2,"SourceLineIndex":1,"MemoBlockNumber":19},{"Name":"COLWIDTH","Value":"2400","RecordIndex":0,"FieldIndex":2,"SourceLineIndex":2,"MemoBlockNumber":19},{"Name":"COLSPACING","Value":"180","RecordIndex":0,"FieldIndex":2,"SourceLineIndex":3,"MemoBlockNumber":19},{"Name":"PAPERLENGTH","Value":"4318","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":4,"MemoBlockNumber":18},{"Name":"PAPERWIDTH","Value":"2794","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":5,"MemoBlockNumber":18},{"Name":"TOPMARGIN","Value":"40","RecordIndex":0,"FieldIndex":3,"MemoBlockNumber":0},{"Name":"LEFTMARGIN","Value":"35","RecordIndex":0,"FieldIndex":10,"MemoBlockNumber":0},{"Name":"RIGHTMARGIN","Value":"45","RecordIndex":0,"FieldIndex":11,"MemoBlockNumber":0},{"Name":"TAG","Value":"deleted.customer.country","RecordIndex":0,"FieldIndex":9,"MemoBlockNumber":21},{"Name":"DRIVER","Value":"deleted.winspool","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":10,"MemoBlockNumber":18},{"Name":"DEVICE","Value":"Deleted Printer","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":11,"MemoBlockNumber":18},{"Name":"OUTPUT","Value":"DPRN:","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":12,"MemoBlockNumber":18},{"Name":"DEFAULTSOURCE","Value":"16","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":13,"MemoBlockNumber":18},{"Name":"PRINTQUALITY","Value":"1200","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":14,"MemoBlockNumber":18},{"Name":"YRESOLUTION","Value":"1200","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":15,"MemoBlockNumber":18},{"Name":"TTOPTION","Value":"2","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":16,"MemoBlockNumber":18},{"Name":"COLOR","Value":"2","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":17,"MemoBlockNumber":18},{"Name":"ASCII","Value":"8","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":18,"MemoBlockNumber":18},{"Name":"COLLATE","Value":"0","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":19,"MemoBlockNumber":18},{"Name":"COPIES","Value":"2","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":20,"MemoBlockNumber":18}],"Sections":[{"Id":"detail_1","Title":"Detail","BandKind":"detail","RecordIndex":42,"Top":2000,"Height":5000,"Objects":[]}],"DeletedSections":[],"UnplacedObjects":[]}}}
""";
    }

    private static string BuildDeletedSettingsMissingCollateHostResponseJson()
    {
        return """
{"Status":"ok","Document":{"AssetFamily":"report","FieldCount":5,"Objects":[],"ReportLayout":{"DocumentTitle":"Deleted Customer Invoice","DeletedPreviewBoundsAvailable":true,"DeletedPreviewBoundsLeft":1000,"DeletedPreviewBoundsTop":2600,"DeletedPreviewBoundsRight":2200,"DeletedPreviewBoundsBottom":2900,"DeletedPreviewBoundsWidth":1200,"DeletedPreviewBoundsHeight":300,"DeletedSettings":[{"Name":"ORIENTATION","Value":"1","RecordIndex":0,"FieldIndex":2,"SourceLineIndex":0,"MemoBlockNumber":19},{"Name":"COLS","Value":"3","RecordIndex":0,"FieldIndex":2,"SourceLineIndex":1,"MemoBlockNumber":19},{"Name":"COLWIDTH","Value":"2400","RecordIndex":0,"FieldIndex":2,"SourceLineIndex":2,"MemoBlockNumber":19},{"Name":"COLSPACING","Value":"180","RecordIndex":0,"FieldIndex":2,"SourceLineIndex":3,"MemoBlockNumber":19},{"Name":"PAPERLENGTH","Value":"4318","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":4,"MemoBlockNumber":18},{"Name":"PAPERWIDTH","Value":"2794","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":5,"MemoBlockNumber":18},{"Name":"TOPMARGIN","Value":"40","RecordIndex":0,"FieldIndex":3,"MemoBlockNumber":0},{"Name":"LEFTMARGIN","Value":"35","RecordIndex":0,"FieldIndex":10,"MemoBlockNumber":0},{"Name":"RIGHTMARGIN","Value":"45","RecordIndex":0,"FieldIndex":11,"MemoBlockNumber":0},{"Name":"TAG","Value":"deleted.customer.country","RecordIndex":0,"FieldIndex":9,"MemoBlockNumber":21},{"Name":"DRIVER","Value":"deleted.winspool","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":10,"MemoBlockNumber":18},{"Name":"DEVICE","Value":"Deleted Printer","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":11,"MemoBlockNumber":18},{"Name":"OUTPUT","Value":"DPRN:","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":12,"MemoBlockNumber":18},{"Name":"DEFAULTSOURCE","Value":"16","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":13,"MemoBlockNumber":18},{"Name":"PRINTQUALITY","Value":"1200","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":14,"MemoBlockNumber":18},{"Name":"YRESOLUTION","Value":"1200","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":15,"MemoBlockNumber":18},{"Name":"TTOPTION","Value":"2","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":16,"MemoBlockNumber":18},{"Name":"COLOR","Value":"2","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":17,"MemoBlockNumber":18},{"Name":"ASCII","Value":"10","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":18,"MemoBlockNumber":18},{"Name":"COLLATE","Value":"1","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":19,"MemoBlockNumber":18},{"Name":"COPIES","Value":"2","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":20,"MemoBlockNumber":18}],"Sections":[{"Id":"detail_1","Title":"Detail","BandKind":"detail","RecordIndex":42,"Top":2000,"Height":5000,"Objects":[]}],"DeletedSections":[],"UnplacedObjects":[]}}}
""";
    }

    private static string BuildDeletedSettingsMissingCopiesHostResponseJson()
    {
        return """
{"Status":"ok","Document":{"AssetFamily":"report","FieldCount":5,"Objects":[],"ReportLayout":{"DocumentTitle":"Deleted Customer Invoice","DeletedPreviewBoundsAvailable":true,"DeletedPreviewBoundsLeft":1000,"DeletedPreviewBoundsTop":2600,"DeletedPreviewBoundsRight":2200,"DeletedPreviewBoundsBottom":2900,"DeletedPreviewBoundsWidth":1200,"DeletedPreviewBoundsHeight":300,"DeletedSettings":[{"Name":"ORIENTATION","Value":"1","RecordIndex":0,"FieldIndex":2,"SourceLineIndex":0,"MemoBlockNumber":19},{"Name":"COLS","Value":"3","RecordIndex":0,"FieldIndex":2,"SourceLineIndex":1,"MemoBlockNumber":19},{"Name":"COLWIDTH","Value":"2400","RecordIndex":0,"FieldIndex":2,"SourceLineIndex":2,"MemoBlockNumber":19},{"Name":"COLSPACING","Value":"180","RecordIndex":0,"FieldIndex":2,"SourceLineIndex":3,"MemoBlockNumber":19},{"Name":"PAPERLENGTH","Value":"4318","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":4,"MemoBlockNumber":18},{"Name":"PAPERWIDTH","Value":"2794","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":5,"MemoBlockNumber":18},{"Name":"TOPMARGIN","Value":"40","RecordIndex":0,"FieldIndex":3,"MemoBlockNumber":0},{"Name":"LEFTMARGIN","Value":"35","RecordIndex":0,"FieldIndex":10,"MemoBlockNumber":0},{"Name":"RIGHTMARGIN","Value":"45","RecordIndex":0,"FieldIndex":11,"MemoBlockNumber":0},{"Name":"TAG","Value":"deleted.customer.country","RecordIndex":0,"FieldIndex":9,"MemoBlockNumber":21},{"Name":"DRIVER","Value":"deleted.winspool","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":10,"MemoBlockNumber":18},{"Name":"DEVICE","Value":"Deleted Printer","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":11,"MemoBlockNumber":18},{"Name":"OUTPUT","Value":"DPRN:","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":12,"MemoBlockNumber":18},{"Name":"DEFAULTSOURCE","Value":"16","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":13,"MemoBlockNumber":18},{"Name":"PRINTQUALITY","Value":"1200","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":14,"MemoBlockNumber":18},{"Name":"YRESOLUTION","Value":"1200","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":15,"MemoBlockNumber":18},{"Name":"TTOPTION","Value":"2","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":16,"MemoBlockNumber":18},{"Name":"COLOR","Value":"2","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":17,"MemoBlockNumber":18},{"Name":"ASCII","Value":"10","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":18,"MemoBlockNumber":18},{"Name":"COLLATE","Value":"0","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":19,"MemoBlockNumber":18},{"Name":"COPIES","Value":"3","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":20,"MemoBlockNumber":18}],"Sections":[{"Id":"detail_1","Title":"Detail","BandKind":"detail","RecordIndex":42,"Top":2000,"Height":5000,"Objects":[]}],"DeletedSections":[],"UnplacedObjects":[]}}}
""";
    }

    private static string BuildDeletedSettingsMissingColorHostResponseJson()
    {
        return """
{"Status":"ok","Document":{"AssetFamily":"report","FieldCount":5,"Objects":[],"ReportLayout":{"DocumentTitle":"Deleted Customer Invoice","DeletedPreviewBoundsAvailable":true,"DeletedPreviewBoundsLeft":1000,"DeletedPreviewBoundsTop":2600,"DeletedPreviewBoundsRight":2200,"DeletedPreviewBoundsBottom":2900,"DeletedPreviewBoundsWidth":1200,"DeletedPreviewBoundsHeight":300,"DeletedSettings":[{"Name":"ORIENTATION","Value":"1","RecordIndex":0,"FieldIndex":2,"SourceLineIndex":0,"MemoBlockNumber":19},{"Name":"COLS","Value":"3","RecordIndex":0,"FieldIndex":2,"SourceLineIndex":1,"MemoBlockNumber":19},{"Name":"COLWIDTH","Value":"2400","RecordIndex":0,"FieldIndex":2,"SourceLineIndex":2,"MemoBlockNumber":19},{"Name":"COLSPACING","Value":"180","RecordIndex":0,"FieldIndex":2,"SourceLineIndex":3,"MemoBlockNumber":19},{"Name":"PAPERLENGTH","Value":"4318","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":4,"MemoBlockNumber":18},{"Name":"PAPERWIDTH","Value":"2794","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":5,"MemoBlockNumber":18},{"Name":"TOPMARGIN","Value":"40","RecordIndex":0,"FieldIndex":3,"MemoBlockNumber":0},{"Name":"LEFTMARGIN","Value":"35","RecordIndex":0,"FieldIndex":10,"MemoBlockNumber":0},{"Name":"RIGHTMARGIN","Value":"45","RecordIndex":0,"FieldIndex":11,"MemoBlockNumber":0},{"Name":"TAG","Value":"deleted.customer.country","RecordIndex":0,"FieldIndex":9,"MemoBlockNumber":21},{"Name":"DRIVER","Value":"deleted.winspool","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":10,"MemoBlockNumber":18},{"Name":"DEVICE","Value":"Deleted Printer","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":11,"MemoBlockNumber":18},{"Name":"OUTPUT","Value":"DPRN:","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":12,"MemoBlockNumber":18},{"Name":"DEFAULTSOURCE","Value":"16","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":13,"MemoBlockNumber":18},{"Name":"PRINTQUALITY","Value":"1200","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":14,"MemoBlockNumber":18},{"Name":"YRESOLUTION","Value":"1200","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":15,"MemoBlockNumber":18},{"Name":"TTOPTION","Value":"2","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":16,"MemoBlockNumber":18},{"Name":"COLOR","Value":"1","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":17,"MemoBlockNumber":18},{"Name":"ASCII","Value":"10","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":18,"MemoBlockNumber":18},{"Name":"COLLATE","Value":"0","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":19,"MemoBlockNumber":18},{"Name":"COPIES","Value":"2","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":20,"MemoBlockNumber":18}],"Sections":[{"Id":"detail_1","Title":"Detail","BandKind":"detail","RecordIndex":42,"Top":2000,"Height":5000,"Objects":[]}],"DeletedSections":[],"UnplacedObjects":[]}}}
""";
    }

    private static string BuildDeletedSettingsMissingColSpacingHostResponseJson()
    {
        return """
{"Status":"ok","Document":{"AssetFamily":"report","FieldCount":5,"Objects":[],"ReportLayout":{"DocumentTitle":"Deleted Customer Invoice","DeletedPreviewBoundsAvailable":true,"DeletedPreviewBoundsLeft":1000,"DeletedPreviewBoundsTop":2600,"DeletedPreviewBoundsRight":2200,"DeletedPreviewBoundsBottom":2900,"DeletedPreviewBoundsWidth":1200,"DeletedPreviewBoundsHeight":300,"DeletedSettings":[{"Name":"ORIENTATION","Value":"1","RecordIndex":0,"FieldIndex":2,"SourceLineIndex":0,"MemoBlockNumber":19},{"Name":"COLS","Value":"3","RecordIndex":0,"FieldIndex":2,"SourceLineIndex":1,"MemoBlockNumber":19},{"Name":"COLWIDTH","Value":"2400","RecordIndex":0,"FieldIndex":2,"SourceLineIndex":2,"MemoBlockNumber":19},{"Name":"COLSPACING","Value":"240","RecordIndex":0,"FieldIndex":2,"SourceLineIndex":3,"MemoBlockNumber":19},{"Name":"PAPERLENGTH","Value":"4318","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":4,"MemoBlockNumber":18},{"Name":"PAPERWIDTH","Value":"2794","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":5,"MemoBlockNumber":18},{"Name":"TOPMARGIN","Value":"40","RecordIndex":0,"FieldIndex":3,"MemoBlockNumber":0},{"Name":"LEFTMARGIN","Value":"35","RecordIndex":0,"FieldIndex":10,"MemoBlockNumber":0},{"Name":"RIGHTMARGIN","Value":"45","RecordIndex":0,"FieldIndex":11,"MemoBlockNumber":0},{"Name":"TAG","Value":"deleted.customer.country","RecordIndex":0,"FieldIndex":9,"MemoBlockNumber":21},{"Name":"DRIVER","Value":"deleted.winspool","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":10,"MemoBlockNumber":18},{"Name":"DEVICE","Value":"Deleted Printer","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":11,"MemoBlockNumber":18},{"Name":"OUTPUT","Value":"DPRN:","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":12,"MemoBlockNumber":18},{"Name":"DEFAULTSOURCE","Value":"16","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":13,"MemoBlockNumber":18},{"Name":"PRINTQUALITY","Value":"1200","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":14,"MemoBlockNumber":18},{"Name":"YRESOLUTION","Value":"1200","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":15,"MemoBlockNumber":18},{"Name":"TTOPTION","Value":"2","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":16,"MemoBlockNumber":18},{"Name":"COLOR","Value":"2","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":17,"MemoBlockNumber":18},{"Name":"ASCII","Value":"10","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":18,"MemoBlockNumber":18},{"Name":"COLLATE","Value":"0","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":19,"MemoBlockNumber":18},{"Name":"COPIES","Value":"2","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":20,"MemoBlockNumber":18}],"Sections":[{"Id":"detail_1","Title":"Detail","BandKind":"detail","RecordIndex":42,"Top":2000,"Height":5000,"Objects":[]}],"DeletedSections":[],"UnplacedObjects":[]}}}
""";
    }

    private static string BuildDeletedSettingsMissingColWidthHostResponseJson()
    {
        return """
{"Status":"ok","Document":{"AssetFamily":"report","FieldCount":5,"Objects":[],"ReportLayout":{"DocumentTitle":"Deleted Customer Invoice","DeletedPreviewBoundsAvailable":true,"DeletedPreviewBoundsLeft":1000,"DeletedPreviewBoundsTop":2600,"DeletedPreviewBoundsRight":2200,"DeletedPreviewBoundsBottom":2900,"DeletedPreviewBoundsWidth":1200,"DeletedPreviewBoundsHeight":300,"DeletedSettings":[{"Name":"ORIENTATION","Value":"1","RecordIndex":0,"FieldIndex":2,"SourceLineIndex":0,"MemoBlockNumber":19},{"Name":"COLS","Value":"3","RecordIndex":0,"FieldIndex":2,"SourceLineIndex":1,"MemoBlockNumber":19},{"Name":"COLWIDTH","Value":"3000","RecordIndex":0,"FieldIndex":2,"SourceLineIndex":2,"MemoBlockNumber":19},{"Name":"COLSPACING","Value":"180","RecordIndex":0,"FieldIndex":2,"SourceLineIndex":3,"MemoBlockNumber":19},{"Name":"PAPERLENGTH","Value":"4318","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":4,"MemoBlockNumber":18},{"Name":"PAPERWIDTH","Value":"2794","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":5,"MemoBlockNumber":18},{"Name":"TOPMARGIN","Value":"40","RecordIndex":0,"FieldIndex":3,"MemoBlockNumber":0},{"Name":"LEFTMARGIN","Value":"35","RecordIndex":0,"FieldIndex":10,"MemoBlockNumber":0},{"Name":"RIGHTMARGIN","Value":"45","RecordIndex":0,"FieldIndex":11,"MemoBlockNumber":0},{"Name":"TAG","Value":"deleted.customer.country","RecordIndex":0,"FieldIndex":9,"MemoBlockNumber":21},{"Name":"DRIVER","Value":"deleted.winspool","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":10,"MemoBlockNumber":18},{"Name":"DEVICE","Value":"Deleted Printer","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":11,"MemoBlockNumber":18},{"Name":"OUTPUT","Value":"DPRN:","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":12,"MemoBlockNumber":18},{"Name":"DEFAULTSOURCE","Value":"16","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":13,"MemoBlockNumber":18},{"Name":"PRINTQUALITY","Value":"1200","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":14,"MemoBlockNumber":18},{"Name":"YRESOLUTION","Value":"1200","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":15,"MemoBlockNumber":18},{"Name":"TTOPTION","Value":"2","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":16,"MemoBlockNumber":18},{"Name":"COLOR","Value":"2","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":17,"MemoBlockNumber":18},{"Name":"ASCII","Value":"10","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":18,"MemoBlockNumber":18},{"Name":"COLLATE","Value":"0","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":19,"MemoBlockNumber":18},{"Name":"COPIES","Value":"2","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":20,"MemoBlockNumber":18}],"Sections":[{"Id":"detail_1","Title":"Detail","BandKind":"detail","RecordIndex":42,"Top":2000,"Height":5000,"Objects":[]}],"DeletedSections":[],"UnplacedObjects":[]}}}
""";
    }

    private static string BuildDeletedSettingsMissingColsHostResponseJson()
    {
        return """
{"Status":"ok","Document":{"AssetFamily":"report","FieldCount":5,"Objects":[],"ReportLayout":{"DocumentTitle":"Deleted Customer Invoice","DeletedPreviewBoundsAvailable":true,"DeletedPreviewBoundsLeft":1000,"DeletedPreviewBoundsTop":2600,"DeletedPreviewBoundsRight":2200,"DeletedPreviewBoundsBottom":2900,"DeletedPreviewBoundsWidth":1200,"DeletedPreviewBoundsHeight":300,"DeletedSettings":[{"Name":"ORIENTATION","Value":"1","RecordIndex":0,"FieldIndex":2,"SourceLineIndex":0,"MemoBlockNumber":19},{"Name":"COLS","Value":"4","RecordIndex":0,"FieldIndex":2,"SourceLineIndex":1,"MemoBlockNumber":19},{"Name":"COLWIDTH","Value":"2400","RecordIndex":0,"FieldIndex":2,"SourceLineIndex":2,"MemoBlockNumber":19},{"Name":"COLSPACING","Value":"180","RecordIndex":0,"FieldIndex":2,"SourceLineIndex":3,"MemoBlockNumber":19},{"Name":"PAPERLENGTH","Value":"4318","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":4,"MemoBlockNumber":18},{"Name":"PAPERWIDTH","Value":"2794","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":5,"MemoBlockNumber":18},{"Name":"TOPMARGIN","Value":"40","RecordIndex":0,"FieldIndex":3,"MemoBlockNumber":0},{"Name":"LEFTMARGIN","Value":"35","RecordIndex":0,"FieldIndex":10,"MemoBlockNumber":0},{"Name":"RIGHTMARGIN","Value":"45","RecordIndex":0,"FieldIndex":11,"MemoBlockNumber":0},{"Name":"TAG","Value":"deleted.customer.country","RecordIndex":0,"FieldIndex":9,"MemoBlockNumber":21},{"Name":"DRIVER","Value":"deleted.winspool","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":10,"MemoBlockNumber":18},{"Name":"DEVICE","Value":"Deleted Printer","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":11,"MemoBlockNumber":18},{"Name":"OUTPUT","Value":"DPRN:","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":12,"MemoBlockNumber":18},{"Name":"DEFAULTSOURCE","Value":"16","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":13,"MemoBlockNumber":18},{"Name":"PRINTQUALITY","Value":"1200","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":14,"MemoBlockNumber":18},{"Name":"YRESOLUTION","Value":"1200","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":15,"MemoBlockNumber":18},{"Name":"TTOPTION","Value":"2","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":16,"MemoBlockNumber":18},{"Name":"COLOR","Value":"2","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":17,"MemoBlockNumber":18},{"Name":"ASCII","Value":"10","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":18,"MemoBlockNumber":18},{"Name":"COLLATE","Value":"0","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":19,"MemoBlockNumber":18},{"Name":"COPIES","Value":"2","RecordIndex":0,"FieldIndex":6,"SourceLineIndex":20,"MemoBlockNumber":18}],"Sections":[{"Id":"detail_1","Title":"Detail","BandKind":"detail","RecordIndex":42,"Top":2000,"Height":5000,"Objects":[]}],"DeletedSections":[],"UnplacedObjects":[]}}}
""";
    }

    private static string BuildObjectUpdateHostResponseJson()
    {
        return """
{"Status":"ok","Document":{"AssetFamily":"report","FieldCount":5,"Objects":[{"RecordIndex":6,"Title":"customer.company","Subtitle":"field","Properties":[{"Name":"HPOS","Value":"1500"},{"Name":"VPOS","Value":"2600"},{"Name":"WIDTH","Value":"4000"},{"Name":"HEIGHT","Value":"500"},{"Name":"EXPR","Value":"customer.company"}]}],"ReportLayout":{"Sections":[{"Id":"detail_1","Title":"Detail","BandKind":"detail","RecordIndex":42,"Top":2000,"Height":5000,"Objects":[{"RecordIndex":6,"ObjectKind":"field","Title":"customer.company","Expression":"customer.company","Left":1500,"Top":2600,"Width":4000,"Height":500}]}],"DeletedSections":[],"UnplacedObjects":[]}}}
""";
    }

    private static string BuildUnplacedReportObjectUpdateHostResponseJson()
    {
        return """
{"Status":"ok","Document":{"AssetFamily":"report","FieldCount":5,"Objects":[{"RecordIndex":6,"Title":"customer.company","Subtitle":"field","Properties":[{"Name":"HPOS","Value":"1200"},{"Name":"VPOS","Value":"2600"},{"Name":"WIDTH","Value":"4000"},{"Name":"HEIGHT","Value":"500"},{"Name":"EXPR","Value":"customer.company"}]},{"RecordIndex":9,"Title":"orphan.note","Subtitle":"field","Properties":[{"Name":"HPOS","Value":"1100"},{"Name":"VPOS","Value":"700"},{"Name":"WIDTH","Value":"2400"},{"Name":"HEIGHT","Value":"450"},{"Name":"EXPR","Value":"orphan.note"}]}],"ReportLayout":{"Sections":[{"Id":"detail_1","Title":"Detail","BandKind":"detail","RecordIndex":42,"Top":2000,"Height":5000,"Objects":[{"RecordIndex":6,"ObjectKind":"field","Title":"customer.company","Expression":"customer.company","Left":1200,"Top":2600,"Width":4000,"Height":500}]}],"DeletedSections":[],"UnplacedObjects":[{"RecordIndex":9,"ObjectKind":"field","Title":"orphan.note","Expression":"orphan.note","Left":1100,"Top":700,"Width":2400,"Height":450}]}}}
""";
    }

    private static string BuildUnplacedReportObjectPreviewRefreshHostResponseJson()
    {
        return """
{"Status":"ok","Document":{"AssetFamily":"report","FieldCount":5,"Objects":[{"RecordIndex":6,"Title":"customer.company","Subtitle":"field","Properties":[{"Name":"HPOS","Value":"1200"},{"Name":"VPOS","Value":"2600"},{"Name":"WIDTH","Value":"4000"},{"Name":"HEIGHT","Value":"500"},{"Name":"EXPR","Value":"customer.company"}]},{"RecordIndex":9,"Title":"orphan.note","Subtitle":"field","Properties":[{"Name":"HPOS","Value":"1100"},{"Name":"VPOS","Value":"700"},{"Name":"WIDTH","Value":"2400"},{"Name":"HEIGHT","Value":"450"},{"Name":"EXPR","Value":"orphan.note"}]}],"ReportLayout":{"PreviewBoundsAvailable":true,"PreviewBoundsLeft":1100,"PreviewBoundsTop":700,"PreviewBoundsRight":5200,"PreviewBoundsBottom":3100,"PreviewBoundsWidth":4100,"PreviewBoundsHeight":2400,"Sections":[{"Id":"detail_1","Title":"Detail","BandKind":"detail","RecordIndex":42,"Top":2000,"Height":5000,"Objects":[{"RecordIndex":6,"ObjectKind":"field","Title":"customer.company","Expression":"customer.company","Left":1200,"Top":2600,"Width":4000,"Height":500}]}],"DeletedSections":[],"UnplacedObjects":[{"RecordIndex":9,"ObjectKind":"field","Title":"orphan.note","Expression":"orphan.note","Left":1100,"Top":700,"Width":2400,"Height":450}]}}}
""";
    }

    private static string BuildUnplacedReportObjectPlacementIntoSectionHostResponseJson()
    {
        return """
{"Status":"ok","Document":{"AssetFamily":"report","FieldCount":5,"Objects":[{"RecordIndex":6,"Title":"customer.company","Subtitle":"field","Properties":[{"Name":"HPOS","Value":"1200"},{"Name":"VPOS","Value":"2600"},{"Name":"WIDTH","Value":"4000"},{"Name":"HEIGHT","Value":"500"},{"Name":"EXPR","Value":"customer.company"}]},{"RecordIndex":9,"Title":"orphan.note","Subtitle":"field","Properties":[{"Name":"HPOS","Value":"1100"},{"Name":"VPOS","Value":"2400"},{"Name":"WIDTH","Value":"2400"},{"Name":"HEIGHT","Value":"450"},{"Name":"EXPR","Value":"orphan.note"}]}],"ReportLayout":{"PreviewBoundsAvailable":true,"PreviewBoundsLeft":1100,"PreviewBoundsTop":2400,"PreviewBoundsRight":5200,"PreviewBoundsBottom":3100,"PreviewBoundsWidth":4100,"PreviewBoundsHeight":700,"Sections":[{"Id":"detail_1","Title":"Detail","BandKind":"detail","RecordIndex":42,"Top":2000,"Height":5000,"Objects":[{"RecordIndex":6,"ObjectKind":"field","Title":"customer.company","Expression":"customer.company","Left":1200,"Top":2600,"Width":4000,"Height":500},{"RecordIndex":9,"ObjectKind":"field","Title":"orphan.note","Expression":"orphan.note","Left":1100,"Top":2400,"Width":2400,"Height":450}]}],"DeletedSections":[],"UnplacedObjects":[]}}}
""";
    }

    private static string BuildUndoPreviewRefreshHostResponseJson()
    {
        return """
{"Status":"ok","Document":{"AssetFamily":"report","FieldCount":5,"Objects":[{"RecordIndex":6,"Title":"customer.company","Subtitle":"field","Properties":[{"Name":"HPOS","Value":"1200"},{"Name":"VPOS","Value":"2600"},{"Name":"WIDTH","Value":"4000"},{"Name":"HEIGHT","Value":"500"},{"Name":"EXPR","Value":"customer.company"}]},{"RecordIndex":9,"Title":"orphan.note","Subtitle":"field","Properties":[{"Name":"HPOS","Value":"1100"},{"Name":"VPOS","Value":"700"},{"Name":"WIDTH","Value":"2400"},{"Name":"HEIGHT","Value":"450"},{"Name":"EXPR","Value":"orphan.note"}]}],"CommandUndoAvailable":false,"CommandUndoLabel":"","ReportLayout":{"PreviewBoundsAvailable":true,"PreviewBoundsLeft":1100,"PreviewBoundsTop":700,"PreviewBoundsRight":5200,"PreviewBoundsBottom":3100,"PreviewBoundsWidth":4100,"PreviewBoundsHeight":2400,"Sections":[{"Id":"detail_1","Title":"Detail","BandKind":"detail","RecordIndex":42,"Top":2000,"Height":5000,"Objects":[{"RecordIndex":6,"ObjectKind":"field","Title":"customer.company","Expression":"customer.company","Left":1200,"Top":2600,"Width":4000,"Height":500}]}],"DeletedSections":[],"UnplacedObjects":[{"RecordIndex":9,"ObjectKind":"field","Title":"orphan.note","Expression":"orphan.note","Left":1100,"Top":700,"Width":2400,"Height":450}]}}}
""";
    }

    private static string BuildDeletedReportUndoPreviewRefreshHostResponseJson()
    {
        return """
{"Status":"ok","Document":{"AssetFamily":"report","FieldCount":5,"Objects":[{"RecordIndex":6,"Title":"customer.company","Subtitle":"field","Properties":[{"Name":"HPOS","Value":"1200"},{"Name":"VPOS","Value":"2600"},{"Name":"WIDTH","Value":"4000"},{"Name":"HEIGHT","Value":"500"},{"Name":"EXPR","Value":"customer.company"}]},{"RecordIndex":13,"Deleted":true,"Title":"deleted.footer.total","Subtitle":"field","Properties":[{"Name":"HPOS","Value":"1600"},{"Name":"VPOS","Value":"9400"},{"Name":"WIDTH","Value":"3600"},{"Name":"HEIGHT","Value":"600"},{"Name":"EXPR","Value":"deleted.footer.total"}]}],"CommandUndoAvailable":false,"CommandUndoLabel":"","ReportLayout":{"DeletedPreviewBoundsAvailable":true,"DeletedPreviewBoundsLeft":1400,"DeletedPreviewBoundsTop":9400,"DeletedPreviewBoundsRight":5000,"DeletedPreviewBoundsBottom":10000,"DeletedPreviewBoundsWidth":3600,"DeletedPreviewBoundsHeight":600,"Sections":[{"Id":"detail_1","Title":"Detail","BandKind":"detail","RecordIndex":42,"Top":2000,"Height":5000,"Objects":[{"RecordIndex":6,"ObjectKind":"field","Title":"customer.company","Expression":"customer.company","Left":1200,"Top":2600,"Width":4000,"Height":500}]}],"DeletedSections":[{"Id":"deleted_footer","Title":"Summary","BandKind":"summary","RecordIndex":51,"Deleted":true,"Top":9000,"Height":1400,"Objects":[{"RecordIndex":13,"ObjectKind":"field","Title":"deleted.footer.total","Expression":"deleted.footer.total","Left":1600,"Top":9400,"Width":3600,"Height":600}]}],"UnplacedObjects":[]}}}
""";
    }

    private static string BuildLabelUndoPreviewRefreshHostResponseJson()
    {
        return """
{"Status":"ok","Document":{"AssetFamily":"label","FieldCount":5,"Objects":[{"RecordIndex":6,"Title":"customer.company","Subtitle":"label","Properties":[{"Name":"HPOS","Value":"1200"},{"Name":"VPOS","Value":"2600"},{"Name":"WIDTH","Value":"4000"},{"Name":"HEIGHT","Value":"500"},{"Name":"EXPR","Value":"customer.company"}]},{"RecordIndex":9,"Title":"orphan.note","Subtitle":"label","Properties":[{"Name":"HPOS","Value":"1100"},{"Name":"VPOS","Value":"700"},{"Name":"WIDTH","Value":"2400"},{"Name":"HEIGHT","Value":"450"},{"Name":"EXPR","Value":"orphan.note"}]}],"CommandUndoAvailable":false,"CommandUndoLabel":"","ReportLayout":{"IsLabel":true,"PreviewBoundsAvailable":true,"PreviewBoundsLeft":1100,"PreviewBoundsTop":700,"PreviewBoundsRight":5200,"PreviewBoundsBottom":3100,"PreviewBoundsWidth":4100,"PreviewBoundsHeight":2400,"Sections":[{"Id":"detail_1","Title":"Detail","BandKind":"detail","RecordIndex":42,"Top":2000,"Height":5000,"Objects":[{"RecordIndex":6,"ObjectKind":"label","Title":"customer.company","Expression":"customer.company","Left":1200,"Top":2600,"Width":4000,"Height":500}]}],"DeletedSections":[],"UnplacedObjects":[{"RecordIndex":9,"ObjectKind":"label","Title":"orphan.note","Expression":"orphan.note","Left":1100,"Top":700,"Width":2400,"Height":450}]}}}
""";
    }

    private static string BuildDeletedLabelUndoPreviewRefreshHostResponseJson()
    {
        return """
{"Status":"ok","Document":{"AssetFamily":"label","FieldCount":5,"Objects":[{"RecordIndex":6,"Title":"customer.company","Subtitle":"label","Properties":[{"Name":"HPOS","Value":"1200"},{"Name":"VPOS","Value":"2600"},{"Name":"WIDTH","Value":"4000"},{"Name":"HEIGHT","Value":"500"},{"Name":"EXPR","Value":"customer.company"}]},{"RecordIndex":13,"Deleted":true,"Title":"deleted.footer.total","Subtitle":"label","Properties":[{"Name":"HPOS","Value":"1600"},{"Name":"VPOS","Value":"9400"},{"Name":"WIDTH","Value":"3600"},{"Name":"HEIGHT","Value":"600"},{"Name":"EXPR","Value":"deleted.footer.total"}]}],"CommandUndoAvailable":false,"CommandUndoLabel":"","ReportLayout":{"IsLabel":true,"DeletedPreviewBoundsAvailable":true,"DeletedPreviewBoundsLeft":1400,"DeletedPreviewBoundsTop":9400,"DeletedPreviewBoundsRight":5000,"DeletedPreviewBoundsBottom":10000,"DeletedPreviewBoundsWidth":3600,"DeletedPreviewBoundsHeight":600,"Sections":[{"Id":"detail_1","Title":"Detail","BandKind":"detail","RecordIndex":42,"Top":2000,"Height":5000,"Objects":[{"RecordIndex":6,"ObjectKind":"label","Title":"customer.company","Expression":"customer.company","Left":1200,"Top":2600,"Width":4000,"Height":500}]}],"DeletedSections":[{"Id":"deleted_footer","Title":"Summary","BandKind":"summary","RecordIndex":51,"Deleted":true,"Top":9000,"Height":1400,"Objects":[{"RecordIndex":13,"ObjectKind":"label","Title":"deleted.footer.total","Expression":"deleted.footer.total","Left":1600,"Top":9400,"Width":3600,"Height":600}]}],"UnplacedObjects":[]}}}
""";
    }

    private static string BuildLabelSectionUpdateHostResponseJson()
    {
        return """
{"Status":"ok","Document":{"AssetFamily":"label","FieldCount":5,"Objects":[{"RecordIndex":6,"Title":"customer.company","Subtitle":"label","Properties":[{"Name":"HPOS","Value":"1200"},{"Name":"VPOS","Value":"3800"},{"Name":"WIDTH","Value":"4000"},{"Name":"HEIGHT","Value":"500"},{"Name":"EXPR","Value":"customer.company"}]}],"ReportLayout":{"IsLabel":true,"Sections":[{"Id":"detail_1","Title":"Detail","BandKind":"detail","RecordIndex":42,"Top":3200,"Height":5000,"Objects":[{"RecordIndex":6,"ObjectKind":"label","Title":"customer.company","Expression":"customer.company","Left":1200,"Top":3800,"Width":4000,"Height":500}]}],"DeletedSections":[],"UnplacedObjects":[]}}}
""";
    }

    private static string BuildLabelObjectUpdateHostResponseJson()
    {
        return """
{"Status":"ok","Document":{"AssetFamily":"label","FieldCount":5,"Objects":[{"RecordIndex":6,"Title":"customer.company","Subtitle":"label","Properties":[{"Name":"HPOS","Value":"1500"},{"Name":"VPOS","Value":"2600"},{"Name":"WIDTH","Value":"4000"},{"Name":"HEIGHT","Value":"500"},{"Name":"EXPR","Value":"customer.company"}]}],"ReportLayout":{"IsLabel":true,"Sections":[{"Id":"detail_1","Title":"Detail","BandKind":"detail","RecordIndex":42,"Top":2000,"Height":5000,"Objects":[{"RecordIndex":6,"ObjectKind":"label","Title":"customer.company","Expression":"customer.company","Left":1500,"Top":2600,"Width":4000,"Height":500}]}],"DeletedSections":[],"UnplacedObjects":[]}}}
""";
    }

    private static string BuildLabelObjectPlacementIntoUnplacedHostResponseJson()
    {
        return """
{"Status":"ok","Document":{"AssetFamily":"label","FieldCount":5,"Objects":[{"RecordIndex":6,"Title":"customer.company","Subtitle":"label","Properties":[{"Name":"HPOS","Value":"1200"},{"Name":"VPOS","Value":"9000"},{"Name":"WIDTH","Value":"4000"},{"Name":"HEIGHT","Value":"500"},{"Name":"EXPR","Value":"customer.company"}]}],"ReportLayout":{"IsLabel":true,"PreviewBoundsAvailable":true,"PreviewBoundsLeft":1200,"PreviewBoundsTop":9000,"PreviewBoundsRight":5200,"PreviewBoundsBottom":9500,"PreviewBoundsWidth":4000,"PreviewBoundsHeight":500,"Sections":[{"Id":"detail_1","Title":"Detail","BandKind":"detail","RecordIndex":42,"Top":2000,"Height":5000,"Objects":[]}],"DeletedSections":[],"UnplacedObjects":[{"RecordIndex":6,"ObjectKind":"label","Title":"customer.company","Expression":"customer.company","Left":1200,"Top":9000,"Width":4000,"Height":500}]}}}
""";
    }

    private static string BuildUnplacedLabelObjectUpdateHostResponseJson()
    {
        return """
{"Status":"ok","Document":{"AssetFamily":"label","FieldCount":5,"Objects":[{"RecordIndex":6,"Title":"customer.company","Subtitle":"label","Properties":[{"Name":"HPOS","Value":"1200"},{"Name":"VPOS","Value":"2600"},{"Name":"WIDTH","Value":"4000"},{"Name":"HEIGHT","Value":"500"},{"Name":"EXPR","Value":"customer.company"}]},{"RecordIndex":9,"Title":"orphan.note","Subtitle":"label","Properties":[{"Name":"HPOS","Value":"1100"},{"Name":"VPOS","Value":"700"},{"Name":"WIDTH","Value":"2400"},{"Name":"HEIGHT","Value":"450"},{"Name":"EXPR","Value":"orphan.note"}]}],"ReportLayout":{"IsLabel":true,"Sections":[{"Id":"detail_1","Title":"Detail","BandKind":"detail","RecordIndex":42,"Top":2000,"Height":5000,"Objects":[{"RecordIndex":6,"ObjectKind":"label","Title":"customer.company","Expression":"customer.company","Left":1200,"Top":2600,"Width":4000,"Height":500}]}],"DeletedSections":[],"UnplacedObjects":[{"RecordIndex":9,"ObjectKind":"label","Title":"orphan.note","Expression":"orphan.note","Left":1100,"Top":700,"Width":2400,"Height":450}]}}}
""";
    }

    private static string BuildUnplacedLabelObjectPreviewRefreshHostResponseJson()
    {
        return """
{"Status":"ok","Document":{"AssetFamily":"label","FieldCount":5,"Objects":[{"RecordIndex":6,"Title":"customer.company","Subtitle":"label","Properties":[{"Name":"HPOS","Value":"1200"},{"Name":"VPOS","Value":"2600"},{"Name":"WIDTH","Value":"4000"},{"Name":"HEIGHT","Value":"500"},{"Name":"EXPR","Value":"customer.company"}]},{"RecordIndex":9,"Title":"orphan.note","Subtitle":"label","Properties":[{"Name":"HPOS","Value":"1100"},{"Name":"VPOS","Value":"700"},{"Name":"WIDTH","Value":"2400"},{"Name":"HEIGHT","Value":"450"},{"Name":"EXPR","Value":"orphan.note"}]}],"ReportLayout":{"IsLabel":true,"PreviewBoundsAvailable":true,"PreviewBoundsLeft":1100,"PreviewBoundsTop":700,"PreviewBoundsRight":5200,"PreviewBoundsBottom":3100,"PreviewBoundsWidth":4100,"PreviewBoundsHeight":2400,"Sections":[{"Id":"detail_1","Title":"Detail","BandKind":"detail","RecordIndex":42,"Top":2000,"Height":5000,"Objects":[{"RecordIndex":6,"ObjectKind":"label","Title":"customer.company","Expression":"customer.company","Left":1200,"Top":2600,"Width":4000,"Height":500}]}],"DeletedSections":[],"UnplacedObjects":[{"RecordIndex":9,"ObjectKind":"label","Title":"orphan.note","Expression":"orphan.note","Left":1100,"Top":700,"Width":2400,"Height":450}]}}}
""";
    }

    private static string BuildDeletedLabelObjectUpdateHostResponseJson()
    {
        return """
{"Status":"ok","Document":{"AssetFamily":"label","FieldCount":5,"Objects":[{"RecordIndex":6,"Title":"customer.company","Subtitle":"label","Properties":[{"Name":"HPOS","Value":"1200"},{"Name":"VPOS","Value":"2600"},{"Name":"WIDTH","Value":"4000"},{"Name":"HEIGHT","Value":"500"},{"Name":"EXPR","Value":"customer.company"}]},{"RecordIndex":13,"Deleted":true,"Title":"deleted.footer.total","Subtitle":"label","Properties":[{"Name":"HPOS","Value":"1600"},{"Name":"VPOS","Value":"9400"},{"Name":"WIDTH","Value":"3600"},{"Name":"HEIGHT","Value":"600"},{"Name":"EXPR","Value":"deleted.footer.total"}]}],"ReportLayout":{"IsLabel":true,"Sections":[{"Id":"detail_1","Title":"Detail","BandKind":"detail","RecordIndex":42,"Top":2000,"Height":5000,"Objects":[{"RecordIndex":6,"ObjectKind":"label","Title":"customer.company","Expression":"customer.company","Left":1200,"Top":2600,"Width":4000,"Height":500}]}],"DeletedSections":[{"Id":"deleted_footer","Title":"Summary","BandKind":"summary","RecordIndex":51,"Deleted":true,"Top":9000,"Height":1400,"Objects":[{"RecordIndex":13,"ObjectKind":"label","Title":"deleted.footer.total","Expression":"deleted.footer.total","Left":1600,"Top":9400,"Width":3600,"Height":600}]}],"UnplacedObjects":[]}}}
""";
    }

    private static string BuildDeletedLabelObjectPreviewRefreshHostResponseJson()
    {
        return """
{"Status":"ok","Document":{"AssetFamily":"label","FieldCount":5,"Objects":[{"RecordIndex":6,"Title":"customer.company","Subtitle":"label","Properties":[{"Name":"HPOS","Value":"1200"},{"Name":"VPOS","Value":"2600"},{"Name":"WIDTH","Value":"4000"},{"Name":"HEIGHT","Value":"500"},{"Name":"EXPR","Value":"customer.company"}]},{"RecordIndex":13,"Deleted":true,"Title":"deleted.footer.total","Subtitle":"label","Properties":[{"Name":"HPOS","Value":"1600"},{"Name":"VPOS","Value":"9400"},{"Name":"WIDTH","Value":"3600"},{"Name":"HEIGHT","Value":"600"},{"Name":"EXPR","Value":"deleted.footer.total"}]}],"ReportLayout":{"IsLabel":true,"DeletedPreviewBoundsAvailable":true,"DeletedPreviewBoundsLeft":1400,"DeletedPreviewBoundsTop":9400,"DeletedPreviewBoundsRight":5000,"DeletedPreviewBoundsBottom":10000,"DeletedPreviewBoundsWidth":3600,"DeletedPreviewBoundsHeight":600,"Sections":[{"Id":"detail_1","Title":"Detail","BandKind":"detail","RecordIndex":42,"Top":2000,"Height":5000,"Objects":[{"RecordIndex":6,"ObjectKind":"label","Title":"customer.company","Expression":"customer.company","Left":1200,"Top":2600,"Width":4000,"Height":500}]}],"DeletedSections":[{"Id":"deleted_footer","Title":"Summary","BandKind":"summary","RecordIndex":51,"Deleted":true,"Top":9000,"Height":1400,"Objects":[{"RecordIndex":13,"ObjectKind":"label","Title":"deleted.footer.total","Expression":"deleted.footer.total","Left":1600,"Top":9400,"Width":3600,"Height":600}]}],"UnplacedObjects":[]}}}
""";
    }

    private static string BuildRestoreLabelObjectHostResponseJson()
    {
        return """
{"Status":"ok","Document":{"AssetFamily":"label","FieldCount":5,"Objects":[{"RecordIndex":13,"Title":"deleted.footer.total","Subtitle":"label","Properties":[{"Name":"UNIQUEID","Value":"deleted-footer-guid"},{"Name":"HPOS","Value":"1700"},{"Name":"VPOS","Value":"2800"},{"Name":"WIDTH","Value":"3600"},{"Name":"HEIGHT","Value":"600"},{"Name":"EXPR","Value":"deleted.footer.total"}]}],"ReportLayout":{"IsLabel":true,"PreviewBoundsAvailable":true,"PreviewBoundsLeft":1700,"PreviewBoundsTop":2800,"PreviewBoundsRight":5300,"PreviewBoundsBottom":3400,"PreviewBoundsWidth":3600,"PreviewBoundsHeight":600,"Sections":[{"Id":"detail_1","Title":"Detail","BandKind":"detail","RecordIndex":42,"Top":2000,"Height":5000,"Objects":[{"RecordIndex":13,"ObjectKind":"label","Title":"deleted.footer.total","Expression":"deleted.footer.total","Left":1700,"Top":2800,"Width":3600,"Height":600}]}],"DeletedSections":[],"UnplacedObjects":[]}}}
""";
    }

    private static string BuildDeleteLabelObjectHostResponseJson()
    {
        return """
{"Status":"ok","Document":{"AssetFamily":"label","FieldCount":5,"Objects":[{"RecordIndex":6,"Deleted":true,"Title":"customer.company","Subtitle":"label","Properties":[{"Name":"UNIQUEID","Value":"live-detail-guid"},{"Name":"HPOS","Value":"1500"},{"Name":"VPOS","Value":"2600"},{"Name":"WIDTH","Value":"4000"},{"Name":"HEIGHT","Value":"500"},{"Name":"EXPR","Value":"customer.company"}]}],"ReportLayout":{"IsLabel":true,"DeletedPreviewBoundsAvailable":true,"DeletedPreviewBoundsLeft":1500,"DeletedPreviewBoundsTop":2600,"DeletedPreviewBoundsRight":5500,"DeletedPreviewBoundsBottom":3100,"DeletedPreviewBoundsWidth":4000,"DeletedPreviewBoundsHeight":500,"Sections":[{"Id":"detail_1","Title":"Detail","BandKind":"detail","RecordIndex":42,"Top":2000,"Height":5000,"Objects":[]}],"DeletedSections":[{"Id":"deleted_detail","Title":"Detail","BandKind":"detail","RecordIndex":52,"Deleted":true,"Top":2000,"Height":5000,"Objects":[{"RecordIndex":6,"ObjectKind":"label","Title":"customer.company","Expression":"customer.company","Left":1500,"Top":2600,"Width":4000,"Height":500}]}],"UnplacedObjects":[]}}}
""";
    }

    private static string BuildRenameDeletedLabelObjectHostResponseJson()
    {
        return """
{"Status":"ok","Document":{"AssetFamily":"label","FieldCount":5,"Objects":[{"RecordIndex":13,"Deleted":true,"Title":"deleted.footer.total","Subtitle":"label","Properties":[{"Name":"UNIQUEID","Value":"deleted-footer-renamed-guid"},{"Name":"HPOS","Value":"1600"},{"Name":"VPOS","Value":"9400"},{"Name":"WIDTH","Value":"3600"},{"Name":"HEIGHT","Value":"600"},{"Name":"EXPR","Value":"deleted.footer.total"}]}],"ReportLayout":{"IsLabel":true,"DeletedPreviewBoundsAvailable":true,"DeletedPreviewBoundsLeft":1600,"DeletedPreviewBoundsTop":9400,"DeletedPreviewBoundsRight":5200,"DeletedPreviewBoundsBottom":10000,"DeletedPreviewBoundsWidth":3600,"DeletedPreviewBoundsHeight":600,"Sections":[],"DeletedSections":[{"Id":"deleted_footer","Title":"Summary","BandKind":"summary","RecordIndex":51,"Deleted":true,"Top":9000,"Height":1400,"Objects":[{"RecordIndex":13,"ObjectKind":"label","Title":"deleted.footer.total","Expression":"deleted.footer.total","Left":1600,"Top":9400,"Width":3600,"Height":600}]}],"UnplacedObjects":[]}}}
""";
    }

    private static string BuildReorderBackLabelObjectHostResponseJson()
    {
        return """
{"Status":"ok","Document":{"AssetFamily":"label","FieldCount":5,"Objects":[{"RecordIndex":7,"Title":"middle.value","Subtitle":"label","Properties":[{"Name":"UNIQUEID","Value":"middle-field-guid"},{"Name":"HPOS","Value":"2500"},{"Name":"VPOS","Value":"2600"},{"Name":"WIDTH","Value":"3400"},{"Name":"HEIGHT","Value":"600"},{"Name":"EXPR","Value":"middle.value"}]},{"RecordIndex":8,"Title":"last.value","Subtitle":"label","Properties":[{"Name":"UNIQUEID","Value":"last-field-guid"},{"Name":"HPOS","Value":"3600"},{"Name":"VPOS","Value":"2600"},{"Name":"WIDTH","Value":"3200"},{"Name":"HEIGHT","Value":"600"},{"Name":"EXPR","Value":"last.value"}]},{"RecordIndex":6,"Title":"first.value","Subtitle":"label","Properties":[{"Name":"UNIQUEID","Value":"first-field-guid"},{"Name":"HPOS","Value":"1400"},{"Name":"VPOS","Value":"2600"},{"Name":"WIDTH","Value":"3400"},{"Name":"HEIGHT","Value":"600"},{"Name":"EXPR","Value":"first.value"}]}],"ReportLayout":{"IsLabel":true,"PreviewBoundsAvailable":true,"PreviewBoundsLeft":1400,"PreviewBoundsTop":2600,"PreviewBoundsRight":6800,"PreviewBoundsBottom":3200,"PreviewBoundsWidth":5400,"PreviewBoundsHeight":600,"Sections":[{"Id":"detail_1","Title":"Detail","BandKind":"detail","RecordIndex":42,"Top":2000,"Height":5000,"Objects":[{"RecordIndex":7,"ObjectKind":"label","Title":"middle.value","Expression":"middle.value","Left":2500,"Top":2600,"Width":3400,"Height":600},{"RecordIndex":8,"ObjectKind":"label","Title":"last.value","Expression":"last.value","Left":3600,"Top":2600,"Width":3200,"Height":600},{"RecordIndex":6,"ObjectKind":"label","Title":"first.value","Expression":"first.value","Left":1400,"Top":2600,"Width":3400,"Height":600}]}],"DeletedSections":[],"UnplacedObjects":[]}}}
""";
    }

    private static string BuildAlignLeftLabelObjectHostResponseJson()
    {
        return """
{"Status":"ok","Document":{"AssetFamily":"label","FieldCount":5,"Objects":[{"RecordIndex":6,"Title":"first.value","Subtitle":"label","Properties":[{"Name":"UNIQUEID","Value":"first-field-guid"},{"Name":"HPOS","Value":"1400"},{"Name":"VPOS","Value":"2600"},{"Name":"WIDTH","Value":"3400"},{"Name":"HEIGHT","Value":"600"},{"Name":"EXPR","Value":"first.value"}]},{"RecordIndex":7,"Title":"middle.value","Subtitle":"label","Properties":[{"Name":"UNIQUEID","Value":"middle-field-guid"},{"Name":"HPOS","Value":"1400"},{"Name":"VPOS","Value":"2600"},{"Name":"WIDTH","Value":"3400"},{"Name":"HEIGHT","Value":"600"},{"Name":"EXPR","Value":"middle.value"}]},{"RecordIndex":8,"Title":"last.value","Subtitle":"label","Properties":[{"Name":"UNIQUEID","Value":"last-field-guid"},{"Name":"HPOS","Value":"3600"},{"Name":"VPOS","Value":"2600"},{"Name":"WIDTH","Value":"3200"},{"Name":"HEIGHT","Value":"600"},{"Name":"EXPR","Value":"last.value"}]}],"ReportLayout":{"IsLabel":true,"PreviewBoundsAvailable":true,"PreviewBoundsLeft":1400,"PreviewBoundsTop":2600,"PreviewBoundsRight":6800,"PreviewBoundsBottom":3200,"PreviewBoundsWidth":5400,"PreviewBoundsHeight":600,"Sections":[{"Id":"detail_1","Title":"Detail","BandKind":"detail","RecordIndex":42,"Top":2000,"Height":5000,"Objects":[{"RecordIndex":6,"ObjectKind":"label","Title":"first.value","Expression":"first.value","Left":1400,"Top":2600,"Width":3400,"Height":600},{"RecordIndex":7,"ObjectKind":"label","Title":"middle.value","Expression":"middle.value","Left":1400,"Top":2600,"Width":3400,"Height":600},{"RecordIndex":8,"ObjectKind":"label","Title":"last.value","Expression":"last.value","Left":3600,"Top":2600,"Width":3200,"Height":600}]}],"DeletedSections":[],"UnplacedObjects":[]},"SelectedReportSelectionAvailable":true,"SelectedReportSelectionKind":"object","SelectedReportObjectAvailable":true,"SelectedReportObject":{"RecordIndex":6,"Title":"first.value","Subtitle":"label","Properties":[{"Name":"UNIQUEID","Value":"first-field-guid"},{"Name":"HPOS","Value":"1400"},{"Name":"VPOS","Value":"2600"},{"Name":"WIDTH","Value":"3400"},{"Name":"HEIGHT","Value":"600"},{"Name":"EXPR","Value":"first.value"}]},"SelectedReportObjectSectionAvailable":true,"SelectedReportObjectSection":{"Id":"detail_1","Title":"Detail","BandKind":"detail","RecordIndex":42}}}
""";
    }

    private static string BuildAlignTopLabelObjectHostResponseJson()
    {
        return """
{"Status":"ok","Document":{"AssetFamily":"label","FieldCount":5,"Objects":[{"RecordIndex":6,"Title":"first.value","Subtitle":"label","Properties":[{"Name":"UNIQUEID","Value":"first-field-guid"},{"Name":"HPOS","Value":"1400"},{"Name":"VPOS","Value":"2600"},{"Name":"WIDTH","Value":"3400"},{"Name":"HEIGHT","Value":"600"},{"Name":"EXPR","Value":"first.value"}]},{"RecordIndex":7,"Title":"middle.value","Subtitle":"label","Properties":[{"Name":"UNIQUEID","Value":"middle-field-guid"},{"Name":"HPOS","Value":"2500"},{"Name":"VPOS","Value":"2600"},{"Name":"WIDTH","Value":"3400"},{"Name":"HEIGHT","Value":"600"},{"Name":"EXPR","Value":"middle.value"}]},{"RecordIndex":8,"Title":"last.value","Subtitle":"label","Properties":[{"Name":"UNIQUEID","Value":"last-field-guid"},{"Name":"HPOS","Value":"3600"},{"Name":"VPOS","Value":"2600"},{"Name":"WIDTH","Value":"3200"},{"Name":"HEIGHT","Value":"600"},{"Name":"EXPR","Value":"last.value"}]}],"ReportLayout":{"IsLabel":true,"PreviewBoundsAvailable":true,"PreviewBoundsLeft":1400,"PreviewBoundsTop":2600,"PreviewBoundsRight":6800,"PreviewBoundsBottom":3200,"PreviewBoundsWidth":5400,"PreviewBoundsHeight":600,"Sections":[{"Id":"detail_1","Title":"Detail","BandKind":"detail","RecordIndex":42,"Top":2000,"Height":5000,"Objects":[{"RecordIndex":6,"ObjectKind":"label","Title":"first.value","Expression":"first.value","Left":1400,"Top":2600,"Width":3400,"Height":600},{"RecordIndex":7,"ObjectKind":"label","Title":"middle.value","Expression":"middle.value","Left":2500,"Top":2600,"Width":3400,"Height":600},{"RecordIndex":8,"ObjectKind":"label","Title":"last.value","Expression":"last.value","Left":3600,"Top":2600,"Width":3200,"Height":600}]}],"DeletedSections":[],"UnplacedObjects":[]},"SelectedReportSelectionAvailable":true,"SelectedReportSelectionKind":"object","SelectedReportObjectAvailable":true,"SelectedReportObject":{"RecordIndex":6,"Title":"first.value","Subtitle":"label","Properties":[{"Name":"UNIQUEID","Value":"first-field-guid"},{"Name":"HPOS","Value":"1400"},{"Name":"VPOS","Value":"2600"},{"Name":"WIDTH","Value":"3400"},{"Name":"HEIGHT","Value":"600"},{"Name":"EXPR","Value":"first.value"}]},"SelectedReportObjectSectionAvailable":true,"SelectedReportObjectSection":{"Id":"detail_1","Title":"Detail","BandKind":"detail","RecordIndex":42}}}
""";
    }

    private static string BuildResizeToAnchorLabelObjectHostResponseJson()
    {
        return """
{"Status":"ok","Document":{"AssetFamily":"label","FieldCount":5,"Objects":[{"RecordIndex":6,"Title":"first.value","Subtitle":"label","Properties":[{"Name":"UNIQUEID","Value":"first-field-guid"},{"Name":"HPOS","Value":"1400"},{"Name":"VPOS","Value":"2600"},{"Name":"WIDTH","Value":"3400"},{"Name":"HEIGHT","Value":"600"},{"Name":"EXPR","Value":"first.value"}]},{"RecordIndex":7,"Title":"middle.value","Subtitle":"label","Properties":[{"Name":"UNIQUEID","Value":"middle-field-guid"},{"Name":"HPOS","Value":"2500"},{"Name":"VPOS","Value":"2600"},{"Name":"WIDTH","Value":"3400"},{"Name":"HEIGHT","Value":"600"},{"Name":"EXPR","Value":"middle.value"}]}],"ReportLayout":{"IsLabel":true,"PreviewBoundsAvailable":true,"PreviewBoundsLeft":1400,"PreviewBoundsTop":2600,"PreviewBoundsRight":5900,"PreviewBoundsBottom":3200,"PreviewBoundsWidth":4500,"PreviewBoundsHeight":600,"Sections":[{"Id":"detail_1","Title":"Detail","BandKind":"detail","RecordIndex":42,"Top":2000,"Height":5000,"Objects":[{"RecordIndex":6,"ObjectKind":"label","Title":"first.value","Expression":"first.value","Left":1400,"Top":2600,"Width":3400,"Height":600},{"RecordIndex":7,"ObjectKind":"label","Title":"middle.value","Expression":"middle.value","Left":2500,"Top":2600,"Width":3400,"Height":600}]}],"DeletedSections":[],"UnplacedObjects":[]},"SelectedReportSelectionAvailable":true,"SelectedReportSelectionKind":"object","SelectedReportObjectAvailable":true,"SelectedReportObject":{"RecordIndex":6,"Title":"first.value","Subtitle":"label","Properties":[{"Name":"UNIQUEID","Value":"first-field-guid"},{"Name":"HPOS","Value":"1400"},{"Name":"VPOS","Value":"2600"},{"Name":"WIDTH","Value":"3400"},{"Name":"HEIGHT","Value":"600"},{"Name":"EXPR","Value":"first.value"}]},"SelectedReportObjectSectionAvailable":true,"SelectedReportObjectSection":{"Id":"detail_1","Title":"Detail","BandKind":"detail","RecordIndex":42}}}
""";
    }

    private static string BuildResizeToAnchorWidthLabelObjectHostResponseJson()
    {
        return """
{"Status":"ok","Document":{"AssetFamily":"label","FieldCount":5,"Objects":[{"RecordIndex":6,"Title":"first.value","Subtitle":"label","Properties":[{"Name":"UNIQUEID","Value":"first-field-guid"},{"Name":"HPOS","Value":"1400"},{"Name":"VPOS","Value":"2600"},{"Name":"WIDTH","Value":"3400"},{"Name":"HEIGHT","Value":"600"},{"Name":"EXPR","Value":"first.value"}]},{"RecordIndex":7,"Title":"middle.value","Subtitle":"label","Properties":[{"Name":"UNIQUEID","Value":"middle-field-guid"},{"Name":"HPOS","Value":"2500"},{"Name":"VPOS","Value":"2600"},{"Name":"WIDTH","Value":"3400"},{"Name":"HEIGHT","Value":"900"},{"Name":"EXPR","Value":"middle.value"}]}],"ReportLayout":{"IsLabel":true,"PreviewBoundsAvailable":true,"PreviewBoundsLeft":1400,"PreviewBoundsTop":2600,"PreviewBoundsRight":5900,"PreviewBoundsBottom":3500,"PreviewBoundsWidth":4500,"PreviewBoundsHeight":900,"Sections":[{"Id":"detail_1","Title":"Detail","BandKind":"detail","RecordIndex":42,"Top":2000,"Height":5000,"Objects":[{"RecordIndex":6,"ObjectKind":"label","Title":"first.value","Expression":"first.value","Left":1400,"Top":2600,"Width":3400,"Height":600},{"RecordIndex":7,"ObjectKind":"label","Title":"middle.value","Expression":"middle.value","Left":2500,"Top":2600,"Width":3400,"Height":900}]}],"DeletedSections":[],"UnplacedObjects":[]},"SelectedReportSelectionAvailable":true,"SelectedReportSelectionKind":"object","SelectedReportObjectAvailable":true,"SelectedReportObject":{"RecordIndex":6,"Title":"first.value","Subtitle":"label","Properties":[{"Name":"UNIQUEID","Value":"first-field-guid"},{"Name":"HPOS","Value":"1400"},{"Name":"VPOS","Value":"2600"},{"Name":"WIDTH","Value":"3400"},{"Name":"HEIGHT","Value":"600"},{"Name":"EXPR","Value":"first.value"}]},"SelectedReportObjectSectionAvailable":true,"SelectedReportObjectSection":{"Id":"detail_1","Title":"Detail","BandKind":"detail","RecordIndex":42}}}
""";
    }

    private static string BuildResizeToAnchorHeightLabelObjectHostResponseJson()
    {
        return """
{"Status":"ok","Document":{"AssetFamily":"label","FieldCount":5,"Objects":[{"RecordIndex":6,"Title":"first.value","Subtitle":"label","Properties":[{"Name":"UNIQUEID","Value":"first-field-guid"},{"Name":"HPOS","Value":"1400"},{"Name":"VPOS","Value":"2600"},{"Name":"WIDTH","Value":"3400"},{"Name":"HEIGHT","Value":"600"},{"Name":"EXPR","Value":"first.value"}]},{"RecordIndex":7,"Title":"middle.value","Subtitle":"label","Properties":[{"Name":"UNIQUEID","Value":"middle-field-guid"},{"Name":"HPOS","Value":"2500"},{"Name":"VPOS","Value":"2600"},{"Name":"WIDTH","Value":"4200"},{"Name":"HEIGHT","Value":"600"},{"Name":"EXPR","Value":"middle.value"}]}],"ReportLayout":{"IsLabel":true,"PreviewBoundsAvailable":true,"PreviewBoundsLeft":1400,"PreviewBoundsTop":2600,"PreviewBoundsRight":6700,"PreviewBoundsBottom":3200,"PreviewBoundsWidth":5300,"PreviewBoundsHeight":600,"Sections":[{"Id":"detail_1","Title":"Detail","BandKind":"detail","RecordIndex":42,"Top":2000,"Height":5000,"Objects":[{"RecordIndex":6,"ObjectKind":"label","Title":"first.value","Expression":"first.value","Left":1400,"Top":2600,"Width":3400,"Height":600},{"RecordIndex":7,"ObjectKind":"label","Title":"middle.value","Expression":"middle.value","Left":2500,"Top":2600,"Width":4200,"Height":600}]}],"DeletedSections":[],"UnplacedObjects":[]},"SelectedReportSelectionAvailable":true,"SelectedReportSelectionKind":"object","SelectedReportObjectAvailable":true,"SelectedReportObject":{"RecordIndex":6,"Title":"first.value","Subtitle":"label","Properties":[{"Name":"UNIQUEID","Value":"first-field-guid"},{"Name":"HPOS","Value":"1400"},{"Name":"VPOS","Value":"2600"},{"Name":"WIDTH","Value":"3400"},{"Name":"HEIGHT","Value":"600"},{"Name":"EXPR","Value":"first.value"}]},"SelectedReportObjectSectionAvailable":true,"SelectedReportObjectSection":{"Id":"detail_1","Title":"Detail","BandKind":"detail","RecordIndex":42}}}
""";
    }

    private static string BuildDistributeHorizontalLabelObjectHostResponseJson()
    {
        return """
{"Status":"ok","Document":{"AssetFamily":"label","FieldCount":5,"Objects":[{"RecordIndex":6,"Title":"first.value","Subtitle":"label","Properties":[{"Name":"UNIQUEID","Value":"first-field-guid"},{"Name":"HPOS","Value":"1400"},{"Name":"VPOS","Value":"2600"},{"Name":"WIDTH","Value":"3400"},{"Name":"HEIGHT","Value":"600"},{"Name":"EXPR","Value":"first.value"}]},{"RecordIndex":7,"Title":"middle.value","Subtitle":"label","Properties":[{"Name":"UNIQUEID","Value":"middle-field-guid"},{"Name":"HPOS","Value":"2500"},{"Name":"VPOS","Value":"2600"},{"Name":"WIDTH","Value":"3400"},{"Name":"HEIGHT","Value":"600"},{"Name":"EXPR","Value":"middle.value"}]},{"RecordIndex":8,"Title":"last.value","Subtitle":"label","Properties":[{"Name":"UNIQUEID","Value":"last-field-guid"},{"Name":"HPOS","Value":"3600"},{"Name":"VPOS","Value":"2600"},{"Name":"WIDTH","Value":"3200"},{"Name":"HEIGHT","Value":"600"},{"Name":"EXPR","Value":"last.value"}]}],"ReportLayout":{"IsLabel":true,"PreviewBoundsAvailable":true,"PreviewBoundsLeft":1400,"PreviewBoundsTop":2600,"PreviewBoundsRight":6800,"PreviewBoundsBottom":3200,"PreviewBoundsWidth":5400,"PreviewBoundsHeight":600,"Sections":[{"Id":"detail_1","Title":"Detail","BandKind":"detail","RecordIndex":42,"Top":2000,"Height":5000,"Objects":[{"RecordIndex":6,"ObjectKind":"label","Title":"first.value","Expression":"first.value","Left":1400,"Top":2600,"Width":3400,"Height":600},{"RecordIndex":7,"ObjectKind":"label","Title":"middle.value","Expression":"middle.value","Left":2500,"Top":2600,"Width":3400,"Height":600},{"RecordIndex":8,"ObjectKind":"label","Title":"last.value","Expression":"last.value","Left":3600,"Top":2600,"Width":3200,"Height":600}]}],"DeletedSections":[],"UnplacedObjects":[]},"SelectedReportSelectionAvailable":true,"SelectedReportSelectionKind":"object","SelectedReportObjectAvailable":true,"SelectedReportObject":{"RecordIndex":7,"Title":"middle.value","Subtitle":"label","Properties":[{"Name":"UNIQUEID","Value":"middle-field-guid"},{"Name":"HPOS","Value":"2500"},{"Name":"VPOS","Value":"2600"},{"Name":"WIDTH","Value":"3400"},{"Name":"HEIGHT","Value":"600"},{"Name":"EXPR","Value":"middle.value"}]},"SelectedReportObjectSectionAvailable":true,"SelectedReportObjectSection":{"Id":"detail_1","Title":"Detail","BandKind":"detail","RecordIndex":42}}}
""";
    }

    private static string BuildDistributeVerticalLabelObjectHostResponseJson()
    {
        return """
{"Status":"ok","Document":{"AssetFamily":"label","FieldCount":5,"Objects":[{"RecordIndex":6,"Title":"first.value","Subtitle":"label","Properties":[{"Name":"UNIQUEID","Value":"first-field-guid"},{"Name":"HPOS","Value":"1400"},{"Name":"VPOS","Value":"2200"},{"Name":"WIDTH","Value":"3400"},{"Name":"HEIGHT","Value":"600"},{"Name":"EXPR","Value":"first.value"}]},{"RecordIndex":7,"Title":"middle.value","Subtitle":"label","Properties":[{"Name":"UNIQUEID","Value":"middle-field-guid"},{"Name":"HPOS","Value":"1900"},{"Name":"VPOS","Value":"3200"},{"Name":"WIDTH","Value":"3400"},{"Name":"HEIGHT","Value":"600"},{"Name":"EXPR","Value":"middle.value"}]},{"RecordIndex":8,"Title":"last.value","Subtitle":"label","Properties":[{"Name":"UNIQUEID","Value":"last-field-guid"},{"Name":"HPOS","Value":"3600"},{"Name":"VPOS","Value":"4200"},{"Name":"WIDTH","Value":"3200"},{"Name":"HEIGHT","Value":"600"},{"Name":"EXPR","Value":"last.value"}]}],"ReportLayout":{"IsLabel":true,"PreviewBoundsAvailable":true,"PreviewBoundsLeft":1400,"PreviewBoundsTop":2200,"PreviewBoundsRight":6800,"PreviewBoundsBottom":4800,"PreviewBoundsWidth":5400,"PreviewBoundsHeight":2600,"Sections":[{"Id":"detail_1","Title":"Detail","BandKind":"detail","RecordIndex":42,"Top":2000,"Height":5000,"Objects":[{"RecordIndex":6,"ObjectKind":"label","Title":"first.value","Expression":"first.value","Left":1400,"Top":2200,"Width":3400,"Height":600},{"RecordIndex":7,"ObjectKind":"label","Title":"middle.value","Expression":"middle.value","Left":1900,"Top":3200,"Width":3400,"Height":600},{"RecordIndex":8,"ObjectKind":"label","Title":"last.value","Expression":"last.value","Left":3600,"Top":4200,"Width":3200,"Height":600}]}],"DeletedSections":[],"UnplacedObjects":[]},"SelectedReportSelectionAvailable":true,"SelectedReportSelectionKind":"object","SelectedReportObjectAvailable":true,"SelectedReportObject":{"RecordIndex":7,"Title":"middle.value","Subtitle":"label","Properties":[{"Name":"UNIQUEID","Value":"middle-field-guid"},{"Name":"HPOS","Value":"1900"},{"Name":"VPOS","Value":"3200"},{"Name":"WIDTH","Value":"3400"},{"Name":"HEIGHT","Value":"600"},{"Name":"EXPR","Value":"middle.value"}]},"SelectedReportObjectSectionAvailable":true,"SelectedReportObjectSection":{"Id":"detail_1","Title":"Detail","BandKind":"detail","RecordIndex":42}}}
""";
    }

    private static string BuildSnapToGridLabelObjectHostResponseJson()
    {
        return """
{"Status":"ok","Document":{"AssetFamily":"label","FieldCount":5,"Objects":[{"RecordIndex":7,"Title":"snap.value","Subtitle":"label","Properties":[{"Name":"UNIQUEID","Value":"snap-field-guid"},{"Name":"HPOS","Value":"1908"},{"Name":"VPOS","Value":"2604"},{"Name":"WIDTH","Value":"3400"},{"Name":"HEIGHT","Value":"600"},{"Name":"EXPR","Value":"snap.value"}]}],"ReportLayout":{"IsLabel":true,"PreviewBoundsAvailable":true,"PreviewBoundsLeft":1908,"PreviewBoundsTop":2604,"PreviewBoundsRight":5308,"PreviewBoundsBottom":3204,"PreviewBoundsWidth":3400,"PreviewBoundsHeight":600,"Settings":[{"Name":"GRIDH","Value":"12","RecordIndex":0},{"Name":"GRIDV","Value":"12","RecordIndex":0}],"Sections":[{"Id":"detail_1","Title":"Detail","BandKind":"detail","RecordIndex":42,"Top":2000,"Height":5000,"Objects":[{"RecordIndex":7,"ObjectKind":"label","Title":"snap.value","Expression":"snap.value","Left":1908,"Top":2604,"Width":3400,"Height":600}]}],"DeletedSections":[],"UnplacedObjects":[]},"SelectedReportSelectionAvailable":true,"SelectedReportSelectionKind":"object","SelectedReportObjectAvailable":true,"SelectedReportObject":{"RecordIndex":7,"Title":"snap.value","Subtitle":"label","Properties":[{"Name":"UNIQUEID","Value":"snap-field-guid"},{"Name":"HPOS","Value":"1908"},{"Name":"VPOS","Value":"2604"},{"Name":"WIDTH","Value":"3400"},{"Name":"HEIGHT","Value":"600"},{"Name":"EXPR","Value":"snap.value"}]},"SelectedReportObjectSectionAvailable":true,"SelectedReportObjectSection":{"Id":"detail_1","Title":"Detail","BandKind":"detail","RecordIndex":42}}}
""";
    }

    private static string BuildSnapHorizontalLabelObjectHostResponseJson()
    {
        return """
{"Status":"ok","Document":{"AssetFamily":"label","FieldCount":5,"Objects":[{"RecordIndex":7,"Title":"snap.value","Subtitle":"label","Properties":[{"Name":"UNIQUEID","Value":"snap-field-guid"},{"Name":"HPOS","Value":"1908"},{"Name":"VPOS","Value":"2605"},{"Name":"WIDTH","Value":"3400"},{"Name":"HEIGHT","Value":"600"},{"Name":"EXPR","Value":"snap.value"}]}],"ReportLayout":{"IsLabel":true,"PreviewBoundsAvailable":true,"PreviewBoundsLeft":1908,"PreviewBoundsTop":2605,"PreviewBoundsRight":5308,"PreviewBoundsBottom":3205,"PreviewBoundsWidth":3400,"PreviewBoundsHeight":600,"Settings":[{"Name":"GRIDH","Value":"12","RecordIndex":0},{"Name":"GRIDV","Value":"12","RecordIndex":0}],"Sections":[{"Id":"detail_1","Title":"Detail","BandKind":"detail","RecordIndex":42,"Top":2000,"Height":5000,"Objects":[{"RecordIndex":7,"ObjectKind":"label","Title":"snap.value","Expression":"snap.value","Left":1908,"Top":2605,"Width":3400,"Height":600}]}],"DeletedSections":[],"UnplacedObjects":[]},"SelectedReportSelectionAvailable":true,"SelectedReportSelectionKind":"object","SelectedReportObjectAvailable":true,"SelectedReportObject":{"RecordIndex":7,"Title":"snap.value","Subtitle":"label","Properties":[{"Name":"UNIQUEID","Value":"snap-field-guid"},{"Name":"HPOS","Value":"1908"},{"Name":"VPOS","Value":"2605"},{"Name":"WIDTH","Value":"3400"},{"Name":"HEIGHT","Value":"600"},{"Name":"EXPR","Value":"snap.value"}]},"SelectedReportObjectSectionAvailable":true,"SelectedReportObjectSection":{"Id":"detail_1","Title":"Detail","BandKind":"detail","RecordIndex":42}}}
""";
    }

    private static string BuildSnapVerticalLabelObjectHostResponseJson()
    {
        return """
{"Status":"ok","Document":{"AssetFamily":"label","FieldCount":5,"Objects":[{"RecordIndex":7,"Title":"snap.value","Subtitle":"label","Properties":[{"Name":"UNIQUEID","Value":"snap-field-guid"},{"Name":"HPOS","Value":"1901"},{"Name":"VPOS","Value":"2604"},{"Name":"WIDTH","Value":"3400"},{"Name":"HEIGHT","Value":"600"},{"Name":"EXPR","Value":"snap.value"}]}],"ReportLayout":{"IsLabel":true,"PreviewBoundsAvailable":true,"PreviewBoundsLeft":1901,"PreviewBoundsTop":2604,"PreviewBoundsRight":5301,"PreviewBoundsBottom":3204,"PreviewBoundsWidth":3400,"PreviewBoundsHeight":600,"Settings":[{"Name":"GRIDH","Value":"12","RecordIndex":0},{"Name":"GRIDV","Value":"12","RecordIndex":0}],"Sections":[{"Id":"detail_1","Title":"Detail","BandKind":"detail","RecordIndex":42,"Top":2000,"Height":5000,"Objects":[{"RecordIndex":7,"ObjectKind":"label","Title":"snap.value","Expression":"snap.value","Left":1901,"Top":2604,"Width":3400,"Height":600}]}],"DeletedSections":[],"UnplacedObjects":[]},"SelectedReportSelectionAvailable":true,"SelectedReportSelectionKind":"object","SelectedReportObjectAvailable":true,"SelectedReportObject":{"RecordIndex":7,"Title":"snap.value","Subtitle":"label","Properties":[{"Name":"UNIQUEID","Value":"snap-field-guid"},{"Name":"HPOS","Value":"1901"},{"Name":"VPOS","Value":"2604"},{"Name":"WIDTH","Value":"3400"},{"Name":"HEIGHT","Value":"600"},{"Name":"EXPR","Value":"snap.value"}]},"SelectedReportObjectSectionAvailable":true,"SelectedReportObjectSection":{"Id":"detail_1","Title":"Detail","BandKind":"detail","RecordIndex":42}}}
""";
    }

    private static string BuildDuplicateLabelObjectHostResponseJson()
    {
        return """
{"Status":"ok","Document":{"AssetFamily":"label","FieldCount":5,"Objects":[{"RecordIndex":7,"Title":"middle.value","Subtitle":"label","Properties":[{"Name":"UNIQUEID","Value":"middle-field-guid"},{"Name":"HPOS","Value":"1200"},{"Name":"VPOS","Value":"2600"},{"Name":"WIDTH","Value":"4000"},{"Name":"HEIGHT","Value":"600"},{"Name":"EXPR","Value":"middle.value"}]},{"RecordIndex":10,"Title":"middle.value.copy","Subtitle":"label","Properties":[{"Name":"UNIQUEID","Value":"middle-copy-guid"},{"Name":"HPOS","Value":"2200"},{"Name":"VPOS","Value":"2600"},{"Name":"WIDTH","Value":"4000"},{"Name":"HEIGHT","Value":"600"},{"Name":"EXPR","Value":"middle.value"}]}],"ReportLayout":{"IsLabel":true,"PreviewBoundsAvailable":true,"PreviewBoundsLeft":1200,"PreviewBoundsTop":2600,"PreviewBoundsRight":6200,"PreviewBoundsBottom":3200,"PreviewBoundsWidth":5000,"PreviewBoundsHeight":600,"Sections":[{"Id":"detail_1","Title":"Detail","BandKind":"detail","RecordIndex":42,"Top":2000,"Height":5000,"Objects":[{"RecordIndex":7,"ObjectKind":"label","Title":"middle.value","Expression":"middle.value","Left":1200,"Top":2600,"Width":4000,"Height":600},{"RecordIndex":10,"ObjectKind":"label","Title":"middle.value.copy","Expression":"middle.value","Left":2200,"Top":2600,"Width":4000,"Height":600}]}],"DeletedSections":[],"UnplacedObjects":[]}}}
""";
    }

    private static string BuildDeletedReportObjectUpdateHostResponseJson()
    {
        return """
{"Status":"ok","Document":{"AssetFamily":"report","FieldCount":5,"Objects":[{"RecordIndex":6,"Title":"customer.company","Subtitle":"field","Properties":[{"Name":"HPOS","Value":"1200"},{"Name":"VPOS","Value":"2600"},{"Name":"WIDTH","Value":"4000"},{"Name":"HEIGHT","Value":"500"},{"Name":"EXPR","Value":"customer.company"}]},{"RecordIndex":13,"Deleted":true,"Title":"deleted.footer.total","Subtitle":"field","Properties":[{"Name":"HPOS","Value":"1600"},{"Name":"VPOS","Value":"9400"},{"Name":"WIDTH","Value":"3600"},{"Name":"HEIGHT","Value":"600"},{"Name":"EXPR","Value":"deleted.footer.total"}]}],"ReportLayout":{"Sections":[{"Id":"detail_1","Title":"Detail","BandKind":"detail","RecordIndex":42,"Top":2000,"Height":5000,"Objects":[{"RecordIndex":6,"ObjectKind":"field","Title":"customer.company","Expression":"customer.company","Left":1200,"Top":2600,"Width":4000,"Height":500}]}],"DeletedSections":[{"Id":"deleted_footer","Title":"Summary","BandKind":"summary","RecordIndex":51,"Deleted":true,"Top":9000,"Height":1400,"Objects":[{"RecordIndex":13,"ObjectKind":"field","Title":"deleted.footer.total","Expression":"deleted.footer.total","Left":1600,"Top":9400,"Width":3600,"Height":600}]}],"UnplacedObjects":[]}}}
""";
    }

    private static string BuildDeletedReportObjectPreviewRefreshHostResponseJson()
    {
        return """
{"Status":"ok","Document":{"AssetFamily":"report","FieldCount":5,"Objects":[{"RecordIndex":6,"Title":"customer.company","Subtitle":"field","Properties":[{"Name":"HPOS","Value":"1200"},{"Name":"VPOS","Value":"2600"},{"Name":"WIDTH","Value":"4000"},{"Name":"HEIGHT","Value":"500"},{"Name":"EXPR","Value":"customer.company"}]},{"RecordIndex":13,"Deleted":true,"Title":"deleted.footer.total","Subtitle":"field","Properties":[{"Name":"HPOS","Value":"1600"},{"Name":"VPOS","Value":"9400"},{"Name":"WIDTH","Value":"3600"},{"Name":"HEIGHT","Value":"600"},{"Name":"EXPR","Value":"deleted.footer.total"}]}],"ReportLayout":{"DeletedPreviewBoundsAvailable":true,"DeletedPreviewBoundsLeft":1400,"DeletedPreviewBoundsTop":9400,"DeletedPreviewBoundsRight":5000,"DeletedPreviewBoundsBottom":10000,"DeletedPreviewBoundsWidth":3600,"DeletedPreviewBoundsHeight":600,"Sections":[{"Id":"detail_1","Title":"Detail","BandKind":"detail","RecordIndex":42,"Top":2000,"Height":5000,"Objects":[{"RecordIndex":6,"ObjectKind":"field","Title":"customer.company","Expression":"customer.company","Left":1200,"Top":2600,"Width":4000,"Height":500}]}],"DeletedSections":[{"Id":"deleted_footer","Title":"Summary","BandKind":"summary","RecordIndex":51,"Deleted":true,"Top":9000,"Height":1400,"Objects":[{"RecordIndex":13,"ObjectKind":"field","Title":"deleted.footer.total","Expression":"deleted.footer.total","Left":1600,"Top":9400,"Width":3600,"Height":600}]}],"UnplacedObjects":[]}}}
""";
    }

    private static string BuildDeleteReportObjectHostResponseJson()
    {
        return """
{"Status":"ok","Document":{"AssetFamily":"report","FieldCount":5,"Objects":[{"RecordIndex":6,"Deleted":true,"Title":"customer.company","Subtitle":"field","Properties":[{"Name":"UNIQUEID","Value":"live-detail-guid"},{"Name":"HPOS","Value":"1500"},{"Name":"VPOS","Value":"2600"},{"Name":"WIDTH","Value":"4000"},{"Name":"HEIGHT","Value":"500"},{"Name":"EXPR","Value":"customer.company"}]}],"ReportLayout":{"DeletedPreviewBoundsAvailable":true,"DeletedPreviewBoundsLeft":1500,"DeletedPreviewBoundsTop":2600,"DeletedPreviewBoundsRight":5500,"DeletedPreviewBoundsBottom":3100,"DeletedPreviewBoundsWidth":4000,"DeletedPreviewBoundsHeight":500,"Sections":[{"Id":"detail_1","Title":"Detail","BandKind":"detail","RecordIndex":42,"Top":2000,"Height":5000,"Objects":[]}],"DeletedSections":[{"Id":"deleted_detail","Title":"Detail","BandKind":"detail","RecordIndex":52,"Deleted":true,"Top":2000,"Height":5000,"Objects":[{"RecordIndex":6,"ObjectKind":"field","Title":"customer.company","Expression":"customer.company","Left":1500,"Top":2600,"Width":4000,"Height":500}]}],"UnplacedObjects":[]}}}
""";
    }

    private static string BuildRestoreReportObjectHostResponseJson()
    {
        return """
{"Status":"ok","Document":{"AssetFamily":"report","FieldCount":5,"Objects":[{"RecordIndex":13,"Title":"deleted.footer.total","Subtitle":"field","Properties":[{"Name":"UNIQUEID","Value":"deleted-footer-guid"},{"Name":"HPOS","Value":"1700"},{"Name":"VPOS","Value":"2800"},{"Name":"WIDTH","Value":"3600"},{"Name":"HEIGHT","Value":"600"},{"Name":"EXPR","Value":"deleted.footer.total"}]}],"ReportLayout":{"PreviewBoundsAvailable":true,"PreviewBoundsLeft":1700,"PreviewBoundsTop":2800,"PreviewBoundsRight":5300,"PreviewBoundsBottom":3400,"PreviewBoundsWidth":3600,"PreviewBoundsHeight":600,"Sections":[{"Id":"detail_1","Title":"Detail","BandKind":"detail","RecordIndex":42,"Top":2000,"Height":5000,"Objects":[{"RecordIndex":13,"ObjectKind":"field","Title":"deleted.footer.total","Expression":"deleted.footer.total","Left":1700,"Top":2800,"Width":3600,"Height":600}]}],"DeletedSections":[],"UnplacedObjects":[]}}}
""";
    }

    private static string BuildRenameReportObjectHostResponseJson()
    {
        return """
{"Status":"ok","Document":{"AssetFamily":"report","FieldCount":5,"Objects":[{"RecordIndex":7,"Title":"middle.value","Subtitle":"field","Properties":[{"Name":"UNIQUEID","Value":"middle-renamed-guid"},{"Name":"HPOS","Value":"1200"},{"Name":"VPOS","Value":"2600"},{"Name":"WIDTH","Value":"4000"},{"Name":"HEIGHT","Value":"600"},{"Name":"EXPR","Value":"middle.value"}]}],"ReportLayout":{"PreviewBoundsAvailable":true,"PreviewBoundsLeft":1200,"PreviewBoundsTop":2600,"PreviewBoundsRight":5200,"PreviewBoundsBottom":3200,"PreviewBoundsWidth":4000,"PreviewBoundsHeight":600,"Sections":[{"Id":"detail_1","Title":"Detail","BandKind":"detail","RecordIndex":42,"Top":2000,"Height":5000,"Objects":[{"RecordIndex":7,"ObjectKind":"field","Title":"middle.value","Expression":"middle.value","Left":1200,"Top":2600,"Width":4000,"Height":600}]}],"DeletedSections":[],"UnplacedObjects":[]}}}
""";
    }

    private static string BuildReorderFrontReportObjectHostResponseJson()
    {
        return """
{"Status":"ok","Document":{"AssetFamily":"report","FieldCount":5,"Objects":[{"RecordIndex":7,"Title":"middle.value","Subtitle":"field","Properties":[{"Name":"UNIQUEID","Value":"middle-field-guid"},{"Name":"HPOS","Value":"2100"},{"Name":"VPOS","Value":"2400"},{"Name":"WIDTH","Value":"3200"},{"Name":"HEIGHT","Value":"700"},{"Name":"EXPR","Value":"middle.value"}]},{"RecordIndex":6,"Title":"first.value","Subtitle":"field","Properties":[{"Name":"UNIQUEID","Value":"first-field-guid"},{"Name":"HPOS","Value":"1000"},{"Name":"VPOS","Value":"2400"},{"Name":"WIDTH","Value":"3200"},{"Name":"HEIGHT","Value":"700"},{"Name":"EXPR","Value":"first.value"}]},{"RecordIndex":8,"Title":"last.value","Subtitle":"field","Properties":[{"Name":"UNIQUEID","Value":"last-field-guid"},{"Name":"HPOS","Value":"3200"},{"Name":"VPOS","Value":"2400"},{"Name":"WIDTH","Value":"2700"},{"Name":"HEIGHT","Value":"700"},{"Name":"EXPR","Value":"last.value"}]}],"ReportLayout":{"PreviewBoundsAvailable":true,"PreviewBoundsLeft":1000,"PreviewBoundsTop":2400,"PreviewBoundsRight":5900,"PreviewBoundsBottom":3100,"PreviewBoundsWidth":4900,"PreviewBoundsHeight":700,"Sections":[{"Id":"detail_1","Title":"Detail","BandKind":"detail","RecordIndex":42,"Top":2000,"Height":5000,"Objects":[{"RecordIndex":7,"ObjectKind":"field","Title":"middle.value","Expression":"middle.value","Left":2100,"Top":2400,"Width":3200,"Height":700},{"RecordIndex":6,"ObjectKind":"field","Title":"first.value","Expression":"first.value","Left":1000,"Top":2400,"Width":3200,"Height":700},{"RecordIndex":8,"ObjectKind":"field","Title":"last.value","Expression":"last.value","Left":3200,"Top":2400,"Width":2700,"Height":700}]}],"DeletedSections":[],"UnplacedObjects":[]}}}
""";
    }

    private static string BuildAlignLeftReportObjectHostResponseJson()
    {
        return """
{"Status":"ok","Document":{"AssetFamily":"report","FieldCount":5,"Objects":[{"RecordIndex":6,"Title":"first.value","Subtitle":"field","Properties":[{"Name":"UNIQUEID","Value":"first-field-guid"},{"Name":"HPOS","Value":"1000"},{"Name":"VPOS","Value":"2400"},{"Name":"WIDTH","Value":"3200"},{"Name":"HEIGHT","Value":"700"},{"Name":"EXPR","Value":"first.value"}]},{"RecordIndex":7,"Title":"middle.value","Subtitle":"field","Properties":[{"Name":"UNIQUEID","Value":"middle-field-guid"},{"Name":"HPOS","Value":"1000"},{"Name":"VPOS","Value":"2400"},{"Name":"WIDTH","Value":"3200"},{"Name":"HEIGHT","Value":"700"},{"Name":"EXPR","Value":"middle.value"}]},{"RecordIndex":8,"Title":"last.value","Subtitle":"field","Properties":[{"Name":"UNIQUEID","Value":"last-field-guid"},{"Name":"HPOS","Value":"3200"},{"Name":"VPOS","Value":"2400"},{"Name":"WIDTH","Value":"2700"},{"Name":"HEIGHT","Value":"700"},{"Name":"EXPR","Value":"last.value"}]}],"ReportLayout":{"PreviewBoundsAvailable":true,"PreviewBoundsLeft":1000,"PreviewBoundsTop":2400,"PreviewBoundsRight":5900,"PreviewBoundsBottom":3100,"PreviewBoundsWidth":4900,"PreviewBoundsHeight":700,"Sections":[{"Id":"detail_1","Title":"Detail","BandKind":"detail","RecordIndex":42,"Top":2000,"Height":5000,"Objects":[{"RecordIndex":6,"ObjectKind":"field","Title":"first.value","Expression":"first.value","Left":1000,"Top":2400,"Width":3200,"Height":700},{"RecordIndex":7,"ObjectKind":"field","Title":"middle.value","Expression":"middle.value","Left":1000,"Top":2400,"Width":3200,"Height":700},{"RecordIndex":8,"ObjectKind":"field","Title":"last.value","Expression":"last.value","Left":3200,"Top":2400,"Width":2700,"Height":700}]}],"DeletedSections":[],"UnplacedObjects":[]},"SelectedReportSelectionAvailable":true,"SelectedReportSelectionKind":"object","SelectedReportObjectAvailable":true,"SelectedReportObject":{"RecordIndex":6,"Title":"first.value","Subtitle":"field","Properties":[{"Name":"UNIQUEID","Value":"first-field-guid"},{"Name":"HPOS","Value":"1000"},{"Name":"VPOS","Value":"2400"},{"Name":"WIDTH","Value":"3200"},{"Name":"HEIGHT","Value":"700"},{"Name":"EXPR","Value":"first.value"}]},"SelectedReportObjectSectionAvailable":true,"SelectedReportObjectSection":{"Id":"detail_1","Title":"Detail","BandKind":"detail","RecordIndex":42}}}
""";
    }

    private static string BuildAlignTopReportObjectHostResponseJson()
    {
        return """
{"Status":"ok","Document":{"AssetFamily":"report","FieldCount":5,"Objects":[{"RecordIndex":6,"Title":"first.value","Subtitle":"field","Properties":[{"Name":"UNIQUEID","Value":"first-field-guid"},{"Name":"HPOS","Value":"1000"},{"Name":"VPOS","Value":"2400"},{"Name":"WIDTH","Value":"3200"},{"Name":"HEIGHT","Value":"700"},{"Name":"EXPR","Value":"first.value"}]},{"RecordIndex":7,"Title":"middle.value","Subtitle":"field","Properties":[{"Name":"UNIQUEID","Value":"middle-field-guid"},{"Name":"HPOS","Value":"2100"},{"Name":"VPOS","Value":"2400"},{"Name":"WIDTH","Value":"3200"},{"Name":"HEIGHT","Value":"700"},{"Name":"EXPR","Value":"middle.value"}]},{"RecordIndex":8,"Title":"last.value","Subtitle":"field","Properties":[{"Name":"UNIQUEID","Value":"last-field-guid"},{"Name":"HPOS","Value":"3200"},{"Name":"VPOS","Value":"2400"},{"Name":"WIDTH","Value":"2700"},{"Name":"HEIGHT","Value":"700"},{"Name":"EXPR","Value":"last.value"}]}],"ReportLayout":{"PreviewBoundsAvailable":true,"PreviewBoundsLeft":1000,"PreviewBoundsTop":2400,"PreviewBoundsRight":5900,"PreviewBoundsBottom":3100,"PreviewBoundsWidth":4900,"PreviewBoundsHeight":700,"Sections":[{"Id":"detail_1","Title":"Detail","BandKind":"detail","RecordIndex":42,"Top":2000,"Height":5000,"Objects":[{"RecordIndex":6,"ObjectKind":"field","Title":"first.value","Expression":"first.value","Left":1000,"Top":2400,"Width":3200,"Height":700},{"RecordIndex":7,"ObjectKind":"field","Title":"middle.value","Expression":"middle.value","Left":2100,"Top":2400,"Width":3200,"Height":700},{"RecordIndex":8,"ObjectKind":"field","Title":"last.value","Expression":"last.value","Left":3200,"Top":2400,"Width":2700,"Height":700}]}],"DeletedSections":[],"UnplacedObjects":[]},"SelectedReportSelectionAvailable":true,"SelectedReportSelectionKind":"object","SelectedReportObjectAvailable":true,"SelectedReportObject":{"RecordIndex":6,"Title":"first.value","Subtitle":"field","Properties":[{"Name":"UNIQUEID","Value":"first-field-guid"},{"Name":"HPOS","Value":"1000"},{"Name":"VPOS","Value":"2400"},{"Name":"WIDTH","Value":"3200"},{"Name":"HEIGHT","Value":"700"},{"Name":"EXPR","Value":"first.value"}]},"SelectedReportObjectSectionAvailable":true,"SelectedReportObjectSection":{"Id":"detail_1","Title":"Detail","BandKind":"detail","RecordIndex":42}}}
""";
    }

    private static string BuildResizeToAnchorReportObjectHostResponseJson()
    {
        return """
{"Status":"ok","Document":{"AssetFamily":"report","FieldCount":5,"Objects":[{"RecordIndex":6,"Title":"first.value","Subtitle":"field","Properties":[{"Name":"UNIQUEID","Value":"first-field-guid"},{"Name":"HPOS","Value":"1000"},{"Name":"VPOS","Value":"2400"},{"Name":"WIDTH","Value":"3200"},{"Name":"HEIGHT","Value":"700"},{"Name":"EXPR","Value":"first.value"}]},{"RecordIndex":7,"Title":"middle.value","Subtitle":"field","Properties":[{"Name":"UNIQUEID","Value":"middle-field-guid"},{"Name":"HPOS","Value":"2100"},{"Name":"VPOS","Value":"2400"},{"Name":"WIDTH","Value":"3200"},{"Name":"HEIGHT","Value":"700"},{"Name":"EXPR","Value":"middle.value"}]}],"ReportLayout":{"PreviewBoundsAvailable":true,"PreviewBoundsLeft":1000,"PreviewBoundsTop":2400,"PreviewBoundsRight":5300,"PreviewBoundsBottom":3100,"PreviewBoundsWidth":4300,"PreviewBoundsHeight":700,"Sections":[{"Id":"detail_1","Title":"Detail","BandKind":"detail","RecordIndex":42,"Top":2000,"Height":5000,"Objects":[{"RecordIndex":6,"ObjectKind":"field","Title":"first.value","Expression":"first.value","Left":1000,"Top":2400,"Width":3200,"Height":700},{"RecordIndex":7,"ObjectKind":"field","Title":"middle.value","Expression":"middle.value","Left":2100,"Top":2400,"Width":3200,"Height":700}]}],"DeletedSections":[],"UnplacedObjects":[]},"SelectedReportSelectionAvailable":true,"SelectedReportSelectionKind":"object","SelectedReportObjectAvailable":true,"SelectedReportObject":{"RecordIndex":6,"Title":"first.value","Subtitle":"field","Properties":[{"Name":"UNIQUEID","Value":"first-field-guid"},{"Name":"HPOS","Value":"1000"},{"Name":"VPOS","Value":"2400"},{"Name":"WIDTH","Value":"3200"},{"Name":"HEIGHT","Value":"700"},{"Name":"EXPR","Value":"first.value"}]},"SelectedReportObjectSectionAvailable":true,"SelectedReportObjectSection":{"Id":"detail_1","Title":"Detail","BandKind":"detail","RecordIndex":42}}}
""";
    }

    private static string BuildResizeToAnchorWidthReportObjectHostResponseJson()
    {
        return """
{"Status":"ok","Document":{"AssetFamily":"report","FieldCount":5,"Objects":[{"RecordIndex":6,"Title":"first.value","Subtitle":"field","Properties":[{"Name":"UNIQUEID","Value":"first-field-guid"},{"Name":"HPOS","Value":"1000"},{"Name":"VPOS","Value":"2400"},{"Name":"WIDTH","Value":"3200"},{"Name":"HEIGHT","Value":"700"},{"Name":"EXPR","Value":"first.value"}]},{"RecordIndex":7,"Title":"middle.value","Subtitle":"field","Properties":[{"Name":"UNIQUEID","Value":"middle-field-guid"},{"Name":"HPOS","Value":"2100"},{"Name":"VPOS","Value":"2400"},{"Name":"WIDTH","Value":"3200"},{"Name":"HEIGHT","Value":"900"},{"Name":"EXPR","Value":"middle.value"}]}],"ReportLayout":{"PreviewBoundsAvailable":true,"PreviewBoundsLeft":1000,"PreviewBoundsTop":2400,"PreviewBoundsRight":5300,"PreviewBoundsBottom":3300,"PreviewBoundsWidth":4300,"PreviewBoundsHeight":900,"Sections":[{"Id":"detail_1","Title":"Detail","BandKind":"detail","RecordIndex":42,"Top":2000,"Height":5000,"Objects":[{"RecordIndex":6,"ObjectKind":"field","Title":"first.value","Expression":"first.value","Left":1000,"Top":2400,"Width":3200,"Height":700},{"RecordIndex":7,"ObjectKind":"field","Title":"middle.value","Expression":"middle.value","Left":2100,"Top":2400,"Width":3200,"Height":900}]}],"DeletedSections":[],"UnplacedObjects":[]},"SelectedReportSelectionAvailable":true,"SelectedReportSelectionKind":"object","SelectedReportObjectAvailable":true,"SelectedReportObject":{"RecordIndex":6,"Title":"first.value","Subtitle":"field","Properties":[{"Name":"UNIQUEID","Value":"first-field-guid"},{"Name":"HPOS","Value":"1000"},{"Name":"VPOS","Value":"2400"},{"Name":"WIDTH","Value":"3200"},{"Name":"HEIGHT","Value":"700"},{"Name":"EXPR","Value":"first.value"}]},"SelectedReportObjectSectionAvailable":true,"SelectedReportObjectSection":{"Id":"detail_1","Title":"Detail","BandKind":"detail","RecordIndex":42}}}
""";
    }

    private static string BuildResizeToAnchorHeightReportObjectHostResponseJson()
    {
        return """
{"Status":"ok","Document":{"AssetFamily":"report","FieldCount":5,"Objects":[{"RecordIndex":6,"Title":"first.value","Subtitle":"field","Properties":[{"Name":"UNIQUEID","Value":"first-field-guid"},{"Name":"HPOS","Value":"1000"},{"Name":"VPOS","Value":"2400"},{"Name":"WIDTH","Value":"3200"},{"Name":"HEIGHT","Value":"700"},{"Name":"EXPR","Value":"first.value"}]},{"RecordIndex":7,"Title":"middle.value","Subtitle":"field","Properties":[{"Name":"UNIQUEID","Value":"middle-field-guid"},{"Name":"HPOS","Value":"2100"},{"Name":"VPOS","Value":"2400"},{"Name":"WIDTH","Value":"4200"},{"Name":"HEIGHT","Value":"700"},{"Name":"EXPR","Value":"middle.value"}]}],"ReportLayout":{"PreviewBoundsAvailable":true,"PreviewBoundsLeft":1000,"PreviewBoundsTop":2400,"PreviewBoundsRight":6300,"PreviewBoundsBottom":3100,"PreviewBoundsWidth":5300,"PreviewBoundsHeight":700,"Sections":[{"Id":"detail_1","Title":"Detail","BandKind":"detail","RecordIndex":42,"Top":2000,"Height":5000,"Objects":[{"RecordIndex":6,"ObjectKind":"field","Title":"first.value","Expression":"first.value","Left":1000,"Top":2400,"Width":3200,"Height":700},{"RecordIndex":7,"ObjectKind":"field","Title":"middle.value","Expression":"middle.value","Left":2100,"Top":2400,"Width":4200,"Height":700}]}],"DeletedSections":[],"UnplacedObjects":[]},"SelectedReportSelectionAvailable":true,"SelectedReportSelectionKind":"object","SelectedReportObjectAvailable":true,"SelectedReportObject":{"RecordIndex":6,"Title":"first.value","Subtitle":"field","Properties":[{"Name":"UNIQUEID","Value":"first-field-guid"},{"Name":"HPOS","Value":"1000"},{"Name":"VPOS","Value":"2400"},{"Name":"WIDTH","Value":"3200"},{"Name":"HEIGHT","Value":"700"},{"Name":"EXPR","Value":"first.value"}]},"SelectedReportObjectSectionAvailable":true,"SelectedReportObjectSection":{"Id":"detail_1","Title":"Detail","BandKind":"detail","RecordIndex":42}}}
""";
    }

    private static string BuildDistributeHorizontalReportObjectHostResponseJson()
    {
        return """
{"Status":"ok","Document":{"AssetFamily":"report","FieldCount":5,"Objects":[{"RecordIndex":6,"Title":"first.value","Subtitle":"field","Properties":[{"Name":"UNIQUEID","Value":"first-field-guid"},{"Name":"HPOS","Value":"1000"},{"Name":"VPOS","Value":"2400"},{"Name":"WIDTH","Value":"3200"},{"Name":"HEIGHT","Value":"700"},{"Name":"EXPR","Value":"first.value"}]},{"RecordIndex":7,"Title":"middle.value","Subtitle":"field","Properties":[{"Name":"UNIQUEID","Value":"middle-field-guid"},{"Name":"HPOS","Value":"2100"},{"Name":"VPOS","Value":"2400"},{"Name":"WIDTH","Value":"3200"},{"Name":"HEIGHT","Value":"700"},{"Name":"EXPR","Value":"middle.value"}]},{"RecordIndex":8,"Title":"last.value","Subtitle":"field","Properties":[{"Name":"UNIQUEID","Value":"last-field-guid"},{"Name":"HPOS","Value":"3200"},{"Name":"VPOS","Value":"2400"},{"Name":"WIDTH","Value":"2700"},{"Name":"HEIGHT","Value":"700"},{"Name":"EXPR","Value":"last.value"}]}],"ReportLayout":{"PreviewBoundsAvailable":true,"PreviewBoundsLeft":1000,"PreviewBoundsTop":2400,"PreviewBoundsRight":5900,"PreviewBoundsBottom":3100,"PreviewBoundsWidth":4900,"PreviewBoundsHeight":700,"Sections":[{"Id":"detail_1","Title":"Detail","BandKind":"detail","RecordIndex":42,"Top":2000,"Height":5000,"Objects":[{"RecordIndex":6,"ObjectKind":"field","Title":"first.value","Expression":"first.value","Left":1000,"Top":2400,"Width":3200,"Height":700},{"RecordIndex":7,"ObjectKind":"field","Title":"middle.value","Expression":"middle.value","Left":2100,"Top":2400,"Width":3200,"Height":700},{"RecordIndex":8,"ObjectKind":"field","Title":"last.value","Expression":"last.value","Left":3200,"Top":2400,"Width":2700,"Height":700}]}],"DeletedSections":[],"UnplacedObjects":[]},"SelectedReportSelectionAvailable":true,"SelectedReportSelectionKind":"object","SelectedReportObjectAvailable":true,"SelectedReportObject":{"RecordIndex":7,"Title":"middle.value","Subtitle":"field","Properties":[{"Name":"UNIQUEID","Value":"middle-field-guid"},{"Name":"HPOS","Value":"2100"},{"Name":"VPOS","Value":"2400"},{"Name":"WIDTH","Value":"3200"},{"Name":"HEIGHT","Value":"700"},{"Name":"EXPR","Value":"middle.value"}]},"SelectedReportObjectSectionAvailable":true,"SelectedReportObjectSection":{"Id":"detail_1","Title":"Detail","BandKind":"detail","RecordIndex":42}}}
""";
    }

    private static string BuildDistributeVerticalReportObjectHostResponseJson()
    {
        return """
{"Status":"ok","Document":{"AssetFamily":"report","FieldCount":5,"Objects":[{"RecordIndex":6,"Title":"first.value","Subtitle":"field","Properties":[{"Name":"UNIQUEID","Value":"first-field-guid"},{"Name":"HPOS","Value":"1000"},{"Name":"VPOS","Value":"2400"},{"Name":"WIDTH","Value":"3200"},{"Name":"HEIGHT","Value":"700"},{"Name":"EXPR","Value":"first.value"}]},{"RecordIndex":7,"Title":"middle.value","Subtitle":"field","Properties":[{"Name":"UNIQUEID","Value":"middle-field-guid"},{"Name":"HPOS","Value":"1600"},{"Name":"VPOS","Value":"3300"},{"Name":"WIDTH","Value":"3200"},{"Name":"HEIGHT","Value":"700"},{"Name":"EXPR","Value":"middle.value"}]},{"RecordIndex":8,"Title":"last.value","Subtitle":"field","Properties":[{"Name":"UNIQUEID","Value":"last-field-guid"},{"Name":"HPOS","Value":"3200"},{"Name":"VPOS","Value":"4200"},{"Name":"WIDTH","Value":"2700"},{"Name":"HEIGHT","Value":"700"},{"Name":"EXPR","Value":"last.value"}]}],"ReportLayout":{"PreviewBoundsAvailable":true,"PreviewBoundsLeft":1000,"PreviewBoundsTop":2400,"PreviewBoundsRight":5900,"PreviewBoundsBottom":4900,"PreviewBoundsWidth":4900,"PreviewBoundsHeight":2500,"Sections":[{"Id":"detail_1","Title":"Detail","BandKind":"detail","RecordIndex":42,"Top":2000,"Height":5000,"Objects":[{"RecordIndex":6,"ObjectKind":"field","Title":"first.value","Expression":"first.value","Left":1000,"Top":2400,"Width":3200,"Height":700},{"RecordIndex":7,"ObjectKind":"field","Title":"middle.value","Expression":"middle.value","Left":1600,"Top":3300,"Width":3200,"Height":700},{"RecordIndex":8,"ObjectKind":"field","Title":"last.value","Expression":"last.value","Left":3200,"Top":4200,"Width":2700,"Height":700}]}],"DeletedSections":[],"UnplacedObjects":[]},"SelectedReportSelectionAvailable":true,"SelectedReportSelectionKind":"object","SelectedReportObjectAvailable":true,"SelectedReportObject":{"RecordIndex":7,"Title":"middle.value","Subtitle":"field","Properties":[{"Name":"UNIQUEID","Value":"middle-field-guid"},{"Name":"HPOS","Value":"1600"},{"Name":"VPOS","Value":"3300"},{"Name":"WIDTH","Value":"3200"},{"Name":"HEIGHT","Value":"700"},{"Name":"EXPR","Value":"middle.value"}]},"SelectedReportObjectSectionAvailable":true,"SelectedReportObjectSection":{"Id":"detail_1","Title":"Detail","BandKind":"detail","RecordIndex":42}}}
""";
    }

    private static string BuildSnapToGridReportObjectHostResponseJson()
    {
        return """
{"Status":"ok","Document":{"AssetFamily":"report","FieldCount":5,"Objects":[{"RecordIndex":7,"Title":"snap.value","Subtitle":"field","Properties":[{"Name":"UNIQUEID","Value":"snap-field-guid"},{"Name":"HPOS","Value":"1008"},{"Name":"VPOS","Value":"2400"},{"Name":"WIDTH","Value":"3200"},{"Name":"HEIGHT","Value":"700"},{"Name":"EXPR","Value":"snap.value"}]}],"ReportLayout":{"PreviewBoundsAvailable":true,"PreviewBoundsLeft":1008,"PreviewBoundsTop":2400,"PreviewBoundsRight":4208,"PreviewBoundsBottom":3100,"PreviewBoundsWidth":3200,"PreviewBoundsHeight":700,"Settings":[{"Name":"GRIDH","Value":"12","RecordIndex":0},{"Name":"GRIDV","Value":"12","RecordIndex":0}],"Sections":[{"Id":"detail_1","Title":"Detail","BandKind":"detail","RecordIndex":42,"Top":2000,"Height":5000,"Objects":[{"RecordIndex":7,"ObjectKind":"field","Title":"snap.value","Expression":"snap.value","Left":1008,"Top":2400,"Width":3200,"Height":700}]}],"DeletedSections":[],"UnplacedObjects":[]},"SelectedReportSelectionAvailable":true,"SelectedReportSelectionKind":"object","SelectedReportObjectAvailable":true,"SelectedReportObject":{"RecordIndex":7,"Title":"snap.value","Subtitle":"field","Properties":[{"Name":"UNIQUEID","Value":"snap-field-guid"},{"Name":"HPOS","Value":"1008"},{"Name":"VPOS","Value":"2400"},{"Name":"WIDTH","Value":"3200"},{"Name":"HEIGHT","Value":"700"},{"Name":"EXPR","Value":"snap.value"}]},"SelectedReportObjectSectionAvailable":true,"SelectedReportObjectSection":{"Id":"detail_1","Title":"Detail","BandKind":"detail","RecordIndex":42}}}
""";
    }

    private static string BuildSnapHorizontalReportObjectHostResponseJson()
    {
        return """
{"Status":"ok","Document":{"AssetFamily":"report","FieldCount":5,"Objects":[{"RecordIndex":7,"Title":"snap.value","Subtitle":"field","Properties":[{"Name":"UNIQUEID","Value":"snap-field-guid"},{"Name":"HPOS","Value":"1008"},{"Name":"VPOS","Value":"2405"},{"Name":"WIDTH","Value":"3200"},{"Name":"HEIGHT","Value":"700"},{"Name":"EXPR","Value":"snap.value"}]}],"ReportLayout":{"PreviewBoundsAvailable":true,"PreviewBoundsLeft":1008,"PreviewBoundsTop":2405,"PreviewBoundsRight":4208,"PreviewBoundsBottom":3105,"PreviewBoundsWidth":3200,"PreviewBoundsHeight":700,"Settings":[{"Name":"GRIDH","Value":"12","RecordIndex":0},{"Name":"GRIDV","Value":"12","RecordIndex":0}],"Sections":[{"Id":"detail_1","Title":"Detail","BandKind":"detail","RecordIndex":42,"Top":2000,"Height":5000,"Objects":[{"RecordIndex":7,"ObjectKind":"field","Title":"snap.value","Expression":"snap.value","Left":1008,"Top":2405,"Width":3200,"Height":700}]}],"DeletedSections":[],"UnplacedObjects":[]},"SelectedReportSelectionAvailable":true,"SelectedReportSelectionKind":"object","SelectedReportObjectAvailable":true,"SelectedReportObject":{"RecordIndex":7,"Title":"snap.value","Subtitle":"field","Properties":[{"Name":"UNIQUEID","Value":"snap-field-guid"},{"Name":"HPOS","Value":"1008"},{"Name":"VPOS","Value":"2405"},{"Name":"WIDTH","Value":"3200"},{"Name":"HEIGHT","Value":"700"},{"Name":"EXPR","Value":"snap.value"}]},"SelectedReportObjectSectionAvailable":true,"SelectedReportObjectSection":{"Id":"detail_1","Title":"Detail","BandKind":"detail","RecordIndex":42}}}
""";
    }

    private static string BuildSnapVerticalReportObjectHostResponseJson()
    {
        return """
{"Status":"ok","Document":{"AssetFamily":"report","FieldCount":5,"Objects":[{"RecordIndex":7,"Title":"snap.value","Subtitle":"field","Properties":[{"Name":"UNIQUEID","Value":"snap-field-guid"},{"Name":"HPOS","Value":"1001"},{"Name":"VPOS","Value":"2400"},{"Name":"WIDTH","Value":"3200"},{"Name":"HEIGHT","Value":"700"},{"Name":"EXPR","Value":"snap.value"}]}],"ReportLayout":{"PreviewBoundsAvailable":true,"PreviewBoundsLeft":1001,"PreviewBoundsTop":2400,"PreviewBoundsRight":4201,"PreviewBoundsBottom":3100,"PreviewBoundsWidth":3200,"PreviewBoundsHeight":700,"Settings":[{"Name":"GRIDH","Value":"12","RecordIndex":0},{"Name":"GRIDV","Value":"12","RecordIndex":0}],"Sections":[{"Id":"detail_1","Title":"Detail","BandKind":"detail","RecordIndex":42,"Top":2000,"Height":5000,"Objects":[{"RecordIndex":7,"ObjectKind":"field","Title":"snap.value","Expression":"snap.value","Left":1001,"Top":2400,"Width":3200,"Height":700}]}],"DeletedSections":[],"UnplacedObjects":[]},"SelectedReportSelectionAvailable":true,"SelectedReportSelectionKind":"object","SelectedReportObjectAvailable":true,"SelectedReportObject":{"RecordIndex":7,"Title":"snap.value","Subtitle":"field","Properties":[{"Name":"UNIQUEID","Value":"snap-field-guid"},{"Name":"HPOS","Value":"1001"},{"Name":"VPOS","Value":"2400"},{"Name":"WIDTH","Value":"3200"},{"Name":"HEIGHT","Value":"700"},{"Name":"EXPR","Value":"snap.value"}]},"SelectedReportObjectSectionAvailable":true,"SelectedReportObjectSection":{"Id":"detail_1","Title":"Detail","BandKind":"detail","RecordIndex":42}}}
""";
    }

    private static string BuildDuplicateReportObjectHostResponseJson()
    {
        return """
{"Status":"ok","Document":{"AssetFamily":"report","FieldCount":5,"Objects":[{"RecordIndex":7,"Title":"middle.value","Subtitle":"field","Properties":[{"Name":"UNIQUEID","Value":"middle-field-guid"},{"Name":"HPOS","Value":"1200"},{"Name":"VPOS","Value":"2600"},{"Name":"WIDTH","Value":"4000"},{"Name":"HEIGHT","Value":"600"},{"Name":"EXPR","Value":"middle.value"}]},{"RecordIndex":10,"Title":"middle.value.copy","Subtitle":"field","Properties":[{"Name":"UNIQUEID","Value":"middle-copy-guid"},{"Name":"HPOS","Value":"2200"},{"Name":"VPOS","Value":"2600"},{"Name":"WIDTH","Value":"4000"},{"Name":"HEIGHT","Value":"600"},{"Name":"EXPR","Value":"middle.value"}]}],"ReportLayout":{"PreviewBoundsAvailable":true,"PreviewBoundsLeft":1200,"PreviewBoundsTop":2600,"PreviewBoundsRight":6200,"PreviewBoundsBottom":3200,"PreviewBoundsWidth":5000,"PreviewBoundsHeight":600,"Sections":[{"Id":"detail_1","Title":"Detail","BandKind":"detail","RecordIndex":42,"Top":2000,"Height":5000,"Objects":[{"RecordIndex":7,"ObjectKind":"field","Title":"middle.value","Expression":"middle.value","Left":1200,"Top":2600,"Width":4000,"Height":600},{"RecordIndex":10,"ObjectKind":"field","Title":"middle.value.copy","Expression":"middle.value","Left":2200,"Top":2600,"Width":4000,"Height":600}]}],"DeletedSections":[],"UnplacedObjects":[]}}}
""";
    }

    private static string BuildDeletedReportSectionUpdateHostResponseJson()
    {
        return """
{"Status":"ok","Document":{"AssetFamily":"report","FieldCount":5,"Objects":[{"RecordIndex":6,"Title":"customer.company","Subtitle":"field","Properties":[{"Name":"HPOS","Value":"1200"},{"Name":"VPOS","Value":"2600"},{"Name":"WIDTH","Value":"4000"},{"Name":"HEIGHT","Value":"500"},{"Name":"EXPR","Value":"customer.company"}]},{"RecordIndex":13,"Deleted":true,"Title":"deleted.footer.total","Subtitle":"field","Properties":[{"Name":"HPOS","Value":"1400"},{"Name":"VPOS","Value":"9700"},{"Name":"WIDTH","Value":"3600"},{"Name":"HEIGHT","Value":"600"},{"Name":"EXPR","Value":"deleted.footer.total"}]}],"ReportLayout":{"Sections":[{"Id":"detail_1","Title":"Detail","BandKind":"detail","RecordIndex":42,"Top":2000,"Height":5000,"Objects":[{"RecordIndex":6,"ObjectKind":"field","Title":"customer.company","Expression":"customer.company","Left":1200,"Top":2600,"Width":4000,"Height":500}]}],"DeletedSections":[{"Id":"deleted_footer","Title":"Summary","BandKind":"summary","RecordIndex":51,"Deleted":true,"Top":9300,"Height":1400,"Objects":[{"RecordIndex":13,"ObjectKind":"field","Title":"deleted.footer.total","Expression":"deleted.footer.total","Left":1400,"Top":9700,"Width":3600,"Height":600}]}],"UnplacedObjects":[]}}}
""";
    }

    private static string BuildDeletedLabelSectionUpdateHostResponseJson()
    {
        return """
{"Status":"ok","Document":{"AssetFamily":"label","FieldCount":5,"Objects":[{"RecordIndex":6,"Title":"customer.company","Subtitle":"label","Properties":[{"Name":"HPOS","Value":"1200"},{"Name":"VPOS","Value":"2600"},{"Name":"WIDTH","Value":"4000"},{"Name":"HEIGHT","Value":"500"},{"Name":"EXPR","Value":"customer.company"}]},{"RecordIndex":13,"Deleted":true,"Title":"deleted.footer.total","Subtitle":"label","Properties":[{"Name":"HPOS","Value":"1400"},{"Name":"VPOS","Value":"9700"},{"Name":"WIDTH","Value":"3600"},{"Name":"HEIGHT","Value":"600"},{"Name":"EXPR","Value":"deleted.footer.total"}]}],"ReportLayout":{"IsLabel":true,"Sections":[{"Id":"detail_1","Title":"Detail","BandKind":"detail","RecordIndex":42,"Top":2000,"Height":5000,"Objects":[{"RecordIndex":6,"ObjectKind":"label","Title":"customer.company","Expression":"customer.company","Left":1200,"Top":2600,"Width":4000,"Height":500}]}],"DeletedSections":[{"Id":"deleted_footer","Title":"Summary","BandKind":"summary","RecordIndex":51,"Deleted":true,"Top":9300,"Height":1400,"Objects":[{"RecordIndex":13,"ObjectKind":"label","Title":"deleted.footer.total","Expression":"deleted.footer.total","Left":1400,"Top":9700,"Width":3600,"Height":600}]}],"UnplacedObjects":[]}}}
""";
    }

    private static string CreateFakeStudioHostScriptPath(string tempRoot, bool? isWindowsOverride = null)
    {
        var extension = IsWindowsPlatform(isWindowsOverride) ? ".cmd" : ".sh";
        return Path.Combine(tempRoot, "fake-studio-host" + extension);
    }

    private static void CreateFakeStudioHostScript(string scriptPath, string responseJson, bool? isWindowsOverride = null)
    {
        var isWindows = IsWindowsPlatform(isWindowsOverride);
        var script = isWindows
            ? BuildFakeStudioHostBatchScript(responseJson)
            : BuildFakeStudioHostPosixScript(responseJson);

        File.WriteAllText(scriptPath, script);
        if (!isWindows)
        {
            MakeExecutable(scriptPath);
        }
    }

    private static bool IsWindowsPlatform(bool? isWindowsOverride = null)
    {
        return isWindowsOverride ?? Path.DirectorySeparatorChar == '\\';
    }

    private static string BuildFakeStudioHostPosixScript(string responseJson)
    {
        return string.Join(
            "\n",
            "#!/usr/bin/env bash",
            "set -e",
            "log_file=\"${COPPERFIN_SMOKE_LOG:?}\"",
            "{",
            "  printf '%s\\n' 'BEGIN'",
            "  for arg in \"$@\"; do",
            "    printf '%s\\n' \"$arg\"",
            "  done",
            "} >> \"$log_file\"",
            "cat <<'JSON'",
            responseJson,
            "JSON",
            string.Empty);
    }

    private static string BuildFakeStudioHostBatchScript(string responseJson)
    {
        var lines = new List<string>
        {
            "@echo off",
            "if \"%COPPERFIN_SMOKE_LOG%\"==\"\" exit /b 1",
            ">> \"%COPPERFIN_SMOKE_LOG%\" echo BEGIN",
            ":args",
            "if \"%~1\"==\"\" goto output",
            ">> \"%COPPERFIN_SMOKE_LOG%\" echo %~1",
            "shift",
            "goto args",
            ":output"
        };

        foreach (var line in responseJson.Replace("\r\n", "\n").Split('\n'))
        {
            lines.Add("echo(" + EscapeBatchEchoLine(line));
        }

        lines.Add("exit /b 0");
        lines.Add(string.Empty);
        return string.Join("\r\n", lines);
    }

    private static string EscapeBatchEchoLine(string line)
    {
        return line
            .Replace("^", "^^")
            .Replace("&", "^&")
            .Replace("|", "^|")
            .Replace("<", "^<")
            .Replace(">", "^>")
            .Replace("(", "^(")
            .Replace(")", "^)")
            .Replace("%", "%%");
    }

    private static void MakeExecutable(string path)
    {
        var processResult = CopperfinProcessRunner.Run(
            new ProcessStartInfo
            {
                FileName = "/bin/chmod",
                Arguments = $"+x \"{path}\"",
                UseShellExecute = false,
                RedirectStandardError = true,
                RedirectStandardOutput = true,
                CreateNoWindow = true
            });
        if (!processResult.Started)
        {
            throw new InvalidOperationException("Could not start chmod for the fake Studio host script.");
        }
        if (processResult.ExitCode != 0)
        {
            throw new InvalidOperationException(
                string.IsNullOrWhiteSpace(processResult.StandardError)
                    ? processResult.StandardOutput
                    : processResult.StandardError);
        }
    }

    private static string BuildGuidanceText(CopperfinAssetEditorControl control, string assetFamily)
    {
        var method = typeof(CopperfinAssetEditorControl).GetMethod("BuildGuidanceText", BindingFlags.Instance | BindingFlags.NonPublic);
        if (method is null)
        {
            throw new InvalidOperationException("Could not find CopperfinAssetEditorControl guidance smoke hook.");
        }

        return (string)(method.Invoke(control, new object[] { assetFamily }) ?? string.Empty);
    }

    private static CopperfinStudioSnapshotDocument BuildStatusSmokeSnapshot()
    {
        return new CopperfinStudioSnapshotDocument
        {
            FieldCount = 7,
            IndexCount = 3,
            CommandUndoAvailable = true,
            CommandUndoLabel = "Reordenar",
            Objects = new List<CopperfinStudioSnapshotObject>
            {
                new(),
                new()
            }
        };
    }

    private static string InvokeAssetEditorString(CopperfinAssetEditorControl control, string methodName, params object[] args)
    {
        var method = ResolveNonPublicInstanceMethod(typeof(CopperfinAssetEditorControl), methodName, args);
        return (string)(method.Invoke(control, args) ?? string.Empty);
    }

    private static void InvokeAssetEditorVoid(CopperfinAssetEditorControl control, string methodName, params object?[] args)
    {
        var method = ResolveNonPublicInstanceMethod(typeof(CopperfinAssetEditorControl), methodName, args);
        method.Invoke(control, args);
    }

    private static object? InvokeAssetEditorObject(CopperfinAssetEditorControl control, string methodName, params object?[] args)
    {
        var method = ResolveNonPublicInstanceMethod(typeof(CopperfinAssetEditorControl), methodName, args);
        return method.Invoke(control, args);
    }

    private static MethodInfo ResolveNonPublicInstanceMethod(Type targetType, string methodName, object?[] args)
    {
        var candidates = targetType
            .GetMethods(BindingFlags.Instance | BindingFlags.NonPublic)
            .Where(method => string.Equals(method.Name, methodName, StringComparison.Ordinal))
            .Where(method => method.GetParameters().Length == args.Length)
            .Where(method => MethodParametersMatch(method.GetParameters(), args))
            .ToList();

        if (candidates.Count == 1)
        {
            return candidates[0];
        }

        if (candidates.Count == 0)
        {
            throw new InvalidOperationException($"Could not find {targetType.Name} smoke hook {methodName} for the supplied argument shape.");
        }

        throw new InvalidOperationException($"Ambiguous {targetType.Name} smoke hook {methodName} for the supplied argument shape.");
    }

    private static bool MethodParametersMatch(ParameterInfo[] parameters, object?[] args)
    {
        for (var index = 0; index < parameters.Length; index++)
        {
            if (!MethodParameterMatches(parameters[index].ParameterType, args[index]))
            {
                return false;
            }
        }

        return true;
    }

    private static bool MethodParameterMatches(Type parameterType, object? argument)
    {
        if (argument is null)
        {
            return !parameterType.IsValueType || Nullable.GetUnderlyingType(parameterType) is not null;
        }

        return parameterType.IsInstanceOfType(argument);
    }

    private static string InvokeDesignSurfaceString(CopperfinDesignSurfaceControl surface, string methodName, params object[] args)
    {
        var method = typeof(CopperfinDesignSurfaceControl).GetMethod(methodName, BindingFlags.Instance | BindingFlags.NonPublic);
        if (method is null)
        {
            throw new InvalidOperationException($"Could not find CopperfinDesignSurfaceControl smoke hook {methodName}.");
        }

        return (string)(method.Invoke(surface, args) ?? string.Empty);
    }

    private static float InvokeDesignSurfaceFloat(CopperfinDesignSurfaceControl surface, string methodName, params object[] args)
    {
        var method = typeof(CopperfinDesignSurfaceControl).GetMethod(methodName, BindingFlags.Instance | BindingFlags.NonPublic);
        if (method is null)
        {
            throw new InvalidOperationException($"Could not find CopperfinDesignSurfaceControl smoke hook {methodName}.");
        }

        return method.Invoke(surface, args) is float value
            ? value
            : throw new InvalidOperationException($"Could not read float result from CopperfinDesignSurfaceControl smoke hook {methodName}.");
    }

    private static void ClickDesignSurface(CopperfinDesignSurfaceControl surface, Point location)
    {
        var method = typeof(CopperfinDesignSurfaceControl).GetMethod("OnMouseDown", BindingFlags.Instance | BindingFlags.NonPublic);
        if (method is null)
        {
            throw new InvalidOperationException("Could not find shared report design-surface mouse hook.");
        }

        method.Invoke(surface, new object[] { new MouseEventArgs(MouseButtons.Left, 1, location.X, location.Y, 0) });
    }

    private static void DragDesignSurface(CopperfinDesignSurfaceControl surface, Point start, int deltaX, int deltaY)
    {
        var mouseDown = typeof(CopperfinDesignSurfaceControl).GetMethod("OnMouseDown", BindingFlags.Instance | BindingFlags.NonPublic);
        var mouseMove = typeof(CopperfinDesignSurfaceControl).GetMethod("OnMouseMove", BindingFlags.Instance | BindingFlags.NonPublic);
        var mouseUp = typeof(CopperfinDesignSurfaceControl).GetMethod("OnMouseUp", BindingFlags.Instance | BindingFlags.NonPublic);
        if (mouseDown is null || mouseMove is null || mouseUp is null)
        {
            throw new InvalidOperationException("Could not find shared report design-surface drag hooks.");
        }

        mouseDown.Invoke(surface, new object[] { new MouseEventArgs(MouseButtons.Left, 1, start.X, start.Y, 0) });
        mouseMove.Invoke(surface, new object[] { new MouseEventArgs(MouseButtons.Left, 0, start.X + deltaX, start.Y + deltaY, 0) });
        mouseUp.Invoke(surface, new object[] { new MouseEventArgs(MouseButtons.Left, 1, start.X + deltaX, start.Y + deltaY, 0) });
    }

    private static void RenderDesignSurface(CopperfinDesignSurfaceControl surface)
    {
        using var bitmap = new Bitmap(surface.Width, surface.Height);
        surface.DrawToBitmap(bitmap, new Rectangle(0, 0, bitmap.Width, bitmap.Height));
    }

    private static Rectangle ReadSurfaceObjectRectangle(CopperfinDesignSurfaceControl surface, int index)
    {
        var field = typeof(CopperfinDesignSurfaceControl).GetField("objects", BindingFlags.Instance | BindingFlags.NonPublic);
        if (field?.GetValue(surface) is not System.Collections.IList objects || objects.Count <= index)
        {
            throw new InvalidOperationException("Could not read shared report surface objects.");
        }

        var item = objects[index]!;
        var property = item.GetType().GetProperty("PixelBounds", BindingFlags.Instance | BindingFlags.Public | BindingFlags.NonPublic);
        if (property?.GetValue(item) is not Rectangle value)
        {
            throw new InvalidOperationException("Could not read shared report surface object bounds.");
        }

        return value;
    }

}
