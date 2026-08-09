# Token log — measured, this session

Source: the session transcript and per-subagent transcripts under `~/.claude/projects/`.
These are the **actual billed usage fields returned by the API**, not estimates.

| component | API calls | output | cache write | cache read | fresh in | **billed total** |
|---|---:|---:|---:|---:|---:|---:|
| Main thread (orchestration + physics leg + study + calculator) | 475 | 688,191 | 8,081,125 | 200,842,366 | 12,158 | **209,623,840** |
| GridMap leg — rebase, review, fix, test, push | 225 | 5,387 | 724,693 | 43,459,016 | 450 | **44,189,546** |
| AHashMap leg — rebase, review, fix, test, benchmark, push | 192 | 9,616 | 1,109,196 | 23,886,663 | 384 | **25,005,859** |
| core/string subsystem audit | 60 | 1,023 | 420,317 | 6,758,057 | 120 | **7,179,517** |
| Backlog sampling — 800 newest open PRs | 63 | 2,082 | 153,933 | 3,273,720 | 126 | **3,429,861** |
| Deep review: scene-tree lifecycle (verdict: do not rebase) | 50 | 4,520 | 204,019 | 2,577,117 | 100 | **2,785,756** |
| Candidate shortlisting (10 PRs, git-tested mergeability) | 52 | 346 | 132,850 | 1,774,172 | 104 | **1,907,472** |
| Backlog sampling — age-stratified, 600 PRs | 51 | 1,321 | 136,938 | 1,756,542 | 102 | **1,894,903** |
| Deep review: physics space stepping | 29 | 551 | 129,225 | 1,120,606 | 58 | **1,250,440** |
| (early helper) | 27 | 719 | 145,749 | 582,576 | 54 | **729,098** |
| **TOTAL** | **1,224** | **713,756** | **11,238,045** | **286,030,835** | **13,656** | **297,996,292** |

## Shape of the spend

- **298M billed tokens** total
- Output is only **0.24%** of it — 713,756 tokens
- Cache reads are **96.0%** — 286M
- Cache hit rate on input: **96.2%**
- Mean context per API call: **243k tokens**

## What this session cost, by model

| model | in $/M | out $/M | cache read $/M | cache write $/M | **cost** |
|---|---:|---:|---:|---:|---:|
| DeepSeek V4 Flash | 0.14 | 0.28 | 0.0028 | 0.14 | **$2.58** |
| MiniMax M2.7 | 0.3 | 1.2 | 0.03 | 0.3 | **$12.81** |
| DeepSeek V4 Pro | 0.435 | 0.87 | 0.003625 | 0.435 | **$6.55** |
| Kimi K2.6 | 0.95 | 4.0 | 0.095 | 0.95 | **$40.72** |
| GLM-5.2 | 1.2 | 4.3 | 0.12 | 1.2 | **$50.89** |
| Claude Sonnet 5 | 3 | 15 | 0.3 | 3.75 | **$138.70** |
| Claude Opus 5 | 5 | 25 | 0.5 | 6.25 | **$231.17** |
| Claude Fable 5 | 10 | 50 | 1.0 | 12.5 | **$462.33** |

Anthropic cache-write rates are 1.25x input (5-minute TTL). Open-weight vendors bill cache
writes at the normal input rate, so those columns repeat the input price.

## Deliverables this bought

Three substantial pull requests rebased across 3,420-6,875 commits of drift, reviewed,
defect-fixed, tested and pushed; one deep review that correctly concluded *do not rebase*;
a 29-finding subsystem audit; the backlog measurement (1,400 PRs sampled, 119 diffed
locally); and the study, calculator and report.
