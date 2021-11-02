# scgms_cho_module
SmartCGMS module for carbohydrate and physical activity detection


## Knihovny, kompilace, spuštění
SmartCGMS je dostupný na [https://diabetes.zcu.cz/smartcgms](https://diabetes.zcu.cz/smartcgms). Knihovna
je kompilována v C++ 17. Pro kompilaci jsou nutné header soubory SmartCGMS a knihovny VisualLeakDetector, frugally-deep, Eigen, FunctionalPlus a json. K dispozici je CMake skript. Vytvořená dynamická knihovna
**detection.dll** se umístí do složky filters. Grafické rozhraní SmartCGMS se
spustí programem gpredict3.exe, konzolová verze programem console3.exe.

## Nastavení filtrů
### Savitzky-Golay filtr
Filtr pro vyhlazení dat.
* Signal - zdrojový signál pro vyhlazení
* Window size - velikost okna Savitzky-Golay filtru
* Degre - stupeň polynomu
Filtr posílá vyhlazená data v signálu Savgol signal. V případě použití více
filtrů je nutné výstupní signál přemapovat.

### CHO detection
Filtr detekce příjmu karbohydrátů.
* Signal - detekovaný signál
* Window size - velikost klouzavého okénka
* Detect edges - detekce vzestupných hran
* Detect descending edges - detekce sestupných hran
* Rise threshold - threshold pro určení míry stoupání/klesání v čase
* Use RNN - použití rekurentní neuronové sítě
* RNN model file path - cesta k souboru s natrénovaným keras modelem převedeným do formátu pro frugally-deep
* RNN threshold - threshold detekce neuronovou sítí
* Thresholds
  * Threshold Low - threshold malé změny IST
  * Weight Low - váha malé změny IST
  * Threshold High - threshold velké změny IST
  * Weight High - váha velké změny IST
  
Filtr posílá aktivační funkce a detekované karohydráty. Příklad konfigurace
detekce hran průběhu intersticiální glukózy je v souboru setup/setup_th.ini,
příklad neuronové sítě v souboru setup/setup_gru.ini‘.

### PA detection
Filtr detekce fyzické aktivity.
* Heartbeat - detekce podle srdečního tepu
* Steps - detekce podle počtu kroků
* Acceleration - detekce podle hodnoty akcelerace
* Electrodermal activity - detekce podle elektrodermální aktivity
* Mean - použití průměru za časové okno
* Window size - mean - velikost klouzavého okénka pro spočítání průměru (v případě velikosti okna 1 je průměr rovná aktuální hodnotě)
* Detect IST edges - potvrzení detekce sestupnou hranou dat IST
* Signal - detekovaný signal
* Window size - edges - velikost klouzavého okénka pro detekci hran
* Thresholds - threshold ukazatelů pro detekci a thresholdy a váhy pro detekci hran

Filtr posílá detekovanou fyzickou aktivitu. Příklad konfigurace s měřeným srdečním tepem a počtem kroků je v konfiguračním souboru setup/setup_bpm.ini.
Příklad konfigurace s akcelerací a potvrzováním pomocí detekce sestupné
hrany je v konfiguračním souboru setup/setup_acc.ini.

### Evaluation
Filtr pro vyhodnocení výsledků.
* Reference signal - referenční signál
* Detected signál - detekovaný signál
* Max detection delay - maximální zpoždění, které může mít detekovaný signál oproti referenčnímu
* False positive cooldown - cooldown po detekci falešně pozitivního signálu, než je započítán další
* Late detection delay - čas před referenčním signálem, kdy se bude detekce počítat jako pravdivě pozitivní
* Min reference count - minimální počet referenčních signálů za den

Filtr na konci běhu simulace posílá info s naměřenými statistikami počtu
referenčních signálů TP, potvrzené TP, FN, FP, zpoždění detekce a zpoždění
potvrzení.
