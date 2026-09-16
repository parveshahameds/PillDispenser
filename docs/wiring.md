# PillDispenser Wiring Guide

## Connections

| Component | Pin / connection |
|---|---|
| Servo signal | ESP32 GPIO 18 |
| Servo VCC | External appropriate 5V supply |
| Servo GND | Common GND |
| Buzzer signal | ESP32 GPIO 19 |
| Buzzer GND | ESP32 GND |
| ESP32 | USB power |

## Simplified wiring

```text
                 ESP32
            ┌─────────────┐
            │             │
 GPIO 18 ───┼─────────────┼── Servo SIGNAL
 GPIO 19 ───┼─────────────┼── Buzzer SIGNAL
 GND ───────┼─────────────┼──┬── Servo GND
            │             │  └── Buzzer GND
            └─────────────┘
                         
 External 5V ─────────────── Servo VCC
 External GND ───────────── Common GND
```

## Servo power

Do not use the ESP32 3.3V output to power a typical servo. The servo should have an appropriate external supply. The external supply ground and ESP32 ground must be connected together so the signal has a common reference.

## Current mechanical model

The servo acts as the selector/actuator for three positions:

```text
             [Medicine mechanism]
                      │
        ┌─────────────┼─────────────┐
        ▼             ▼             ▼
     60°             120°          180°
   Morning           Noon          Night
```

The firmware returns the servo to `0°` after the two-second dispensing movement.

## Important prototype limitation

There is currently no IR/optical sensor. The system can confirm that the servo command was executed and publish a `dispensed` event, but it cannot independently verify that a pill was removed or consumed.
