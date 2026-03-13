# Switchs, PIR Motion Sensor, Temperature, Humidity and Air Quality sensors detection on ESP32 8266
## Scenario
1. Switch 1: `Turn on` the `lamp1` while `PIR motion sensor` is detected.`Turn off` the `lamp1` when there is no `PIR Motion sensor`  detected during `60 seconds`. 
2. Switch 2: `Turn on` the `lamp2` while `PIR Motion sensor` is detected.`Turn off` the `lamp2` when there is no `PIR Motion sensor` detected during `30 seconds`.
3. Humidity / Temperature / Air Quality sensors: Detect from sensors, and then publish value to mqtt broker.

## Hardware & PIN IN / PIN OUT connection
1. ESP32 8266 Node Mcu Lua Wifi v3
2. HC-HR501: digital in (GPIO 12)
3. Relay 1: digital out (GPIO 13)
4. Relay 2: digital out (GPIO 15)
5. DHT-22: digital in (GPIO 14)
6. MQ-135: analog in (GPIO 17)

## PIR Motion Detection Control Flow
```bash
+--------------------------------------------------+
|                    LOOP                          |
+--------------------------------------------------+
                     |
                     v
+-----------------------------+
| Read PIR Motion Sensor      |
+-----------------------------+
      | 
      v
================ SWITCH 1 : LAMP 1 =================

      |
      v
+------------------------------+     (skip)
| Motion Detected ?            |
+------------------------------+
      |
   +--+----------------------------+
   |                               |
  YES                              NO
   |                               |
   v                               v
+----------------------+   +-----------------------------+
| Turn ON Lamp 1       |   | lamp1_timer > 60 sec ?       |
| Relay 1 = ON         |   +------------------------------+
+----------------------+        |
                                |
                           +----+------------------+
                           |                       |
                          YES                      NO
                           |                      (SKIP)
                           v                      
                   +---------------------+ 
                   | Turn OFF Lamp 1     |
                   | Relay 1 = OFF       |
                   +---------------------+


================ SWITCH 2 : Lamp 2 =================

      |
      v
+------------------------------+ 
| Motion Detected ?            |
+------------------------------+
      |
   +--+----------------------------+
   |                               |
  YES                              NO
   |                               |
   v                               v
+----------------------+   +-----------------------------+
| Turn ON Lamp 2       |   | lamp2_timer > 300 sec ?     |
| Relay 2 = ON         |   +-----------------------------+
+----------------------+        |
                                |
                           +----+------------------+
                           |                       |
                          YES                      NO
                           |                      (SKIP)
                           v                      
                   +---------------------+ 
                   | Turn OFF Lamp 2     |
                   | Relay 2 = OFF       |
                   +---------------------+

                     |
                     v
                (repeat loop)
```

