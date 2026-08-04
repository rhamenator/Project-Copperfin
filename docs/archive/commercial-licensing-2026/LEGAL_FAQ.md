# Project Copperfin Licensing FAQ

> **Inactive archive:** preserved for possible future reconsideration; these
> answers do not describe the current GPL-3.0-only release.

Copyright © 2026 Richard M. Hamilton. All rights reserved.

This FAQ is a plain-language guide to how Project Copperfin is licensed. It
is a convenience summary only — if anything here conflicts with
[SOURCE_AVAILABLE_LICENSE.md](SOURCE_AVAILABLE_LICENSE.md),
[COMMERCIAL_LICENSE.md](COMMERCIAL_LICENSE.md), or [CLA.md](CLA.md), those
documents control.

## Is Project Copperfin open source?

No. Since the Effective Date described in
[MIGRATION_NOTICE.md](MIGRATION_NOTICE.md), Project Copperfin is
**source-available**, not open source. You can read and modify the source
code, but the license restricts commercial use and restricts relicensing of
derivative works, which disqualifies it from the Open Source Definition.
This is intentional. Versions released before the Effective Date remain
under GPL-3.0, which *is* open source, and that does not change.

## Can I use it for free?

Yes, if you are an individual using it non-commercially — for personal
projects, learning, evaluation, or hobby use, not on behalf of any
employer or organization and not to generate revenue. See
[SOURCE_AVAILABLE_LICENSE.md](SOURCE_AVAILABLE_LICENSE.md) for the exact
terms.

## What counts as "commercial use"?

