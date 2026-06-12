/*!
 * @file Wemos.cpp
 * @brief Implementatie van de Wemos-klasse voor RFID-functionaliteit.
 *
 * Dit bestand bevat de functies voor het initialiseren en uitlezen van de
 * MFRC522 RFID-lezer. De Wemos controleert of een RFID-kaart of tag aanwezig
 * is en berekent op basis van de UID-bytes een chip-ID.
 *
 * Als de berekende chip-ID overeenkomt met de ingestelde waarde, wordt via
 * de WifiServerManager een bericht naar Raspberry Pi-A verstuurd.
 *
 * @author Quinten van Ewijk
 * @date 2026
 */

#include "Wemos.h"
#include "WifiServerManager.h"

/*!
 * @brief Globale variabele voor de berekende RFID-chip-ID.
 *
 * Deze variabele wordt bij elke nieuwe kaartlezing opnieuw opgebouwd uit de
 * UID-bytes van de RFID-kaart of tag.
 */
long chipID = 0;

/*!
 * @brief RFID-lezerobject voor de MFRC522-module.
 *
 * Dit object verzorgt de communicatie met de RFID-lezer via SPI.
 * De pinnen SS_PIN en RST_PIN worden gebruikt voor chip-select en reset.
 */
MFRC522 mfrc522(SS_PIN, RST_PIN);

/*!
 * @brief Object voor WiFi- en TCP-servercommunicatie.
 *
 * Dit object maakt verbinding met het opgegeven WiFi-netwerk en start een
 * TCP-server. Wanneer een geldige RFID-chip wordt gelezen, wordt via dit object
 * een bericht naar Raspberry Pi-A verstuurd.
 */
WifiServerManager wifi(
    "NSELab",
    "NSELabWiFi"
);

/*!
 * @brief Initialiseert de RFID-module en WiFi-server.
 *
 * Deze functie start eerst de SPI-bus en initialiseert daarna de MFRC522
 * RFID-lezer. Vervolgens wordt de versie-informatie van de RFID-chip via de
 * seriële monitor weergegeven.
 *
 * Daarna wordt de WiFi-servermanager gestart, zodat Raspberry Pi-A verbinding
 * kan maken met de Wemos. Als alles klaar is, wordt via de seriële monitor
 * gemeld dat de RFID-chip klaar is om kaarten of tags te ontvangen.
 */
void Wemos::setupRFID() {
  SPI.begin();
  mfrc522.PCD_Init();
  delay(10);
  Serial.println("");
  mfrc522.PCD_DumpVersionToSerial();
  wifi.begin();
  Serial.println("RFID-Chip is klaar om te ontvangen...");
  
}

/*!
 * @brief Controleert en leest RFID-kaarten of tags uit.
 *
 * Deze functie controleert eerst of er een WiFi-client verbonden is. Daarna
 * wordt gekeken of er een nieuwe RFID-kaart aanwezig is. Als er een kaart is,
 * wordt de UID uitgelezen en omgerekend naar één numerieke chip-ID.
 *
 * Wanneer de berekende chip-ID overeenkomt met de ingestelde waarde 189980,
 * wordt het bericht "RFID:1" via WiFi/TCP verstuurd. Dit bericht kan door
 * Raspberry Pi-A gebruikt worden om een vervolgactie in het systeem te starten.
 *
 * Als de chip-ID niet overeenkomt, wordt via de seriële monitor een melding
 * weergegeven. De berekende chip-ID wordt altijd geprint voor debugging.
 */
void Wemos::readRFID() {
  wifi.checkClient();
  if (mfrc522.PICC_IsNewCardPresent()) {
    chipID = 0;
    mfrc522.PICC_ReadCardSerial();

    for (byte i = 0; i < mfrc522.uid.size; i++) {
      chipID = ((chipID + mfrc522.uid.uidByte[i]) * 10);
    }
    if (189980 == chipID){
      wifi.sendMessage("RFID:1");
      Serial.println(true);
    }else{
      Serial.println("false");
    }
    Serial.println(chipID);
    delay(1000)
  }
}