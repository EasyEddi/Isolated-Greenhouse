# Release Builds

GitHub Actions baut Release-Downloads automatisch, wenn auf `main` gepusht oder gemerged wird.

Workflow:

```text
.github/workflows/release-builds.yml
```

## Ergebnis

Der Workflow erstellt einen GitHub Release, sobald der macOS-Build fertig ist.

Release-Titel und Version folgen `Documentation/RELEASE-RULES.md`:

```text
MVP Alpha X.Y.Z
```

Der macOS-Download wird versioniert benannt:

- `IsolatedGreenhouse_X.Y.Z_macOS.zip`

Wenn der Windows-Build aktiviert ist und ein Windows-Runner online ist, wird danach zusaetzlich angehaengt:

- `IsolatedGreenhouse_X.Y.Z_Windows.zip`

Windows enthaelt den packaged Unreal-Build mit `.exe`. macOS enthaelt den packaged Mac-Build.

Ein einzelnes Windows-`.exe`-Asset soll erst hochgeladen werden, wenn ein echter eigenstaendiger Installer oder self-contained Build existiert. Eine einzelne Unreal-Game-`.exe` aus dem packaged Ordner reicht nicht als funktionierender Download.

## Runner-Anforderung

Unreal Engine 5.8 ist nicht auf normalen GitHub-hosted Runnern installiert. Deshalb braucht der Workflow selbst gehostete Runner:

- macOS Runner mit Labels: `self-hosted`, `macOS`
- optionaler Windows Runner mit Labels: `self-hosted`, `Windows`

Auf jedem aktivierten Runner muss Unreal Engine 5.8 installiert sein.

Erwartete Standardpfade:

```text
Windows: C:\Program Files\Epic Games\UE_5.8
macOS: /Users/Shared/Epic Games/UE_5.8
```

Wenn Unreal woanders liegt, die Pfade in `.github/workflows/release-builds.yml` anpassen:

```text
WINDOWS_UE_ROOT
MAC_UE_ROOT
```

Der Windows-Build ist standardmaessig aus, damit der Release nicht ewig in der Warteschlange haengt, wenn kein Windows-Runner registriert ist. Zum Aktivieren gibt es zwei Wege:

- Repo-Variable `BUILD_WINDOWS_RELEASE` auf `true` setzen, damit Windows bei `main`-Pushes automatisch mitgebaut wird.
- Beim manuellen Workflow-Start `build_windows` aktivieren.

## Git LFS

Der Workflow checkt LFS-Dateien aus und fuehrt `git lfs pull` aus. Die Runner brauchen daher Git LFS.

## Release-Version und Notes

Der Workflow sucht den neuesten Tag im Format:

```text
mvp-alpha-X.Y.Z
```

Bei einem normalen `main`-Push wird standardmaessig ein Patch-Release erstellt. Fuer groessere Releases den Workflow manuell starten und `version_bump`, `release_description` und `release_changelog` setzen.

Der Release-Body verwendet immer:

```text
Description
Changelog
Contributors
```

Keine automatisch generierten `What's Changed`-/`Full Changelog`-Abschnitte verwenden.

## macOS Runner Stabilitaet

Der macOS Release-Build laeuft mit `caffeinate`, damit der Self-hosted Runner waehrend `RunUAT BuildCookRun` nicht einschlaeft. Der Job hat ein Timeout von 180 Minuten.

Wenn der macOS Build fehlschlaegt oder abgebrochen wird, laedt der Workflow zusaetzlich das Artifact `IsolatedGreenhouse-macOS-logs` hoch. Darin liegen Unreal-/BuildTool-Logs sowie Snapshot-Dateien zu Speicher, Speicherplatz und laufenden Prozessen. Bei Runner-Verbindungsproblemen zuerst dieses Log-Artifact und danach die lokalen Runner-Diagnose-Logs unter dem Runner-Ordner `_diag/` pruefen.

## Manuell starten

Der Workflow kann auch manuell ueber GitHub gestartet werden:

```text
Actions -> Build Game Releases -> Run workflow
```

Wenn dabei auch Windows gebaut werden soll, `build_windows` aktivieren und sicherstellen, dass der Windows-Runner online ist.
