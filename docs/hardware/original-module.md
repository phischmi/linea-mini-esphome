# Original-IoT-Modul (aus Fotos des Nutzers)

Quelle: zwei Fotos der Platine (Oberseite und Unterseite), vom Nutzer aufgenommen am 2026-10-19. Die Fotos liegen **nicht** im Repo. Status: **V (Foto)** = Aufdruck auf dem Foto klar lesbar, **U** = meine Deutung, nicht geprüft. Alles, was nicht klar lesbar war, steht unten unter „Nicht lesbar“.

## Was auf den Fotos zu lesen ist
| Beobachtung | Status |
|---|---|
| ESP32-Modul von Espressif (Aufdruck „ESP32-W…“, Rest vom Aufkleber verdeckt), Blech-Abschirmung, FCC-/CE-/CMIIT-Aufdrucke; Leiterbahnantenne am Modulende, die **über den Platinenrand hinausragt** | V (Foto) |
| Aufkleber auf dem Modul: **„FW V1.0rc7“** und **„9.5.41.09G00“** | V (Foto) |
| Auf der Unterseite: **GICAR**-Logo, Aufdruck **8.527.57**, UL-94V0-Zeichen. Die Platine stammt also von GICAR, wie die Hauptplatine | V (Foto) |
| Schwarzes Etikett am Modulende mit **„2109“**, Pb-, ESD- und WEEE-Symbol. Das ist vermutlich ein Datumscode (Jahr 21, Woche 9) | U |
| Beschriftungen: **CN3**, **CN4**, **P1, P2, P3**, **LD1 „3V3“**, **LD2, LD3, LD4**, **IC9** | V (Foto) |
| **CN4:** vier Lötaugen in einer Reihe (ein 4-poliger Anschluss, passend zu unserem CN11-Kabel). Auf der Unterseite sitzt ein **schwarzer** Steckverbinder am unteren Platinenrand | CN4 = 4 Pole: V (Foto); schwarzer Stecker = CN4: U |
| **CN3:** zehn Lötaugen (2 × 5). Auf der Unterseite sitzt daneben ein **weißer**, zweireihiger Steckverbinder mit Rasthaken | zehn Pole: V (Foto); weißer Stecker = CN3: U |
| **P1 bis P3:** eine Stiftleiste mit 3 × 2 Stiften an der Oberseite, Funktion unbekannt | V (Foto) / Funktion U |
| **IC9:** am oberen rechten Rand drei Stifte in einer Reihe (wirkt wie eine kleine Stiftleiste, evtl. serieller Anschluss) | Bauform: V (Foto), Funktion U |
| Auf der Unterseite eine Reihe von **sechs schmalen Pads** (Test-/Programmierpads?) in der Nähe des Moduls | V (Foto) / Funktion U |
| Vier Kunststoff-Abstandshalter mit Innengewinde oben, vier Schrauben unten: Die Platine ist auf vier Punkten verschraubt | V (Foto) |
| Vier LEDs: LD1 (3V3-Anzeige) und LD2 bis LD4 am linken Rand, jede mit Vorwiderstand und einem kleinen SOT-23-Transistor | V (Foto) |
| **Netzteil:** Abwärtswandler-IC im SO-8-Gehäuse mit Aufdruck **54331** (TI-Logo), Spule **4R7** (4,7 µH), Tantal-Elko, Diode „ER2D“ in der Nähe. Passt zu einem **TI TPS54331** (3-A-Abwärtswandler, bis 28 V Eingang; nur aus dem Aufdruck abgeleitet, Datenblatt nicht geprüft) | U |
| Drei SOT-23-Transistoren mit Aufdruck **„702“** (passt zu 2N7002, N-MOSFET) und drei kleine SOT-23-6-ICs neben CN4. Beides sieht nach **Signalanpassung** der Leitungen aus, ihre Funktion ist aber nicht gesichert | U |
| Ein 10-poliges IC (MSOP, Aufdruck „9527“) links der Mitte, zwei große MELF-Dioden bei CN3: Funktion unbekannt | U |
| Größe: etwa **40 × 70–75 mm** (grobe Schätzung aus der Modulbreite von 18 mm; perspektivisch verzerrt, ± 10 %) | U |

