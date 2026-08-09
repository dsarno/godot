# Clearing the Godot PR backlog with agents — a feasibility study

> **Framing note (2026-07-29).** This document evaluates the work as a *cost* question. That turned
> out to be the wrong question — see **`PROJECT-BRIEF.md`**, which supersedes it. The project's
> purpose is to produce reproducible evidence that agent-driven work can help an open-source engine,
> for an audience of sceptical maintainers rather than a budget holder. The measurements below all
> stand; the "is it worthwhile" conclusion does not.

**Date of measurement: 2026-07-29.** Every number here was measured during the study, not
estimated from priors. Where something is an estimate, it says so.

---

## The short answer

Compute is not the problem. **Model spend to process the entire 5,236-PR backlog is roughly
$860 on the cheapest capable open-weight model and about $76,000 on Claude Opus 5** — for
~97 billion billed tokens. Agent wall-clock is under three days at 50-way concurrency. Builds are
~640 machine-hours, which is free on GitHub-hosted runners for a public repo.

**A note on how tokens are counted here, because it is a factor of ~100.** These are *billed*
tokens, taken from the API's own usage fields over 1,224 calls — not an estimate of "content
processed". An agent resends its entire context on every call, so a pull request whose unique
material is ~300k tokens bills **25–44M**. The measured shape of that spend: **96.2% of input is
cache reads**, mean context per call **243k tokens**, and **output is 0.24% of the total**. The
number that governs cost is therefore the *cache-read* rate, not the headline input rate — which
reorders the model table below. Full breakdown in `token-log.md`.

The binding constraint is still the **human audit gate**. At ten minutes of human review per finished
PR, ~2,550 finished PRs is ~425 person-hours — about **ten months at ten hours a week**, roughly
110× the agent time. Every dollar saved by picking a cheaper model is a dollar you were not short of.

There is also a wall on the upstream path that no amount of engineering removes: as of
2026-06-30 Godot's contribution rules state that *"the use of AI to contribute to Godot is
discouraged, and contributions made entirely by AI are prohibited,"* with mandatory disclosure
for any AI use. Working on the fork is the only compliant venue for autonomous agent work.

---

## What was actually done

Not a thought experiment. One pilot leg was carried end to end on the fork, and the numbers
below come from it.

**Setup.** The fork's `master` was 3.5 months stale; it was fast-forwarded to upstream
`d488d1111`. A full Godot editor build with tests was made to work on a 4-core box:
**12 min cold, 23–98 s incremental** with ccache. That build is what makes any of this real —
without the ability to compile and run the engine, an agent is just rewriting text.

**Candidate selection.** A research agent shortlisted 10 substantial open PRs, and — importantly —
tested each one's mergeability with `git merge-tree` against current master rather than trusting
GitHub's cached flag. Three were selected for the pilot, all "big and serious": substantial
engine changes with real design discussion behind them.

**Leg 1 — manual physics space stepping.** Adds `space_step()`, `space_flush_queries()` and
`space_get_last_process_info()` to `PhysicsServer2D/3D` for rollback-netcode and prediction use
cases. Merge-base was **3,696 commits** behind master; **14 of 22 files conflicted**, because
upstream had since moved the physics enums into `PhysicsServer{2,3}DEnums` namespaces. Rebased,
conflicts resolved onto the current API, reviewed, fixed, tested, pushed, PR opened.

**Five real defects were found in code that had been sitting in review for three years:**

1. **State-corruption bug.** `flushing_queries = true` was set *before* argument validation, so an
   invalid RID returned early with the flag still raised — leaving the server permanently in the
   "flushing" state and silently disabling every guard that reads `is_flushing_queries()`,
   including `Area2D::set_monitorable`. Fixed by validating first and save/restoring rather than
   forcing the flag false, so a nested flush can't clear the outer one.
2. **Resource exhaustion.** The Jolt backend's `space_step()` didn't bracket with
   `job_system->pre_step()/post_step()` the way its own `step()` does, so repeated manual steps
   ran Jolt out of job slots.
