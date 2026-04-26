// ===== PRUEBA DE BARRA DE SENSORES - VERSIÓN SIMPLE =====
// Solo para verificar que los 16 sensores funcionan y detectan línea
// Pines del multiplexor (igual que en tu código del robot)
#define MUX_COM  1
#define S0       0
#define S1       2
#define S2       3
#define S3       4

#define UMBRAL   2000  // Ajusta este valor según tu superficie
                       // Mayor a 2000 = blanco, menor = negro

int sensores[16];

void setup() {
  Serial.begin(115200);
  delay(100);
  
  // Configurar pines de selección del multiplexor
  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);
  
  // Configurar pin del ADC (COM)
  pinMode(MUX_COM, INPUT);
  
  Serial.println("=== PRUEBA DE SENSORES ===");
  Serial.println("Ajusta el umbral si es necesario");
  Serial.print("Umbral actual: ");
  Serial.println(UMBRAL);
  Serial.println("Formato: S0 S1 S2 ... S15 | Deteccion");
  Serial.println("----------------------------------------");
}

void loop() {
  // Leer los 16 sensores
  for(int i = 0; i < 16; i++) {
    // Seleccionar canal del multiplexor
    digitalWrite(S0, i & 0x01);	//0001
    digitalWrite(S1, i & 0x02); //0010
    digitalWrite(S2, i & 0x04);	//0100
    digitalWrite(S3, i & 0x08); //1000
    
    delayMicroseconds(50);  // Pequeña pausa para estabilizar
    
    // Leer valor analógico
    sensores[i] = analogRead(MUX_COM);
  }
  
  // Mostrar valores de todos los sensores
  for(int i = 0; i < 16; i++) {
    Serial.print(sensores[i]);
    Serial.print("\t");
  }
  
  // Detectar si hay línea negra (debajo del umbral)
  bool linea_detectada = false;
  for(int i = 0; i < 16; i++) {
    if(sensores[i] < UMBRAL) {
      linea_detectada = true;
      break;
    }
  }
  
  // Mostrar estado de la línea
  if(linea_detectada) {
    Serial.print("| LINEA DETECTADA");
  } else {
    Serial.print("| SIN LINEA");
  }
  
  Serial.println();
  
  // Pequeña pausa para no saturar el monitor serial
  delay(100);
}
