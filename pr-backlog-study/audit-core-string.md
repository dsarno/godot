# Code-quality audit — `core/string` (ustring, string_name, string_builder)

Repo: `/home/user/godot` @ `d488d11119` (upstream master).
Scope: `core/string/ustring.{h,cpp}`, `core/string/string_name.{h,cpp}`, `core/string/string_builder.{h,cpp}`.
Findings only — **no files were modified**.

## Ranked findings

| # | Severity | Category | file:line | One-line summary |
|---|---|---|---|---|
| 1 | High | Correctness (memory safety) | `core/string/ustring.cpp:3936-3948` | `String::repeat()` computes the new size in `int` and ignores the `resize_uninitialized()` failure, then memcpy-writes unconditionally → out-of-bounds heap writes, reachable from GDScript |
| 2 | High | Correctness | `core/string/ustring.cpp:2281` | `_to_int()` overflow guard fires one digit too late: 19-digit values silently wrap instead of clamping (`"9223372036854775808".to_int()` → `INT64_MIN`; a negative input can come back positive) |
| 3 | Med | Correctness | `core/string/ustring.cpp:2085-2088` | `String::append_utf16()` calls `clear()` when the decoded input is empty — an *append* that destroys pre-existing content |
| 4 | Med | Correctness / API | `core/string/ustring.h:565-566` | `append_utf16(Span, bool p_skip_cr)` forwards `p_skip_cr` into the callee's `p_default_little_endian`; no skip-CR behaviour exists and the endianness default silently flips to big-endian |
| 5 | Med | Correctness | `core/string/ustring.cpp:4465-4479` | `c_unescape()` unescapes `\\` *last*, so `\\n` decodes to backslash+LF instead of backslash+`n` |
| 6 | Med | Correctness | `core/string/ustring.cpp:4423-4425`, `4448-4450` | `uri_decode()` / `uri_file_decode()` accept only digits and `A-Z` in percent escapes: lowercase hex (`%2f`) is left undecoded, and `%ZZ` decodes to NUL |
| 7 | Med | Correctness (memory safety) | `core/string/ustring.cpp:223-233`, `186-221`, `292` | `append_utf32_unchecked()` / `append_utf32()` resize before reading the source span → self-append (`s += s`) reads freed memory; `CowData::append()` explicitly guards this case, these do not |
| 8 | Med | Correctness | `core/string/ustring.cpp:3066`, `3199`, `3289`, `3323`, `3137`, `3159` | The Latin-1 `const char *` overloads sign-extend bytes ≥ 0x80, so `find`/`rfind` (1-char path), `begins_with`, `ends_with`, `findn`, `rfindn` disagree with `operator==` for non-ASCII literals |
| 9 | Med | Performance | `core/string/ustring.cpp:5214`, `5591` | `String::sprintf()` (backs `vformat()` and GDScript `%`) appends literal text one char at a time through `append_utf32()`; no `reserve()` |
| 10 | Med | Performance | `core/string/ustring.cpp:469-472`, `642-645` | `nocasecmp_to()` / `naturalnocasecmp_to()` call `_find_upper()` (binary search over a 1505-entry table) up to 4× per character instead of 2×, with no ASCII fast path |
| 11 | Med | Performance | `core/string/ustring.cpp:763`, `779`, `796` | `capitalize()` / `to_camel_case()` / `to_pascal_case()` re-run `get_slice_count(" ")` (full scan) in the loop condition and `get_slicec()` per index → O(n²) |
| 12 | Med | Performance | `core/string/ustring.cpp:4465-4517` | `c_escape` / `c_unescape` / `json_escape` / `xml_escape` chain 8-10 `replace()` calls → 8-10 full scans + allocations per call; `json_escape()` runs per string during JSON serialization |
| 13 | Med | Correctness (robustness) | `core/string/ustring.cpp:3523-3535` | `_wildcard_match()` recurses once per input character on `*` → stack overflow and exponential blowup on script-supplied patterns (`String.match()`) |
| 14 | Low | Correctness | `core/string/ustring.cpp:1077`, `1159` | `split(const char *)` / `rsplit(const char *)` call `strlen(p_splitter)` *before* the `p_splitter == nullptr` check that follows |
| 15 | Low | Correctness | `core/string/ustring.cpp:1675` | `hex_decode()`'s error message appends the `int` index, which binds to `operator+(char32_t)` → prints a control character instead of the index |
| 16 | Low | Correctness / API | `core/string/ustring.cpp:1971`, `2162` | `utf8()` / `utf16()` write through `(uint8_t *)get_data()` — a const-cast that bypasses CoW and would target the static `_null` if the resize had failed |
| 17 | Low | Performance | `core/string/ustring.cpp:4107-4115`, `4399-4414` | `strip_escapes()` and `uri_encode()` build their result one character at a time through `operator+=` |
| 18 | Low | Correctness | `core/string/string_builder.cpp:46-55` | `StringBuilder::append(const char *)` calls `strlen()` with no null check, stores the pointer without copying (undocumented lifetime requirement), and assumes Latin-1 |
| 19 | Low | Correctness | `core/string/string_builder.cpp:57-67`, `.h:37` | `as_string()` ignores the `resize_uninitialized()` error; `string_length` is a `uint32_t` accumulated with no overflow check while write positions are `int` |
| 20 | Low | Dead code | `core/string/ustring.cpp:54-56` | `#define _CRT_SECURE_NO_WARNINGS` sits *after* all `#include`s, so it can never suppress anything |
| 21 | Low | Dead code | `core/string/ustring.cpp:2650-2665` | `to_int(const char32_t *)` builds a diagnostic string with O(n²) `+=` before checking `p_clamp`, then discards it on the clamp path |
| 22 | Low | Duplication | `core/string/ustring.cpp:2266-2300` vs `2615-2677` | Two independent integer parsers with different (and differently wrong) overflow handling |
| 23 | Low | API | `core/string/string_name.cpp:149-155` | `StringName::operator==(const char *)` dereferences `p_name` without the null check that the constructor has |
| 24 | Low | Performance | `core/string/string_name.cpp:111-113` | `unref()` re-checks `configured` although the inline destructor already did, on every StringName destruction |
| 25 | Low | Performance / API | `core/string/ustring.cpp:4389-4397` | `is_valid_string()` has no early exit — it keeps scanning after the first invalid code point |
| 26 | Low | API | `core/string/ustring.h:531-532` | `has_extension()` lowercases only the receiver, so an uppercase argument always returns false (undocumented precondition) |
| 27 | Low | API | `core/string/string_name.h:106-117` | `StringName::operator<` orders by `_Data *` address, so sorted output varies run to run |
| 28 | Low | Performance | `core/string/ustring.cpp:5024`, `5033`, `5042-5043`, `5141-5142` | `get_base_dir` / `get_file` / `get_extension` / `get_basename` each do 2-3 full reverse scans of the path |
| 29 | Low | Dead code | `core/string/ustring.cpp:4528-4532` | Commented-out escaping loop left in `xml_escape()` |

