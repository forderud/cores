 /* HID Power Device mock. */
#define HID_PD_PRESENTSTATUS         0x07 // INPUT OR FEATURE(required by Windows)
#define HID_PD_VOLTAGE               0x0B // 11 INPUT (NA) OR FEATURE(implemented)
#define HID_PD_REMAININGCAPACITY     0x0C // 12 INPUT OR FEATURE(required by Windows)
#define HID_PD_FULLCHRGECAPACITY     0x0E // 14 INPUT OR FEATURE. Last Full Charge Capacity 
#define HID_PD_CAPACITYMODE          0x16
#define HID_PD_DESIGNCAPACITY        0x17

// RawHID packets are always 64 bytes
byte buffer[64];
uint16_t packetCount = 0;

void setFeature (byte report, void* ptr, size_t length) {
    buffer[0] = report;

    memcpy(buffer+1, ptr, length);

    // send the packet
    int n = RawHID.send(buffer, length+1);
    if (n > 0) {
      Serial.print(F("Transmit packet "));
      Serial.println(packetCount);
      packetCount = packetCount + 1;
    } else {
      Serial.println(F("Unable to transmit packet"));
    }
}

void setup() {
  Serial.begin(9600);
  Serial.println(F("RawHID Example"));
  for (int i=0; i<7; i++) {
    pinMode(i, OUTPUT);
  }

  byte iPresentStatus = 0;
  setFeature(HID_PD_PRESENTSTATUS, &iPresentStatus, sizeof(iPresentStatus));
  
  byte bCapacityMode = 1;
  setFeature(HID_PD_CAPACITYMODE, &bCapacityMode, sizeof(bCapacityMode));

  uint16_t iVoltage =1499; // centiVolt
  setFeature(HID_PD_VOLTAGE, &iVoltage, sizeof(iVoltage));

  uint32_t iDesignCapacity = 58003*360/iVoltage; // AmpSec=mWh*360/centiVolt (1 mAh = 3.6 As)
  setFeature(HID_PD_DESIGNCAPACITY, &iDesignCapacity, sizeof(iDesignCapacity));

  uint32_t iFullChargeCapacity = 40690*360/iVoltage; // AmpSec=mWh*360/centiVolt (1 mAh = 3.6 As)
  setFeature(HID_PD_FULLCHRGECAPACITY, &iFullChargeCapacity, sizeof(iFullChargeCapacity));

  uint32_t iRemaining = iFullChargeCapacity/2;
  setFeature(HID_PD_REMAININGCAPACITY, &iRemaining, sizeof(iRemaining));
}

void loop() {
}
