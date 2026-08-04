# Project Copperfin Commercial License

> **Inactive archive:** preserved for possible future reconsideration; this is
> not a current Project Copperfin license or offer.

Copyright © 2026 Richard M. Hamilton. All rights reserved.

This document describes the commercial licensing program for Project
Copperfin. It is a **framework agreement**: the rights, obligations, and
terms below apply to every Commercial License, while the specific fee,
tier, seat count, term, and any custom terms for a given customer are set
out in a signed **Order Form** that incorporates this document by
reference. Where an Order Form conflicts with this document, the Order
Form controls for that customer only.

If you have not signed an Order Form and paid the applicable fee, you do
not have a Commercial License, and your use of the Software must comply
with the [Source-Available License](SOURCE_AVAILABLE_LICENSE.md) instead
(which forbids Commercial Use).

## 1. Who Needs This License

Anyone engaged in **Commercial Use** of the Software, as that term is
defined in [SOURCE_AVAILABLE_LICENSE.md §1](SOURCE_AVAILABLE_LICENSE.md#1-definitions),
needs a Commercial License. This includes, without limitation: for-profit
companies, nonprofits, government bodies, consultancies deploying the
Software for clients, and individuals using the Software in the course of
a business.

There is only **one codebase**. The Individual Edition and the Commercial
("Enterprise") Edition are the same source tree — a Commercial License does
not unlock different code, it unlocks different *rights* to the same code
(see Section 2), plus optional add-ons and support described below.

## 2. Grant of Rights

Subject to payment of applicable fees and compliance with this license and
the governing Order Form, Licensor grants Licensee a non-exclusive,
non-transferable (except per Section 9) license to:

a. use, execute, and internally modify the Software for Commercial Use, in
   the quantity/scope (seats, instances, revenue tier, or usage volume)
   specified in the Order Form;

b. embed, link, or bundle the Software, including Licensee's Modifications,
   into Licensee's own products or services, including **closed-source and
   proprietary products**, without any obligation to disclose or license
   Licensee's own source code as a result;

c. distribute the Software as embedded within such products to Licensee's
   own customers, subject to the scope purchased in the Order Form; and

d. develop and retain as proprietary any add-ons, plugins, or extensions
   Licensee builds against the Software, with no obligation to contribute
   them back or release them under any Copperfin license.

This license does **not** grant rights to redistribute the Software's
source code, standalone and unmodified, as a competing source-available or
open-source offering, nor to sublicense the Software itself to other
software vendors as a general-purpose SDK, without Licensor's prior written
consent.

## 3. Pricing Model

### 3.1 Purchase Models

Every price in this section can be purchased either way, at the **same
per-seat or per-tier rate** — choosing Subscription does not cost more per
year than Perpetual, and choosing Perpetual is not a premium over
Subscription:

- **Annual Subscription.** Licensee pays the applicable rate below each
  year. While the subscription is active, Licensee receives standard
  support (§6) and all updates and upgrades released during the paid term,
  including major versions, at no additional charge. If Licensee does not
  renew, Commercial Use rights end as of the paid-through date (see §8.1);
  Licensee must then either renew, obtain a Perpetual License, or limit
  further use to what the [Source-Available License](SOURCE_AVAILABLE_LICENSE.md)
  permits.
- **Perpetual License.** Licensee pays the applicable rate below **once**,
  for the major version current as of purchase. This grants a perpetual,
  non-expiring right to Commercial Use of that major version line
  (including its minor and patch updates) and standard support for as long
  as that major version remains supported (§6). Moving to a later major
  version requires the Major-Version Upgrade Fee (§3.4); declining it never
  suspends, revokes, or otherwise affects the rights already purchased —
  see §8.2.

Both models are graduated along two active axes; the applicable axis and
exact figures for a given customer are fixed in the Order Form.

### 3.2 Seat-Based Tier (named/concurrent developer or user seats)

| Tier | Seats | Price per seat / year |
| --- | --- | --- |
| Starter | 1–5 seats | $149 |
| Team | 6–25 seats | $119 |
| Business | 26–100 seats | $89 |
| Enterprise | 100+ seats | Custom — contact Licensor |

### 3.3 Revenue-Based Tier (Licensee's annual gross revenue; flat fee, unlimited internal seats)

| Tier | Licensee Annual Revenue | Flat price / year |
| --- | --- | --- |
| Micro | Under $1M | $495 |
| Small Business | $1M – $10M | $1,995 |
| Mid-Market | $10M – $50M | $6,995 |
| Enterprise | Above $50M | Custom — contact Licensor |

A given customer picks whichever axis (seat-based or revenue-based) is more
favorable to them; the Order Form fixes which axis applies.

### 3.4 Major-Version Upgrade Fee (Perpetual License only)

When Licensor releases a new major version, a Perpetual License holder may
obtain rights to that version by paying an Upgrade Fee equal to 50% of the
then-current list price for their seat or revenue tier, one time. Paying
the Upgrade Fee is optional in every case. See §8.2 for what happens if a
Licensee chooses not to pay it — in short, nothing bad: existing rights are
never revoked for declining an upgrade.

### 3.5 Usage-Based Tier (reserved)

Project Copperfin does not currently offer a hosted or metered service, so
no usage-based metric is active at this time. If Licensor later introduces
one (e.g., a hosted build/compile service), a usage metric and price table
will be published here and in the relevant Order Forms; it will not apply
retroactively to existing seat- or revenue-based Order Forms.

### 3.6 General

Actual price points, currency, invoicing cadence, and any volume discounts
are set per customer in the Order Form. Licensor may publish a standard
rate card separately from this document; that rate card does not itself
modify this license.

## 4. Enterprise Rights

Enterprise-tier Order Forms may additionally include, as negotiated:

- custom seat, revenue, or hybrid pricing outside the standard tiers in §3;
- source-code escrow arrangements;
- limited indemnification for third-party IP claims arising from unmodified
  use of the Software;
- a negotiated service-level agreement (SLA) for support response times and
  uptime commitments where Licensor hosts any component;
- white-labeling / removal of the attribution notice required under the
  Source-Available License; and
- priority input into the public roadmap (non-binding).

None of the above are granted by default; they must be specified in a
signed Order Form.

## 5. Proprietary Add-Ons and Closed-Source Embedding

Licensee may develop proprietary, closed-source add-ons, modules, plugins,
or integrations that interoperate with the Software, and may embed the
Software into Licensee's own closed-source products, without any
obligation to release such add-ons or products under the Source-Available
License, the Commercial License, or any other Copperfin license. Licensor
may independently offer its own proprietary add-ons or enterprise features
that are not included in the base Software and are licensed separately.

## 6. Support Terms

The free [Source-Available License](SOURCE_AVAILABLE_LICENSE.md) tier
includes **no support** of any kind. Support is a benefit of holding a
Commercial License, under either purchase model in §3.1:

Unless otherwise specified in an Order Form:

- **Starter/Team tiers**: best-effort email support, target initial
  response within 5 business days, no uptime or resolution-time
  commitment.
- **Business tier**: email support, target initial response within 2
  business days.
- **Enterprise tier**: support terms (channels, response times, escalation
  path, and any SLA credits) are defined in the Order Form.

These response-time targets apply equally whether Licensee holds an Annual
Subscription or a Perpetual License, **while the applicable major version
remains supported**. A Perpetual License's major version remains supported
for as long as it is Licensor's current major version, and in any case for
at least twelve (12) months after a later major version ships. After that
window, a Perpetual License continues to grant full use rights
indefinitely (§8.2), but support becomes best-effort only unless the
Licensee pays the Major-Version Upgrade Fee (§3.4) to move onto a
currently-supported version.

Support covers use of the unmodified Software; Licensor is not obligated to
support Licensee's Modifications or third-party integrations.

## 7. Fees, Taxes, and Payment

Fees are as stated in the Order Form, due per the invoicing schedule
stated there, and are non-refundable except as required by law or as
separately agreed in writing. Fees are exclusive of taxes; Licensee is
responsible for any applicable sales, use, VAT, or similar taxes other than
taxes on Licensor's net income.

## 8. Term and Termination

### 8.1 Annual Subscription

a. **Term.** An Annual Subscription runs for the term stated in the Order
   Form (typically one year) and renews automatically under the same terms
   unless either party gives notice of non-renewal.

b. **Non-renewal.** If Licensee does not renew, the Subscription simply
   expires at the end of the paid term; Commercial Use rights end as of
   that date, and Licensee must either renew, obtain a Perpetual License
   under §3.1, or limit further use to what the Source-Available License
   permits.

c. **Termination for non-payment.** Licensor may suspend or terminate a
   Subscription if fees are more than 30 days past due and remain unpaid 10
   days after written notice.

### 8.2 Perpetual License

a. **No expiration.** A Perpetual License's grant of Commercial Use rights
   to the major version purchased does not expire, and is not conditioned
   on any future payment. It is fully paid up at the time of purchase.

b. **Major-version upgrades are optional, not a renewal.** Declining to
   pay a Major-Version Upgrade Fee (§3.4) for a later major version:

- does **not** suspend, revoke, terminate, or otherwise diminish the
  Licensee's existing rights to Commercial Use of the major version
  already purchased;
- does **not** trigger any termination clause in this Agreement solely
  on that basis; and
- only affects eligibility for the newer major version and, after the
  support window in §6, eligibility for continued standard support.

In short: a Licensee who cannot or chooses not to afford a major-version
upgrade keeps full, unbroken use of the version they already own.

c. **Termination for breach only.** A Perpetual License may only be
   terminated under §8.3 (material breach) — never for declining an
   upgrade, and never for non-payment of a fee that was never owed.

### 8.3 Termination for Breach (Both Models)

Either party may terminate this license for the other's material breach
(e.g., exceeding the licensed seat count, sublicensing in violation of §2,
or non-payment of fees actually owed under §8.1(c)) if the breach is not
cured within 30 days of written notice.

### 8.4 Effect of Termination

Upon termination under §8.1(c) or §8.3, Licensee must cease all Commercial
Use of the Software and, if requested, certify destruction of copies used
under the Commercial License. Continued use after termination must comply
with the Source-Available License instead, meaning continued Commercial
Use is not permitted without a new Commercial License. Termination under
this Section 8 does not apply to, and never claws back, a Perpetual
License's rights to a major version already fully paid for under §8.2.
Sections 9, 10, 11, and 12 survive termination.

## 9. Assignment

Licensee may not assign or transfer this license without Licensor's prior
written consent, except to a successor in a merger, acquisition, or sale of
substantially all of Licensee's assets, provided the successor agrees in
writing to be bound by this license and the Order Form.

## 10. Audit Rights (Lightweight)

a. **Self-certification.** Licensor may request, no more than once per
   calendar year, that Licensee self-certify in writing its current seat
   count, revenue tier, or usage volume relevant to its Order Form.

b. **Records review.** With at least 30 days' written notice, and no more
   than once per calendar year absent evidence of material
   non-compliance, Licensor may request reasonable supporting records
   (e.g., seat rosters, usage logs) to confirm compliance. Any such review
   will be conducted during normal business hours in a manner that
   minimizes disruption to Licensee's operations, and Licensor will treat
   all records reviewed as Licensee's confidential information.

c. **Underpayment.** If a review reveals underpayment of more than 5%,
   Licensee will pay the shortfall plus interest at 1% per month; if 5% or
   less, Licensee pays only the shortfall.

## 11. Warranty Disclaimer and Limitation of Liability

Except as expressly stated in a signed Order Form, the Software is provided
under this Commercial License subject to the same disclaimer of warranty
and limitation of liability set out in
[SOURCE_AVAILABLE_LICENSE.md §§10–11](SOURCE_AVAILABLE_LICENSE.md#10-disclaimer-of-warranty),
which are incorporated here by reference.

## 12. Governing Law and Venue

This license is governed by the laws of the State of Michigan, USA, without
regard to conflict-of-laws principles, and the state and federal courts
located in Michigan have exclusive jurisdiction over any dispute, unless the
Order Form specifies otherwise.

## 13. How to Obtain a Commercial License

Contact Richard M. Hamilton at **[rich@yourfoxprodeveloper.com](mailto:rich@yourfoxprodeveloper.com)** with:

- your organization's name and a description of the intended use;
- the approximate seat count, revenue tier, or usage volume applicable; and
- whether you need any Enterprise-tier terms from Section 4.

Licensor will issue an Order Form referencing this Commercial License for
signature.

---

*Project Copperfin is dual-licensed. See [LICENSE.md](LICENSE.md) for an
overview, [SOURCE_AVAILABLE_LICENSE.md](SOURCE_AVAILABLE_LICENSE.md) for the
free individual-use terms, [CLA.md](CLA.md) for the contributor agreement,
and [LEGAL_FAQ.md](LEGAL_FAQ.md) for common questions.*
