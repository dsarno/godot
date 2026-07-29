# Agent-assisted engine maintenance — project brief

**Status:** proposal · **Last updated:** 2026-07-29

This supersedes the framing in `README.md`, which evaluated the work as a cost question.
That was the wrong question. This document states what the project actually is.

---

## What this is

A **demonstration project**. The goal is not to save money, and not to clear a backlog for its
own sake. It is to produce **legitimate, reproducible evidence** that agent-driven work can help
an open-source engine — get through a stalled pull-request queue *and* improve the engine along
the way — with enough rigour that a sceptical maintainer can check it rather than take it on faith.

The audience is not a budget holder. It is maintainers of large open-source projects who currently
have good reason to be wary of AI contributions.

## Why it matters now

On 2026-06-30 Godot updated its contribution rules:

> "The use of AI to contribute to Godot is discouraged, and contributions made entirely by AI are
> prohibited."

with mandatory disclosure for any AI use, and a requirement that contributors understand and be
able to defend what they submit.

**That policy is a reasonable response to a real problem.** Maintainers were receiving a rising
volume of low-effort AI-generated pull requests, and reviewing them was described as demoralising.
Review capacity is the scarcest resource a project like this has — Godot merges ~362 PRs/month and
essentially two people perform all the merges. Anything that consumes reviewer attention without
returning value is a genuine harm.

The useful response to that is not an argument. It is a **counter-example held to a higher standard
than the policy demands**: work that is verifiable, that reduces rather than increases reviewer
burden, and that is honest about what it did and did not do.

This cuts both ways, and the project has to be willing to publish either result. If a rigorous
attempt still produces work that wastes maintainer time, that is also legitimate data, and Godot's
policy is vindicated.

## Ground rules

1. **Nothing is offered upstream without a human who has read it and can defend it**, with AI use
   disclosed. The policy is respected as written, not lawyered around.
2. **No upstream pollution.** No references to upstream issues or pull requests in any commit
   message, comment, or pull-request body on this fork — those create cross-reference backlinks in
   Godot's tracker and would turn an experiment into an imposition.
3. **The bar is higher than for human contributions, not lower.** A single sloppy change discredits
   the entire argument. Every claimed fix carries a test demonstrated to fail without it.
4. **Publish negative results.** Rejected candidates, wrong verdicts and regressions are part of
   the evidence, not embarrassments to omit.

---

## What "better" means, and how to prove it

The point of a demonstration is a claim someone can check. The claim should be about **the engine**,
not about the process:

> *This build is measurably more correct / faster / smaller / safer than stock, and here is the
> script that reproduces the comparison.*

### An honest correction first

**The fork is not currently better than upstream.** It is important to be clear about this. The
three completed pull requests *add* features (manual physics stepping, hexagonal grid cells, a
shared hash-table core with a new set container). The ~15 defects found and fixed were defects in
**the proposed changes themselves**, not in shipped Godot. Fixing a bug in a patch that was never
merged does not make anybody's engine better.

The workstream that produces a genuine "better than stock" claim is the **audit** — because those
findings are in code that ships today. The `core/string` sweep alone found an integer-overflow guard
that fires one digit late, reproduced against a locally built engine:

```
"9223372036854775807".to_int()  ->  9223372036854775807   (correct)
"9223372036854775808".to_int()  -> -9223372036854775808   (silently wraps)
"-9223372036854775809".to_int() ->  9223372036854775807   (negative in, positive out)
```

No error is printed. That is a defect in shipped Godot 4.7-dev. Fixing it is a claim of the right
shape: *stock does this, ours does that, here is the one-line repro.*

**Implication for sequencing: the audit is not the cheaper alternative to the backlog work — it is
the part that generates the evidence.** The backlog work demonstrates *throughput*; the audit
demonstrates *improvement*. The demonstration needs both, but the audit comes first.

### The benchmark harness

A/B comparison, stock upstream `master` vs. fork `HEAD`, identical compiler, flags and machine, run
by a single committed script so anyone can reproduce it.

| dimension | metric | how | credibility |
|---|---|---|---|
| **Correctness** | count of behaviours fixed, each with a repro that fails on stock | GDScript / doctest repro pairs, run against both binaries | **strongest** — binary, checkable, no interpretation |
| **Safety** | ASan + UBSan clean across the test suite; leak count at exit | sanitizer build, both sides | strong |
| **Performance** | targeted microbenchmarks on paths actually touched; engine startup via `--benchmark` | `g++ -O2` harness + engine timings, median of N, both orders | good, if scoped honestly |
| **Memory** | peak RSS on a fixed headless workload; `sizeof` of hot types | `/usr/bin/time -v`, static asserts | good |
| **Size** | stripped binary bytes | `ls -l`, same flags | weak alone, useful in aggregate |
| **Coverage** | test cases and assertions added | doctest summary delta | supporting |
| **Hygiene** | compiler warnings, clang-tidy findings | build logs, both sides | supporting |

