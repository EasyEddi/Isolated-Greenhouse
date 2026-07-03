# Release Rules

Diese Regeln gelten fuer GitHub Releases von **The Isolated Greenhouse**.

## Release-Titel

Der Release-Titel folgt immer diesem Format:

```text
MVP Alpha X.Y.Z
```

Der erste Release ist:

```text
MVP Alpha 0.0.0
```

## Versionierung

Versionen verwenden `X.Y.Z`.

- Kleiner Patch: `Z` wird um 1 erhoeht.
- Groesserer Patch: `Y` wird um 1 erhoeht, `Z` wird auf 0 gesetzt.
- Neues Feature oder viele grosse Patches: `X` wird um 1 erhoeht, `Y` und `Z` werden auf 0 gesetzt.
- `X` bleibt vorerst bei 0, bis Eddi explizit sagt, dass die erste Zahl erhoeht werden soll.

Wenn unklar ist, welcher Versionssprung passend ist, lieber kleiner bumpen und Eddi fragen.

## Release-Beschreibung

Der Release-Body soll keine automatisch generierten Commit-Verweise enthalten. Keine Abschnitte wie `What's Changed`, `New Contributors` oder `Full Changelog` verwenden, ausser Eddi fragt explizit danach.

Pflichtformat:

```markdown
## Description

Kurze grobe Zusammenfassung des Updates.

## Changelog

- Konkrete Aenderung 1.
- Konkrete Aenderung 2.
- Konkrete Aenderung 3.

## Contributors

- EasyEddi
- Tarek Mahn
```

`Description` ist fuer eine grobe, lesbare Zusammenfassung. `Changelog` ist fuer konkrete Aenderungen ohne Commit-Links.

## Contributors

Tarek muss im ersten Release und bei Releases mit seinen Beitraegen im Body unter `Contributors` stehen, auch wenn GitHub ihn im automatisch berechneten Contributors-Kasten nicht anzeigt.

## Assets

GitHub zeigt in Releases immer einen einzigen Bereich namens `Assets`. Dieser Bereich kann nicht in getrennte Tabs wie `Windows` und `Mac` dupliziert oder umbenannt werden. GitHub fuegt ausserdem automatisch `Source code (zip)` und `Source code (tar.gz)` hinzu; diese Auto-Downloads sind keine normalen Release-Assets und sollen nicht als Projekt-Downloads behandelt werden.

Plattformen werden deshalb ueber Dateinamen getrennt:

```text
IsolatedGreenhouse_X.Y.Z_macOS.zip
IsolatedGreenhouse_X.Y.Z_Windows.zip
```

Ziel fuer Windows ist langfristig:

```text
IsolatedGreenhouse_X.Y.Z.exe
```

Diese `.exe` darf aber nur hochgeladen werden, wenn sie wirklich ein eigenstaendig lauffaehiger Installer oder ein self-contained Build ist. Eine einzelne Unreal-Game-`.exe` aus einem packaged Windows-Ordner reicht normalerweise nicht, weil daneben cooked Content, Paks und weitere Dateien benoetigt werden. Bis ein echter Ein-Datei-Windows-Installer existiert, bleibt Windows als versionierter `.zip`-Download.

## Workflow-Hinweise

Der Release-Workflow berechnet die naechste Version aus dem neuesten Tag im Format:

```text
mvp-alpha-X.Y.Z
```

Bei normalen `main`-Pushes ist der Standard-Bump `patch`. Fuer groessere Releases den Workflow manuell starten und `version_bump`, `release_description` und `release_changelog` passend setzen. Alternativ kann `release_version` fuer eine exakte Version gesetzt werden.
