// ==================== DEFINICIONES Y CONSTANTES ====================

#define iter_calibra 100  // Número de lecturas para calibración

// Pines del multiplexor de sensores (16 sensores por 4 pines)
const int com    = 1;   // Pin analógico común (entrada de todos los sensores)
const int s0     = 0;   // Bit 0 de selección de sensor
const int s1     = 2;   // Bit 1 de selección de sensor
const int s2     = 3;   // Bit 2 de selección de sensor
const int s3     = 4;   // Bit 3 de selección de sensor
const int led_on = 7;   // Enciende LED de la barra de sensores

// Pines de motores
const int dir_izq = 20;  // Dirección motor izquierdo (HIGH=adelante, LOW=atrás)
const int pwm_i   = 21;  // Velocidad motor izquierdo (0-255)
const int dir_der = 6;   // Dirección motor derecho
const int pwm_d   = 5;   // Velocidad motor derecho
const int led     = 8;   // LED indicador de estado
const int btn     = 9;   // Botón de control (INPUT_PULLUP)

// Configuración PWM para ESP32-C3
#define PWM_FREQ 20000   // Frecuencia 20kHz
#define PWM_RES  8       // Resolución 8 bits (0-255)

// ==================== VARIABLES GLOBALES ====================

int sensores[16];        // Valor RAW del sensor (0-4095)
int digital[16];         // Valor digital (0=blanco, 1=negro)
int lectura_fondo[16];   // Calibración: valor en fondo blanco
int lectura_linea[16];   // Calibración: valor en línea negra
int umbral[16];          // Punto medio para binarizar

long int sumap, suma;    // sumap=suma ponderada, suma=conteo de sensores activos
long int pos, last_pos;  // pos=posición actual, last_pos=última posición (por si pierde línea)

int linea = 0;           // 0=negra/blanco, 1=blanca/fondo negro
int barra = 0;           // 0=orden normal, 1=orden espejo
static int bandera_l = 0; // 0=calibra fondo, 1=calibra línea

// Constantes del PID
float KP = 0.4;          // Ganancia proporcional (reacción al error actual)
float KD = 8;            // Ganancia derivativa (reacción a cambios bruscos)
float KI = 0.001;            // Ganancia integral (corrige error acumulado)
int vel = 115;            // Velocidad base
int vel_adelante = 70;   // Velocidad para curvas (adelante)
int vel_atras = 60;      // Velocidad para curvas (atrás)
float derivada_max = 65; // Límite de la componente derivativa

// Buffer de errores para el PID integral
int error1=0,error2=0,error3=0,error4=0,error5=0,error6=0;

// Variables del PID
int proporcional = 0;    // Error actual
int integral = 0;        // Suma de errores
int derivativo = 0;      // Diferencia entre error actual y anterior
int diferencial = 0;     // Salida del PID
int last_prop = 0;       // Error anterior
int setpoint = 750;      // Posición deseada (centro del sensor array)

// ==================== FUNCIONES ====================

// Control de velocidad PWM
void pwmWrite(int pin, int valor) {
  ledcWrite(pin, constrain(valor, 0, 255));  // Limita entre 0 y 255
}

// Espera con anti-rebote (evita falsos pulsos)
void esperarBoton() {
  digitalWrite(led, HIGH);                    // LED encendido: esperando
  bool boton_presionado = false;
  unsigned long tiempoInicio = 0;
  const unsigned long tiempoConfirmacion = 300; // 300ms para confirmar
  
  while (!boton_presionado) {
    if (digitalRead(btn) == LOW) {            // Botón presionado?
      if (tiempoInicio == 0) tiempoInicio = millis();
      if (millis() - tiempoInicio >= tiempoConfirmacion) {
        boton_presionado = true;
        digitalWrite(led, LOW);               // LED apagado: confirmado
        delay(100);
      }
    } else {
      tiempoInicio = 0;                        // Reinicia si soltó antes
      digitalWrite(led, HIGH);
    }
  }
  while (digitalRead(btn) == LOW);            // Espera que suelte
  delay(50);
}

// Calibración: lee fondo o línea según bandera_l
void fondosylinea() {
  for (int i = 0; i < 16; i++) {
    // Selecciona sensor i (combinación de bits)
    digitalWrite(s0, i & 0x01);
    digitalWrite(s1, i & 0x02);
    digitalWrite(s2, i & 0x04);
    digitalWrite(s3, i & 0x08);
    
    if(bandera_l == 0){
      delayMicroseconds(10);
      lectura_fondo[i] = analogRead(com);      // Guarda valor de fondo
    }
    if(bandera_l == 1){
      delayMicroseconds(10);
      lectura_linea[i] = analogRead(com);      // Guarda valor de línea
    }
  }
}

// Calcula umbrales (promedio entre fondo y línea)
void promedio() {
  for (int i = 0; i < 16; i++) {
    umbral[i] = (lectura_fondo[i] + lectura_linea[i]) / 2;
  }
}

