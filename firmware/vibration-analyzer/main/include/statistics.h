#ifndef STATISTICS_H
#define STATISTICS_H

/** @defgroup statistics Deskriptívna štatistika
 *  @{
 */

/**
 * @brief Najnižšia hodnota
 *  
 * @param[in]  x   Vzorky signálu
 * @param[in]  n   Počet vzoriek signálu
 * @return  Mimimum z hodnôt signálu
 */
float minimum(const float *x, int n);
/**
 * @brief Najvyššia hodnota
 *  
 * @param[in]  x   Vzorky signálu
 * @param[in]  n   Počet vzoriek signálu
 * @return  Maximum z hodnôt signálu
 */
float maximum(const float *x, int n);

/**
 * @brief Stredná kvadratická odchýlka
 *  
 * @param[in]  x   Vzorky signálu
 * @param[in]  n   Počet vzoriek signálu
 * @return  RMS z hodnôt signálu
 */
float root_mean_square(const float *x, int n);

/**
 * @brief Aritmetický výberový priemer
 *  
 * @param[in]  x   Vzorky signálu
 * @param[in]  n   Počet vzoriek signálu
 * @return  Priemer hodnôt signálu
 */
float mean(const float *x, int n);

/**
 * @brief Rozptyl populácie (vychýlená štatistika)
 *  
 * @param[in]  x   Vzorky signálu
 * @param[in]  n   Počet vzoriek signálu
 * @return  Rozptyl hodnôt signálu
 */
float variance(const float *x, int n, float mean);

/**
 * @brief Smerodajná odchýlka
 *  
 * @param[in]  variance   Rozptyl signálu
 * @param[in]  n   Počet vzoriek signálu
 * @return  Smerodajná odchýlka hodnôt signálu
 */
float standard_deviation(float variance);

/**
 * @brief Centrálny moment
 *  
 * @param[in]  x    Vzorky signálu
 * @param[in]  n    Počet vzoriek signálu
 * @param[in]  m    Rád centrálneho momentu.  Kladné číslo väčšie ako 1.
 * @param[in]  mean Aritmetický priemer signálu
 * @return  Centrálny moment
 */
float moment(const float *x, int n, int m, float mean);

/**
 * @brief Šikmosť
 *  
 * @param[in]  x    Vzorky signálu
 * @param[in]  n    Počet vzoriek signálu
 * @param[in]  mean Aritmetický priemer signálu
 * @return  Šikmosť
 */
float skewness(const float *x, int n, float mean);

/**
 * @brief Špicatosť
 *  
 * @param[in]  x    Vzorky signálu
 * @param[in]  n    Počet vzoriek signálu
 * @param[in]  mean Aritmetický priemer signálu
 * @return  Špicatosť
 */
float kurtosis(const float *x, int n, float mean);

/**
 * @brief Korelácia z medzivýsledkov
 *  
 * @param[in]  x_diff    Predspracované vzorky prvého signálu odčítané od aritmetického priemeru: \f$ (x_i - \bar{x}) \f$
 * @param[in]  y_diff    Predspracované vzorky druhého signálu odčítané od aritmetického priemeru: \f$ (y_i - \bar{y}) \f$
 * @param[in]  n         Počet vzoriek signálu. Dĺžky oboch polí musia byť rovnaké.
 * @param[in]  x_std     Smerodajná odchýlka prvého signálu
 * @param[in]  y_std     Smerodajná odchýlka druhého signálu
 * @return  Pearsonov korelačný koeficient
 */
float correlation(const float *x_diff, const float *y_diff, int n, float x_std, float y_std);


/**
 * @brief Quickselect. Algoritmus na nájdenie k-teho najmenšieho prvku v nezoradenom poli. Aby nedochádzalo
 * k modifikácii poradia pôvodného poľa kopíruje prvky do poľa z premenlivou dĺžkou (Variable-length array)
 * podľa `n` na zásobníku.
 *  
 * @param[in]  x    Vzorky signálu
 * @param[in]  n    Počet vzoriek signálu
 * @param[in]  k    Rád k-teho najmenšieho prvku
 * @return  k-ty najmenší prvok
 */
float quickselect(const float *x, int n, int k);

/**
 * @brief Medián cez Quickselect
 *  
 * @param[in]  x    Vzorky signálu
 * @param[in]  n    Počet vzoriek signálu
 * @return  Medián
 */
float median(const float *x, int n);

/**
 * @brief Mediánová absolútna odchýlka (MAD). Medzi-výsledky odchýlok na nájdenie mediánu ukladá
 * do poľa z premenlivou dĺžkou (Variable-length array) podľa `n` na zásobníku.
 *  
 * @param[in]  x    Vzorky signálu
 * @param[in]  n    Počet vzoriek signálu
 * @param[in]  med  Medián signálu
 * @return  MAD
 */
float median_abs_deviation(const float *x, int n, float med);

/**
 * @brief Priemerná absolútna odchýlka (AAD)
 *  
 * @param[in]  x    Vzorky signálu
 * @param[in]  n    Počet vzoriek signálu
 * @param[in]  mean Aritmetický priemer signálu
 * @return  AAD
 */
float average_abs_deviation(const float *x, int n, float mean);
/** @} */

#endif
 
