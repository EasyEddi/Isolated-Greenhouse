# Agent Rules for Isolated Greenhouse

Diese Datei ist der Einstieg fuer neue Codex-/cmux-Agenten in diesem Projekt. Lies sie vor jeder Arbeit am Repo komplett durch. Wenn eine User-Anweisung dieser Datei widerspricht, hat die aktuelle User-Anweisung Vorrang, aber sprich den Konflikt kurz an.

## Projektziel

**Isolated Greenhouse** ist ein ruhiges First-Person-Spiel in Unreal Engine. Der Spieler lebt isoliert in einer Halle mit Wohnbereich, Schreibtisch/PC, Lagerbereich und Gewaechshausbereich. Der Kern ist Pflanzenzucht: Pflanzen bekommen, pflegen, wachsen lassen, spaeter Ableger/Bestellungen/Shop-Systeme anschliessen.

Wichtig fuer die Stimmung:

- friedlich, ruhig, verlassen, leicht melancholisch,
- kein Horror,
- kein Survival-Stress,
- kein Kampf,
- keine NPCs fuer den MVP.

Der Fokus liegt auf einem stabilen, kleinen spielbaren Loop statt vielen halb fertigen Features.

## Aktueller Stand

Stand dieser Datei:

- Unreal Engine Version: **5.8**.
- Projektdatei: `game/UnrealProject/TheIsolatedGreenhouse.uproject`.
- Standard-Map: `game/UnrealProject/Content/Maps/L_Greenhouse_MVP.umap`.
- Standard-Branch fuer laufende Arbeit: `eddis-beta-stuff`.
- `main` soll nur aktualisiert werden, wenn der User es explizit verlangt.

Bereits vorhanden:

- First-Person-Bewegung.
- Spawnpoint und spielbare Standard-Map.
- Halle mit Boden, Waenden, Licht und erster Einrichtung.
- C++-Grundsysteme fuer PlayerController, GameMode, Inventory, Held Items und Planting Plot.
- Inventar mit 5 Hotbar-Slots und 3x9 Inventar-Slots.
- Debug UI mit `Give lily`, `Give watering can`, `Give empty pot`, `Give soil bag` und `Give fertilizer bag`.
- Lily, Empty Pot, Soil Bag und Fertilizer Bag stacken bis 10 pro Slot.
- Watering Can ist einzigartig und soll nicht mehrfach ins Inventar gegeben werden.
- Items koennen sichtbar in der Hand gehalten werden.
- Lily kann am Planting Plot gepflanzt werden und waechst ueber einige Sekunden sichtbar.
- Inventar-Items verwenden echte 3D-Mesh-Previews statt nur vereinfachter 2D-Symbole.
- Lily, erstes Lily-Offshoot-Asset, Watering Can, Empty Pot, Soil Bag und Fertilizer Bag liegen als importierte Unreal-Assets im Projekt.
- Soil Bag und Fertilizer Bag sind in `Content/models/equipment/...` mit sauberen Materialslots und reparierten Front-Labels importiert.

Noch nicht final:

- Planting Plot ist noch ein Platzhalter und soll spaeter visuell ersetzt werden.
- Computer-/Order-/Shop-System ist noch nicht final.
- Watering Can hat noch keine finale Gameplay-Funktion.
- Pflanzenpflege, Ableger schneiden, Lily-Offshoot-Gameplay, Bestellungen, Geld und Shop sind noch auszubauen.
- Map/Interior entwickelt sich weiter und kann manuell im Editor angepasst werden.

## Wie das Spiel gebaut wird

Das Projekt ist **nicht Blueprint-only**.

Technik-Mix:

- C++ fuer Grundsysteme und Verhalten, das stabil und wiederverwendbar sein soll.
- Blueprints/Unreal-Assets fuer editornahe Inhalte, visuelle Setups und spaetere Gameplay-Verknuepfungen.
- `.fbx` Models fuer importierte 3D-Modelle.
- `.uasset` und `.umap` fuer Unreal Content.
- Git LFS fuer grosse/binaere Dateien.

Wichtige Pfade:

