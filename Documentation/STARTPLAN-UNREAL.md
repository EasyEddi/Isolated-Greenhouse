# Startplan Unreal

Dieses Dokument beschreibt den aktuellen Arbeitsplan fuer den Unreal-Prototypen von **Isolated Greenhouse**. Alte Setup-Schritte und generische Wochenplaene wurden entfernt; die eigentliche Aufgabenliste soll klein bleiben und nur Dinge enthalten, die fuer den spielbaren Stand relevant sind.

## Aktueller Stand

- Unreal Engine 5.8 ist die gemeinsame Engine-Version.
- Das Projekt liegt unter `game/UnrealProject/IsolatedGreenhouse.uproject`.
- Die Standard-Map ist `Content/Maps/L_Greenhouse_MVP`.
- Das Projekt nutzt C++ fuer Grundsysteme und Unreal-Assets/Blueprints fuer Gameplay-nahe Inhalte.
- First-Person-Bewegung, Inventar, Hotbar, Debug-Give-Buttons und ein erster Lily-Planting-Loop existieren.
- Die Halle ist als einfache statische Map aufgebaut: Boden, Waende, Licht, Spawnpoint und erste Einrichtungsobjekte.
- Lily, Watering Can, Empty Pot, Soil Bag und Fertilizer Bag existieren als Items. Lily, Empty Pot, Soil Bag und Fertilizer Bag stacken bis 10 pro Slot; Watering Can ist einzigartig.
- Inventar-Items nutzen echte 3D-Mesh-Previews.
- Empty Pots koennen per `F` auf passenden Gewaechshaus- und Storage-Shelf-Pot-Slots platziert werden.
- Lily-Offshoot existiert als erstes importiertes Asset, ist aber noch nicht an Ableger-Gameplay angeschlossen.
- Soil Bag und Fertilizer Bag sind als Content-Assets mit sauberen Unreal-Materialslots und reparierten Front-Labels importiert.

## MVP-Fokus

Der naechste spielbare MVP soll nicht das ganze Spiel abbilden. Ziel ist ein kurzer, stabiler Loop:

1. Spieler startet in `L_Greenhouse_MVP`.
2. Spieler nimmt eine Lily ins Inventar.
3. Spieler waehlt Lily in der Hotbar.
4. Spieler pflanzt Lily am markierten Platz.
5. Lily waechst sichtbar ueber einige Sekunden.
6. Danach koennen spaetere Systeme Pflege, Wasser, Bestellungen und Shop daran andocken.

## Offene Kernaufgaben

- Pflanz-Interaktion robuster machen: Feedback anzeigen, wenn keine Lily ausgewaehlt ist.
- Planting Plot visuell ersetzen: kein Checker-Platzhalter mehr.
- Lily-Offshoot in den naechsten Pflanzen-/Ableger-Loop einbinden.
- Equipment-Bags im Editor/in PIE visuell gegenpruefen und nur noch feinpolishen, falls Screenshots konkrete Fehler zeigen.
- Computer-/Order-System als naechsten separaten Loop planen.
- Watering Can spaeter fuer Pflege/Wasser-Interaktion nutzen.
- Map-Polish nur in kleinen Schritten: keine grossen Generator-Rewrites ohne vorherigen `git status`.

## Nicht jetzt

- Keine komplexe Pflanzenbiologie.
- Keine vielen Pflanzenarten.
- Kein Multiplayer.
- Kein SaveGame, solange der MVP-Loop nicht stabil ist.
- Keine Horror-/Survival-Mechaniken.
- Keine parallelen Agent-Aenderungen an derselben `.umap`.

## Arbeitsweise

- Kleine Aufgaben statt "mach das ganze Spiel".
- Vor jeder Unreal-Map- oder Asset-Aenderung `git status` pruefen.
- Wenn jemand die Map im Editor geoeffnet und gespeichert hat, darf kein Generator blind drueberlaufen.
- `item list.md` bleibt manuell gepflegt und wird von Agenten nicht veraendert.
- Source-/Raw-Modelle unter `game/UnrealProject/models/...` und importierte Content-Assets unter `game/UnrealProject/Content/models/...` getrennt behandeln; bei enger Asset-Reparatur nur den vom User freigegebenen Pfad bearbeiten.
- Fuer jeden groesseren Schritt kurz dokumentieren:
  - welche Dateien betroffen waren,
  - ob Unreal/C++ gebaut wurde,
  - ob die Map manuell geoeffnet werden muss.

## Gute Agent-Prompts

### Map/Asset-Aufgabe

```text
Arbeite im Unreal-Projekt Isolated Greenhouse.
Fasse nur die Map/Asset-Dateien an, die fuer diese Aufgabe noetig sind.
Pruefe zuerst git status.
Wenn die Map bereits lokale Aenderungen hat, nicht regenerieren.
Ziel: [konkrete Aufgabe].
Nachher liste Dateien und Tests auf.
```

### Code-Aufgabe

```text
Arbeite nur an C++/UI-Code im Unreal-Projekt.
Fasse keine .umap oder .uasset Dateien an.
Ziel: [konkrete Aufgabe].
Baue danach IsolatedGreenhouseEditor und nenne das Ergebnis.
```

## Definition of Done

Eine Aufgabe ist fertig, wenn:

- die betroffenen Dateien klar genannt sind,
- keine fremden lokalen Aenderungen ueberschrieben wurden,
- C++ bei Code-Aenderungen gebaut wurde,
- Map-/Asset-Aenderungen im Editor sinnvoll pruefbar sind,
- der Branch sauber gepusht wurde, falls Push verlangt wurde.
