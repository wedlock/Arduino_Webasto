# Arduino_Webasto

Projekt sterownika do Webasto oparty na adruino ( w tym przyopadku Adruino nano ). Do działania całości niezbędne są dwa termometry ( DS18B20 ) do odczytu temperatury spalin i cieczy w układzie oraz wyświetlacz LCD 2x16 linii oraz 1 przycisk monostabilny. Sterownik ustawiony jest na maksymalną temperaturę 70 stopni .

Do projektu dołączam pliki EAGLE i PDF niezbędne do wykonania płytki.

Krótka instrukcja działania :

Gdy sterownik znajduje się w stanie IDLE jednoktorne wciśnięcie przycisku powoduje rozpoczęcie sekwencji startowej. Na ekranie wyświetla się napis BURNING UP. Ponowne wciśnięcie przycisku w każdym momencie gdy sterownik jest w trybie BURNING UP, WORKING albo SLEEP powoduje wygaszenie pieca. Przytrzymanie przycisku 2 sek powoduje natychmiastowe zatrzymanie i reset programu.