- C++ Code: `game/UnrealProject/Source/TheIsolatedGreenhouse`
- Config: `game/UnrealProject/Config`
- Map: `game/UnrealProject/Content/Maps/L_Greenhouse_MVP.umap`
- importierte Unreal-Assets: `game/UnrealProject/Content`
- Source-/Raw-Modelle: `game/UnrealProject/models`
- Documentation: `Documentation`
- manuelle Item-Liste: `Documentation/item list.md`

## Wichtigste Sicherheitsregel

Nie ungefragt lokale Aenderungen ueberschreiben.

Vor jeder Arbeit:

```bash
git status --short --branch
```

Zusaetzlich zu Beginn jedes neuen Prompts kurz `Documentation/screenshots/` auf neue Dateien pruefen. Bei visuellen Bugs, UI, Assets, MapCheck-Screenshots oder unklaren In-Editor-Zustaenden immer die neuesten Screenshots zuerst anschauen und als aktuelle Quelle der Wahrheit behandeln.

Wenn lokale Aenderungen existieren:

- nicht resetten,
- nicht checkouten,
- nicht mit Generatoren ueberschreiben,
- nicht automatisch stagen, wenn sie nicht zur aktuellen Aufgabe gehoeren.

Wenn unklar ist, wem eine Aenderung gehoert, mit ihr arbeiten oder nachfragen.

## Branch- und Push-Regeln

- Standard-Arbeitsbranch: `eddis-beta-stuff`.
- Wenn der User "mein Branch" sagt, ist normalerweise `eddis-beta-stuff` gemeint.
- Nicht direkt auf `main` pushen, ausser der User verlangt es ausdruecklich.
- Wenn der User sagt "push auf Branch und main", zuerst `eddis-beta-stuff` pushen, danach `main` auf denselben Stand bringen.
- Wenn nur ein enger Fix gepusht werden soll, keine unfertigen Dokumentations-, Map- oder Asset-Aenderungen mitnehmen.
- Vor jedem Commit pruefen, was staged ist:

```bash
git diff --cached --stat
git status --short --branch
```

## Unreal- und Asset-Regeln

Unreal-Dateien sind oft binaer und schwer zu mergen.

Besonders vorsichtig sein bei:

- `.umap`
- `.uasset`
- `.fbx`
- `.ubulk`
- `.uexp`

Regeln:

- Keine `.umap` bearbeiten, wenn der User gerade im Editor an der Map arbeitet.
- Keine Map-Generatoren laufen lassen, wenn die Map manuell gespeichert wurde, ausser der User bestaetigt es.
- Keine manuell platzierten Moebel/Models durch Generator-Scripts verschieben.
- Keine vorhandenen Assets ersetzen, nur weil sie "unsauber" aussehen.
- Keine Tools/Pflanzen/Gameplay-Props generieren, wenn der User sagt, dass Tarek diese Inhalte macht.
- Statische Map bedeutet: Boden, Waende, Licht, Spawnpoint, Hallenform, statische Einrichtung.
- Pflanzen, Tools und Items sind Gameplay-Inhalte und nur anfassen, wenn die Aufgabe es verlangt.
- Bei Soil/Fertilizer Bag Fixes den vom User gesetzten Source-/Raw-Ordner `game/UnrealProject/models/equipment/` nicht ueberschreiben, wenn er das untersagt; dann nur die importierten Assets unter `game/UnrealProject/Content/models/equipment/` bearbeiten.

## C++ Regeln

Bei C++-Aenderungen:

- vorhandene Klassen und Patterns benutzen,
- kleine, gezielte Aenderungen machen,
- keine grosse Architektur einfuehren, wenn ein enger Fix reicht,
- Windows/MSVC ernst nehmen: keine Warnungen ignorieren, die auf Windows als Error behandelt werden koennen,
- nach Moeglichkeit `TheIsolatedGreenhouseEditor` bauen.

Mac-Build-Beispiel:

