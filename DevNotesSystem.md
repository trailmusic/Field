# Dev Notes System

## Purpose
A single, high‑quality, non‑redundant place to capture decisions, designs, debugging recipes, and release notes. Optimized for skimmability and long‑term value.

## Principles
- Single source of truth for developer notes
- Short, high‑signal, linkable sections
- No duplication; prefer link/refs over copy‑paste
- Time‑boxed notes: every note has an owner and a status
- Visual clarity: consistent headings, callouts, and checklists

## Taxonomy
- ADR: Architecture Decision Record (one decision)
- Decision Note: Smaller decision with minimal context
- Design Brief: Problem → Goals → Constraints → Approach → Risks
- Debug Recipe: Symptom → Hypotheses → Probes → Fix → Verification
- Release Note: Changes, Risks, Migrations, QA
- Postmortem: Incident summary → Impact → Root cause → Fix → Action items

## Formatting Rules
- Title: Short and action‑oriented
- Header block:
  - Status: draft | proposed | accepted | superseded
  - Owner: @name
  - Date: YYYY‑MM‑DD
  - Tags: comma‑separated (e.g., dsp, ui, phase, build)
  - Links: repo paths, PRs
- Body: 5 compact sections max. Use lists over paragraphs.
- Code blocks only when essential.

## File Organization
- This top file: index + templates + conventions
- Notes live under `docs/notes/` with prefixes:
  - `adr/ADR-XXXX-title.md`
  - `decision/DEC-XXXX-title.md`
  - `design/DES-XXXX-title.md`
  - `debug/DBG-XXXX-title.md`
  - `release/REL-XXXX-title.md`
  - `postmortem/PM-XXXX-title.md`
- IDs are monotonically increasing per type

## Index (add entries as notes are created)
- ADR: (none yet)
- Decision:
  - DEC-0001 — Phase Modes: IIR stability, FIR normalization, defaults
- Design: (none yet)
- Debug: (none yet)
- Release: (none yet)
- Postmortem: (none yet)

## Templates

### ADR Template (ADR-XXXX)
- Title
- Status / Owner / Date / Tags / Links
- Context
- Options Considered
- Decision
- Consequences (Pros/Cons)
- Follow‑ups

### Debug Recipe Template (DBG-XXXX)
- Title
- Status / Owner / Date / Tags / Links
- Symptom
- Impact / Severity
- Hypotheses
- Probes (exact commands/edits)
- Findings
- Fix
- Verification (tests / audio checks)

### Design Brief Template (DES-XXXX)
- Title
- Status / Owner / Date / Tags / Links
- Problem
- Goals (bulleted, measurable)
- Constraints
- Approach (1–2 pages max)
- Risks / Open Questions
- Milestones

### Release Note Template (REL-XXXX)
- Version / Date
- Highlights
- Changes (grouped by area)
- Migrations / Breaking
- QA Summary (what we verified)
- Known Issues

### Decision Note Template (DEC-XXXX)
- Title
- Status / Owner / Date / Tags / Links
- Context (3–5 lines)
- Decision
- Rationale
- Alternatives (optional)

### Postmortem Template (PM-XXXX)
- Title
- Status / Owner / Date / Tags / Links
- Summary
- Impact
- Timeline
- Root Cause
- Fix & Verification
- Action Items (with owners & dates)

## Visual Conventions
- Callouts: IMPORTANT, NOTE, WARNING prefixes
- Tables for matrices or comparisons
- Checklists for QA and actions

## Lifecycle & Cleanup
- New notes start as draft → accepted or superseded
- Supersede via Status and link to replacement
- Quarterly cleanup: remove drafts >90 days old without owners; collapse redundant Decision Notes into ADRs
- No “removal logs” or “dead proposals” kept; preserve only accepted/superseded decisions of record

## How to Add a Note
1) Pick a type + next ID (see folder)  2) Copy template  3) Fill header block  4) Keep it short  5) Link PRs/paths  6) Add entry to index above
