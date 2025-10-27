#include "ChipNorm_LED_Matrix.h"

#define SPI_MOSI 23 // MOSI (DIN) mit D23 verbinden
#define SPI_CLK 18 // CLK mit D18 verbinden
#define SPI_CS 15 // CS mit  D15 verbinden

ChipNorm_LED_Matrix matrix(SPI_MOSI, SPI_CLK, SPI_CS); //Initialisiert die LED-Matrix (Objekt "matrix" der Klasse "ChipNorm_LED_Matrix" erstellt) 

void setup() {
    matrix.setBrightness(0); //Helligkeit einstellt (zwischen 0 und 15)
    matrix.clear();  // LED-Matrix Inhalt löschen
    matrix.update(); // LED-Matrix updaten
}

void loop() {
    matrix.clear(); // LED-Matrix Inhalt löschen
    
    // Beispiel 1: Einzelne Buchstaben / Zahlen / Zeichen
    matrix.write('A'); // Schreibt 'A' auf Segment 0
    delay(2000);
    matrix.clear();

    matrix.write('1'); // Schreibt '1' auf Segment 0
    delay(2000);
    matrix.clear();

  
    // Beispiel 2: Manueller Modus: Rahmen zeichnen
    // (Segment, Spalte, Ansteuerung)
    matrix.write_manuell(0, 0, 0b11111111); // Digit 0: Alle an (Untere Zeile)
    matrix.write_manuell(0, 1, 0b10000001); // Digit 1: Rahmen links/rechts
    matrix.write_manuell(0, 2, 0b10000001); // Digit 2: Rahmen links/rechts
    matrix.write_manuell(0, 3, 0b10000001); // Digit 3: Rahmen links/rechts
    matrix.write_manuell(0, 4, 0b10000001); // Digit 4: Rahmen links/rechts
    matrix.write_manuell(0, 5, 0b10000001); // Digit 5: Rahmen links/rechts
    matrix.write_manuell(0, 6, 0b10000001); // Digit 6: Rahmen links/rechts
    matrix.write_manuell(0, 7, 0b11111111); // Digit 7: Alle an (Oberste Zeile)


    delay(1000);

    // Beispiel 3: Manuell den Inahalt nach links schieben
    for (int row = 0; row < 8; ++row) {
      matrix.moveLeft(); // move one row to the left
      matrix.update();
      delay(500); //Speed of scrolling
    }
    
    matrix.clear();

    // Beispiel 4: Text Scrollen "CHIPNORM"
    const char* textToScroll = "C H I P N O R M";
    const int Anzahl_seg = 1; // Wir verwenden nur 1 Segement
    const int durchlauf = 2; // 2 mal durchlaufen
    
    matrix.scroll_text(textToScroll, Anzahl_seg, durchlauf);

    
    delay(3000);
}
