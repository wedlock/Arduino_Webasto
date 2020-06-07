# Arduino_Webasto

Projekt sterownika do Webasto oparty na adruino ( w tym przyopadku Adruino nano ). Do działania całości niezbędne są dwa termometry ( DS18B20 ) do odczytu temperatury spalin i cieczy w układzie oraz wyświetlacz LCD 2x16 linii , 1 przycisk monostabilny, kilka rezystorów, MOSFETY i diody LED  ( wszystkie elementy są wymienione w schemacie ). Sterownik ustawiony jest na maksymalną temperaturę 70 stopni .

Do projektu dołączam pliki EAGLE i PDF niezbędne do wykonania płytki.

Krótka instrukcja działania :

Gdy sterownik znajduje się w stanie IDLE jednoktorne wciśnięcie przycisku powoduje rozpoczęcie sekwencji startowej. Na ekranie wyświetla się napis BURNING UP. Ponowne wciśnięcie przycisku w każdym momencie gdy sterownik jest w trybie BURNING UP, WORKING albo SLEEP powoduje wygaszenie pieca. Przytrzymanie przycisku 2 sek powoduje natychmiastowe zatrzymanie i reset programu.

Podczas pracy sterownik cały czas sprawdza temp. spalin i wody i gdy wykryje nieprawidłowości wyłącza układ. Mi osobiście przez całą zimę nie wyłączył się awaryjnie ani razu. Dla bezpieczeństwa wbudowałem watchdoga, który co kilka milisekund sprawdza czy arduino się zawiesiło, a jeśli tak to resetuje cału układ i wygasza się on bezpiecznie.

Stany sterownika:
1. IDLE oczekuje na włączenie ( sprawdzanie temp. cieczy ciągle włączone i ewentualnie włączy się pompa wody gdy temperatura wzrośnie )
2. BURNING UP rozpalanie pieca. Odbywa się to etapami :
  - przedmuchanie komory spalania celem pozbycia się oparów ropy
  - którkie pomowanie paliwa żeby rozsączyło się na palniku i odczekanie kulku sekund
  - włączenie świecy żarowej
  - gdy nastąpi wzrost temp. wody o 3 stopnie lub temp. wody o 2 stopnie piec przechodzi w tryb działania WORK
3. WORK - normalna praca pieca. Przy temp wody 20 stopni pompa wody jest włączana na 1/2 mocy. Zapobiega to gotowaniu się wody na powierzchni webasta. Po ogrzaniu się wody do 30 stopni sterownik urychamia się na 100% mocy. Gdy temp wody dojdzie do 70 stopni piec usypia się i czeka aż woda ostygnie do 65 stopni po czym znowy się uruchamia
4. BURN OFF wygaszanmie pieca. Nie jest podawane paliwa a wiatrak zwalnia. Zapobiega to cofaniu się płomienia. Gdy temp. spalin i wody przez dłuższą chwilę nie wzrasta, piec wyłącza się.


Na płytce zapomniałem dodać pinów do wlutowania przycisku ( ups. ) ale bez problemu da się to przylutować bezpośrednio do płytki. Są to przewody sygnałowe i ich średnica może być nawet i 0,5mm
