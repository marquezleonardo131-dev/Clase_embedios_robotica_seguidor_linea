//PROGRAMA PARA PROBAR MOTORES

// ===== Pines ESP32-C3 =====
const int dir_izq = 20;
const int pwm_i   = 5; // OG 21

const int dir_der = 6;
const int pwm_d   = 21; // OG 5

const int led = 8;


// ===== Función motores =====
void motores(int izq, int der){

  // IZQUIERDO
  if(izq >= 0){
    digitalWrite(dir_izq, HIGH);
  } else {
    digitalWrite(dir_izq, LOW);
    izq = -izq;
  }
  ledcWrite(pwm_i, constrain(izq, 0, 187)); // 187 = 6V con batería totalmente cargada.

  // DERECHO
  if(der >= 0){
    digitalWrite(dir_der, HIGH);
  } else {
    digitalWrite(dir_der, LOW);
    der = -der;
  }
  ledcWrite(pwm_d, constrain(der, 0, 187));
}


// ===== Setup =====
void setup(){
  Serial.begin(115200);

  pinMode(dir_izq, OUTPUT);
  pinMode(dir_der, OUTPUT);

  // PWM (API nueva ESP32)
  ledcAttach(pwm_i, 1000, 8);   // 2 kHz
  ledcAttach(pwm_d, 1000, 8);
  ledcAttach(led,   5000, 8);

  motores(0,0);

  Serial.println("Prueba 2 motores IFX9201");
}


// ===== Loop =====
void loop(){

  // Serial.println("Ambos adelante suave");
  // motores(100, 100);
  // ledcWrite(led, 80);
  // delay(3000);
  

  // Serial.println("Ambos adelante fuerte");
  // motores(80, 80); // 100 -> 7.02, 110 -> 7.16, 150 -> 7.64,50 -> 6.22
  // ledcWrite(led, 200);
  // delay(3000);

  Serial.println("Giro derecha");
  motores(60, -60);
  ledcWrite(led, 180);
   delay(300);

  // Serial.println("Giro izquierda");
  // motores(40, 40);
  // ledcWrite(led, 180);
  // delay(300);

  Serial.println("Reversa");
  motores(20, -20);
  ledcWrite(led, 120);
  delay(300);

  Serial.println("Stop");
  motores(0, 0);
  ledcWrite(led, 0);
  delay(300);
}