3. **Cross-backend inconsistency.** The GodotPhysics backends rejected stepping an active space;
   Jolt silently allowed it, double-advancing the simulation.
4. **Documentation contradicting the code.** The class reference described stepping an active
   space from `_physics_process()` as *the supported workflow* — which all three backends reject.
5. **Compile and style breakage** surfaced by the rebase: `ProcessInfo` needed namespace
   qualification under the current enum API, and two `GDVIRTUAL_BIND` lines were missing
   semicolons (196 of 198 in those files had them).

**Verification.** A new test file, 8 cases / 29 assertions, all passing. The important part is the
proof that the tests are worth anything: with the fix reverted, the regression test **fails** —

```
ERROR: CHECK( !server.is_flushing_queries() ) is NOT correct!
  values: CHECK( false )
```

— and passes once restored. Also verified locally: `clang-format` clean, and `--doctool`
regenerates the class reference with **zero diff**, so the docs match the bindings.

A wrinkle worth knowing: the test harness installs the *dummy* physics servers, so the real
backends are unreachable through `PhysicsServer3DManager`. The tests construct
`GodotPhysicsServer2D/3D` directly instead. An agent that had not checked would have written
tests that passed against a no-op server and proved nothing.

**Leg 2 — the triage said "don't".** The second candidate (a core scene-tree `active` lifecycle
feature) was researched at a cost of 78k tokens, and the correct output was a recommendation
**not to rebase it**: 9 concrete defects including state corruption and a silent rendering bug;
5 of its 16 touched files no longer exist at those paths upstream; and it is blocked on an
architectural objection from the lead architect that was never retracted. Verdict: *"best treated
as a specification to reimplement rather than a diff to rebase."*

That is not a failed leg. **A correct "close this" or "reimplement this" verdict is the product**,
and reaching it costs real analysis. Any cost model that assumes every PR gets rebased is wrong.

**Leg 3 — hexagonal cells in `GridMap`.** A replacement candidate, run by a delegated agent under
the same rules. **6,875 commits** of drift, 5 files conflicting, 22 conflicting hunks in the editor
plugin alone. Cost: **331k tokens, 129 tool calls, ~7 build iterations, 40 minutes wall-clock.**

Several conflicts were semantic rather than textual — upstream had replaced the edit-axis enum with
a viewport-override mechanism, collapsed six cursor-rotation menu cases into one undo/redo block,
and rebound the shortcut keys the change relied on. Mechanical conflict resolution would have
produced something that compiled and was wrong.

Two High-severity defects were found:

- **`axial_round()` used integer `abs()`** on floating-point remainders, truncating them to zero,
  so any point not dead-centre in a hexagon could resolve to the wrong cell. A one-character fix
  (`Math::abs`) for a bug that silently broke every hex `local_to_map()`.
- **`make_baked_meshes()` kept rectangular placement**, displacing every mesh in a baked hex map.

Plus a `TypedArray<Basis>` that heap-allocated once per cell in three hot paths, three leaked RIDs
(confirmed by editor-exit leak reports), and a public API method passing cell coordinates into a
local-space setter.

Result: **9 test cases / 250 assertions passing**, revert-proofed (reverting the two High fixes
fails 2 cases / 17 assertions), clang-format clean, `--doctool` zero diff. Verified independently
by re-running the suite and inspecting the fixes rather than taking the agent's word for it.

**Leg 4 — shared hash-table core and a new `AHashSet`.** 3,420 commits of drift, 36 source commits.
Cost: **224k tokens, 114 tool calls, ~24 minutes of build time.** Four defects fixed, including a
vestigial `virtual` destructor left over from a CRTP conversion that made every `AHashMap` and
`AHashSet` polymorphic and grew `sizeof(AHashMap<int,int>)` from 24 to 32 bytes engine-wide.
Two of the four were found by mechanically diffing the new container's API against `HashSet`'s.

Three further defects were found and **deliberately not fixed** — a heap-use-after-free on
self-aliasing insert (ASAN trace captured), `reserve()` able to shrink capacity, and capacity
overflow above 2^31 — because all three reproduce against the pre-refactor header and belong in
their own change. Correct scoping is as much a quality signal as the fixes.