## Folgerungen für das Projekt
1. **Versorgung:** Das Original wandelt die 12 V von CN11 selbst mit einem Schaltregler auf 3,3 V (LED „3V3“). Damit ist belegt, dass die 12 V auf dem Kabel ein Modul mit ESP32 und WLAN versorgen können. Den Strom des Moduls kenne ich nicht (U), er ist von der Größenordnung her mit unserem XIAO vergleichbar. Ein Regler mit 3 A Spielraum zeigt, dass GICAR großzügig dimensioniert hat.
2. **Signalanpassung:** Es gibt zwischen CN4 und dem ESP32 eine Schaltung aus Transistoren und kleinen ICs. Sie erklärt vermutlich, wie das Original die 2,5-V-Pegel der Maschine für den ESP32 aufbereitet (offene Frage O11) und ob dort invertiert wird. **Das ist die wichtigste Information für den Entwurf unserer Trägerplatine.** Siehe O16.
3. **Drop-in-Platine:** Wir können die Trägerplatine so gestalten, dass sie an derselben Stelle auf dieselben vier Befestigungspunkte passt und den Stecker an derselben Stelle hat. Dafür Umriss, Lochabstände und die Steckerposition ausmessen (Messschieber).
4. **Antenne:** Das Original sitzt mit der Leiterbahnantenne des Moduls am Platinenrand, frei überstehend. Wenn dein WLAN in der Maschine mit dem Original funktioniert, ist das ein brauchbarer Vergleichswert für die Antenne des XIAO an dieser Stelle (Signalstärke messen, `wifi_signal`).
5. **Zweiter Anschluss CN3 (10 Pole):** Ob und wofür das Original CN3 benutzt, ist offen (O17). Bisher gehe ich nur von CN4 mit vier Adern aus.
6. **Firmware-Stand:** Der Aufkleber nennt „V1.0rc7“, der Nutzer nannte v7.4.2 (O13). Vermutlich Werksstand und späteres Update (U).
7. **Status-LEDs:** Das Original hat drei LEDs für Status. Wenn sie in der Maschine sichtbar sind (Lichtleiter oder Bohrung), kann die Trägerplatine gleichartige LEDs bekommen.

## Nicht lesbar auf den Fotos
Aufdrucke der drei SOT-23-6-ICs, der Transistoren „WG3“ und aller Widerstände (zu klein oder zu unscharf), der genaue Modultyp (Aufkleber verdeckt), Bauform der Steckverbinder (stehend oder liegend) und die Pin-Reihenfolge von CN3/CN4. Für diese Punkte siehe die Fotowünsche in `open-questions.md` (O16).

## Zweite Fotoserie (2026-10-24, Modul ausgebaut, in der Maschine fotografiert)
| Beobachtung | Status |
|---|---|
| Das Kabel führt an einen **schwarzen Stecker mit grünem Aufkleber** (teilweise lesbar, u. a. „211652“). Ich zähle auf dem Foto **etwa sechs** weiße Adern, nicht vier. Wie viele Kontakte der Stecker hat und welche Adern belegt sind, ist offen | Aderzahl: U |
| Der **weiße Stecker (CN3)** der Platine hat kein Kabel eingesteckt | V (Foto) |
| Die drei Stifte am Rand (**IC9**) sitzen auf der Rückseite und sind mit **Klebeband** abgedeckt; auf der Oberseite sind es drei Lötaugen (eines quadratisch). Vermutlich eine kleine serielle Debug- oder Programmierleiste (GND/TX/RX), denn ihre Leiterbahnen laufen zu den Pads der Modulkante, an der beim ESP32-WROOM die UART0-Pins liegen | U |
| **P1–P3** sind drei 2-polige Stiftreihen **ohne aufgesteckte Jumper** | V (Foto) |
| Neben der Platine ist im Gerät ein Aufkleber auf der Hauptplatine zu sehen, teilweise lesbar: „**NEWMIL … v.2.08D … 20/11/20**“. Der Nutzer nannte v2.12 (vermutlich App-Anzeige). Beides kann verschiedene Stände meinen | teilweise lesbar, U |
| Auf den Fotos sind **keine Brand- oder Bruchspuren** zu erkennen (Auflösung begrenzt) | V (Foto), nicht belastbar |
| Das Foto zeigt, dass im Gerät der **Netzanschluss** (grüner Klemmenblock) offen zugänglich ist | V (Foto) |

## Fehlersuche: Modul bleibt dunkel (LD1 „3V3“ und alle LEDs aus)
Stand 2026-10-24: Der Nutzer hat an allen Steckerpins den Durchgang zur Platine geprüft (Lötstellen in Ordnung). Dennoch leuchten keine LEDs, und die Maschine ist nicht erreichbar. LD1 „3V3“ sollte bei vorhandener Versorgung dauernd leuchten (U), die anderen LEDs erst im Betrieb. Dunkel heißt also sehr wahrscheinlich: **keine 3,3 V**.

