# Startplan: Isolated Greenhouse in Unreal Engine

Dieser Plan ist fuer unser Capstone-Projekt gedacht. Wir benutzen Unreal Engine und arbeiten nicht klassisch als Programmierer, sondern mit Agenten/Vibe-Coding. Das heisst: Wir beschreiben sehr genau, was passieren soll, lassen Agenten oder Unreal-Tools die Umsetzung machen, testen das Ergebnis und geben dann die naechste klare Aufgabe.

## Ziel

Wir bauen in zwei Wochen einen kleinen, spielbaren Unreal-Prototypen von **Isolated Greenhouse**.

Der Spieler ist allein in einer warmen, friedlichen Gewaechshaus-Halle. Er pflegt Pflanzen, schneidet Ableger, erfuellt Bestellungen am Computer, verdient Geld und kauft einfache Hilfsmittel.

## Wichtigste Regeln

- Ruhig, warm, friedlich. Kein Horror.
- Kleine Aufgaben statt riesige Features.
- Immer nur ein aktiver Auftrag.
- Erst ein spielbarer Loop, dann mehr Inhalt.
- Jeder arbeitet auf seinem eigenen Branch.
- Jede groessere Aenderung geht ueber Pull Request.
- Agenten bekommen kleine, konkrete Prompts.
- Nach jeder Agenten-Aenderung testen wir in Unreal.

## Technische Basis

- Engine: Unreal Engine 5.8.
- Umsetzung: Blueprint-only, wenn moeglich.
- Repository: GitHub.
- Asset-Dateien: Git LFS verwenden.
- Keine komplexe C++-Architektur fuer den MVP.

## MVP-Abgrenzung

### MVP enthaelt

- Eine kleine Greenhouse-Map.
- First-Person-Bewegung.
- Eine Monstera-Mutterpflanze.
- Pflanzenzustaende: klein, wachsend, bereit.
- Interaktion mit Pflanze.
- Ableger schneiden.
- Computer-UI fuer aktive Bestellung.
- 3 einfache Bestellungen.
- Geldanzeige.
- Einfacher Shop mit einem Item, zum Beispiel Duenger.
- Verpackungs-/Versandpunkt.
- Eine spielbare Runde von Bestellung 1 bis Bestellung 3.

### Nicht im MVP

- Viele Pflanzenarten.
- Realistische Pflanzenbiologie.
- Multiplayer.
- NPCs.
- Login.
- Grosse Halle mit vielen Raeumen.
- Komplexes Speichersystem.
- Horror-, Survival- oder Kampfmechaniken.

## Arbeitsweise mit Agenten

Agenten sollen nie "mach das ganze Spiel" bekommen. Besser sind kleine Aufgaben mit klaren Grenzen.

Guter Prompt:

```text
Wir arbeiten in Unreal Engine Blueprint-only an Isolated Greenhouse.
Erstelle einen Blueprint Actor BP_Plant_Monstera mit den States Small, Growing und Ready.
Der Actor soll eine Interaktion Care() haben. Nach 3x Care wird Small zu Growing, nach 6x Care wird Growing zu Ready.
Bitte keine C++-Dateien erstellen. Nutze Blueprints und erklaere mir danach, welche Assets/Blueprints geaendert wurden.
```

Schlechter Prompt:

```text
Mach unser ganzes Pflanzen-Game fertig.
```

## Branch-Regeln

Beispiele:

- `eddi/greenhouse-map`
- `eddi/plant-interaction`
- `tarek/order-ui`
- `tarek/shop-system`

`main` soll immer funktionieren. Wenn ein Branch kaputt ist, wird er nicht gemerged.

## Zwei-Wochen-Plan

### Woche 1: Spielbarer Kern

#### Phase 1: Unreal-Projekt aufsetzen

- [x] Unreal-Version festlegen: Unreal Engine 5.8.
- [ ] Unreal-Projekt `TheIsolatedGreenhouse` erstellen.
- [ ] Blueprint-only Projekt verwenden.
- [x] Unreal `.gitignore` hinzufuegen.
- [x] Git-LFS-Regeln fuer `.uasset`, `.umap` und grosse Assets hinzufuegen.
- [ ] Git LFS lokal installieren und mit `git lfs install` aktivieren.
- [ ] Projekt einmal in Unreal oeffnen und speichern.
- [ ] Tarek prueft, ob das Projekt bei ihm oeffnet.

