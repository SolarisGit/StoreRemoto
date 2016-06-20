
#include <IRremote.h>            // Remote IRD
#include <Adafruit_NeoPixel.h>   // Smart Led Ring
#include <EEPROM.h>              // Eeprom memory 

#define RECV_PIN  2        // IRD sensor connected to digital pin 2
#define CUENTA_VUELTA 3    // Pin de interrupción para contar el número de vueltas que lleva el store
#define PIXEL_PIN    4     // Digital IO pin connected to the NeoPixels.
#define PIXEL_COUNT 24     // Number of smart Leds
#define SUBIR_PERSIANA 12  // Activar el relé para subir la persiana
#define BAJAR_PERSIANA 11  // Activar el relé para bajar persiana
#define FIN 13             // Número de vueltas de subida o bajada total de la persiana


//*****************************   VARIABLES   *******************************
unsigned int store_state;
unsigned int last_store_state;
uint16_t cycles;
uint16_t reversecycles;
volatile int vuelta = 0;
volatile int last_vuelta = 0;
volatile int fin_carrera = 0;
unsigned long actual_time = 0;
unsigned long first_time = 0;
unsigned long last_time = 0;
int first_enter = 0;
int first_enter1 = 0;
uint16_t check1 = 0;    // Sistema que comprueba que efectivamente ha pasado una vuelta y no se trata de una perturbación
uint16_t check2 = 0;
uint16_t check3 = 0;

//****************************    FUNCTIONS   ********************************

// Parameter 1 = number of pixels in strip,  neopixel stick has 8
// Parameter 2 = pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_RGB     Pixels are wired for RGB bitstream
//   NEO_GRB     Pixels are wired for GRB bitstream, correct for neopixel stick
//   NEO_KHZ400  400 KHz bitstream (e.g. FLORA pixels)
//   NEO_KHZ800  800 KHz bitstream (e.g. High Density LED strip), correct for neopixel stick
Adafruit_NeoPixel strip = Adafruit_NeoPixel(PIXEL_COUNT, PIXEL_PIN, NEO_GRB + NEO_KHZ800);
IRrecv irrecv(RECV_PIN);  // Interrupcion para recibir IRD
decode_results results;


void setup()
{
  //Inicializamos con la persina parada:
  pinMode(SUBIR_PERSIANA, OUTPUT);
  pinMode(BAJAR_PERSIANA, OUTPUT);
  digitalWrite(SUBIR_PERSIANA, HIGH);
  digitalWrite(BAJAR_PERSIANA, HIGH);
  
  //Inicializamos los pines de los sensores hall:
  pinMode(CUENTA_VUELTA, INPUT);
  
  //Inicializamos el bus de comunicación serie:
  Serial.begin(9600);
  
  //Iniciamos el sensor infrarrojo:
  irrecv.enableIRIn(); // Start the receiver
  
  //Inicializamos el Led-ring:
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
  
  // Inicializamos el estado del store
  store_state = 0;
  cycles = 0;
  fin_carrera = 0;
  first_enter = 0;

  //EEPROM.write(0, 5); // DEBUG VUELTA 
  //EEPROM.write(1, 0); // DEBUG FIN DE CARRERA --> 0: en rango   1: store arriba  2: store abajo
  
//  vuelta = EEPROM.read(0); 
  vuelta=0;
  Serial.print("Vuelta numero: ");
  Serial.println(vuelta);

//  fin_carrera = EEPROM.read(1); 
  fin_carrera=0;
  Serial.print("Fin de carrera: ");
  Serial.println(fin_carrera);

  // Inicializamos la interrupción del cuenta vueltas
  pinMode(CUENTA_VUELTA, INPUT);
  attachInterrupt(digitalPinToInterrupt(CUENTA_VUELTA), contador_vueltas, FALLING);
}

void contador_vueltas() {       // Interrupción por cada vuelta de persina activada por el sensor hall

  delay(20);
  check1 = digitalRead(CUENTA_VUELTA);
  delay(30);
  check2 = digitalRead(CUENTA_VUELTA);
  delay(50);
  check3 = digitalRead(CUENTA_VUELTA);
    
  if(check1==0 && check2==0 && check3==0){  

      if(store_state==1){ vuelta--; }
      else if(store_state==2){ vuelta++; }

//      if(vuelta<=0){ parar_persiana(); fin_carrera=1;}              // Ahora sólo puede bajar
//      else if(vuelta>=FIN){ parar_persiana(); fin_carrera=2;}        // Ahora sólo puede subir
//      else {fin_carrera=0;}                                         // Persiana dentro de rango

      Serial.print("Vuelta numero:");  Serial.println(vuelta);      // Debug vueltas

//      EEPROM.write(0, vuelta);                                      // Se guarda en número de vuelta de la persiana por si se va la luz
//      EEPROM.write(1, fin_carrera); 
  }
}