Prüfreihenfolge (Maschine **aus**, Netzstecker gezogen, solange du Hand anlegst):
1. **Sichtprüfung** mit Lupe oder Makrofoto: Tantal-Kondensator, große schwarze Dioden, Regler-IC, Spule, Lötstellen am Modul.
2. **Kabel stromlos prüfen:** Jede Ader End-zu-End durchklingeln (Zuordnung notieren), Isolation zwischen allen Adern (kein Durchgang), Aderzahl und belegte Kontakte notieren.
3. **Widerstand der 3,3-V-Schiene:** an den Anschlüssen des orangen Tantal-Kondensators (vermutlich Ausgangskondensator) gegen die Gegenseite messen. Ein Kurzschluss (wenige Ohm, nicht ansteigend) deutet auf einen defekten Kondensator, Regler oder ESP32.
4. **Spannung am Anschluss CN4** (Maschine an, Platine fixiert, isolierte Klemmleitungen): 12 V zwischen zwei Pins? Wenn nein, liegt es an Kabel, Stecker oder Hauptplatine. Wenn ja, weiter am Modul.
5. **Spannung entlang des Eingangs:** an beiden Seiten der großen schwarzen Dioden und am Eingangspin des Reglers (laut Datenblatt Pin 2 VIN, vor der Messung gegen das TPS54331-Datenblatt prüfen).
6. **Ausgang des Reglers:** am Tantal-Kondensator, erwartet 3,3 V. Die markierte Seite ist bei Tantal meist Plus; ein negatives Vorzeichen am Multimeter ist ungefährlich.
7. **Bank-Test** (Modul außerhalb der Maschine): Spannungsversorgung 12 V mit Strombegrenzung (ca. 200 mA) an CN4, **nach geklärter Polung**. Strom in Reihe messen: Ein gesundes Modul zieht vermutlich grob 50–150 mA (U); null oder ein ansteigender Strom zeigen einen Fehler.
Auswertung: 12 V am CN4, aber keine 3,3 V → Regler oder Last defekt. Keine 12 V am CN4 → Kabel, Stecker, Hauptplatine oder Verpolung. 12 V und 3,3 V vorhanden, aber LEDs aus → LEDs prüfen (sie leuchten vielleicht nur im Betrieb), Reset/Boot des ESP32 (Boot-Log über IC9, nur nach Prüfung der Belegung).

Sicherheit: Das Foto zeigt den offenen Netzanschluss. Handarbeit nur bei gezogenem Netzstecker. Für Messungen unter Spannung die Platine fixieren (nicht in der Hand halten), isolierte Klemmleitungen verwenden, eine Hand am Messgerät, und Abstand zum Klemmenblock halten. Pin 4 (+12 V) liegt beim Abgreifen neben den Signalpins: Ein Kurzschluss kann 12 V auf die UART-Leitungen der **Hauptplatine** legen und diese beschädigen.

## CN4 und CN3: Aufdruck und Pin-Identifikation (Nahaufnahme 2026-10-24)
Beobachtung (Foto der Oberseite, Lötaugen von CN3 und CN4):
- **Kein Klartext** wie „12V“ oder „GND“ am Anschluss. Der Aufdruck liefert keine Polarität.
- **CN4:** vier Lötaugen in einer Reihe. Der Aufdruck markiert Pin 1 an **zwei Stellen, die sich widersprechen**: eine „1“ in der linken Zelle eines Vier-Felder-Kastens oberhalb der Lötaugen, und eine quadratische Umrandung um das **rechte** Lötauge. Da die Steckverbinder auf der Unterseite sitzen, ist die Spiegelung zwischen Aufdruck und Stecker unklar. **Pin 1 ist daher nicht ablesbar (U).**
- **CN3:** 2 × 5 Lötaugen, das Lötauge oben rechts ist quadratisch umrandet (vermutlich Pin 1).
- Neben den Anschlüssen liegen **zwei große MELF-Dioden** mit weißem Ring auf der rechten Seite. Der Ring markiert die Kathode, die Stromrichtung zeigt also von den Anschlüssen zum Regler. Das spricht für eine Diode in der Versorgung (Verpolungsschutz oder ODER-Verknüpfung der Einspeisung von CN3 und CN4, U).
- Zwei kleine SOT-23-Transistoren sitzen direkt unter den CN4-Lötaugen, je mit mehreren kleinen Widerständen und einem 4,7-kΩ-Widerstand („472“) in der Nähe. Das sieht nach **einer Transistorstufe je Datenleitung** aus (Pegelanpassung für TXD und RXD, U).

