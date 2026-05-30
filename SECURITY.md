# Security Policy — DungeonCraft Pro

## Supported Versions

| Version | Supported |
|---|---|
| 1.0.x | ✅ Yes |
| < 1.0.0 | ❌ No (pre-release) |

## Reporting a Vulnerability

If you discover a security vulnerability in DungeonCraft Pro, please **do not** open a public GitHub issue.

Instead, report it via:

1. **GitHub Private Vulnerability Reporting** — use the "Report a vulnerability" button on the [Security tab](../../security/advisories/new) of this repository.
2. **Direct contact** — open a private communication channel with the maintainer.

Please include:
- A description of the vulnerability
- Steps to reproduce
- Potential impact assessment
- Any suggested mitigations

You will receive a response within **7 days**. If the issue is confirmed, a patch will be released promptly and you will be credited in the release notes (unless you prefer to remain anonymous).

## Scope

Security issues relevant to this project include:

- Memory safety violations in the C++ plugin code
- Arbitrary code execution via malformed data assets or dungeon configuration
- Denial-of-service via malicious spawn parameters

Issues that are **out of scope**: Unreal Engine engine-level vulnerabilities (report those to Epic Games).
