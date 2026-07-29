# Pilot measurements — 2026-07-29

## A. Backlog shape (measured, n=119 diffs computed locally from real PR heads)

Sample: 119 open PRs drawn stride-wise from the 800 newest open PRs; diff computed against
`git merge-base <pr-head> upstream/master`.

| metric | mean | median | p75 | p90 | p99 | max |
|---|---|---|---|---|---|---|
| changed lines (+/−) | 553 | **28** | 152 | 569 | 2,208 | 47,119 |
| files touched | 7.1 | **2** | 5 | 16 | 77 | 249 |
| diff bytes | 40,323 | **3,767** | 12,584 | 41,283 | 141,818 | 3,267,571 |
| commits | 2.5 | 1 | 1 | 3 | 24 | 91 |

Size buckets, extrapolated to the full 5,236:

| bucket | share | count |
|---|---|---|
| trivial (<10 lines) | 28.6% | ~1,496 |
| small (10–99) | 38.7% | ~2,024 |
| medium (100–499) | 21.0% | ~1,100 |
| large (500–1,999) | 10.1% | ~528 |
| huge (≥2,000) | 1.7% | ~88 |

**Total raw diff corpus ≈ 211 MB ≈ 59M tokens.** The diffs are not the cost driver.

## B. Backlog age profile (measured via search `total_count` per window — population counts, not sample)

| created | count | cumulative | cum % |
|---|---|---|---|
| <2020 | 6 | 6 | 0.1% |
| 2020 | 51 | 57 | 1.1% |
| 2021 | 156 | 213 | 4.1% |
| 2022 | 307 | 520 | 9.9% |
| 2023 | 571 | 1,091 | 20.8% |
| 2024 | 949 | 2,040 | 39.0% |
| 2025 | 1,599 | 3,639 | 69.5% |
| 2026 (to 04-17) | 793 | 4,432 | 84.6% |
| 2026 (04-18 →) | ~804 | 5,236 | 100% |

Half the backlog is <18 months old; only 4% predates 2022. Drafts ≈ 7.5–14%.
Label mix (newest 800): enhancement 452, bug 338, topic:editor 288, topic:gui 99,
usability 94, topic:core 75, topic:3d 75, topic:rendering 66, documentation 54.

## C. Human throughput baseline (measured from git history)

- 4,339 merge commits in 12 months → **~362 merged PRs/month**
- 655 unique authors/yr, but **~2 people perform essentially all merges**
- Inflow ≈ outflow → backlog is a standing wave

## D. Pilot leg 1 — physics manual space stepping (HARD tier), completed

Selected as one of 3 from a 10-candidate shortlist produced by a research agent that
git-tested mergeability of every candidate against current master.

| quantity | measured |
|---|---|
| upstream diff as fetched | 32,208 bytes ≈ 8,900 tokens |
| merge-base drift | 3,696 commits behind |
| files conflicting on rebase | **14 of 22** |
| root cause of conflicts | upstream moved physics enums into `PhysicsServer{2,3}DEnums` namespaces |
| final branch | 3 commits, 23 files, +592 lines |
| **real defects found** | **5** (1 state-corruption bug, 1 resource-exhaustion bug, 1 cross-backend inconsistency, 1 doc-contradicts-code, 1 compile/style breakage) |
| tests written | 8 cases / 29 assertions, new file |
| revert-proof performed | yes — test fails on unfixed code, passes with fix |
| build: cold | **12 min 07 s** (4 cores, SCU, ccache cold) |
| build: incremental | **98 s**, then 32 s / 23 s on later iterations (ccache warm) |
| build iterations needed | 4 (1 compile error, 1 revert-proof, 1 restore, 1 final) |
| local gates passed | clang-format clean; `--doctool` zero diff; 11/11 tests pass |

**Agent token cost, leg 1.** Exactly measured for delegated work; estimated for main thread.

| component | tokens | basis |
|---|---|---|
| candidate shortlisting (amortised over 3 PRs) | 51,180 / 3 ≈ 17,060 | exact (subagent) |
| deep research on this PR | 58,487 | exact (subagent) |
| main-thread rebase + review + fixes + tests + verification | ~200,000 | estimated from ~50 tool calls with build/test output |
| **total** | **~275,000** | |

## E. Pilot leg selection finding — the triage said "don't"

The second candidate (a core scene-tree lifecycle feature) was researched at a cost of
**78,170 tokens** and the correct output was a recommendation **not to rebase it**:
- 9 concrete defects found, incl. state corruption and a silent rendering bug
- 5 of its 16 touched files no longer exist at those paths upstream
- blocked on an architectural objection from the lead architect that was never retracted
- verdict: "best treated as a specification to reimplement rather than a diff to rebase"

This is a real and important cost-model input: **a correct triage outcome is often "close it"
or "reimplement", and reaching that verdict costs ~80k tokens of genuine analysis.** It is
not a failure mode; it is the product.

## F. Pilot leg 3 — hexagonal GridMap cells (delegated agent), completed

| quantity | measured |
|---|---|
| merge-base drift | 6,875 commits behind |
| files conflicting | 5 (22 conflicting hunks in the editor plugin alone) |
| **agent tokens** | **330,871** (exact — delegated agent) |
| tool calls | 129 |
| build iterations | ~7, 25–40 s each (ccache warm) |
| wall-clock | ~40 min |
| defects found | 2 High, 1 Medium perf, 3 leaked RIDs, 1 public-API bug, ~10 minor |
| tests | 9 cases / 250 assertions passing |
| revert-proof | yes — reverting the 2 High fixes fails 2 cases / 17 assertions |
| gates | clang-format clean, `--doctool` zero diff, no warnings in module |
| independent re-verification | tests re-run and fixes inspected by the orchestrator |

