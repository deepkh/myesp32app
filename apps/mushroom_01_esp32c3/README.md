# Realy, PIR Motion Sensor and Temperature Sensor detection and control on ESP32-C3


<img width="538" height="338" alt="image" src="https://github.com/user-attachments/assets/a9ca5fca-8ff1-426b-8b69-ae50cea13995" />


<img width="538" height="338" alt="image" src="https://github.com/user-attachments/assets/095159fb-7db1-4217-aa19-a80adfbbc0e7" />


## Scenario
1. Switch 1: `Turn on` the `lamp` while `PIR motion sensor` is detected. `Turn off` the `lamp` when there is no `PIR Motion sensor`  detected during `300 seconds`. 
2. Switch 2: `Turn on` the `fan` while `PIR Motion sensor` is detected. `Turn off` the `fan` when there is no `PIR Motion sensor` detected during `10800 seconds`.

## Hardware & PIN IN / PIN OUT connection
1. ESP32-C3 Super Mini
2. HC-HR501: digital in (GPIO 4)
3. Relay 1: digital out (GPIO 1)
4. Relay 2: digital out (GPIO 2)

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
================ Switch 1 : LAMP =================

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
| Turn ON Lamp        |   | lamp_timer > 300 sec ?       |
| Relay 1 = ON        |   +------------------------------+
+----------------------+        |
                                |
                           +----+------------------+
                           |                       |
                          YES                      NO
                           |                      (SKIP)
                           v                      
                   +---------------------+ 
                   | Turn OFF Lamp       |
                   | Relay 1 = OFF       |
                   +---------------------+


================ Switch 2 : FAN =================

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
| Turn ON Fan          |   | fan_timer > 300 sec ?       |
| Relay 2 = ON         |   +------------------------------+
+----------------------+        |
                                |
                           +----+------------------+
                           |                       |
                          YES                      NO
                           |                      (SKIP)
                           v                      
                   +---------------------+ 
                   | Turn OFF Fan        |
                   | Relay 2 = OFF       |
                   +---------------------+

                     |
                     v
                (repeat loop)
```

