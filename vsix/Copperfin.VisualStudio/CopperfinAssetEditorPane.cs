// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

using System;
using System.Runtime.InteropServices;
using Microsoft.VisualStudio;
using Microsoft.VisualStudio.OLE.Interop;
using Microsoft.VisualStudio.Shell;
using Microsoft.VisualStudio.Shell.Interop;
using Microsoft.VisualStudio.TextManager.Interop;

namespace Copperfin.VisualStudio;

[ComVisible(true)]
internal sealed class CopperfinAssetEditorPane : WindowPane, IVsPersistDocData, IOleCommandTarget
{
    private readonly CopperfinAssetEditorControl control;
    private string documentPath;
    private uint docCookie;

    public CopperfinAssetEditorPane(System.IServiceProvider serviceProvider, string documentPath)
        : base(serviceProvider)
    {
        control = new CopperfinAssetEditorControl(CopperfinLocalization.FromCurrentUiCulture());
        control.OpenDocumentRequested += OpenDocumentInVisualStudio;
        control.OpenDocumentAtLineRequested += OpenDocumentAtLineInVisualStudio;
        this.documentPath = documentPath;
        control.LoadDocument(documentPath);
    }

    public override object Content => control;

    public int Close()
    {
        Dispose();
        return VSConstants.S_OK;
    }

    public int GetGuidEditorType(out Guid pClassID)
    {
        pClassID = new Guid(PackageGuids.EditorFactoryString);
        return VSConstants.S_OK;
    }

    public int IsDocDataDirty(out int pfDirty)
    {
        pfDirty = 0;
        return VSConstants.S_OK;
    }

    public int IsDocDataReloadable(out int pfReloadable)
    {
        pfReloadable = 1;
        return VSConstants.S_OK;
    }

    public int LoadDocData(string pszMkDocument)
    {
        documentPath = pszMkDocument;
        control.LoadDocument(documentPath);
        return VSConstants.S_OK;
    }

    public int OnRegisterDocData(uint docCookie, IVsHierarchy pHierNew, uint itemidNew)
    {
        this.docCookie = docCookie;
        return VSConstants.S_OK;
    }

    public int ReloadDocData(uint grfFlags)
    {
        if (!string.IsNullOrWhiteSpace(documentPath))
        {
            control.LoadDocument(documentPath);
        }

        return VSConstants.S_OK;
    }

    public int RenameDocData(
        uint grfAttribs,
        IVsHierarchy pHierNew,
        uint itemidNew,
        string pszMkDocumentNew)
    {
        documentPath = pszMkDocumentNew;
        control.LoadDocument(documentPath);
        return VSConstants.S_OK;
    }

    public int SaveDocData(VSSAVEFLAGS dwSave, out string pbstrMkDocumentNew, out int pfSaveCanceled)
    {
        pbstrMkDocumentNew = documentPath;
        pfSaveCanceled = 0;
        return VSConstants.S_OK;
    }

    public int SetUntitledDocPath(string pszDocDataPath)
    {
        documentPath = pszDocDataPath;
        control.LoadDocument(documentPath);
        return VSConstants.S_OK;
    }

    private void OpenDocumentInVisualStudio(string path)
    {
        ThreadHelper.ThrowIfNotOnUIThread();
        VsShellUtilities.OpenDocument(
            ServiceProvider.GlobalProvider,
            path,
            Guid.Empty,
            out IVsUIHierarchy _,
            out uint _,
            out IVsWindowFrame _,
            out IVsTextView _);
    }

    private void OpenDocumentAtLineInVisualStudio(string path, int line)
    {
        ThreadHelper.ThrowIfNotOnUIThread();
        VsShellUtilities.OpenDocument(
            ServiceProvider.GlobalProvider,
            path,
            Guid.Empty,
            out IVsUIHierarchy _,
            out uint _,
            out IVsWindowFrame windowFrame,
            out IVsTextView textView);

        textView ??= VsShellUtilities.GetTextView(windowFrame);
        if (textView is null)
        {
            return;
        }

        var targetLine = Math.Max(0, line - 1);
        ErrorHandler.ThrowOnFailure(textView.SetCaretPos(targetLine, 0));
        ErrorHandler.ThrowOnFailure(textView.CenterLines(targetLine, 1));
    }

    public int QueryStatus(ref Guid pguidCmdGroup, uint cCmds, OLECMD[] prgCmds, IntPtr pCmdText)
    {
        if (pguidCmdGroup == VSConstants.GUID_VSStandardCommandSet97 &&
            cCmds > 0 &&
            prgCmds[0].cmdID == (uint)VSConstants.VSStd97CmdID.Undo)
        {
            prgCmds[0].cmdf = (uint)OLECMDF.OLECMDF_SUPPORTED;
            if (control.CanHandleUndoCommand())
            {
                prgCmds[0].cmdf |= (uint)OLECMDF.OLECMDF_ENABLED;
            }

            return VSConstants.S_OK;
        }

        return VSConstants.E_NOTIMPL;
    }

    public int Exec(ref Guid pguidCmdGroup, uint nCmdID, uint nCmdexecopt, IntPtr pvaIn, IntPtr pvaOut)
    {
        if (pguidCmdGroup == VSConstants.GUID_VSStandardCommandSet97 &&
            nCmdID == (uint)VSConstants.VSStd97CmdID.Undo)
        {
            return control.TryHandleUndoCommand() ? VSConstants.S_OK : VSConstants.E_FAIL;
        }

        return VSConstants.E_NOTIMPL;
    }

    protected override void Dispose(bool disposing)
    {
        if (disposing && docCookie != 0)
        {
            docCookie = 0;
        }

        base.Dispose(disposing);
    }
}
