# PES_RESTAURANT

Dit project bevat het volledige embedded systems restaurantplatform.

Het systeem bestaat uit meerdere componenten:

- Raspberry Pi-A (centrale communicatie en dashboard)
- Raspberry Pi-B (backend communicatie en routing)
- STM32 modules (sensoren, encoders, servos en LED-matrix)
- Wemos D1 Mini modules (klantinteractie via WiFi)

Alle componenten communiceren via:
- TCP sockets (Raspberry Pi communicatie)
- CANBUS (STM32 onderlinge communicatie)
- WiFi sockets (Wemos modules)

## Documentatie

De volledige technische documentatie is beschikbaar via Doxygen:

👉 https://pes-hhs-2526.github.io/PES_RESTAURANT/ 🔥🔥🔥🔥🔥🔥🔥🔥🔥 😎👌

## Modules

- STM32_ENCODER → sensoren, CO2, LED-matrix en CAN communicatie  
- STM32_GIMBAL_BENEDEN → automatische deur + RFID servo besturing  
- STM32_GIMBAL_BOVEN → klantinteractie en tafelbesturing  
- RASPBERRY-PI-A → dashboard + Wemos communicatie  
- RASPBERRY-PI-B → centrale routering en STM32 communicatie  
- WEMOS systemen → klantinterface (lichtkrant, RFID, tafels)

## Auteur

Haki Abdulovski & Arbër Deda & Quinten van Ewijk
