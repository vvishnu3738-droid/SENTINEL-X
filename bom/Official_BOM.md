# SENTINEL-X Official Bill of Materials (BOM)

## Main Controller

| Component                | Qty | Status               | 
|------------------------- |-----|----------------------|
| Arduino UNO Q            | 1   | Mandatory            | 


---

## Sensors

| Component                 | Qty | Status               | Purpose                              |
|---------------------------|-----|----------------------|--------------------------------------|
| BME688                    | 1   | Recommended          | Temperature, Humidity, Pressure, Gas |
| HC-SR04 Ultrasonic Sensor | 2   | Required             | Obstacle Detection                   |
| MPU6050                   | 1   | Required             | Tilt & Motion Detection              |
| INA219 Current Sensor     | 1   | Required             | Battery Monitoring                   |
---

## Motion System

| Component                          | Qty | Status     | Purpose                               |
|------------------------------------|-----|------------|---------------------------------------|
| 12V DC Geared Motor (200–300 RPM)  | 4   | Required   | Robot Movement                        |
| 65mm Rubber Wheel                  | 4   | Required   | Traction                              |
| TB6612FNG Motor Driver             | 2   | Required   | Motor Control                         |
| 4WD Chassis Base                   | 1   | Required   | Robot Base                            |

---

## Power System

| Component                            | Qty    | Status               | Purpose                  |
|--------------------------------------|--------|----------------------|--------------------------|  
| 3S 18650 Li-ion Battery Pack (11.1V) | 1      | Required             | Main Power Source        |
| 3S BMS Protection Board              | 1      | Required             | Battery Protection       |
| XT60 Connector                       | 1 Pair | Required             | Main Power Connector     |
| Rocker ON/OFF Switch                 | 1      | Required             | Main Power Switch        |
| 12V to 5V Buck Converter (LM2596)    | 1      | Required             | Supply 5V to Electronics |
| Battery Charger (12.6V, 2A)          | 1      | Required             | Battery Charging         |

---

## Charging Dock

| Component                     | Qty    | Status        | Purpose                      |
|-------------------------------|--------|---------------|------------------------------|
| 2-Pin Magnetic Pogo Connector | 1 Pair | Required      | Automatic Charging Contact   |
| Charging Dock Base            | 1      | Required      | Dock Structure               |
(3D Printed / MDF / Acrylic)    |        |               |                              |
| Dock Guide Rails              | 2      | Required      | Robot Alignment              |
| Charging Status LED           | 1      | Optional      | Charging Indicator           |
---

## Charging Dock

| Component                     | Qty    | Status   | Purpose                    |
|-------------------------------|--------|----------|----------------------------|
| 2-Pin Magnetic Pogo Connector | 1 Pair | Required | Automatic Charging Contact |
| Dock Guide Rails              | 2      | Required | Align Robot During Docking |
| Charging Dock Base            | 1      | Required | Dock Structure             |
| Dock Status LED               | 1      | Optional | Charging Status            |
---

## Miscellaneous 

| Component                          | Qty     | Status            | Purpose          |
|------------------------------------|---------|-------------------|------------------|
| Jumper Wires                       | 1 Set   | Required          | Sensor           |
|                                    |         |                   | Connections      | 
| Screw & Nut Kit (M3)               | 1 Box   | Required          | Robot Assembly   | 
| Brass Spacers (M3)                 | 10      | Required          | PCB              |
|                                    |         |                   | Mounting         | 
| Cable Ties                         | 20      | Required          | Cable Management | 
| Heat Shrink Tubes                  | 1 Pack  | Required          | Wire Insulation  |
| Terminal Block Connectors          | 5       | Recommended       |Power Distribution|
| JST-XH Connector Set               | 1 Set   | Recommended       | Battery/Sensor   |                 
|                                    |         |                   | Connections      | 
| PCB Standoffs                      | 10      | Required          | Mount Arduino    |                 
|                                    |         |                   |& Modules         |
| Double-Sided Foam Tape             | 1 Roll  | Required          | Mount Small      |                 
|                                    |         |                   | Modules          |  
| Spiral Cable Sleeve                | 1       | Optional          | Neat Wire        |
|                                    |         |                   |Management        | 
