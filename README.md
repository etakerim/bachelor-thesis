# Bakalársky projekt - Osnova



## Letný semester

#### Osnova praktickej časti Bakalárskej práce

#### Návrh

- Bloková schéma hardvéru
- Diagram komponentov pre subsystémy firmvéru
- Diagram aktivít pipeline
- Diagram aktivít správy konfigurácie
- Sekvenčný diagram rozdelenia spracovania medzi úlohy
- Stavový diagram: online algoritmus na detekciu udalostí

#### Implementácia

- Programovacie jazyky a knižnice: C, esp-idf 4.0, esp-dsp, mpack
- Diagram balíčkov - hlavičkové súbory
- Uchovávanie udalostí - opis štruktúry
- Opis struct-ov konfigurácie
- Formát msgpack správ

#### Overenie

- Profil spotreby pamäte (Pre všetky N: 8, 16, 32, 64, 128, 256, 512, 1024)  - otázka: aký je najväčšia vyrovnávacia pamäť vzoriek? (graf)
- Veľkosti prenášaných správ (tabuľka) - otázka: od akého počtu vzoriek sa oplatí agregovať dáta vzhľadom na prenesené správy?
  - Min. počet vzoriek pre štatistiky?
  - Max. počet udalostí < ako počet prenášaných frekvenčných bins
- Časové vykonávanie algoritmov (1 os). Priemerované cez 10 behov, Pre všetky N: 32, 64, 128, 256, 512, 1024
  - N=512, T Smoothing (k = 4, 8, 16, 32, 64)
  - DFT dB
  - DCT dB
  - Algoritmus č.1
  - Algoritmus č.2
  - Algoritmus č.3
  - Event detection 
- Časové vykonávanie celej pipeline pre rôzne N (graf) vs. real time deadline T * n (resp. n / fs). Priemer 10 behov. Scenáre (to isté pre 3 osi) - tabuľka:
- Vhodné parametre algoritmov na detekciu špičiek (tab.) - z reálneho datasetu syntetický a na ňom grid search pre rôzne veľkosti n
  - Algoritmus č.1
  - Algoritmus č.2
  - Algoritmus č.3

#### Záver

Ukázali sme možnosti hardvéru na okraji siete pre predspracovanie udalostí a poskytli základný náčrt redukcie množstva zosnímaných dát agregovaných do udalostí. Potenciálne rozšírenia: samoučiace algoritmy detekcie špičiek, porovnať rýchlejšie implementácie DCT, Posielanie cez Bluetooth, anotované datasety z reálnej prepravy automobilom

#### A - Plán práce

#### B - Technická dokumentácia

- [ ] Návod na inštaláciu
- [ ] Návod na použitie  - flashovanie firmvéru, prihlásenie do siete, zmena konfigurácie cez python klienta
- [ ] Návod na použitie jupyter notebook pre exploráciu datasetov z vibrácii a generovanie syntetických dát

#### C - Obsah digitálneho média





## Zimný semester

#### Analýza

1. **Senzorová sieť** - charakteristika prostredia, obmedzenia, očakávania
4. **Senzorika - Senzorová jednotka** - skladba, senzory: akcelerometer, gyroskop - opis fungovania MEMS, vlastnosti prístroja,  vlastnosti meraných veličín (m/s^2, ˚/s), vzorkovanie a A/D-prevodník a konverzia na veličinu
5. **Formalizácia meranej veličiny (signálov)** - časový rad, vzorkovanie, okná, online algoritmus, požiadavky na efektívne algoritmy
6. **Meranie vibrácií a šoku** - definícia, časovo-frekvenčná analýza, spektrogram, power spectrum density (dbFS)
7. **Metódy analýzy signálu v časovej doméne**
   1. Číselné charakteristiky štatistického rozdelenia - momenty, kvantily, extrémy
   2. Výchylky (anomálie) - definícia, bodové, kolektívne (disonanancie), kontextové výchylky / anomálie, binárny klasifikátor
   3. Algoritmy na identifikáciu špičiek - lokálne minimá a maximá, topografická prominencia, Z-score, T-score, generalized extreme Studentized deviate test, Median Absolute Deviate, Shewart Control Chart, CUSUM
   5. Konvolúcia a vzájomná korelácia - spárovanie hľadaných vzoriek v podobe kernelu s priebehom signálu
   7. Stabilná numerická integrácia rýchlosti a pozície, geometria akceleračného vektora - magnitúda, uhly, kvaternióny
8. **Frekvenčná a časovo-frekvenčná analýza** 
   1. Spektrálny analyzátor, vlastnosti frekvenčného spektra
   2. Transfomácia na ortogonálne bázy - Fourierová transformácia (DFT), Kosínusová transfomácia (MDCT), Vlnková transformácia (CWT)
   3. Algoritmus FFT - rýchla Fourierová tranformácia na DFT a MDCT, optimalizácia využitím symetrií
   4. Digitálne FIR a IIR filtre - dolná priepusť (odšumenie), pásmová, horná priepusť (odstránenie DC), filter bank, sub-band coding
   5. Oknovacie funkcie - význam, príklady, prekrytie
   6. Stratová kompresia pre redukciu prenosu - porovanie DFT, DCT, DWT, Real time PCA. Využitie techník obrazovej a zvukovej kompresie (z JPEG2000, MP3) - napr. Huffmanove kódovanie = Feature extraction (Redukcia dimenzionality)



#### Návrh

Senzorová jednotka s akcelerometrom na meranie vibrácii a nárazov pri prevoze krehkých látok/materiálov upozorňujúca na základe konfigurovateľných pravidiel alebo nezvyčajných vzorov (pozn.:lepšie dodefinovať). Preskúmanie možností redukcie zberu alebo nasnímaných údajov - nastavenie vzorkovacej frekvencie / akceptovanie nad stanovenú amplitúdu / kompresia. Rozšírenie: /distribuovaná/ korelácia údajov z viacerých akcelometrov z jedného balíka / vozidla.

#### Výskumné / Inžinierske otázky:

- Najvhodnejší spôsob identifikácie lokálnych extrémov - špičiek
- Možnosti redukcie zberu dát za cenu redukcie energetické nároky a prenosu významných čŕt signálu
- Pravidlový systém na definíciu udalostí záujmu operátorov a ich spoľahlivá identifikácia
- Upozornenie na nezvyčajnosti pri prevoze (detekciou anomálií) - veľká nerovnosť, neočakávaný pohyb