// Lee sensores y calcula posición de la línea
int lectura(void) {
  // Lectura de 16 sensores
  for (int i = 0; i < 16; i++) {
    digitalWrite(s0, i & 0x01);
    digitalWrite(s1, i & 0x02);
    digitalWrite(s2, i & 0x04);
    digitalWrite(s3, i & 0x08);
    delayMicroseconds(10);
    sensores[i] = analogRead(com);
    
    // Binarización: convierte analógico a digital (0 o 1)
    if (linea == 0) 
      digital[i] = (sensores[i] <= umbral[i] ? 0 : 1);  // Negra/blanco
    else 
      digital[i] = (sensores[i] <= umbral[i] ? 1 : 0);  // Blanco/negro
  }
  
  // Cálculo de posición ponderada (cada sensor tiene un peso)
  if (barra == 0) {
    // Orden normal: sensor 0 = izquierda
    sumap = (1500*digital[0] + 1400*digital[1] + 1300*digital[2] +
             1200*digital[3] + 1100*digital[4] + 1000*digital[5] +
              900*digital[6] + 800*digital[7] + 700*digital[8] +
              600*digital[9] + 500*digital[10] + 400*digital[11] +
              300*digital[12] + 200*digital[13] + 100*digital[14] +
                0*digital[15]);
  } else {
    // Orden inverso: sensor 15 = izquierda
    sumap = (1500*digital[15] + 1400*digital[14] + 1300*digital[13] +
             1200*digital[12] + 1100*digital[11] + 1000*digital[10] +
              900*digital[9] + 800*digital[8] + 700*digital[7] +
              600*digital[6] + 500*digital[5] + 400*digital[4] +
              300*digital[3] + 200*digital[2] + 100*digital[1] +
                0*digital[0]);
  }
  
  // Cuenta cuántos sensores ven línea
  suma = 0;
  for (int i = 0; i < 16; i++) suma += digital[i];
  
  // Si perdió la línea, usa la última posición conocida
  if (suma == 0) return last_pos;
  
  // Posición final = promedio ponderado
  pos = sumap / suma;
  last_pos = pos;
  return pos;
}

// Control de motores (valores positivos=adelante, negativos=atrás)
void motores(int izq, int der) {
  // Motor izquierdo
  if (izq >= 0) digitalWrite(dir_izq, HIGH);
  else { digitalWrite(dir_izq, LOW); izq *= -1; }
  pwmWrite(pwm_i, izq);
  
  // Motor derecho
  if (der >= 0) digitalWrite(dir_der, HIGH);
  else { digitalWrite(dir_der, LOW); der *= -1; }
  pwmWrite(pwm_d, der);
}

// Control PID: corrige la dirección automáticamente
void PID() {
  proporcional = pos - setpoint;                    // Error actual
  derivativo = proporcional - last_prop;            // Velocidad del error
  if (derivativo > derivada_max) derivativo = derivada_max;
  if (derivativo < -derivada_max) derivativo = -derivada_max;
  
  integral = error1+error2+error3+error4+error5+error6;  // Error acumulado
  
  last_prop = proporcional;
  // Desplaza buffer de errores
  error6=error5; error5=error4; error4=error3;
  error3=error2; error2=error1; error1=proporcional;
  
  // Ecuación del PID
  diferencial = (proporcional*KP) + (derivativo*KD) + (integral*KI);
  
  // Limita la corrección máxima
  if (diferencial > (vel+20)) diferencial = (vel+20);
  if (diferencial < -(vel+20)) diferencial = -(vel+20);
  
  // Velocidad dinámica: más lento en curvas cerradas
  int vel_dyn = vel - abs(diferencial)/4;
  if (vel_dyn < 20) vel_dyn = 20;
  
  // Aplica corrección a los motores
  if (diferencial < 0)
    motores(vel_dyn, vel_dyn + diferencial);
  else
    motores(vel_dyn - diferencial, vel_dyn);
}

// ==================== CONFIGURACIÓN INICIAL ====================
void setup() {
  bandera_l = 0;  // Inicia calibración de fondo
  
  // Configura pines
  pinMode(s0, OUTPUT); pinMode(s1, OUTPUT);
  pinMode(s2, OUTPUT); pinMode(s3, OUTPUT);
  pinMode(led_on, OUTPUT);
  pinMode(dir_izq, OUTPUT); pinMode(dir_der, OUTPUT);
  pinMode(led, OUTPUT);
  pinMode(btn, INPUT_PULLUP);
  
  // Configura PWM del ESP32-C3
  ledcAttach(pwm_i, PWM_FREQ, PWM_RES);
  ledcAttach(pwm_d, PWM_FREQ, PWM_RES);
  
  analogReadResolution(12);    // ADC de 12 bits (0-4095)
  digitalWrite(led_on, HIGH);  // Enciende LED de sensores
  
  // === CALIBRACIÓN ===
  esperarBoton();  // Espera botón para empezar
  
  // Calibra fondo blanco
  for (int i = 0; i < iter_calibra; i++) {
    digitalWrite(led, (i % 2));  // Parpadea LED
    fondosylinea();               // Lee fondo
    delay(40);
  }
  
  esperarBoton();  // Espera botón para siguiente paso
  
  // Calibra línea negra
  for (int i = 0; i < iter_calibra; i++) {
    bandera_l = 1;                // Cambia a modo línea
    digitalWrite(led, (i % 2));   // Parpadea LED
    fondosylinea();               // Lee línea
    delay(40);
  }
  
  promedio();      // Calcula umbrales
  esperarBoton();  // ¡Listo! Espera botón para iniciar carrera
}

// ==================== PROGRAMA PRINCIPAL ====================
void loop() {
  lectura();      // Lee sensores y calcula posición
  PID();          // Controla motores según PID
  
  // Si presionan el botón durante la carrera
  while(digitalRead(btn) == LOW){
    motores(0,0);      // Detiene robot
    esperarBoton();    // Espera que suelten botón
    ESP.restart();     // Reinicia completamente el ESP32
  }
}
