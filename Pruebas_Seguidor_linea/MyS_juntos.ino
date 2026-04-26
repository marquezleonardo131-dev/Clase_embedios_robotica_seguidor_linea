//PROGRAMA DE PRUEBA DE MOTORES Y BARRA DE SENSORES JUNTOS CON RTOS

// ===== Pines Motores ESP32-C3 =====
const int dir_izq = 20;
const int pwm_i   = 21;
const int dir_der = 6;
const int pwm_d   = 5;
const int led     = 8;

// ===== Pines Sensores =====
const int com = 1; // Pin analógico
const int s0  = 0;
const int s1  = 2;
const int s2  = 3;
const int s3  = 4;

// Variables Globales
volatile int sensores[16]; // volatile porque se modifica en otra tarea
int umbral = 1500;        // Ajustado para el rango de 0-4095 de la ESP32

// ===== Función para mover motores ====
void moverMotores(int izq, int der) {
  digitalWrite(dir_izq, (izq >= 0) ? HIGH : LOW);
  digitalWrite(dir_der, (der >= 0) ? HIGH : LOW);

  // Usamos la nueva API ledcWrite de la versión 3.0+ de ESP32 para Arduino
  ledcWrite(pwm_i, abs(constrain(izq, -255, 255)));
  ledcWrite(pwm_d, abs(constrain(der, -255, 255)));
}

// ===== Tarea de Motores (Lógica de seguimiento) =====
void taskMotores(void *pv) {
  
  while (1) {
    bool centro = false;
    bool izquierda = false;
    bool derecha = false;

    // Evaluamos los sensores centrales (7 y 8)
    if (sensores[7] > umbral || sensores[8] > umbral) centro = true;

    // Sensores del lado izquierdo (0 al 6)
    for (int i = 0; i <= 6; i++) {
      if (sensores[i] > umbral) izquierda = true;
    }

    // Sensores del lado derecho (9 al 15)
    for (int i = 9; i <= 15; i++) {
      if (sensores[i] > umbral) derecha = true;
    }

    // Lógica simple de decisiones
    if (centro) {
      moverMotores(100, 100); // Adelante
    } else if (izquierda) {
      moverMotores(-50, 50); // Giro fuerte izquierda
    } else if (derecha) {
      moverMotores(50, -50); // Giro fuerte derecha
    } else {
      moverMotores(0, 0);     // Paro
    }

    vTaskDelay(pdMS_TO_TICKS(10)); // Ciclo de control rápido
  }
}

// ===== Tarea de Lectura de Sensores =====
void taskSensores(void *pv) {
  while (1) {
    for (int i = 0; i < 16; i++) {
      // Manejo de bits para el multiplexor
      digitalWrite(s0, (i >> 0) & 0x01);
      digitalWrite(s1, (i >> 1) & 0x01);
      digitalWrite(s2, (i >> 2) & 0x01);
      digitalWrite(s3, (i >> 3) & 0x01);

      delayMicroseconds(20); // Tiempo de estabilización mucho más rápido
      sensores[i] = analogRead(com);
    }
    
    //Debug opcional (Comenta esto en competencia para ganar velocidad)
    
    for(int i = 0; i < 16; i++) {
      Serial.print(sensores[i]); Serial.print(" ");
    }
    Serial.println();
    
    
    vTaskDelay(pdMS_TO_TICKS(5)); // Frecuencia de muestreo alta
  }
}

void setup() {
  Serial.begin(115200);
  
  pinMode(dir_izq, OUTPUT);
  pinMode(dir_der, OUTPUT);
  pinMode(s0, OUTPUT);
  pinMode(s1, OUTPUT);
  pinMode(s2, OUTPUT);
  pinMode(s3, OUTPUT);
  pinMode(led, OUTPUT);

  // Configuración de PWM para ESP32 Arduino Core 3.0+
  ledcAttach(pwm_i, 5000, 8); // 5 kHz, 8 bits de resolución
  ledcAttach(pwm_d, 5000, 8);
  ledcAttach(led, 1000, 8);

  // Creación de tareas
  xTaskCreate(taskSensores, "Sensores", 4096, NULL, 3, NULL);
  xTaskCreate(taskMotores,  "Motores",  4096, NULL, 2, NULL);
}

void loop() {
  // El loop se queda vacío, pero si quieres hacer algo... ¡puedes!
  // digitalWrite(led, HIGH);
  vTaskDelay(pdMS_TO_TICKS(500));
}
