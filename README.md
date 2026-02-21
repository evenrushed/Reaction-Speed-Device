# Reaction-Speed-Device
A  hardware device built to measure reaction speed based on a diode lighting up. 


# Reaction Speed Controller

A reaction speed testing game built on an ESP32. Hit a button the moment an LED lights up.
Response time gets measured in milliseconds and is shown on an OLED display
Responses are automatically submitted to a shared leaderboard over Wi-Fi.

Built as a workshop project in February 2026.

---

## How It Works

Press START to kick off a round. The screen shows "get ready… wait for LED" while the device picks a random delay somewhere between 750ms and 5 seconds. The moment the LED fires, try to hit the REACT button as fast as possible. Your time shows up on screen and gets posted to the hub leaderboard wirelessly.

If you hit the button before the LED lights up, you get a false start. 

Press START to try again.

### The 5 States

The game runs on a finite state machine with five states:

```
IDLE → WAITING → GO → RESULT
              ↓
          FALSE START
```

- **IDLE** — sitting at the start screen, waiting for someone to press START
- **WAITING** — counting down a random delay in the background while showing "get ready"
- **GO** — LED is on, timer is running, waiting for the react button
- **RESULT** — displays your time, submits the score, then waits to play again
- **FALSE START** — you pressed too early; press START to reset

---

## Leaderboard

Each device connects to an ESP32 acting as a Wi-Fi access point. This is set up in the code, however will need a seperate microcontroller to function. 
After every round it sends an HTTP GET request to the hub with your team name and time:

The hub collects everyone's scores and keeps a running local leaderboard. 

Should the Wi-Fi connection fail on startup, it will just run in offline mode.

---

## Hardware

| Component | Details | Pin |
|---|---|---|
| ESP32 | Main microcontroller | — |
| SSD1306 OLED | 128×64px display over I²C | SDA / SCL (0x3C) |
| LED | Reaction signal light | GPIO 26 |
| START button | Tactile, pulled up | GPIO 18 |
| REACT button | Tactile, pulled up | GPIO 14 |

Both buttons use `INPUT_PULLUP`, so pressed = LOW.

---

## Setup

**Libraries needed** (install via Arduino Library Manager):
- Adafruit SSD1306
- Adafruit GFX

The ESP32 board package from Espressif covers the rest (`WiFi.h`, `HTTPClient.h`, `Wire.h`).

---

## Notes

- Timing uses `micros()` for precision, then converts to milliseconds for display
- The random delay range (750–5000ms) is intentionally wide to prevent anticipation
- The debounce on both buttons waits for full release before registering a press, so holding the button down won't spam inputs
- If the OLED doesn't initialize, the device halts with an error on Serial — usually an I²C wiring issue