Two things from this leg generalise:

**The tests had never been compiled.** `tests/SCsub` globs `*/**/*.cpp`; the new test file was named
`.h`. Its 263 lines of assertions had never been built or run, and renaming it exposed a latent
compile error immediately. A green CI badge on that branch would have meant nothing. *Verify that
the tests actually execute* belongs on the gate list above, ahead of "the tests pass."

**The benchmark contradicted the change's own premise, and was reported anyway.** New versus
pre-refactor `AHashMap` came out within ±2% on every operation at N=10⁶ — the extraction is
performance-*neutral*, which is correct for a deduplication. The large wins in the numbers
(~1.6× insert, ~6× iteration over `HashMap`) predate the change entirely. An agent optimising to
look good reports the second comparison and omits the first.

**What all three completed legs have in common** is the useful signal: the *rebase* was routine in
every case, and the *value* was in the review. Five defects, six-plus, and four — all in code humans
had already reviewed, some of it for years. The stale-PR backlog is not merely a queue of work to
redo; it is a corpus of partially-reviewed code with real bugs still sitting in it.

Token cost per hard PR: **275k, 331k, 224k** — call it ~275k for the hard tier. Since two-thirds of
the backlog changes fewer than 100 lines, the 200k blended default in the model is if anything
conservative for the median PR and low for the tail.

---

## The backlog, measured

119 open PRs were fetched by head SHA and diffed locally against their own merge-base.

| | mean | median | p90 | p99 | max |
|---|---|---|---|---|---|
| changed lines | 553 | **28** | 569 | 2,208 | 47,119 |
| files touched | 7.1 | **2** | 16 | 77 | 249 |
| diff bytes | 40,323 | **3,767** | 41,283 | 141,818 | 3,267,571 |

The median PR is **28 lines across 2 files**. Two-thirds are under 100 lines. The entire diff
corpus for all 5,236 PRs is about **211 MB ≈ 59M tokens** — the diffs are emphatically *not* the
cost driver. What costs tokens is the surrounding context an agent must read to judge a change,
and the build-test-fix loop.

**Age profile** (population counts, not a sample): half the backlog is under 18 months old; only
4% predates 2022. This is a standing wave, not a swamp — humans merge ~362 PRs/month and inflow
roughly matches it. Two people perform essentially all merges.

---

## Where the money goes

Default scenario: 35% closed at triage, 25% of the remainder rejected or sent back for
reimplementation, ~2,550 PRs carried through to a tested branch. 880M tokens, 20% output,
60% cache hit rate.

| model | cache read $/M | total spend | per PR |
|---|---|---|---|
| DeepSeek V4 Flash | 0.0028 | **$858** | $0.16 |
| DeepSeek V4 Pro | 0.0036 | $2,194 | $0.42 |
| MiniMax M2.7 | 0.030 | $4,191 | $0.80 |
| Kimi K2.6 | 0.095 | $13,310 | $2.54 |
| GLM-5.2 | 0.120 | $16,666 | $3.18 |
| Claude Sonnet 5 | 0.300 | $45,401 | $8.67 |
| Claude Opus 5 | 0.500 | $75,669 | $14.45 |
| Claude Fable 5 | 1.000 | $151,338 | $28.90 |

Ranked by cache-read price, since that is 96% of the bill. The spread from cheapest to most
expensive is ~175×, and at the open-weight end the absolute numbers are still small **Model choice should be made on defect-catch rate, not
price.** Spending $6,000 instead of $90 to find one more state-corruption bug is obviously
correct; spending $90 and shipping a silent rendering regression into a fork you then have to
debug is not a saving.

An honest caveat, and a correction. An earlier draft of this study put the total at 880M tokens
and $6,017 on Opus 5. That was wrong by about 100× on volume and 12× on cost, because it counted
unique content rather than billed tokens. The figures above are measured: 1.05B for triage, 6.8B
for deep review, 89.3B for full treatment, from per-stage costs of 0.19M / 1.25–2.79M / 25.0–44.2M
billed tokens respectively. Those per-stage numbers come from the hard tier; two-thirds of the
backlog changes fewer than 100 lines and will be materially cheaper, which the interactive model
lets you vary.