---

## Detail

### 1. `String::repeat()` — OOB heap write from unchecked size math (High, Verified)
```
ustring.cpp:3936   int len = length();
ustring.cpp:3937   String new_string = *this;
ustring.cpp:3938   new_string.resize_uninitialized(p_count * len + 1);
ustring.cpp:3940   char32_t *dst = new_string.ptrw();
ustring.cpp:3944       memcpy(dst + offset * len, dst, stride * len * sizeof(char32_t));
ustring.cpp:3948   dst[p_count * len] = _null;
```
`p_count * len` is `int * int`. `repeat`, `lpad` and `rpad` are all bound to script (`core/variant/variant_call.cpp:1999,2072,2073`), so `p_count` is attacker/user-controlled and `len` comes from the receiver. `"ab".repeat(1073741824)` overflows to `INT_MIN`; `resize_uninitialized()` then hits `ERR_FAIL_COND_V(p_size < 0, …)` in `CowData::resize` (`core/templates/cowdata.h:458`) and returns an error **that is discarded**. `new_string` therefore keeps the original 3-element buffer, `ptrw()` succeeds (the fork of a 3-element buffer works fine), and the loop immediately memcpys 8, then 16, then 32… bytes past the end, followed by a wild `dst[-2147483648] = 0`. `"x".lpad(2000000000, "ab")` reaches the same path through `String::repeat` at `ustring.cpp:5170/5180`.
Independently of the overflow, a plain allocation failure for a legitimately huge count leaves the same undersized buffer.
**Fix:** compute the size in `int64_t`, bail out if it exceeds the string-size limit, and check the `Error` returned by `resize_uninitialized()` before touching `ptrw()`. Also worth an early return for `len == 0` — today `"".repeat(2000000000)` spins two billion no-op iterations.
**Confidence: Verified** (arithmetic and the ignored-error path were traced through `CowData::resize`/`_insert_uninitialized`/`ptrw`).