Pin-Identifikation ohne Spannung (Netzstecker gezogen, Kabel und Modul getrennt):
1. **GND:** Durchgang zum Abschirmblech des ESP32-Moduls und zum unmarkierten Anschluss des Tantal-Kondensators. Das Lötauge von CN4 mit Durchgang (nahe 0 Ω) ist GND (U, Abschirmblech und Kondensatorseite vorher gegeneinander prüfen).
2. **+12 V:** Durchgang vom Lötauge zum linken Ende (Anode) einer der großen Dioden. Das ist der Versorgungspin, falls die Diode in der +12-V-Leitung liegt (U).
3. **Signalleitungen:** die beiden übrigen Lötaugen von CN4, an denen die Transistorstufen hängen.
Mit Spannung (Maschine an, Platine fixiert): an den **Diodenenden** messen, statt an den Lötaugen. Anode ≈ 12 V, Kathode etwa 0,3–0,7 V darunter, wenn die Diode leitet. So kann ein ausrutschender Messfinger keine Signalleitung treffen.

## Messprotokoll des Nutzers (Original-Modul)
| Datum | Messung | Bedingung | Ergebnis | Aussage |
|---|---|---|---|---|
| 2026-10-24 | „GND gegen 3V3“, Anzeige „−.338 V“ (DC) | **ohne Spannung** (vom Nutzer bestätigt) | Lesart −0,338 V oder −3,38 V unklar | **keine** (stromlos, Restladung/Messrauschen) |
| 2026-10-24 | Durchgang vom CN4-Lötauge mit quadratischer Umrandung zum Ende **ohne Ring** der oberen großen Diode | stromlos | ja | Das Lötauge ist sehr wahrscheinlich der **Versorgungseingang (+12 V)**; die Diode liegt dann in Reihe (Anode am Eingang, Kathode zum Regler). **Nicht unter Spannung bestätigt (U)** |

Folgerung (U): CN4 **Pin 1 (Quadrat) = +12 V**. Wenn der CN11-Stecker der Hauptplatine +12 V an Pin 4 hat (Quelle), ist die Nummerierung am Modul-Ende des Kabels gespiegelt (1↔4). Sicher ist das erst nach Messung unter Spannung und Durchklingeln des Kabels.

### Bank-Test mit externem 12-V-Netzteil (empfohlen, ohne Netzspannung der Maschine)
Das Modul braucht nur 12 V Gleichspannung. Ein externes Netzteil vermeidet jeden Kontakt mit dem offenen Netzteil der Maschine.
1. **Stromlos prüfen:** Die Diode in Reihe bestätigen (Diodentest: Rot an das Ende ohne Ring, Schwarz an das Ende mit Ring ergibt 0,2 bis 0,7 V). Durchgang vom **Ende mit Ring** zum Eingangspin des Reglers (laut Datenblatt Pin 2 VIN, Datenblatt vorher prüfen) und zu den Keramikkondensatoren dort. GND-Lötauge von CN4 über Durchgang zum Abschirmblech des ESP32 finden. Widerstand 3V3 gegen GND am orangen Tantal-Kondensator: Ein Kurzschluss (wenige Ohm, nicht ansteigend) → nicht einschalten.
2. **Strombegrenzung:** Labornetzteil auf 12 V und 200 mA. Ohne Labornetzteil ein kleines 12-V-Steckernetzteil (höchstens etwa 0,5 A) mit einer flinken Sicherung 250 mA in Reihe oder einer 12-V-Glühlampe (z. B. 5 W) als Strombegrenzer in Reihe.
3. **Anschließen:** Plus an das **Quadrat-Lötauge**, Minus an das GND-Lötauge. Das Modul liegt auf einer isolierenden Unterlage, keine Metallflächen darunter.
4. **Messen:** Strom (mA, in Reihe), Spannung an der Kathode der Diode (≈ 11,3–11,7 V erwartet), am Reglereingang, am Tantal-Kondensator (3,3 V erwartet), LD1 beobachten.
Auswertung: kein Strom und keine Spannung hinter der Diode → Diode oder Pfad unterbrochen. Strom grob 50–150 mA und 3,3 V → Modul lebt (LEDs prüfen, Boot-Log). Hoher Strom oder einbrechendes Netzteil → Kurzschluss (Tantal-Kondensator oder ESP32), sofort abschalten.