---

## What actually binds

| phase | work | elapsed |
|---|---|---|
| agent work | 3,269 agent-hours | **2.7 days** at 50 concurrent |
| builds & tests | 638 build-hours | ~13 hours parallelised |
| human audit | 425 person-hours | **~10 months** at 10 h/week |

The human gate is ~110× everything else. Three consequences:

1. **Raising the close-at-triage rate is the highest-leverage lever**, because it removes PRs
   from the human queue entirely. Moving 35% → 55% cuts the human gate by nearly a third.
2. **Batching by subsystem beats batching by age.** A reviewer who has just loaded the physics
   server into their head can audit six physics PRs far faster than six unrelated ones.
3. **Agent thoroughness is nearly free, human thoroughness is not.** Spend tokens generously on
   verification, adversarial review, and revert-proofs — anything that lets a human accept a
   change in two minutes instead of twenty pays for itself ~100×.

---

## Does the quality hold up?

On the evidence of one hard leg: yes, but only with the gates in place.

What worked: the agent found five genuine defects in three-year-old reviewed code, including one
that would have wedged the physics server; it correctly refused to mechanically rebase a PR that
needed reimplementation; it noticed the test harness was running a dummy server and would have
produced meaningless tests; and it proved its own regression test by reverting the fix.

What that depended on: **a working build**, a real test suite, and a revert-proof discipline. Take
any of those away and the same agent produces confident, plausible, unverified changes. The
quality is in the harness, not the model.

Recommended gates, in the order they pay off:
1. It must compile, and the full test suite must pass.
2. **Confirm the tests actually run.** One leg's change shipped 263 lines of assertions that the
   build never compiled, because the file extension didn't match the glob. "Tests pass" and "tests
   exist" are different claims, and only one of them is cheap to fake by accident.
3. Every claimed bug fix must have a test that fails without the fix. Demonstrate the failure.
4. Style and doc consistency checks (`clang-format`, `--doctool` zero-diff) — cheap, and they catch
   real breakage rather than cosmetics. **Run the project's lint scripts with the project's own file
   filters, and diff the tree afterwards.** Several of Godot's `misc/scripts/*.py` hooks rewrite files
   in place and are scoped by a `files:` regex in `.pre-commit-config.yaml` rather than by any check
   inside the script — `header_guards.py` given a `.cpp` file will cheerfully insert `#pragma once`
   into it and print `FIXED`. Invoking those scripts directly without replicating the filter silently
   corrupts source. A `git status --porcelain` assertion after any "check" step is the cheap guard,
   and it belongs in the pipeline: an autofixing linter is a write, not a read.
5. Where a change claims a performance win, benchmark it against the thing it claims to beat, and
   report the result even when it disagrees with the premise.
6. An independent reviewer agent that did not write the change, prompted to refute rather than confirm.
7. Human sampling — not every PR, but every PR in a subsystem the agent has not been audited on yet.

---

## The other workstream: auditing code that nobody sent you

A bounded audit was run over one hot subsystem — `core/string` (`ustring`, `string_name`,
`string_builder`, ~7,400 lines read end to end). Cost: **202k tokens, 33 tool calls, ~30 minutes**.
Yield: **29 findings — 2 High, 11 Medium, 16 Low**, 28 of them verified by reading the surrounding
code rather than asserted.

The two High findings are real bugs in shipped code:

1. **Heap overflow in `String::repeat()`** (`ustring.cpp:3936`). `p_count * len` is computed in
   `int` and the `resize_uninitialized()` error return is discarded, then the code memcpys
   unconditionally. `repeat`, `lpad` and `rpad` are all script-bound, so `"ab".repeat(1073741824)`
   is a one-line reach from GDScript.
