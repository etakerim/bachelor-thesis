## Bakalársky projekt - Osnova

###### Tématické okruhy

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