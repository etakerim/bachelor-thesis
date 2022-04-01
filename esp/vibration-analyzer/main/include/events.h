#ifndef EVENTS_H
#define EVENTS_H

/** 
 * @defgroup events Udalosti
 * @brief Binárne klasifikátory na označenie význačných úrovní v posuvnom okne vzoriek
 *  @{
 */

/**
 * @brief Časové vymedzenie udalosti frekvenčného spektra
 */
typedef enum {              
    SPECTRUM_EVENT_NONE,    /**< @brief Udalosť prebieha alebo žiadna nie je aktívna **/
    SPECTRUM_EVENT_START,   /**< @brief Značka začiatku udalosti **/
    SPECTRUM_EVENT_FINISH   /**< @brief Značka ukončenia udalosti **/
} SpectrumEventAction;

/**
 * @brief Stav udalosti frekvenčného vedierka
 */
typedef struct {
    SpectrumEventAction action;  /**< @brief Značka vymedzenia udalosti: žiadna, začiatok, koniec **/
    uint32_t start;              /**< @brief Časová pečiatka alebo poradie posuvného okna, kedy začala aktuálne aktívna udalosť **/ 
    uint32_t duration;           /**< @brief Trvanie aktívnej udalosti v počte posuvných okien **/
    int32_t last_seen;           /**< @brief Počet posuvných okien do minulosti, kedy bol detegovaný posledný výskyt špičky vo frekvencii **/
    float amplitude;             /**< @brief Priemerná amplitúda frekvenčného vedierka počas trvania udalosti **/
} SpectrumEvent;

#define min(x, y)   (((x) < (y)) ? (x): (y))
#define max(x, y)   (((x) > (y)) ? (x): (y))

/**
 * @brief Hľadanie špičiek absolútnou prahovou úrovňou amplitúdy signálu
 * 
 * @param[out]  peaks   Nájdené špičky v signále. Dĺžka poľa musí byť rovnaká počet vzoriek
 * @param[in]   y       Vzorky signálu
 * @param[in]   n       Počet vzoriek
 * @param[in]   t       Prahová úroveň amplitúdy. Odporúčaný hodnota je z prípustných hodôt rozsahu pre vzorky
 */
void find_peaks_above_threshold(bool *peaks, const float *y, int n, float t);

/** 
 * @brief Hľadanie špičiek s algoritmom význačnosti vrchola spomedzi susedov 
 * 
 * \f$ f[t-i] < f[t] > f[t+i],\quad \forall i \in 1, 2, ..., k \f$
 * 
 * @param[out]  peaks   Nájdené špičky v signále. Dĺžka poľa musí byť rovnaká počet vzoriek
 * @param[in]   y       Vzorky signálu
 * @param[in]   n       Počet vzoriek
 * @param[in]   k       Počet najbližších uvažovaných susedov na každú zo strán od kandidátnej špičky \f$[t-k; t+k]\f$; Rozsah: \f$[1, n / 2]\f$
 * @param[in]   e       Relatívna tolerancia pre vyššiu úroveň v susedstve od vrchola
 * @param[in]   h_rel   Minimálna relatívna výška špičky v susedstve
 * @param[in]   h       Absolútna prahová úroveň amplitúdy špičky
 */
void find_peaks_neighbours(bool *peaks, const float *y, int n, int k, float e, float h_rel, float h);

/**
 * @brief Hľadanie špičiek s algoritmom prechodu nulou do záporu
 * 
 * \f$ \Delta f[i] = 0 \f$
 * 
 * @param[out]  peaks   Nájdené špičky v signále. Dĺžka poľa musí byť rovnaká počet vzoriek
 * @param[in]   y       Vzorky signálu
 * @param[in]   n       Počet vzoriek
 * @param[in]   k       Dĺžka sečnice na každú stranu od kandidátnej špičky \f$[t-k; t+k]\f$; Rozsah: \f$[1, n / 2]\f$
 * @param[in]   slope   Prahová úroveň strmosti kopca, čiže rozdielu medzi hladiny medzi konacmi sečnice. Rozsah: \f$ slope \geq 0 \f$
 */
void find_peaks_zero_crossing(bool *peaks, const float *y, int n, int k, float slope);

/**
 * @brief Hľadanie špičiek s modikovaným algoritmom horského turistu
 * 
 * \f$ \Delta f[i] = 0 \f$
 * 
 * @param[out]  peaks       Nájdené špičky v signále. Dĺžka poľa musí byť rovnaká počet vzoriek
 * @param[in]   y           Vzorky signálu
 * @param[in]   n           Počet vzoriek
 * @param[in]   tolerance   Prahová úroveň vo vertikálnej osi. Rozsah: \f$[min(y), max(y)]\f$
 * @param[in]   hole        Prahová úroveň v horizontálnej osi. Rozsah: \f$[0, n]\f$
 * @param[in]   prominence  Relatívna výška oproti predošlej navštívenej doline. Rozsah: \f$[0, max(y) - min(y)]\f$
 * @param[in]   isolation   Vzdialenosť ku najbližšiemu predošlému vrcholu. Rozsah: \f$[0, n]\f$
 */
void find_peaks_hill_walker(bool *peaks, const float *y, int n, float tolerance, int hole, float prominence, float isolation);


/**
 * @brief Nastavenie počiatočného stavu online detektora udalostí vo frekvenciách
 * 
 * @param[out]  events     Pole udalostí frekvenčného spektra
 * @param[in]   bins       Počet frekvenčných vedierok a zároveň dĺžka poľa uadlostí
 */
void event_init(SpectrumEvent *events, uint16_t bins);


/**
 * @brief Online detekcia zmien v časovom priebehu frekvenčných spektier 
 * 
 * @param       t               Poradové číslo posuvného okna. S každým ďalším volaním funckie musí byť navýšené o 1 
 * @param       events          Pole udalostí frekvenčného spektra s dĺžkou počtu vedierok
 * @param[in]   peaks           Nájdené špičky vo aktuálnom frekvenčnom spektre jedným z klasifikátorov find_peak_*
 * @param[in]   spectrum        Frekvenčné spektrum aktuálneho posuvného okna vzoriek na zistenie priemernej amplitúdy udalostí
 * @param[in]   bins            Počet frekvenčných vedierok
 * @param[in]   min_duration    Minimálne trvanie po koľkých oknách je vyhlásená špička za udalosť. Udáva oneskorenie vyhlásenia začiatku udalosti.
 * @param[in]   time_proximity  Najväčia vzdialenosť súvislej udalosti v počte okien. Najväčšia dĺžka časovej medzery medzi nájdenými špičkami.
 *                              Udáva oneskorenie vyhlásenia ukončujúcej udalosti.
 * 
 * @return Počet detegovaných zmien, čiže začiatočných a ukončujúcich udalostí v danom spektre posuvného okna
 */
size_t event_detection(size_t t, SpectrumEvent *events, const bool *peaks, const float *spectrum, 
                       uint16_t bins, uint16_t min_duration, uint16_t time_proximity);

/** @} */

#endif