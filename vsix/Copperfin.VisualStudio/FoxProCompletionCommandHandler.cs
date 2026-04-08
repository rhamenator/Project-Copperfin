using System;
using System.Runtime.InteropServices;
using Microsoft.VisualStudio;
using Microsoft.VisualStudio.Editor;
using Microsoft.VisualStudio.Language.Intellisense;
using Microsoft.VisualStudio.OLE.Interop;
using Microsoft.VisualStudio.Shell;
using Microsoft.VisualStudio.Text.Editor;
using Microsoft.VisualStudio.TextManager.Interop;

namespace Copperfin.VisualStudio;

internal sealed class FoxProCompletionCommandHandler : IOleCommandTarget
{
    private readonly IVsTextView textViewAdapter;
    private readonly ITextView textView;
    private readonly ICompletionBroker completionBroker;
    private IOleCommandTarget? nextCommandTarget;
    private ICompletionSession? completionSession;

    public FoxProCompletionCommandHandler(
        IVsTextView textViewAdapter,
        ITextView textView,
        ICompletionBroker completionBroker)
    {
        ThreadHelper.ThrowIfNotOnUIThread();
        this.textViewAdapter = textViewAdapter;
        this.textView = textView;
        this.completionBroker = completionBroker;
        ErrorHandler.ThrowOnFailure(textViewAdapter.AddCommandFilter(this, out nextCommandTarget));
    }

    public int QueryStatus(ref Guid pguidCmdGroup, uint cCmds, OLECMD[] prgCmds, IntPtr pCmdText)
    {
        ThreadHelper.ThrowIfNotOnUIThread();
        return nextCommandTarget?.QueryStatus(ref pguidCmdGroup, cCmds, prgCmds, pCmdText) ?? VSConstants.S_OK;
    }

    public int Exec(ref Guid pguidCmdGroup, uint nCmdID, uint nCmdexecopt, IntPtr pvaIn, IntPtr pvaOut)
    {
        ThreadHelper.ThrowIfNotOnUIThread();
        if (pguidCmdGroup == VSConstants.VSStd2K)
        {
            switch ((VSConstants.VSStd2KCmdID)nCmdID)
            {
                case VSConstants.VSStd2KCmdID.AUTOCOMPLETE:
                case VSConstants.VSStd2KCmdID.COMPLETEWORD:
                    TriggerCompletion();
                    return VSConstants.S_OK;

                case VSConstants.VSStd2KCmdID.RETURN:
                case VSConstants.VSStd2KCmdID.TAB:
                    if (completionSession is not null && !completionSession.IsDismissed)
                    {
                        completionSession.Commit();
                        return VSConstants.S_OK;
                    }
                    break;

                case VSConstants.VSStd2KCmdID.CANCEL:
                    if (completionSession is not null && !completionSession.IsDismissed)
                    {
                        completionSession.Dismiss();
                        completionSession = null;
                        return VSConstants.S_OK;
                    }
                    break;

                case VSConstants.VSStd2KCmdID.TYPECHAR:
                    var typedChar = (char)(ushort)Marshal.GetObjectForNativeVariant(pvaIn);
                    var result = nextCommandTarget?.Exec(ref pguidCmdGroup, nCmdID, nCmdexecopt, pvaIn, pvaOut) ?? VSConstants.S_OK;

                    if (char.IsLetterOrDigit(typedChar) || typedChar == '_' || typedChar == '.' || typedChar == '#')
                    {
                        if (completionSession is null || completionSession.IsDismissed)
                        {
                            TriggerCompletion();
                        }
                        else
                        {
                            completionSession.Filter();
                        }
                    }
                    else if (completionSession is not null && !completionSession.IsDismissed)
                    {
                        completionSession.Filter();
                    }

                    return result;
            }
        }

        return nextCommandTarget?.Exec(ref pguidCmdGroup, nCmdID, nCmdexecopt, pvaIn, pvaOut) ?? VSConstants.S_OK;
    }

    private void TriggerCompletion()
    {
        ThreadHelper.ThrowIfNotOnUIThread();
        if (completionSession is not null && !completionSession.IsDismissed)
        {
            completionSession.Filter();
            return;
        }

        completionSession = completionBroker.TriggerCompletion(textView);
        if (completionSession is null)
        {
            return;
        }

        completionSession.Dismissed += (_, _) => completionSession = null;
    }
}