```bash
'/Users/Shared/Epic Games/UE_5.8/Engine/Build/BatchFiles/Mac/Build.sh' TheIsolatedGreenhouseEditor Mac Development -Project='/Users/eddi/The-Isolated-Greenhouse/game/UnrealProject/TheIsolatedGreenhouse.uproject' -WaitMutex
```

Windows braucht Visual Studio 2022 mit `Game development with C++`. Wenn Module auf Windows nicht bauen, zuerst echte `error C...` Zeilen aus dem UnrealBuildTool-Log lesen, nicht nur das Popup.

## Documentation-Regeln

Dokumentation soll helfen, nicht aufblasen.

- `Documentation/item list.md` nicht anfassen, ausser der User verlangt es ausdruecklich.
- Keine alten Browser-/Vercel-/Web-App-Schritte wieder einfuegen.
- Keine langen generischen Checklisten, die nicht mehr zum Unreal-Projekt passen.
- Wenn ein neues Feature final eingebaut wurde, relevante Dokumentation aktualisieren.
- Wenn ein Feature nur experimentell/halb fertig ist, nicht als final dokumentieren.
- Wenn eine Regel, ein Workflow oder ein aktueller Projektstand sich aendert, diese Datei oder die passende Documentation-Datei aktualisieren.
- Bei neuen GitHub Releases immer die Regeln in `Documentation/RELEASE-RULES.md` einhalten.

Finale Features sollen mindestens hier vermerkt werden:

- `Documentation/PRODUCT-BRIEF.md` fuer Gameplay-/Produktstand.
- `Documentation/STARTPLAN-UNREAL.md` fuer aktuellen Plan und offene Kernaufgaben.
- `Documentation/UNREAL-SETUP.md` fuer Setup-/Build-Aenderungen.
- `Documentation/TEAM-WORKFLOW.md` oder diese Datei fuer Agent-/Team-Regeln.

## Cmux/Codex Sessions

Wenn mehrere Sessions laufen, braucht jede Session eine klare Verantwortung.

Sinnvolle Rollen:

- `map-layout`: Halle, Boden, Waende, Licht, Spawnpoint, statische Map.
- `inventory-items`: Inventar, Hotbar, Item-Stacks, Hand-Items.
- `models-assets`: Importierte Models, Materialien, Scale/Rotation.
- `gameplay-planting`: Planting Plot, Wachstum, Pflege-Interaktionen.
- `git-sync`: Status, Commit, Push, Merge, Konfliktpruefung.
- `docs`: Documentation aktualisieren.

Eine Session darf nicht spontan Aufgaben einer anderen Session uebernehmen. Besonders keine parallele Arbeit an derselben `.umap` oder denselben `.uasset` Dateien.

## Was Agenten nicht tun sollen

- Nicht "das ganze Spiel" bauen.
- Nicht ungefragt `main` pushen.
- Nicht ungefragt lokale User-Aenderungen committen.
- Nicht ungefragt Dateien loeschen.
- Nicht ungefragt Unreal-Map neu generieren.
- Nicht mit `git reset --hard` oder `git checkout --` arbeiten, ausser der User verlangt es eindeutig.
- Nicht `Documentation/item list.md` bearbeiten.
- Nicht grosse Refactors machen, wenn der User einen konkreten Bugfix will.
- Nicht behaupten, etwas sei in Unreal getestet, wenn nur ein Code-Build gemacht wurde.

## Wenn etwas fertig ist

Wenn eine Aufgabe final fertig ist, muss die Antwort und ggf. Dokumentation klar machen:

- was gebaut/geaendert wurde,
- welche Dateien betroffen sind,
- ob es final oder nur ein Platzhalter ist,
- wie es getestet wurde,
- ob es gepusht wurde und auf welchen Branch.

Wenn ein neues finales Gameplay-Feature hinzugefuegt wurde, aktualisiere die passende Documentation, damit der naechste Agent nicht von altem Stand ausgeht.

## Vor der finalen Antwort

Immer kurz pruefen:

```bash
git status --short --branch
```

Dann knapp sagen:

- was erledigt wurde,
- was bewusst nicht angefasst wurde,
- ob Builds/Tests liefen,
- ob offene lokale Aenderungen von vorher existieren.
