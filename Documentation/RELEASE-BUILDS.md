# Release Builds

GitHub Actions builds release downloads automatically when changes are pushed or merged into `main`.

Workflow:

```text
.github/workflows/release-builds.yml
```

## Result

The workflow creates a GitHub Release as soon as the macOS build finishes.

Release titles and versions follow `Documentation/RELEASE-RULES.md`:

```text
MVP Alpha X.Y.Z
```

The macOS download uses a versioned filename:

- `IsolatedGreenhouse_X.Y.Z_macOS.zip`

If the Windows build is enabled and a Windows runner is online, the workflow also attaches:

- `IsolatedGreenhouse_X.Y.Z_Windows.zip`

Windows contains the packaged Unreal build with the `.exe`. macOS contains the packaged Mac build.

Only upload a single Windows `.exe` asset after a real standalone installer or self-contained build exists. A single Unreal game `.exe` from the packaged folder is not a working download by itself.

## Runner Requirements

Unreal Engine 5.8 is not installed on normal GitHub-hosted runners. The workflow therefore needs self-hosted runners:

- macOS runner with labels: `self-hosted`, `macOS`
- optional Windows runner with labels: `self-hosted`, `Windows`

Unreal Engine 5.8 must be installed on every enabled runner.

Expected default paths:

```text
Windows: C:\Program Files\Epic Games\UE_5.8
macOS: /Users/Shared/Epic Games/UE_5.8
```

If Unreal is installed somewhere else, update these values in `.github/workflows/release-builds.yml`:

```text
WINDOWS_UE_ROOT
MAC_UE_ROOT
```

The Windows build is disabled by default so releases do not stay queued forever when no Windows runner is registered. Enable it in one of two ways:

- Set the repository variable `BUILD_WINDOWS_RELEASE` to `true` to include Windows on automatic `main` releases.
- Enable `build_windows` when manually starting the workflow.

## Git LFS

The workflow checks out LFS files and runs `git lfs pull`. The runners therefore need Git LFS installed.

## Release Version And Notes

The workflow searches for the newest tag matching:

```text
mvp-alpha-X.Y.Z
```

For a normal `main` push, the default bump is `patch`. For larger releases, start the workflow manually and set `version_bump`, `release_description`, and `release_changelog`.

The release body always uses:

```text
Description
Changelog
```

Do not add a manual `Contributors` section to the release body. Use GitHub's native contributor block only. Do not use automatically generated `What's Changed` or `Full Changelog` sections unless Eddi explicitly asks for a fully generated GitHub release.

## macOS Runner Stability

The macOS release build runs with `caffeinate` so the self-hosted runner does not sleep during `RunUAT BuildCookRun`. The job has a 180-minute timeout.

If the macOS build fails or gets cancelled, the workflow also uploads the artifact `IsolatedGreenhouse-macOS-logs`. It contains Unreal and BuildTool logs, plus snapshots for memory, disk space, and running processes. For runner connection problems, check this log artifact first, then the local runner diagnostic logs under the runner folder `_diag/`.

## Manual Runs

The workflow can also be started manually through GitHub:

```text
Actions -> Build Game Releases -> Run workflow
```

If Windows should be included, enable `build_windows` and make sure the Windows runner is online.