High defects: `axial_round()` used integer `abs()` on float remainders (truncating to 0, so
off-centre points resolved to the wrong hex cell); `make_baked_meshes()` kept rectangular
placement, displacing every mesh in a baked hex map.

**Cross-leg observation.** In both completed legs the rebase was routine and the value was in the
review — 5 defects in leg 1, 6+ in leg 3, all in code humans had already reviewed. Token cost per
hard PR landed at 275k (leg 1, partly estimated) and 331k (leg 3, exact).

## G. Pilot leg 4 — shared hash-table core + AHashSet (delegated agent), completed

| quantity | measured |
|---|---|
| merge-base drift | 3,420 commits behind |
| source commits | 36, squashed on rebase |
| final branch | 2 commits, 6 files, +1,046 / -207 |
| **agent tokens** | **223,793** (exact) |
| tool calls | 114 |
| build time | ~24 min across 5 scons invocations |
| defects found & fixed | 4 |
| defects found & deliberately NOT fixed | 3 (pre-existing; reproduce on the merge-base header) |
| tests | 58 cases / **57,217 assertions** passing; net +18 cases / +4,824 assertions vs master |
| revert-proof | yes — reverting 3 testable fixes fails exactly 4 cases / 7 assertions |
| extra verification | ASAN+UBSAN differential fuzz vs `std::unordered_map`/`set` (40 seeds x 4 workloads); behavioural-signature diff pre/post refactor byte-identical over 60 seeds x 6,000 ops |

**The single most valuable finding was procedural, not technical:** `tests/SCsub` globs `*/**/*.cpp`,
and the change's new test file was named `.h` — so its **263 lines of tests had never been compiled
or run**. Renaming it exposed a latent compile error immediately. A green CI badge on the original
branch would have meant nothing for this container.

Defects fixed: vestigial `virtual` destructor from a pre-CRTP design making every container
polymorphic (`sizeof(AHashMap<int,int>)` 24 -> 32 bytes engine-wide); `AHashSet::insert()`
overwriting a stored duplicate where `HashSet::insert()` keeps it; missing move assignment causing
silent deep copies; `reset()` leaving a dangling metadata pointer.

Two of the four were found by **diffing the new container's API against `HashSet`'s** — a cheap,
repeatable technique worth building into the pipeline for any new API surface.

Not fixed (correctly scoped out): a heap-use-after-free when an `insert()` argument aliases the
container's own storage and triggers a rehash (ASAN trace captured); `reserve()` able to shrink
capacity; capacity math overflowing above 2^31 entries.

**Benchmarks were run and reported honestly against the change's own premise.** New vs pre-refactor
`AHashMap` is within +/-2% on insert/lookup/erase/iterate at N=1e6 — the extraction is
performance-neutral, as a deduplication should be. The large wins over `HashMap`/`HashSet`
(~1.6x insert, ~1.7-2.2x lookup, ~6x iteration) predate the change. An agent optimising for
looking good would have reported the second table and omitted the first.

## H. Subsystem audit — `core/string`

| quantity | measured |
|---|---|
| scope | `ustring.{h,cpp}`, `string_name.{h,cpp}`, `string_builder.{h,cpp}` |
| lines read | ~7,360 in scope + ~600 supporting = ~8,000 |
| **tokens** | **202,370** (exact) |
| tool calls | 33 |
| wall-clock | ~30 min |
| findings | **29** — 2 High, 11 Medium, 16 Low; 28 verified, 1 likely |

High findings: unchecked `int` overflow in `String::repeat()` capacity math with a discarded
`resize_uninitialized()` error and an unconditional memcpy (script-reachable heap overflow);
`_to_int()`'s overflow guard fires one digit late, so all 19-digit inputs bypass it.

The second was **independently reproduced against a locally built engine**:

```
"9223372036854775807".to_int()  ->  9223372036854775807   (correct)
"9223372036854775808".to_int()  -> -9223372036854775808   (silently wraps)
"9999999999999999999".to_int()  -> -8446744073709551617
"-9223372036854775809".to_int() ->  9223372036854775807   (negative in, positive out)
```

Extrapolation: ~35 comparable subsystems x ~200k tokens = ~7M tokens for a first full sweep.

## I. Infrastructure findings

- **Fork CI does not run.** All 9 Godot workflows are present and `state: active` on the fork,
  but GitHub disables Actions on forked repos until the owner enables them; the only run in
  the fork's history is a Copilot review from May. `workflow_dispatch` via API returns
  `403 Resource not accessible by integration`. **Verification in this pilot was entirely local.**
- Local build box: 4 cores / 15 GB RAM. Cold full editor build w/ tests = 12 min; ccache-warm
  incremental = 23–98 s. ccache is what makes multi-branch work affordable.
- Godot's own CI matrix is 7 jobs incl. ASan/UBSan/TSan, mono, doubles, 4 desktop + mobile +
  web platforms. Reproducing that per PR is the dominant compute cost at scale, not tokens.

## J. Governance finding (decisive for the upstream path)

`godotengine/godot-contributing-docs` → `pull_requests/pull_request_guidelines.rst`:

> "The use of AI to contribute to Godot is discouraged, and contributions made entirely by AI
> are prohibited."

Plus a disclosure requirement for any AI use, and: "Please only submit code that you understand
and are prepared to explain to a maintainer." Effective ~2026-06-30.

Press framed this as a flat ban; the actual text is narrower (discouraged / entirely-AI prohibited /
disclosure mandatory). Either way the operative constraint on the upstream path is a **human who
understands and can defend each change** — which is exactly the bottleneck the exercise set out
to relieve.
