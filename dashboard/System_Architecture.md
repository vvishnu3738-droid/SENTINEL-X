# SENTINEL-X System Architecture

## System Overview

SENTINEL-X is an AI-powered autonomous industrial safety patrol robot built around the Arduino UNO Q platform. The robot continuously patrols industrial environments, collects environmental data, processes the data using embedded AI, alerts workers in real time, uploads live data to a dashboard, and automatically returns to its charging dock when the battery is low.

---

## Architecture Flow

Environment

↓

Sensors

↓

Arduino UNO Q

↓

Embedded AI Engine

↓

Decision Engine

↓

Motor Control

↓

Robot Movement

↓

Worker Alerts

↓

Wi-Fi Communication

↓

Web Dashboard

↓

Battery Monitor

↓

Auto Dock Charging

↓

Resume Patrol

---

## Sensor Layer

Collects:

- Temperature
- Humidity
- Air Quality
- Gas
- Smoke
- Distance
- Battery Voltage

---

## AI Layer

Responsible for:

- Sensor Fusion
- Environment Classification
- Risk Assessment
- Hazard Detection
- Decision Making

---

## Control Layer

Responsible for:

- Patrol Navigation
- Obstacle Avoidance
- Speed Control
- Dock Navigation

---

## Communication Layer

Uses Wi-Fi for:

- Live Dashboard
- Alert Messages
- Sensor Streaming

---

## Power Layer

Responsible for:

- Battery Monitoring
- Charging Detection
- Automatic Dock Charging
- Resume Patrol