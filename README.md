# CFG Quiz Engine

A terminal-based quiz application written in C++ that tests a user's understanding of Context-Free Grammars (CFGs). The engine dynamically generates strings from loaded grammar definitions, presents them as multiple-choice questions, and tracks scores across sessions via a persistent scoreboard.

---

## Table of Contents

- [Overview](#overview)
- [Project Structure](#project-structure)
- [Core Concepts](#core-concepts)
- [Architecture](#architecture)
  - [Data Structures](#data-structures)
  - [CFG Class](#cfg-class)
  - [String Derivation Algorithm](#string-derivation-algorithm)
- [Grammar File Format](#grammar-file-format)
- [Difficulty Levels](#difficulty-levels)
- [Quiz Logic](#quiz-logic)
- [Admin Panel](#admin-panel)
- [Scoreboard](#scoreboard)
- [Building and Running](#building-and-running)
- [Known Limitations and Future Work](#known-limitations-and-future-work)

---

## Overview

The CFG Quiz Engine is a C++ console application designed as an educational tool for students studying formal language theory and compiler design. It loads grammar definitions from external text files, generates valid strings by performing rule-based derivations, and quizzes users on their ability to identify valid and invalid strings belonging to a given grammar.

The project demonstrates applied use of formal language theory, recursive string derivation, file I/O, randomized question generation, and a two-mode application architecture (user and admin).

---

## Project Structure

```
CFG-Quiz-Engine/
|
|-- main.cpp              # All application logic: CFG class, quiz engine, admin panel, file I/O
|-- cfgs.txt              # Master grammar file (used by the admin panel)
|-- easy_cfgs.txt         # Grammar definitions for easy difficulty
|-- medium_cfgs.txt       # Grammar definitions for medium difficulty
|-- hard_cfgs.txt         # Grammar definitions for hard difficulty
|-- Scoreboard.txt        # Persistent record of user scores
```

---

## Core Concepts

**Context-Free Grammar (CFG):** A formal grammar consisting of a set of production rules that describe all possible strings in a formal language. A CFG is defined by a 4-tuple: a set of non-terminal symbols, a set of terminal symbols, a set of production rules, and a designated start symbol.

**Derivation:** The process of repeatedly applying production rules to a sentential form, beginning from the start symbol, until only terminal symbols remain. The engine implements this recursively with a configurable depth limit.

**Non-Terminal:** A grammar symbol that can be expanded using a production rule (e.g., `S`, `A`, `B`).

**Terminal:** An atomic symbol that cannot be expanded further (e.g., `a`, `b`, `c`, `d`).

**Empty production:** A rule that maps a non-terminal to the empty string, denoted as `empty` in grammar files and stored internally as `""`.

---

## Architecture

### Data Structures

**`Production` struct**

Represents a single production rule in the grammar.

```cpp
struct Production {
    string lhs;           // Left-hand side non-terminal
    vector<string> rhs;   // List of alternative right-hand side strings (each alternative is one string)
};
```

Each entry in `rhs` is a raw concatenated string of symbols (e.g., `"aSb"` represents the sequence `a`, `S`, `b`). The derivation engine then splits these character-by-character at runtime.

**`cfg_arr` (global `vector<CFG>`)**

A globally maintained array of loaded `CFG` objects. It is populated by `readGrammarArrayFromFile()` at startup and mutated by the admin panel.

---

### CFG Class

The `CFG` class encapsulates a single context-free grammar.

| Member | Type | Description |
|---|---|---|
| `rules` | `map<string, Production>` | Maps each non-terminal symbol to its production rule |
| `startSymbol` | `string` | The designated start symbol for this grammar |

**Public methods:**

- `CFG(const string& start)` — Constructor. Initializes the grammar with a given start symbol.
- `void addRule(const string& lhs, const vector<string>& alternatives)` — Registers a production rule for a non-terminal.
- `string generateString(int maxDepth = 5)` — Entry point for string generation. Initializes the symbol list with the start symbol and delegates to `derive()`.

**Friend functions:**

- `operator<<` — Pretty-prints all non-terminals and their production rules to stdout.
- `writeGrammarArrayToFile` / `readGrammarArrayFromFile` — Declared as friends to allow direct access to private `rules` and `startSymbol` during serialization.

---

### String Derivation Algorithm

The core derivation logic is implemented in the private `derive()` method using recursive descent with backtracking.

```
derive(symbols, checked, depth):
    if depth < 0:
        if any non-terminal remains in symbols → return "E" (failure)
        else → return concatenated terminals

    collect all non-terminals from symbols into toDerive

    if no non-terminals remain → return concatenated result (base case)

    for each non-terminal sym in toDerive:
        loop:
            randomly select a rule from rules[sym].rhs
            if rule is empty-string and depth > 0 → retry (avoid premature termination)
            if rule was already tried for this sym (tracked in `checked`) → retry
            recursively derive the expansion of this rule
            if derivation fails ("E") → mark rule as checked for sym, retry
            if derivation succeeds → replace sym in symbols with derived string, continue

    return concatenated symbols
```

The `checked` map serves as a per-call memoization structure that prevents the engine from retrying production rules that have already led to derivation failure, ensuring termination and variety.

The helper function `replaceStringWithVector()` performs an in-place substitution in the working `symbols` vector, replacing the first occurrence of a non-terminal with its derived terminal string.

---

## Grammar File Format

Grammar files use a plain-text block format. Each grammar block is delimited by `START` and `END` keywords.

```
START <start_symbol>
RULES <number_of_rules>
<NonTerminal> -> <NonTerminal> <rule1> <rule2> ... <ruleN>
...
END
```

**Conventions:**
- Each rule alternative is written as a single token (e.g., `aSb` represents the string `a S b`).
- The keyword `empty` represents the empty string production (stored as `""` internally).
- The second token after `->` is the left-hand side non-terminal repeated (artifact of the serialization format; it mirrors the key).

**Example:**

```
START S
RULES 1
S -> S aSb empty
END
```

This defines the grammar `S → aSb | ε`, which generates the language of all strings of the form `a^n b^n` for `n >= 0`.

---

## Difficulty Levels

Grammars are separated into three difficulty tiers stored in dedicated files.

**Easy (`easy_cfgs.txt`):** Single or two-rule grammars producing straightforward palindromic or bracketed languages. No primed non-terminals. Example languages: `{ a^n b^n }`, `{ ww^R | w ∈ {a,b,c,d}* }`.

**Medium (`medium_cfgs.txt`):** Two-rule grammars introducing a primed non-terminal `S'` as a wrapper, increasing structural ambiguity. Includes grammars for equal-count symbol languages and non-palindromic nested structures.

**Hard (`hard_cfgs.txt`):** Four to five-rule grammars with multiple interacting non-terminals, primed start symbols, and cross-dependent productions. These grammars describe complex intersection-like languages that require careful multi-step reasoning to analyze.

---

## Quiz Logic

The quiz is initiated from the main menu. A session consists of 10 questions generated by the `Quiz()` function.

For each question, a question type is randomly selected via `rand() % 3`. Currently, all active question types dispatch to `type1()`, which implements the following flow:

1. A random grammar is selected from the loaded `cfg_arr`.
2. Three valid strings are generated using `generateString(6)` (maximum derivation depth of 6).
3. A fourth string is generated from a **different** randomly selected grammar, making it invalid with respect to the displayed grammar.
4. The four options are shuffled using a time-seeded `default_random_engine`.
5. The user selects the option they believe does not belong to the grammar.
6. The answer is validated and feedback is printed immediately.

The question type stubs `type2()` and `type3()` are defined but not yet implemented, reserved for future question formats such as identifying a valid string or constructing a derivation.

---

## Admin Panel

The admin panel is accessible from the main menu and provides grammar management functionality.

**Add CFGs (`Add_CFGs()`):**
- Prompts the user to enter non-terminals and their production rules interactively.
- Type `next` to finalize the rules for a non-terminal.
- Type `stop` to finish adding rules and commit the new grammar.
- The updated grammar set is immediately persisted to `cfgs.txt`.

**View / Remove CFGs (`View_CFGs()`):**
- Reads and displays all grammars from `cfgs.txt` with 1-based indices.
- Allows deletion of a grammar by entering its index.
- Entering `0` exits without making changes.
- Changes are written back to `cfgs.txt` immediately.

---

## Scoreboard

Scores are persisted in `Scoreboard.txt` as space-separated integers. Each entry represents a score from a completed session. The file is appended to across sessions, maintaining a historical record of all attempts.

Example content:
```
9 1 
17 
```

---

## Building and Running

**Requirements:**
- A C++17-compliant compiler (e.g., `g++ 7+`, `clang++ 5+`)
- All grammar `.txt` files must be present in the same directory as the compiled executable

**Compile:**

```bash
g++ -std=c++17 -o cfg_quiz main.cpp
```

**Run:**

```bash
./cfg_quiz
```

**Main Menu:**

```
1. Quiz       -- Start a 10-question quiz session
2. Admin      -- Open the grammar management panel
3. Exit       -- Terminate the program
```

---


- **Single-character symbol assumption:** The derivation engine splits rule strings character-by-character (`string(1, test[i])`), which means multi-character non-terminal names (e.g., `S'`) will be incorrectly split. Multi-character terminals or non-terminals require a tokenized rule representation.
