using System;
using System.Windows.Forms;

namespace Copperfin.VisualStudio;

internal static class Program
{
    [STAThread]
    private static void Main(string[] args)
    {
        Application.EnableVisualStyles();
        Application.SetCompatibleTextRenderingDefault(false);

        var initialPath = args.Length > 0 ? args[0] : string.Empty;
        using var form = new StudioMainForm();
        if (!string.IsNullOrWhiteSpace(initialPath))
        {
            form.OpenDocument(initialPath);
        }

        Application.Run(form);
    }
}