#### Phase 2: Erste Map

- [ ] Eine kleine Greenhouse-Map anlegen.
- [ ] Boden, Waende, Dachfenster und Arbeitsbereich platzieren.
- [ ] Warme Lichtstimmung einrichten.
- [ ] Keine dunkle Horror-Atmosphaere verwenden.
- [ ] Einen Platz fuer Computer, Pflanze und Versandpunkt markieren.

#### Phase 3: First-Person und Interaktion

- [ ] First-Person-Controller einrichten.
- [ ] Interaktion per Taste, zum Beispiel `E`, einbauen.
- [ ] Interaktions-Hinweis anzeigen, wenn der Spieler ein Objekt anschaut.
- [ ] Basis-Interface fuer interaktive Objekte definieren.

#### Phase 4: Pflanze

- [ ] Blueprint `BP_Plant_Monstera` erstellen.
- [ ] States erstellen: `Small`, `Growing`, `Ready`.
- [ ] Pflege-Interaktion einbauen.
- [ ] Sichtbar machen, in welchem Zustand die Pflanze ist.
- [ ] Ableger schneiden, wenn die Pflanze `Ready` ist.
- [ ] Ableger-Anzahl im Spielstatus speichern.

#### Phase 5: Computer und Bestellungen

- [ ] Computer-Actor in die Map setzen.
- [ ] Computer-UI oeffnen, wenn der Spieler interagiert.
- [ ] Aktive Bestellung anzeigen.
- [ ] 3 MVP-Bestellungen anlegen.
- [ ] Bestellung abschliessen, wenn genug Ableger vorhanden sind.
- [ ] Nach Abschluss automatisch naechste Bestellung aktivieren.

#### Phase 6: Geld, Shop, Versand

- [ ] Geldwert speichern und anzeigen.
- [ ] Geld nach abgeschlossener Bestellung erhoehen.
- [ ] Einfachen Shop im Computer-UI bauen.
- [ ] Ein Item kaufen koennen, zum Beispiel Duenger.
- [ ] Duenger soll Pflege beschleunigen oder Bonus geben.
- [ ] Versandpunkt/Packstation einbauen.

#### Phase 7: Integration und erster Playtest

- [ ] Spieler startet in der Map.
- [ ] Spieler pflegt Monstera bis `Ready`.
- [ ] Spieler schneidet Ableger.
- [ ] Spieler verschickt Bestellung.
- [ ] Spieler bekommt Geld.
- [ ] Spieler kauft Duenger.
- [ ] Spieler schafft alle 3 MVP-Bestellungen.
- [ ] Bugs als GitHub Issues notieren.

### Woche 2: Inhalt, Polish, Demo

#### Phase 8: Mehr Pflanzen und Auftraege

- [ ] Zweite Pflanzenart hinzufuegen, zum Beispiel Sukkulente oder Kaktus.
- [ ] Pflanzen unterschiedlich machen, zum Beispiel Wachstum oder Bedarf.
- [ ] Insgesamt 8 bis 10 Bestellungen anlegen.
- [ ] Spaetere Bestellungen etwas komplexer machen.

#### Phase 9: Gewaechshaus voller machen

- [ ] Mehr Pflanzen, Regale, Toepfe und Werkzeuge platzieren.
- [ ] Fortschritt sichtbar machen: Nach Bestellungen erscheinen mehr Dinge.
- [ ] Drohnenlieferung oder Dachluke andeuten.
- [ ] Tageszeit/Lichtverlauf andeuten, wenn Zeit bleibt.

#### Phase 10: UI und Spielgefuehl

- [ ] Computer-UI lesbarer machen.
- [ ] HUD fuer Geld, Ableger und aktuelle Aufgabe verbessern.
- [ ] Interaktions-Hinweise klarer machen.
- [ ] Feedback fuer erfolgreiche Aktionen geben.
- [ ] Ruhige Soundeffekte optional einbauen.

#### Phase 11: Speichern und Build

- [ ] Entscheiden, ob Speichern wirklich noetig ist.
- [ ] Optional einfachen SaveGame-Blueprint bauen.
- [ ] Build/Package-Test machen.
- [ ] Demo-Version auf einem Rechner starten und komplett durchspielen.

