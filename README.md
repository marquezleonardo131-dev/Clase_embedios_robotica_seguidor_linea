## Clase_embedios_robotica_seguidor_linea
#  Robot Seguidor de Línea - 16 Sensores

![Versión](https://img.shields.io/badge/versión-1.0-blue)
![ESP32](https://img.shields.io/badge/ESP32--C3-SuperMini-green)
![Licencia](https://img.shields.io/badge/licencia-MIT-yellow)

Robot seguidor de línea de alta resolución diseñado para competencias de robótica (Copa RoboTec 2026). Utiliza 16 sensores ópticos multiplexados por corrimiento de bits, control diferencial con motores N20 y arquitectura modular para mantenimiento rápido.

---

##  Tabla de Contenidos

- [Características](#-características)
- [Especificaciones Técnicas](#-especificaciones-técnicas)
- [Esquemático y PCB](#-esquemático-y-pcb)
- [Instalación y Configuración](#-instalación-y-configuración)
- [Estructura del Código](#-estructura-del-código)
- [Filosofía de Diseño](#-filosofía-de-diseño)
- [Próximas Mejoras](#-próximas-mejoras)
- [Autores](#-autores)
- [Licencia](#-licencia)

---

##  Características

| Característica | Descripción |
|----------------|-------------|
| **Sensores** | 16 sensores ópticos con LEDs integrados |
| **Lectura** | Multiplexado por shift registers 74HC165 (solo 3 pines GPIO) |
| **Control** | PID reactivo con ajuste por Bluetooth |
| **Motores** | 2x N20 (3000 RPM) + rueda loca trasera |
| **Driver** | IFX9201 (6A pico, protección sobrecorriente) |
| **Microcontrolador** | ESP32-C3 SuperMini (RISC-V, 160 MHz, WiFi/BLE) |
| **Alimentación** | LiPo 2S 7.4V, 350 mAh, 75C |
| **Peso** | ~1.6 kg |
| **Dimensiones** | [XX] x [XX] x [XX] cm |

---

##  Especificaciones Técnicas

### Hardware

| Componente | Modelo | Especificación |
|------------|--------|----------------|
| MCU | ESP32-C3 SuperMini | 160 MHz, 400 KB SRAM, WiFi 4 |
| Driver motores | IFX9201 | 6A pico, 36V max, protección térmica |
| Shift registers | 2x 74HC165 | Entrada paralelo, salida serie |
| Regulador | Mini-360 (MP1584EN) | Buck síncrono, 7.4V → 3.3V |
| Motores | N20 | 3000 RPM, 6V nominal |
| Batería | LiPo 2S | 350 mAh, 75C (pico 26.25A) |
| Ruedas | Rodillos de impresora | Poliuretano + rodamiento |

### Sensores

- **Tipo:** Óptico reflectivo (LED infrarrojo + fototransistor)
- **Cantidad:** 16
- **Resolución espacial:** ~1.5 mm
- **Rango de detección:** 3-5 mm del suelo
- **Calibración:** Fabricados en cuarto oscuro (alta sensibilidad en túneles)

---

## Esquemático y PCB

- **Diseño:** EasyEDA
- **Fabricación:** JLC PCB
- **Chasis:** FR4 de 2 mm de espesor
- **Componentes:** 0402 y 0603 (soldados por reflow)

### Componentes Soldados vs Modulares

| Tipo | Componentes | Razón |
|------|-------------|-------|
| **Soldados** | Condensadores, resistencias, shift registers, driver, regulador | No fallan con uso normal |
| **Modulares (no soldados)** | Cables de motores, ESP32-C3, módulo de inicio | Cambio rápido y seguridad térmica |

### Conectores

- **Motores:** Conectores externos con lengüeta de seguro
- **Barra de sensores:** Jumper macho-hembra + cable FPC reforzado con cinta kapton
- **ESP32 y módulo inicio:** Zócalos de pines hembra

---

## Instalación y Configuración

### Requisitos

- [PlatformIO](https://platformio.org/) o [Arduino IDE](https://www.arduino.cc/)
- Biblioteca para ESP32-C3 (instalada vía Boards Manager)

```bash
git clone https://github.com/tuusuario/robot-seguidor-linea-16-sensores.git
cd robot-seguidor-linea-16-sensores