### 2. `_to_int()` overflow guard is off by one digit (High, Verified)
```
ustring.cpp:2281   if (unlikely(digits > 18)) {
ustring.cpp:2282       bool overflow = (integer > INT64_MAX / 10) || …
```
`digits` is the count of digits accumulated *so far*, so the guard first runs while processing the **20th** digit. Every 19-digit input therefore bypasses it, and `INT64_MAX` has 19 digits. I extracted the function verbatim into a standalone program and ran it:

| input | returned | expected |
|---|---|---|
| `9223372036854775807` | `9223372036854775807` | ok |
| `9223372036854775808` | `-9223372036854775808` | `INT64_MAX` + error |
| `9999999999999999999` | `-8446744073709551617` | `INT64_MAX` + error |
| `-9223372036854775809` | `+9223372036854775807` | `INT64_MIN` + error |
| `10000000000000000000` | `9223372036854775807` | ok (20 digits, guard fires) |

Note the fourth row: a negative input returns a **positive** value. This backs `String::to_int()`, `String::to_int(const char *)` and `String::to_int(const wchar_t *)` — used all over parsing/deserialization code — and the failure is silent (no error printed).
**Fix:** change the condition to `digits >= 18` (equivalently, check before the 19th digit is folded in).
**Confidence: Verified** (repro program run; output above).

### 3. `append_utf16()` clears the string on empty input (Med, Verified)
```
ustring.cpp:2085   if (str_size == 0) {
ustring.cpp:2086       clear();
ustring.cpp:2087       return OK; // empty string
```
Every sibling returns without touching existing content — `append_latin1` (`:162`), `append_ascii` (`:1724`), `append_utf32` (`:187`). Only `append_utf16` wipes `*this`. It also triggers for an input consisting solely of a BOM. The one in-tree caller that appends to a possibly non-empty string (`modules/text_server_fb/text_server_fb.cpp:958`) happens to call `clear()` first, so this is currently latent, but it is exposed through GDExtension's `string_new_with_utf16_chars*`.
**Fix:** replace `clear(); return OK;` with a plain `return OK;`.
**Confidence: Verified.**

### 4. `append_utf16(Span, p_skip_cr)` passes the wrong flag (Med, Verified)
```
ustring.h:564   Error append_utf16(const char16_t *p_utf16, int p_len = -1, bool p_default_little_endian = true);
ustring.h:565   Error append_utf16(const Span<char16_t> p_range, bool p_skip_cr = false) {
ustring.h:566       return append_utf16(p_range.ptr(), p_range.size(), p_skip_cr);
```
`p_skip_cr` does not exist anywhere else in the code base except an unrelated `FileAccess::get_as_text_bind_compat_110867` parameter (`core/io/file_access.h:139`); the implementation has no skip-CR logic at all. Worse, the forwarding changes the semantic default from little-endian to **big-endian**: any caller writing `s.append_utf16(span)` gets `byteswap = true` (`ustring.cpp:2027`) and every non-BOM code unit byte-swapped. No in-tree caller uses this overload today (all use the pointer form), so it is a trap rather than an active bug.
**Fix:** rename the parameter to `p_default_little_endian` and default it to `true`, matching the pointer overload.
**Confidence: Verified** (checked all `append_utf16(` call sites).

