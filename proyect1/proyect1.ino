#include <TFT_eSPI.h>
#include <SPI.h>

TFT_eSPI tft = TFT_eSPI();


void setup() {
  // put your setup code here, to run once:

  tft.init();
  //Con este código cambio el color del fondo de la pantalla
  tft.fillScreen(0x0000);
  //Código para imprimir un texto en la pantalla, el primer argumento es la posición x, el segundo es la posición y, el tercero es el tamaño de la fuente
  tft.drawString("Hola Mundo",10,10,4);
  
}

int i=0;

void loop() {
  // put your main code here, to run repeatedly:
  tft.drawString(String(i), 30, 100, 7);
  i++;
  delay(1000);
}