void loop() {

  if (irrecv.decode(&results)) {                     // Mirar si hemos recibido algo nuevo por el infrarrojo
    irrecv.resume(); // Receive the next value
    Serial.println(results.value, HEX);
    
     switch(results.value){
        case 0xE0E012ED:                             // Subir persiana hasta arriba
            if(store_state==2){parar_persiana();}
            store_state = 1;
            //reversecycles=25600;
            //if(fin_carrera==1){break;}
            subir_persiana();
            break;  
        case 0xE0E0A25D:                             // Bajar persiana hasta abajo
            if(store_state==1){parar_persiana();}
            store_state = 2;
            cycles=0; 
            //if(fin_carrera==2){break;}
            bajar_persiana();              
            break;
        case 0xE0E052AD:                             // Parar la persiana en este momento
            store_state = 3;
            parar_persiana();
            //colorWipe(strip.Color(0, 0, 0), 50);
            break;
     }  
  }
/*
  switch(store_state){                          // Actuar conforme al estado y posición del estore

        case 1:                                 // Subir la persiana poco a poco
            reversecycles--;
            reverseRainbowCycle(reversecycles);
            if(reversecycles<1){store_state=0; reversecycles=256; parar_persiana(); } 
        break;
        case 2:                                 // Bajar la persiana poco a poco
            cycles++;        
            rainbowCycle(cycles);
            if(cycles>256){store_state=0; cycles=0; parar_persiana(); }  
        break;
        case 3:                                 // Subir la persiana hasta arriba
            reversecycles--;
            reverseRainbowCycle(reversecycles); 
            if(fin_carrera==1){store_state=0; }
        break;
        case 4:                                 // Bajar la persiana hasta abajo
            cycles++;        
            rainbowCycle(cycles); 
            if(fin_carrera==2){store_state=0; }        
        break;
     }

  // Apagar el anillo led después de un tiempo
  if(store_state==0){
    actual_time = millis();
    if(first_enter==0){first_enter=1; first_time=millis();}
    if((actual_time-first_time)>5000){first_enter=0; colorWipe(strip.Color(0, 0, 0), 20);}
  }else{first_enter=0;}
*/   

  if(store_state==1 || store_state==2){
    if(vuelta == last_vuelta){
      if(last_store_state == store_state){
        if(first_enter1==0){first_enter1=1; last_time=millis();}
        if((millis()-last_time)>3000){
            first_enter1=0;
            parar_persiana();
            if(store_state==1){fin_carrera=1; vuelta=0;}      //Store arriba del todo
            else{fin_carrera=2; vuelta=13;}                   //Store abajo del todo
            store_state = 3;
          }
        }else{last_store_state = store_state; first_enter1=0;}
      }else{last_vuelta = vuelta; first_enter1=0;}
      
  }else{first_enter1=0;}

  delay(1);
  
}

// *********************************      MOVIMIENTOS DE LA PERSIANA     ****************************************************

void subir_persiana(){ 
    digitalWrite(SUBIR_PERSIANA, HIGH);
    delay(500); 
    digitalWrite(BAJAR_PERSIANA, LOW); 
    delay(500);   
    Serial.println("Subiendo store...");  
}

void bajar_persiana(){
    digitalWrite(SUBIR_PERSIANA, LOW);
    delay(500); 
    digitalWrite(BAJAR_PERSIANA, HIGH);  
    delay(500);  
    Serial.println("Bajando store...");  
}

void parar_persiana(){ 
    digitalWrite(SUBIR_PERSIANA, HIGH);
    delay(500); 
    digitalWrite(BAJAR_PERSIANA, HIGH);  
    delay(500); 
    Serial.println("Store parado"); 
}


// *******************************      ANILLO DE COLORES     ***************************************************************

void rainbowCycle(uint16_t j) {
    uint16_t i;
    for(i=0; i< strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
    strip.show();
}

void reverseRainbowCycle(uint16_t j) {
  int i;
    for(i=strip.numPixels(); i>=0; i--) {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
    strip.show();
}

// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}


