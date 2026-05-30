# Contributing to DungeonCraft Pro

Thank you for your interest in contributing! This document explains how to report bugs, suggest features, and submit code changes.

---

## Table of Contents

- [Code of Conduct](#code-of-conduct)
- [Reporting Bugs](#reporting-bugs)
- [Requesting Features](#requesting-features)
- [Development Setup](#development-setup)
- [Submitting a Pull Request](#submitting-a-pull-request)
- [Code Style Guidelines](#code-style-guidelines)
- [Commit Message Format](#commit-message-format)
- [Documentation Contributions](#documentation-contributions)

---

## Code of Conduct

Be respectful, constructive, and professional in all interactions. Harassment or personal attacks will not be tolerated.

---

## Reporting Bugs

1. Search [existing issues](../../issues) to avoid duplicates.
2. Open a new issue with the **bug** label.
3. Use the following template:

```
**Environment**
- Unreal Engine version:
- Platform (Windows/Mac/Linux):
- Plugin version:

**Steps to Reproduce**
1. 
2. 
3. 

**Expected Behavior**
[What should happen]

**Actual Behavior**
[What actually happens]

**Output Log**
[Paste relevant DungeonGenerator log lines here]
```

---

## Requesting Features

1. Search [existing issues](../../issues) for similar requests.
2. Open a new issue with the **enhancement** label.
3. Describe the use case and why the current system does not address it.

---

## Development Setup

### Prerequisites

- Unreal Engine 5.5+
- Visual Studio 2022 or JetBrains Rider with Unreal plugin
- Git

### Setup Steps

1. Fork the repository on GitHub.
2. Clone your fork:
   ```sh
   git clone https://github.com/<your-username>/DungeonCraft_Pro.git
   ```
3. Copy `Plugins/DungeonCraft_Pro_1` into a test UE5 project's `Plugins/` folder.
4. Generate project files and open in the IDE.
5. Build and verify the plugin compiles cleanly.

---

## Submitting a Pull Request

1. **Create a branch** from `main`:
   ```sh
   git checkout -b feature/your-feature-name
   ```
2. **Make your changes** — keep PRs focused. One feature or fix per PR.
3. **Test your changes:**
   - Verify compilation with no warnings.
   - Test in a fresh level with PIE (single player and multiplayer).
   - Run `RunGenerationCycles(10)` and confirm no memory delta.
4. **Update documentation** if you add or change public-facing behavior.
5. **Commit** following the [commit message format](#commit-message-format).
6. **Push** and open a Pull Request against `main`.
7. Fill in the PR description template:
   - What problem does this solve?
   - What changes were made?
   - How was it tested?
   - Any breaking changes?

---

## Code Style Guidelines

- Follow Unreal Engine's [Coding Standard](https://dev.epicgames.com/documentation/en-us/unreal-engine/coding-standard).
- All public functions and properties must have doxygen-style comments.
- Use the `DungeonGenerator` log category (not `LogTemp`) for all plugin log output.
- No `check()` or `ensure()` calls in release code paths — use guarded `if` with `UE_LOG(DungeonGenerator, Error, ...)` instead.
- New properties: include `ClampMin`/`ClampMax` meta if the value has a valid range.
- New spawn functions: always null-check all incoming pointers before dereferencing.

### Example comment style

```cpp
/**
 * Generates a new dungeon layout and spawns all associated meshes and actors.
 * 
 * This function is the main entry point for dungeon generation. It validates
 * the current configuration, clears any existing dungeon, initializes the
 * tile matrix, places rooms, connects them with corridors, and spawns all
 * visual elements.
 *
 * Must be called on the game thread. Re-entrant calls are blocked.
 * In multiplayer, call only on the server (HasAuthority()).
 * 
 * @see ClearDungeon(), OnDungeonSpawned
 */
UFUNCTION(BlueprintCallable, CallInEditor, Category = "Dungeon Generation")
void GenerateDungeon();
```

---

## Commit Message Format

Use the following format:

```
<type>: <short summary>

[optional longer description]

[optional: Closes #issue-number]
```

Types:
- `feat` — new feature
- `fix` — bug fix
- `docs` — documentation only
- `refactor` — code restructure without behavior change
- `perf` — performance improvement
- `test` — adding or updating tests
- `chore` — build, tooling, or config change

**Examples:**
```
feat: add async generation option for large dungeons
fix: blueprint actors now correctly receive DungeonBP tag
docs: expand multiplayer guide with late-join handling
perf: reduce room placement retry overhead on dense grids
```

---

## Documentation Contributions

Documentation lives in the root Markdown files:

| File | Purpose |
|---|---|
| `README.md` | Project overview |
| `QUICKSTART.md` | Step-by-step setup guide |
| `API.md` | Public API reference |
| `CONFIGURATION.md` | Configuration options |
| `TROUBLESHOOTING.md` | Common issues |
| `PERFORMANCE.md` | Performance tuning |
| `MULTIPLAYER.md` | Multiplayer guide |
| `THEMES.md` | Theme system guide |
| `VERSION.md` | Changelog |

When updating documentation:
- Keep language beginner-friendly.
- Verify any code examples compile.
- Update cross-links between documents if pages are restructured.
- Run a markdown linter if available.
