# Product Brief

## Projektname
Isolated Greenhouse

## Kurzbeschreibung
Isolated Greenhouse (TIG) ist ein PC Game, das vorraussichtlich über UnrealEngine laufen wird. In dem Spiel geht es darum, einen Online Shop für Pflanzen zu verwalten. Man kriegt immer neue Bestellungen von seinen Kunden rein, welche man dann bearbeiten muss. Man verkauft Pflanzen Ableger und später auch gesamte Pflanzen, dabei muss man auf bestimmte Vorraussetzungen achten. Jede Pflanze braucht eine gewisse Art von Dünger, jede Pflanze braucht einen bestimmten Topf, unterschiedliche Mengen an Wasser, usw. Equipment für sein die Pflanzen Zucht kann der Spieler Online bestellen. Es findet in einer abgelegenen Halle statt. In dieser Halle gibt es nur ein Gewächshaus, den Schreibtisch, auf dem ein PC für den Online shop steht und der Wohnraum des Spielers. Der Spieler ist dabei zu 100% von der Außenwelt abgeschottet und sieht nur ab und zu das Tageslicht. Werkzeuge kommen dann per Liefer Drone durch eine Luke an der Decke der Halle herein geflogen, das ist der einzige Moment im Spiel, wo der Spielertageslich erblickt.

## Fuer wen ist das Spiel?
Für jeden, der gerne Computerspiele spielt und Pflanzen mag.

## Warum ist die Idee interessant?
Die Idee ist interessant, weil es mal eine andere Art von Spiel ist, was es sonst noch nicht gibt.

## Spielziel
So viele Pflanzen (und auch Mutationen) sammeln und verkaufen wie möglich

## Wichtigste Funktionen

- realistisches Pflanzenwachstum
- Bestellmechanismen (Werkzeuge etc.)
- Progressionsystem

## MVP: Was muss in der ersten Version funktionieren?

- Pflanzenwachstum
- Pflanzenpflege
- Ableger abschneiden

## Was lassen wir erstmal weg?

- Mutationen
- Multiplayer

## Technik

- Engine: Unreal Engine 5.8
- Projektart: Unreal First-Person Game mit C++-Grundsystemen und Blueprint-Gameplay
- Plattform: PC / Mac, lokal spielbar im Unreal Editor

## Aufgaben-Aufteilung

### Eddi

- [x] Unreal-Projekt-Setup
- [x] Greenhouse-Map
- [ ] Licht und Atmosphäre
- [x] First-Person-Bewegung
- [ ] Interaktionssystem
- [ ] erste Pflanzen
- [ ] Pflanzenzustände
- [ ] Pflege-Interaktion
- [ ] Ableger schneiden
- [ ] Auftragssystem
- [ ] Shop-UI
- [ ] Shop-Item, zum Beispiel Dünger

### Tarek

- [ ] Computer-Terminal
- [ ] Order-UI
- [ ] 3 MVP-Bestellungen
- [ ] Geldsystem
- [ ] Packstation / Versandpunkt
- [ ] Verbindung von Shop und Pflanzenpflege
- [ ] HUD für Geld, Ableger und aktive Aufgabe
- [ ] Weitere Bestellungen für Woche 2
- [ ] Visueller Ausbau des Gewächshauses
- [ ] Pflanzen, Töpfe, Regale und Werkzeuge platzieren
- [ ] Optional SaveGame

### Gemeinsam

- [x] Unreal 5.8 installieren
- [x] Repo klonen und Git LFS prüfen
- [x] Projekt bei beiden starten
- [ ] Pull Requests reviewen
- [ ] Integration testen
- [ ] Bugs als Issues aufschreiben
- [ ] Demo vorbereiten

## Erfolgskriterien

- [x] Der Spieler kann sich in der Map in First-Person bewegen
- [ ] Der Spieler kann eine Pflanze pflegen, bis sie bereit für Ableger ist
- [ ] Der Spieler kann Ableger schneiden und damit Bestellungen abschließen
- [ ] Es gibt mindestens 3 Bestellungen, die nacheinander freigeschaltet werden
- [ ] Der Spieler bekommt Geld für abgeschlossene Bestellungen
- [ ] Der Spieler kann im Shop mindestens ein Item kaufen, zum Beispiel Dünger
- [ ] Die Atmosphaere wirkt ruhig, warm und friedlich, nicht gruselig
- [ ] Das Projekt lässt sich im Unreal Editor starten und vorführen

### mockups
<img width="599" height="400" alt="image" src="https://github.com/user-attachments/assets/e151da7d-0958-4fa0-9135-e960d7d23e69" />

- [x] weiß = Halle
- [x] schwarz = spawnpoint
- [ ] gelb = Wohnbereich
- [ ] cyan = Gewächshäuser
- [ ] rot = Schreibtisch (PC)
- [ ] braun = Storage für equipment (Dünger, Erde etc.)
- [ ] orange = erstmal frei