2. **Integer-overflow guard in `String::to_int()` fires one digit too late** (`ustring.cpp:2281`),
   so every 19-digit input bypasses it. **Independently reproduced here against the built engine:**

   ```
   "9223372036854775807".to_int()  ->  9223372036854775807   (correct)
   "9223372036854775808".to_int()  -> -9223372036854775808   (silently wraps)
   "9999999999999999999".to_int()  -> -8446744073709551617
   "-9223372036854775809".to_int() ->  9223372036854775807   (negative in, positive out)
   ```

   No error is printed. Any project parsing untrusted numeric text is exposed.

Also found: `append_utf16()` calls `clear()` when its input decodes to nothing — an *append* that
destroys existing content, unlike all four sibling `append_*` functions; `c_unescape()` mishandles
`\\n`; `uri_decode()` silently ignores lowercase hex escapes like `%2f`; and `sprintf()` — which
backs `vformat()` and every engine error message — appends literal text one character at a time
through the full UTF-32 validation path with no `reserve()`.

**This is the better investment.** Per token it produced more defensible value than PR processing,
and it has no human-throughput ceiling baked in: findings can be queued and triaged at whatever
rate suits, they do not depend on a stranger's three-year-old patch, and each one is independently
verifiable. The `core/string` sweep billed **7.2M tokens**. Godot has on the order of 30–40 comparable
subsystems, so a first full pass is roughly **250M billed tokens — about $2 on DeepSeek V4 Flash,
$190 on Opus 5.**
Full findings in `audit-core-string.md`.

## The upstream wall

The value produced here cannot simply be pushed upstream. Godot's rules, effective ~2026-06-30:

> "The use of AI to contribute to Godot is discouraged, and contributions made entirely by AI are prohibited."

Plus mandatory disclosure of any AI use, and: *"Please only submit code that you understand and
are prepared to explain to a maintainer."* Press coverage framed this as a flat ban; the actual
text is narrower — discouraged in general, prohibited only when a contribution is *entirely* AI,
disclosure always. But the operative constraint is a human who understands and can defend each
change, which is precisely the bottleneck the exercise set out to relieve.

Practical consequence: the fork is the venue. If any of this work is ever offered upstream it has
to go one PR at a time, through a human who has genuinely read it, with AI use disclosed — and
that person's throughput, not the agents', sets the rate.

One infrastructure note: the fork's CI **does not run**. All 9 Godot workflows are present and
active, but GitHub disables Actions on forks until the owner enables them, and `workflow_dispatch`
via API returns `403`. All verification in this pilot was local. Enabling Actions on the fork is
the single highest-value setup step for scaling this, since it replaces a 4-core box with
Godot's full 7-platform matrix including ASan/UBSan/TSan.

---

## Recommendation

**Do it, on the fork, in this order:**

1. **Enable GitHub Actions on the fork.** Everything else is gated on being able to verify at scale.
2. **Run a triage-only pass first** (~$25 on a cheap model, a few days). Output: a ranked
   dashboard of what is obsolete, what is a clean rebase, what needs reimplementation, and what
   is genuinely valuable. This is the cheapest thing in the whole project and it is what tells
   you whether the rest is worth doing.
3. **Pick one subsystem** and run the full pipeline on it — 30–50 PRs, batched so a human audits
   them in one sitting. Measure the human minutes per PR for real; that single number determines
   the project's duration more than everything else combined.
4. **Then scale**, with model choice set by measured defect-catch rate on that subsystem rather
   than by list price.

**Run the audit sweep first, regardless.** It is the cheapest thing on this list — a full pass over
every major subsystem is single-digit dollars on an open-weight model — and the `core/string` pilot
alone surfaced two exploitable bugs in code that ships today. It needs no backlog, no rebasing, and
no upstream cooperation.

---

## Files

- `measurements.md` — every raw number, with how it was obtained
- `audit-core-string.md` — 29 findings from the subsystem audit, ranked, with file:line
- `token-log.md` — the measured token spend for this session, per component, priced across 8 models
- `calculator.html` — interactive model; vary any assumption and see which constraint binds
