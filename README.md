# JerzyKrell — pady sterowane sekwencerem drumlogue

Nowy sześciogłosowy syntezator USER ENGINE. Pady inspirowane charakterem Krell/Buchla: FM sinusów, wavefolder, powolna losowa modulacja, dryf stroju, filtr powiązany z obwiednią oraz stereo ping-pong echo. To autorska interpretacja, nie emulacja konkretnego modułu. Instrument nie korzysta z sampli.

## Wgranie
1. Pobierz gotowy plik jerzy_krell.drmlgunit z artefaktu JerzyKrell-drumlogue w GitHub Actions.
2. Wyłącz drumlogue, podłącz USB TO HOST do komputera i uruchom urządzenie, trzymając REC.
3. Skopiuj tylko jerzy_krell.drmlgunit do Units/Synths/ na dysku drumlogue.
4. Bezpiecznie wysuń dysk, naciśnij PLAY/STOP, wybierz JerzyKrell jako źródło USER.

## Sterowanie z SEQ
Domyślnie MODE=Bloom: krótki krok uruchamia pełną obwiednię ATTACK → HOLD → RELEASE. Zacznij od kroków 1 i 9, tempa 80–100 BPM i CHORD=Sus2. Długie ogony mogą się nakładać; dostępnych jest sześć głosów (dwa trzydźwiękowe akordy).

Ustaw NOTE i nagraj jego zmiany przez Motion, np. C3 → D3 → G2 → F3. NOTE jest zapamiętywany przy wyzwoleniu kroku; sama zmiana NOTE nie tworzy nuty ani nie przestroi już trwającego pada. Motion może także sterować FM, FOLD, TONE i DEPTH. Dokładny moment startu i dynamika pochodzą z bramki sekwencera.

MODE=Gate: pad narasta i trwa do zamknięcia bramki, potem wybrzmiewa przez RELEASE. Długość bramki musi być odpowiednio duża względem ATTACK. MIDI korzysta z rzeczywistej wysokości i velocity nuty. Pitch bend ±2 półtony. All Notes Off/reset usuwa także echo. Po uruchomieniu instrument milczy do pierwszego kroku lub nuty MIDI.

## Parametry (sześć stron po cztery)
| Strona | Parametry | Działanie |
|---|---|---|
| 1 | NOTE, CHORD, MODE, VELOCITY | Wysokość, Single/Fifth/Minor/Major/Sus2/Quartal, Bloom/Gate, wpływ velocity |
| 2 | ATTACK, HOLD, RELEASE, VARIATION | Czasy w ms; losowe odchylenie czasów do ±50% |
| 3 | FM, RATIO, FOLD, TONE | Głębokość FM, stosunek częstotliwości, wavefolder, filtr w Hz |
| 4 | MOTION, DEPTH, DRIFT, LPG | Czas losowej modulacji w ms, jej głębokość, dryf w centach, filtr zależny od obwiedni |
| 5 | SPREAD, DETUNE, ECHO, FEEDBACK | Ruch stereo, rozstrojenie drugiego oscylatora, echo, sprzężenie |
| 6 | SEED, AIR, LEVEL, FREEZE | Ziarno losowości, delikatny szum, poziom, zatrzymanie płynnej modulacji |

Echo: 3/4 ćwierćnuty według tempa urządzenia, maksymalnie 1 sekunda. FREEZE zatrzymuje modulację barwy i panoramy; obwiednie i nuty nadal działają. SEED zmienia sekwencję losowości. Ogranicz FM/FOLD przy wysokich nutach, jeżeli chcesz łagodniejszą barwę.

### Przykładowe ustawienia
- Szklany pad: FM=60, RATIO=7, FOLD=15, TONE=6500, ATTACK=900, HOLD=5000, RELEASE=9000.
- Ciemna chmura: CHORD=Minor, FM=15, RATIO=0.5, FOLD=45, TONE=1000, ATTACK=3000, HOLD=6000, RELEASE=12000, DEPTH=70.
- Krótszy pad do rytmu: ATTACK=200, HOLD=300, RELEASE=2000, VARIATION=20.

## Kompilacja i weryfikacja
GitHub Actions używa przypiętego oficjalnego KORG logue-sdk oraz obrazu kompilatora. Testy sprawdzają bramki, obwiednie, wysokość Motion, MIDI, przepełnienie kolejki, ciszę, skrajne parametry i zgodność różnych rozmiarów bufora z ASan/UBSan. Validator sprawdza ELF32 ARM hard-float, nagłówek drumlogue API 2.0 i symbole. Brak testu na fizycznym drumlogue; wydajność i brzmienie należy potwierdzić na urządzeniu.

Kod DSP bez alokacji w renderze, 48 kHz stereo. Kolejka SPSC zakłada serializowane callbacki sterujące hosta. Demo WAV to rendering tego samego DSP na komputerze, z wyzwoleniami imitującymi SEQ.

Inspiracja: [Buchla 266e](https://buchla.com/product/266e/), [Todd Barton](https://www.toddbarton.com/workshops).
SDK i interfejs: [KORG logue-sdk](https://github.com/korginc/logue-sdk). Licencja SDK w LICENSE_KORG.txt; autorski DSP w synth/synth.h na MIT.
