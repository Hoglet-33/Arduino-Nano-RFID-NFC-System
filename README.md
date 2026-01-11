# Arduino-Nano-RFID-NFC-System

A simple NFC-based access control system using PN532, Arduino Nano, LEDs, buzzer, and EEPROM.

---

## Materials Needed

- Arduino Nano (or compatible)
- PN532 NFC module (I2C mode)
- NFC tags/cards (MIFARE)
- 2 LEDs (Green and Red)
- Passive buzzer
- Push button (for programming mode)
- Resistors (220Ω for LEDs)
- Breadboard and jumper wires
- USB cable for programming Arduino

---

## Wiring

| Component      | Arduino Pin  |
|----------------|--------------|
| PN532 GND      | GND          |
| PN532 VCC      | 5V (3.3V if needed) |
| PN532 SDA      | A4           |
| PN532 SCL      | A5           |
| Green LED (+)  | D6 (with 220Ω resistor to GND) |
| Red LED (+)    | D7 (with 220Ω resistor to GND) |
| Buzzer (+)     | D9           |
| Program Button | D4 (with pull-up resistor or use internal pull-up) |
| Onboard LED    | D13          |

---

## Setup Instructions

1. Connect all components as shown in the wiring table.
2. Install Arduino IDE and add the `Adafruit_PN532` library.
3. Upload the `NFC-System.ino` sketch to the Arduino Nano.
4. Open the Serial Monitor at 115200 baud rate.
5. Use the program button + tag to add or delete authorized NFC tags.
6. Scan authorized tags to unlock (green LED + happy beep).
7. Scan unauthorized tags to test denial and alarm.

---

## Usage

- **Add/Delete tag**: Hold program button and scan a tag.
- **Scan tag**: If authorized, green LED and success beep; if not, red LED and denial beep.
- **Alarm**: After 3 consecutive wrong scans of the same tag, alarm triggers.

---

## Troubleshooting

- Make sure wiring matches the pin definitions.
- Ensure the PN532 is in I2C mode (switches on the module).
- If PN532 is not detected, check connections and power.
- Use Serial Monitor messages for debugging.

---

## License

MIT License

---

## Author

Hoglet-33
