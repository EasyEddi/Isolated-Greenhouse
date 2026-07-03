# Release Rules

These rules apply to GitHub Releases for **The Isolated Greenhouse**.

## Release Title

Every release title must use this format:

```text
MVP Alpha X.Y.Z
```

The first release is:

```text
MVP Alpha 0.0.0
```

## Versioning

Versions use `X.Y.Z`.

- Small patch: increase `Z` by 1.
- Larger patch: increase `Y` by 1 and reset `Z` to 0.
- New feature or many large patches: increase `X` by 1 and reset `Y` and `Z` to 0.
- Keep `X` at 0 until Eddi explicitly says the first number should increase.

If the right bump is unclear, choose the smaller bump or ask Eddi.

## Release Body

The release body must not use automatically generated commit references. Do not use sections such as `What's Changed`, `New Contributors`, or `Full Changelog` unless Eddi explicitly asks for them.

Required format:

```markdown
## Description

Short, readable summary of the update.

## Changelog

- Specific change 1.
- Specific change 2.
- Specific change 3.
```

`Description` is for a broad, readable summary. `Changelog` is for concrete changes without commit links.

## Contributors

Do not add a manual `Contributors` section to the release body. The native GitHub contributor block below the release body is the desired display because it looks official and matches GitHub's own release UI.

The current project contributor profiles are:

- `@EasyEddi`: <https://github.com/EasyEddi>
- `@Tarekke`: <https://github.com/Tarekke>

GitHub's automatic contributor box cannot be manually edited. It is triggered by real GitHub account mentions in the release notes and by accounts GitHub can infer from commits and generated release metadata.

If a contributor's commits use a local email that is not connected to GitHub, mention their GitHub account naturally in the `Changelog`. Do not replace the native block with custom Markdown avatars unless Eddi explicitly asks for a manual workaround.

## Assets

GitHub always shows one release area named `Assets`. This area cannot be duplicated or renamed into separate `Windows` and `Mac` tabs. GitHub also automatically adds `Source code (zip)` and `Source code (tar.gz)` downloads; those are not normal release assets and should not be treated as game downloads.

Separate platforms through asset filenames:

```text
IsolatedGreenhouse_X.Y.Z_macOS.zip
IsolatedGreenhouse_X.Y.Z_Windows.zip
```

Long-term Windows target:

```text
IsolatedGreenhouse_X.Y.Z.exe
```

Only upload a `.exe` if it is a genuinely standalone installer or self-contained build. A single Unreal game `.exe` from a packaged Windows folder is usually not enough because it needs cooked content, Paks, and supporting files next to it. Until a real one-file Windows installer exists, keep Windows as a versioned `.zip` download.

## Workflow Notes

The release workflow calculates the next version from the newest tag matching:

```text
mvp-alpha-X.Y.Z
```

For normal `main` pushes, the default bump is `patch`. For larger releases, start the workflow manually and set `version_bump`, `release_description`, and `release_changelog`. Alternatively, set `release_version` to force an exact version.