#### Phase 12: Abschluss

- [ ] Bug Bash mit Eddi und Tarek.
- [ ] Kritische Bugs fixen.
- [ ] Demo-Ablauf festlegen.
- [ ] Jeder erklaert, was er gebaut hat.
- [ ] Praktikumsbericht fuer Woche 2 und Woche 3 ergaenzen.

## Aufgaben-Aufteilung

### Eddi

- Greenhouse-Map.
- Licht und Atmosphaere.
- First-Person-Gefuehl.
- Pflanzen-Blueprint.
- Pflege-Interaktion.
- Ableger schneiden.
- Packstation/Versandpunkt.
- Visueller Fortschritt im Gewaechshaus.

### Tarek

- Computer-UI.
- Auftragssystem.
- Geldsystem.
- Shop-System.
- Bestellungs-Progression.
- Mehr Auftraege.
- Optional SaveGame.
- Build-/Demo-Test.

### Gemeinsam

- Unreal Engine 5.8 verwenden.
- Projekt oeffnet bei beiden.
- Git LFS pruefen.
- Pull Requests reviewen.
- Integration testen.
- Bugs als Issues schreiben.
- Demo vorbereiten.

## Erste konkrete Schritte

1. Unreal Engine 5.8 installieren.
2. Dieses Repo lokal bei beiden klonen.
3. Git LFS installieren und aktivieren.
4. `game/UnrealProject/TheIsolatedGreenhouse.uproject` in Unreal 5.8 oeffnen.
5. Erste leere Map `L_Greenhouse_MVP` speichern.
6. Branch `eddi/unreal-project-setup` erstellen.
7. Setup committen und Pull Request oeffnen.
8. Tarek reviewed den PR.
9. Nach Merge beginnt Eddi mit Map/Pflanze und Tarek mit Computer/Order-System.

## Agenten-Prompts fuer den Start

### Prompt 1: Projekt-Setup

```text
Wir bauen ein Unreal Engine Blueprint-only Projekt namens TheIsolatedGreenhouse.
Bitte richte die Projektstruktur fuer ein kleines First-Person-Spiel ein.
Ziel: eine leere Greenhouse-Map, First-Person-Spielmodus und saubere Ordnerstruktur.
Keine C++-Klassen erstellen.
Erstelle oder nutze nur Blueprints und Standard-Unreal-Assets.
Danach liste genau auf, welche Dateien/Assets erstellt wurden.
```

### Prompt 2: Interaktionssystem

```text
Wir brauchen in Unreal Blueprint-only ein einfaches Interaktionssystem.
Der Spieler soll mit Taste E Objekte benutzen koennen, die er anschaut.
Bitte erstelle eine einfache Blueprint-Loesung mit Interface oder Component.
Zielobjekte: Pflanze, Computer, Packstation.
Halte es simpel und MVP-tauglich.
```

### Prompt 3: Pflanzenloop

```text
Erstelle in Unreal Blueprint-only einen Monstera-Pflanzen-Actor.
Die Pflanze hat States: Small, Growing, Ready.
Care erhoeht einen Pflegezaehler.
Bei 3 Pflegepunkten wird sie Growing, bei 6 Ready.
Wenn Ready, kann der Spieler einen Ableger schneiden.
Nach dem Schneiden geht die Pflanze wieder auf Growing zurueck.
```

### Prompt 4: Order-System

```text
Erstelle ein simples Auftragssystem in Unreal Blueprint-only.
Es gibt immer nur einen aktiven Auftrag.
MVP-Auftraege:
1. 1 Monstera-Ableger fuer 35 Geld
2. 2 Monstera-Ableger fuer 80 Geld
3. 3 Monstera-Ableger fuer 140 Geld
Wenn ein Auftrag abgeschlossen ist, wird automatisch der naechste aktiv.
```

## Definition of Done

Eine Aufgabe ist fertig, wenn:

- sie im Unreal Editor getestet wurde,
- sie nicht den aktuellen Haupt-Spielablauf kaputt macht,
- sie als Pull Request vorliegt,
- der andere sie reviewed hat,
- kurz im PR steht, was getestet wurde.