### 5. `c_unescape()` unescapes backslashes last (Med, Verified)
```
ustring.cpp:4470   escaped = escaped.replace("\\n", "\n");
…
ustring.cpp:4476   escaped = escaped.replace("\\\\", "\\");
```
For input `\` `\` `n` (an escaped backslash followed by the letter n) the `\n` rule matches at index 1 and produces `\` + LF; the correct result is `\` + `n`. Any escape sequence preceded by an escaped backslash is mis-decoded. `c_escape()` (`:4481`) and `json_escape()` (`:4505`) get the ordering right because they escape `\` first; only the inverse operation is wrong. `String.c_unescape()` is script-bound and used for CSV/translation and config parsing.
**Fix:** single-pass scanner that consumes one escape at a time, left to right.
**Confidence: Verified.**

### 6. `uri_decode()` rejects lowercase percent escapes (Med, Verified)
```
ustring.cpp:4423   if (is_digit(ord1) || is_ascii_upper_case(ord1)) {
ustring.cpp:4425       if (is_digit(ord2) || is_ascii_upper_case(ord2)) {
```
`%2f` is a legal (and common) encoding of `/`, but `is_ascii_upper_case('f')` is false, so the `%` is copied verbatim and the string is left half-encoded. Conversely the test accepts *any* uppercase letter, so `%ZZ` passes the filter and `strtol("ZZ", …, 16)` yields `0`, injecting a NUL byte into the `CharString` (which then truncates the result in `String::utf8`). `uri_file_decode()` (`:4442`) has the identical code.
**Fix:** use `is_hex_digit()` for both nibbles.
**Confidence: Verified.**

### 7. Self-append reads freed memory (Med, Likely)
```
ustring.cpp:228   const int prev_length = length();
ustring.cpp:229   resize_uninitialized(prev_length + p_span.size() + 1);
ustring.cpp:230   char32_t *dst = ptrw() + prev_length;
ustring.cpp:231   memcpy(dst, p_span.ptr(), p_span.size() * sizeof(char32_t));
```
When the source span aliases the destination's own buffer (`s += s`, refcount 1) the `resize` path goes through `_realloc_exact()` (`cowdata.h:506`), which may move the allocation, leaving `p_span.ptr()` dangling before the `memcpy`. `append_utf32()` (`:195-197`) has the same shape. `CowData::append(Span<T>)` explicitly protects against exactly this case (`cowdata.h:331-338`, comment "Otherwise it might be invalidated on growth"), which shows the hazard is understood elsewhere in the container layer but not here. `String::operator+(const String &)` is safe by accident: it copies first, so the refcount is 2 and the CoW fork path preserves the old buffer.
I found no in-tree `s += s`, so this is latent; a reproduction would be an ASAN build running `String s = "long…"; s += s;`.
**Fix:** mirror the `span_in_self` check from `CowData::append`, or re-derive the source pointer after the resize.
**Confidence: Likely** (verified by reading both code paths; no runtime repro run).

### 8. Latin-1 sign extension in the `const char *` overloads (Med, Verified)
`String::operator==(const char *)` deliberately reinterprets as unsigned (`ustring.cpp:324`, `.reinterpret<uint8_t>()`), and the multi-character `find` path does the same (`:3069`). These do not:
- `ustring.cpp:3066` — `find_char(*p_str, p_from)` passes a `char`; `(char32_t)(char)0xE9` is `0xFFFFFFE9`, so a one-character non-ASCII needle never matches.
- `ustring.cpp:3199` — same in `rfind(const char *)`.
- `ustring.cpp:3289` — `ends_with(const char *)`: `static_cast<char32_t>(p_string[i])`.
- `ustring.cpp:3323` — `begins_with(const char *)`: `(char32_t)*p_string`.
- `ustring.cpp:3137`/`3159` — `strings_equal_lower()` (`:70`) feeds `char` into `_find_lower(int)` (`ucaps.h:3057`), which sign-extends.

Net effect: `s.contains("é")` and `s.find("é")` return false/-1 while `s == "é"` and `s.find(String("é"))` succeed. Only affects source literals with bytes ≥ 0x80, which is why it has survived, but it is a genuine inconsistency between siblings.
**Fix:** cast through `uint8_t` in all of these, as `operator==` already does.
**Confidence: Verified.**

### 9. `String::sprintf()` appends literal text one character at a time (Med, Verified)
```
ustring.cpp:5591   formatted += c;
```
`sprintf` is the implementation behind GDScript's `%` operator **and** `vformat()` (`core/variant/variant.h:916`), which is used for essentially every error message, warning, editor label and debug string in the engine. Every literal character between format specifiers goes through `String::operator+=(char32_t)` → `append_utf32(Span(&c,1))`, which per character performs an empty check, a `resize_uninitialized()` call (capacity check inside `CowData::resize`/`_insert_uninitialized`), the full NUL/unpaired-surrogate/`>0x10FFFF` validation chain, and a NUL terminator write. Growth itself is amortized (CowData grows 1.5×, `cowdata.h:83-88`), so this is a constant-factor problem, not a quadratic one — but it is a large constant on the hottest string routine in the engine.
**Fix:** track the start of each literal run and flush it with one `append_utf32_unchecked()`, and `formatted.reserve(length())` up front (`String::reserve` already exists, `ustring.h:337`).
**Confidence: Verified** (call chain traced; growth policy confirmed in `cowdata.h`).

### 10. Redundant case-folding lookups in the comparators (Med, Verified)
```
ustring.cpp:469   } else if (_find_upper(*this_str) < _find_upper(*that_str)) {
ustring.cpp:471   } else if (_find_upper(*this_str) > _find_upper(*that_str)) {
```
`_find_upper` (`core/string/ucaps.h:3037`) is a binary search over a 1505-entry table (`LTU_LEN 1505`), i.e. ~11 iterations per call. Written this way the source asks for four searches per character position where two suffice, and there is no ASCII short-circuit even though the overwhelming majority of comparisons are ASCII. The compiler *may* CSE the duplicated calls (the table is `static const` and the function is header-inline) but that is not guaranteed. Identical shape at `ustring.cpp:642-645` in `naturalnocasecmp_to_base`. `nocasecmp_to` backs `NoCaseComparator`/`FileNoCaseComparator`, used for sorting file lists, property lists and autocomplete results; `String::to_lower()`/`to_upper()` (`:1372`,`:1391`) have the same missing ASCII fast path and `to_lower()` is called for every file through `has_extension()`.
**Fix:** hoist the two lookups into locals; add `if (c < 0x80)` fast paths in `_find_upper`/`_find_lower`.
**Confidence: Verified** (table length and function body read).

### 11. O(n²) word iteration in `capitalize()` and friends (Med, Verified)
```
ustring.cpp:763   for (int i = 0; i < words.get_slice_count(" "); i++) {
ustring.cpp:764       String slice = words.get_slicec(' ', i);
```
`get_slice_count()` (`:845`) rescans the whole string on **every** loop iteration, and `get_slicec()` (`:946`) rescans from index 0 to reach slice `i`. Same pattern in `to_camel_case()` (`:779`) and `to_pascal_case()` (`:796`). Additionally `_separate_compound_words()` builds its result with `new_string += substr(...) + " "` (`:736`), allocating two temporaries per word boundary, and calls `length()` in the loop condition (`:720`, `:726`). `capitalize()` runs once per property name in the inspector, so n is small in practice — but the fix is trivial (hoist the count, or walk the string once).
**Confidence: Verified.**

### 12. Escape helpers make 8-10 full passes (Med, Verified)
`c_escape()` (`:4481`), `c_unescape()` (`:4465`), `json_escape()` (`:4505`) and `xml_escape()` (`:4519`) each chain 8-10 `replace()` calls. Each `replace()` is a full `find` scan plus, when it matches, a fresh allocation and copy of the whole string (`_replace_common`, `:3608`). `json_escape()` is called per string value by `core/io/json.cpp:195`, so serializing a JSON document scans every string eight times.
**Fix:** one pass with a switch over the character.
**Confidence: Verified.**

### 13. `_wildcard_match()` recurses per character (Med, Verified)
```
ustring.cpp:3528   case '*': return _wildcard_match(p_pattern + 1, …) || (*p_string && _wildcard_match(p_pattern, p_string + 1, …));
```
The second branch advances the *string* while keeping the pattern, so recursion depth is O(strlen(string)) — a 100k-character subject blows a 1 MB thread stack. Patterns with several `*` groups also give the classic exponential backtracking. `String.match()`/`matchn()` are script-bound and are used on user-supplied filter strings in the editor.
**Fix:** iterative backtracking (single saved star position) — same semantics, O(n·m) worst case, O(1) stack.
**Confidence: Verified** (by reading; no crash repro attempted).

### 14. Null check after the dereference in `split`/`rsplit` (Low, Verified)
```
ustring.cpp:1077   const int splitter_length = strlen(p_splitter);
ustring.cpp:1081       if (p_splitter == nullptr || *p_splitter == '\0') {
```
`strlen` already dereferenced the pointer, so the guard four lines later is dead. Identical in `rsplit(const char *)` (`:1159` vs `:1172`). The sibling `get_slice_count(const char *)` (`:849`, `:855`) gets the order right, which is what makes this an oversight rather than a convention.
**Fix:** move the check above the `strlen`.
**Confidence: Verified.**

### 15. `hex_decode()` formats the index as a character (Low, Verified)
```
ustring.cpp:1675   ERR_FAIL_V_MSG(Vector<uint8_t>(), "…at index " + m_index + ".");
```
`m_index` expands to `i * 2` (an `int`). The only viable overload is `String::operator+(char32_t)` (`ustring.h:359`) — `int` → `char32_t` is an integral conversion, whereas `String` has no integer constructor. So index 4 prints as an unprintable control character. Error-message-only, but it makes the diagnostic useless.
**Fix:** `+ itos(m_index)`.
**Confidence: Verified.**

### 16. Writing through `get_data()` (Low, Verified)
```
ustring.cpp:1971   uint8_t *cdst = (uint8_t *)utf8s.get_data();
ustring.cpp:2162   uint16_t *cdst = (uint16_t *)utf16s.get_data();
```
`get_data()` is the const accessor and returns `size() ? ptr() : &_null` (`ustring.h:188`), i.e. a pointer to a **static** `_null` when the buffer is empty. Casting away const to write is currently harmless (the object was just allocated with refcount 1), but if the preceding `resize_uninitialized()` failed the code writes into the shared static. `ptrw()` is the correct accessor and costs nothing here.
**Confidence: Verified.**

### 17. Character-at-a-time result building (Low, Verified)
`strip_escapes()` (`:4112`) and `uri_encode()` (`:4406`, `:4411`) append per character/per 3-char escape. Same constant-factor issue as #9, smaller blast radius. `uri_encode` runs per HTTP request path/query.

### 18-19. `StringBuilder` (Low, Verified)
- `append(const char *)` (`string_builder.cpp:46-55`): `strlen(p_cstring)` with no null guard (the `String` overload guards emptiness at `:34`), and it stores the **pointer**, not a copy — every C string handed to a `StringBuilder` must outlive it. That contract is not stated in the header. It also copies bytes straight into `char32_t` (`:87`), so a UTF-8 literal produces mojibake rather than an error; the `String` path would have gone through `append_latin1`, which at least diagnoses NULs.
- `as_string()` (`:57-67`): the `resize_uninitialized(string_length + 1)` result is discarded before `ptrw()` and the memcpy loop; `string_length` is a `uint32_t` accumulated with no overflow check (`:41`, `:52`) while `current_position` is an `int` (`:66`), so >2 GiB of accumulated text silently corrupts.
- The class also has no `reserve()`/`reserve_chunks()` and keeps three parallel `LocalVector`s; `append(const String &)` copies the `String` into `strings` (refcount traffic) instead of moving.

### 20-22. Dead / duplicated code (Low, Verified)
- `ustring.cpp:54-56`: `_CRT_SECURE_NO_WARNINGS` is defined after `#include "ustring.h"` and `<cstdio>`, so the CRT headers have already been processed — the define can never take effect.
- `ustring.cpp:2650-2665`: on the overflow path `to_int(const char32_t *)` builds a `number` string with a per-character `+=` loop and then, if `p_clamp` is set, returns without ever using it.
- `ustring.cpp:2266-2300` vs `2615-2677`: two integer parsers. The `char32_t` one guards with `integer > INT64_MAX / 10` *before* multiplying (correct except for the exact values `9223372036854775808`-`9223372036854775809`, where the final `+=` is signed-overflow UB); the templated one has the off-by-one of finding #2. They also disagree on whether a leading `+` is accepted and on clamping semantics.

### 23-24. `StringName` (Low, Verified)
- `string_name.cpp:154`: `return p_name[0] == 0;` — the constructor at `:211` checks `!p_name` first, this comparison operator does not, so `StringName() == (const char *)nullptr` is UB.
- `string_name.cpp:112`: `ERR_FAIL_COND(!configured)` in `unref()` duplicates the `likely(configured)` test the inline destructor already performed (`string_name.h:191`); StringName destruction is extremely frequent. The same early return also leaves `_data` non-null on the `!configured` path while the normal path clears it at `:133` — inconsistent, though only reachable after `cleanup()`.

### 25-29. Smaller items (Low, Verified)
- `ustring.cpp:4393-4395`: `valid = valid && (...)` inside the loop — the loop never breaks, so an invalid string still costs a full scan.
- `ustring.h:531-532`: `has_extension()` lowercases `get_extension()` but not the argument; `path.has_extension("PNG")` is always false.
- `string_name.h:106-117`: `operator<` (and `<=`, `>`, `>=`) compare `_Data *` addresses. That is intentional for map keys and `AlphCompare` exists for the alphabetical case, but nothing in the header says so, and sorting a `Vector<StringName>` yields allocation-order results that vary between runs — a reproducible-output hazard for exporters/generators.
- `ustring.cpp:5024`, `5033`, `5042-5043`, `5141-5142`: `MAX(rfind_char('/'), rfind_char('\\'))` performs two reverse scans, and `get_extension()`/`get_basename()` add a third for `'.'`. These run on every path in `ResourceLoader`/`EditorFileSystem`; a single backward pass would find all three.
- `ustring.cpp:4528-4532`: dead commented-out block in `xml_escape()`.

### Checked and found *not* to be bugs (recorded so they are not re-reported)
- `_replace_common` (`:3635`) mixes `size_t key_length` with `int with_length`; the subtraction wraps but two's-complement modular arithmetic makes the final `int64_t` value correct for all non-negative results.
- `humanize_size` (`:4255`): `_div * 1024` overflows `uint64_t` at magnitude 6, but the `magnitude < 6` guard makes it unreachable in effect.
- `String::ascii()` (`:1710`) iterating to `size()` rather than `length()` is deliberate — it copies the NUL terminator.
- `String::num()`'s `char fmt[7]` (`:1436`) is exactly large enough for the clamped 2-digit precision, and `buf[325]` covers `%lf` of `DBL_MAX`.
- The `refcount.ref()`-fails path in the `StringName` constructors (`:229`, `:285`) correctly allocates a replacement node; the concurrently-dying node is unlinked by its own thread under the same mutex.
- `String::operator+(const String &)` self-concatenation is safe (the copy raises the refcount, forcing the CoW fork path) even though `operator+=` self-append is not (finding #7).

---

## Effort

- **Lines of code actually read:** ~7,360 in-scope (all six files read end to end: `ustring.cpp` 5,834; `ustring.h` 828; `string_name.cpp` 301; `string_name.h` 222; `string_builder.cpp` 98; `string_builder.h` 79) plus ~600 lines of supporting code consulted to verify claims (`core/templates/cowdata.h`, `core/templates/span.h`, `core/string/ucaps.h`, `core/variant/variant.h`, `core/extension/gdextension_interface.cpp`, `modules/text_server_fb/text_server_fb.cpp`). **≈ 8,000 lines total.**
- **Tool calls:** 33 (24 read/grep/bash batches, several containing two parallel calls; 1 compile-and-run verification; 1 file write).
- **Wall-clock:** roughly 30 minutes.
- **Findings by severity:** **High 2**, **Medium 11**, **Low 16** — 29 total. Confidence: 28 Verified, 1 Likely (finding #7), 0 Unverified. One finding (#2) was confirmed with an executed repro rather than by reading alone.