Broadly: any use by or for the benefit of a company, nonprofit, government
body, or other organization — including purely internal use that never
generates direct revenue — or any use intended to provide a product or
service to third parties. The full definition is in
[SOURCE_AVAILABLE_LICENSE.md §1](SOURCE_AVAILABLE_LICENSE.md#1-definitions).
If you're unsure whether your use qualifies, contact
[rich@yourfoxprodeveloper.com](mailto:rich@yourfoxprodeveloper.com) before relying on the free tier.

## My company wants to try it before buying. Is that allowed?

Not under the Source-Available License — that license only covers
Individual Use. If you want to evaluate Project Copperfin for a commercial
deployment, contact [rich@yourfoxprodeveloper.com](mailto:rich@yourfoxprodeveloper.com) to discuss an evaluation
arrangement under the Commercial License framework.

## Can I redistribute the source code?

Only under the Source-Available License's non-commercial terms: you may
share the Software and your Modifications with others, but only for their
Individual Use, only under the same license (you cannot relicense it or
strip out the license/attribution notices), and never as part of a paid or
commercial offering.

## Can I fork the project?

You can create a Modification for your own non-commercial use, or share it
with others non-commercially under the same Source-Available License terms.
You may **not** create a "proprietary fork" — a version relicensed under
different, more permissive, or closed terms — nor use a fork for
Commercial Use without a Commercial License.

## What about the commercial forks that already exist of the old GPL version?

They are unaffected and fully legal. GPL-3.0 explicitly permits commercial
use and redistribution, and any code released under GPL-3.0 stays licensed
under GPL-3.0 forever for the people who received it that way. Those forks
simply won't automatically receive new code released after the Effective
Date — that new code is only available under the dual license. See
[MIGRATION_NOTICE.md](MIGRATION_NOTICE.md) for the full explanation.

## Can I sell a product built on Project Copperfin?

Only with a [Commercial License](COMMERCIAL_LICENSE.md). Once you have one,
you may embed the Software in closed-source products, sell those products,
and keep your own additions proprietary.

## Do individual and commercial users get different code?

No. The Individual Edition and the Commercial Edition are the same
codebase. The Commercial License doesn't unlock different features by
default — it unlocks the *right* to put the same code to commercial use,
plus optional enterprise terms (support SLAs, indemnification, etc.) if
negotiated in an Order Form.

## Do I get support with the free Source-Available License?

No. Support is a benefit of the [Commercial License](COMMERCIAL_LICENSE.md)
only — under either purchase model (Annual Subscription or Perpetual
License). Individual, non-commercial users do not receive support.

## Should I buy a Subscription or a Perpetual License?

Whichever suits your budgeting preference — the per-seat (or per-tier)
rate is the same either way, so neither option costs more than the other
on an annual basis. A Subscription bundles in every future update,
including major versions, for as long as you keep paying. A Perpetual
License is a one-time payment for the major version you buy, which you can
keep and use — with support — indefinitely; moving to a future major
version later costs an optional Upgrade Fee. See
[COMMERCIAL_LICENSE.md §3.1](COMMERCIAL_LICENSE.md#31-purchase-models).

## What happens if I have a Perpetual License and can't afford a major-version upgrade?

Nothing happens to your existing rights. Declining to pay the Major-Version
Upgrade Fee never suspends, revokes, or "bricks" your license to the
version you already bought — you keep using it, indefinitely, under the
same terms you originally paid for. You simply don't get the newer major
version, and after a defined support window, standard support for your
older version becomes best-effort only. See
[COMMERCIAL_LICENSE.md §8.2](COMMERCIAL_LICENSE.md#82-perpetual-license).

## What attribution is required?

Any copy or Modification you use or share must keep the copyright and
license notice intact (see
[SOURCE_AVAILABLE_LICENSE.md §4](SOURCE_AVAILABLE_LICENSE.md#4-attribution)).
Commercial licensees may negotiate removal of the attribution requirement
(white-labeling) as part of an Enterprise Order Form.

## Do I need to sign anything to contribute code?

Yes. All Contributions are subject to the [CLA.md](CLA.md), which lets
Richard M. Hamilton continue to dual-license, commercialize, and — if ever
needed — take parts of the project private, without needing to track down
every past contributor for permission.

## What happens if I use it commercially without paying?

You are using the Software outside the scope of any license grant. That is
copyright infringement and a breach of the Source-Available License's
terms, and Licensor may pursue remedies including requiring you to stop use,
back-pay applicable license fees, and other remedies available under
copyright law and contract law.

## How is this license enforced?

Primarily through ordinary legal mechanisms available to any copyright
holder: license termination (Section 8 of the Source-Available License),
breach-of-contract and copyright-infringement claims, DMCA takedown notices
where applicable to infringing distributions, and negotiated resolution
(Licensor generally prefers converting unlicensed commercial users into
paying customers over litigation, but reserves all legal remedies).

The Software also displays a license-status indicator — for example, via a
`--license-status` command, in build manifests, and in the Studio/Visual
Studio summary panel — showing whether it's running under a free,
individual license or a paid Commercial License. This never blocks or
restricts anything the Software does; it exists purely so license status is
visible and auditable, and so that anyone using it commercially without a
license cannot credibly claim they weren't told. Deliberately stripping or
falsifying that notice is a separate legal violation on top of the
underlying license breach — see
[SOURCE_AVAILABLE_LICENSE.md §4](SOURCE_AVAILABLE_LICENSE.md#4-attribution).

## How can a commercial entity become compliant?

Contact [rich@yourfoxprodeveloper.com](mailto:rich@yourfoxprodeveloper.com) with your organization name, intended
use, and approximate seats/revenue/usage. Licensor will issue an Order Form
under the [Commercial License](COMMERCIAL_LICENSE.md) framework covering
your use going forward, which can also address any past unlicensed use as
part of the agreement.

## Can the license change again in the future?

Yes, for future versions — Licensor, as sole copyright owner, retains that
right (see [SOURCE_AVAILABLE_LICENSE.md §7](SOURCE_AVAILABLE_LICENSE.md#7-licensors-reserved-rights)).
Any such change would not revoke rights already granted for versions
already released under a given license, consistent with how this GPL-to-
dual-license migration itself works.

## Where do I go for the actual legal terms?

- [LICENSE.md](LICENSE.md) — overview and index
- [SOURCE_AVAILABLE_LICENSE.md](SOURCE_AVAILABLE_LICENSE.md) — free,
  non-commercial terms
- [COMMERCIAL_LICENSE.md](COMMERCIAL_LICENSE.md) — commercial terms and
  pricing framework
- [CLA.md](CLA.md) — contributor agreement
- [MIGRATION_NOTICE.md](MIGRATION_NOTICE.md) — GPL-to-dual-license
  transition details