Rules that keep it honest:

- **Report regressions as prominently as improvements.** A benchmark table that only goes one
  direction is not believed, and should not be.
- **Report neutral results.** The hash-table work benchmarked within ±2% of the previous
  implementation — that is the correct outcome for a deduplication, and saying so is what makes the
  rest of the numbers trustworthy.
- **Scope performance claims to paths actually touched.** No whole-engine "10% faster" claims from
  a microbenchmark.
- **Two orders, median of N.** Cheap, and removes the most common source of bogus numbers.

---

## Milestones

| # | Milestone | Exit criterion |
|---|---|---|
| **M0** | Fork CI running | Actions enabled; the four open PRs green on the full 7-platform matrix |
| **M1** | Benchmark harness | One script produces the A/B table above against two builds; committed and reproducible |
| **M2** | Full audit sweep | Every major subsystem swept; findings ledger populated and human-triaged |
| **M3** | First *improvement* batch | Audit findings fixed, tested, and showing as green deltas in the A/B table — the first honest "ours is better" claim |
| **M4** | First *throughput* batch | 30–50 backlog PRs through the full pipeline, human-audited in one sitting |
| **M5** | Write-up | Public report: method, results, costs, failures, and the raw ledgers |

M3 is the one that matters. M4 is the volume story; M3 is the quality story, and quality is what
the sceptical reader is actually testing.

### Feedback points, stated in advance

Written down before starting so they cannot be rationalised away later:

- Human audit sustained above **~20 min/PR** → the pipeline is not producing reviewable output.
  Fix the output format before scaling; do not add reviewers.
- False-positive rate on findings above **~20%** → judgment is not trustworthy at this model tier.
- Fewer than **1 real defect per 5 PRs** → the backlog is not the bug-rich corpus the pilot
  suggested; drop to audit-only.
- **Any agent-authored change causing a regression that escapes both the tests and human review** →
  full stop, redesign the gates, and publish the failure. This is the outcome that would make the
  project net-negative, and it is the one worth watching hardest.

---

## Keeping the knowledge

Three artefacts, all **required outputs of the pipeline** rather than documentation written
afterwards. A branch with empty fields is not finished.

**1. Findings ledger** — `engineering/findings/`, one file per finding, structured front-matter so
agents can query it (`id`, `severity`, `subsystem`, `locations`, `status`, `confidence`, `repro`,
`found_by`, `fix_pr`, `upstream_issue`). Mirrored to fork issues, which are the human working
surface. The ledger is the durable record; issues are where discussion happens.

**2. Divergence manifest** — `engineering/DIVERGENCE.md`. One entry per change vs. upstream: what
changed, why, upstream status, **rebase notes** (which upstream changes would conflict and what to
watch), and the test that guards it. This is what makes the fork survivable across upstream syncs;
without it, the fork silently becomes unmaintainable.

**3. Decision records** — `engineering/decisions/`, numbered and immutable, for every place a
judgment was made between defensible options. From the pilot alone: aligning all three physics
backends on rejecting active spaces when the documentation said otherwise; keeping upstream's
`a_hash_map.cpp` where the proposed change deleted it; leaving new editor shortcuts unbound;
declining to fix three pre-existing hash-table defects as out of scope.

**Invert the direction of writing:** the record is authored first, and commit messages and
pull-request bodies are generated from it. Today the prose is primary and the knowledge is trapped
inside it.

Also needed: `SYNC.md`, a runbook for the periodic upstream merge, rehearsed early. That is the
operation that kills forks, and it should be routine before many branches depend on it.

---

## Carried forward from the pilot

Already produced and needing homes in the ledger:

- 29 `core/string` findings (2 High), one reproduced against a built engine
- 3 pre-existing hash-table defects deliberately left unfixed, including a use-after-free with a
  captured ASAN trace
- 9 defects in a scene-tree lifecycle change that was correctly judged *not* worth rebasing
- Hexagonal grid octant-bounds semantics, unresolved and needing a human decision
- Physics threading and GDExtension-compatibility questions raised but not settled
- A build-system hazard: a test file named `.h` while the build globs `.cpp`, so 263 lines of
  assertions had never once been compiled

---

## Open questions

1. **Does the demonstration target Godot specifically, or is Godot the case study?** Publishing to
   the Godot community invites a response the project must be prepared to accept. A neutral write-up
   using Godot as a worked example is lower-friction and possibly more useful.
2. **What is the offer at the end?** A report? A curated set of upstream bug reports, each verified
   by a human? A reusable pipeline other projects can run? These imply different work.
3. **How much fork divergence is acceptable** before the maintenance burden outweighs the
   demonstration value?
4. **Who is the human in the loop**, and what is their realistic weekly capacity? Every projection
   in the study is a function of that one number.
