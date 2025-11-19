# **Power Guardian – STM32F103 Power Management System**

A dual-segment power monitoring and protection system designed to manage overloads, send alerts, and provide emergency override functionality.

---

## **Overview**

This project implements a **power distribution controller** using an **STM32F103** microcontroller.
The system monitors two independent power segments, detects overloads, sends alerts, and performs timed shutdowns when necessary.
An emergency override allows temporary uninterrupted power during critical situations.

The firmware uses:

* GPIO
* External interrupts (EXTI)
* Timers (TIM2, TIM3)
* SysTick
* Simple control logic without HAL/LL libraries

---

## **Summary Logic**

Overload Happens → Alert ON → Start Timer → If overload stays until timer ends → Power OFF.
If load returns to normal → Timer resets + Power stays ON.
Emergency Button → Overrides everything → Forces both power ON for fixed period.

---

## **Key Features**

### **1. Overload Monitoring**

* Each power segment has its own overload sensor:

  * Segment 1 → PA0
  * Segment 2 → PA1
* On overload:

  * Alert signal turns ON
  * A timer starts counting
* If overload continues after the timer expires:

  * Power to that segment is switched OFF
* If load returns to normal:

  * Timer resets
  * Alerts turn OFF
  * Power stays ON

---

### **2. Emergency Override**

* A hardware emergency button on PB12 activates emergency mode.
* In emergency mode:

  * Power to both segments is forced ON
  * Overload rules are overridden temporarily
  * Emergency indicator LED (PA8) turns ON
  * System remains in override for a fixed duration
    (currently 5 seconds, configurable to longer periods)

---

### **3. Independent Control Logic**

Each segment includes:

* Independent overload interrupt
* Independent timer
* Independent alert and power channels

Emergency mode currently applies to both segments globally.

---

## **Hardware Pin Mapping**

| Feature             | Pin  | Direction      | Description            |
| ------------------- | ---- | -------------- | ---------------------- |
| Segment 1 Power     | PB0  | Output         | Power ON/OFF control   |
| Segment 1 Message   | PB1  | Output         | Alert signal           |
| Segment 2 Power     | PA9  | Output         | Power ON/OFF control   |
| Segment 2 Message   | PA10 | Output         | Alert signal           |
| Emergency Indicator | PA8  | Output         | Shows emergency active |
| Segment 1 Overload  | PA0  | Input (EXTI0)  | Rising-edge interrupt  |
| Segment 2 Overload  | PA1  | Input (EXTI1)  | Rising-edge interrupt  |
| Emergency Button    | PB12 | Input (EXTI12) | Rising-edge interrupt  |

---


## **System Logic**

### **1. Normal Operation**

1. If no overload →

   * Power ON
   * Alerts OFF

2. When overload occurs →

   * Interrupt triggers
   * Timer starts
   * Alert turns ON

3. If timer completes and overload is still present →

   * Segment power is turned OFF

4. If overload stops before timer expires →

   * Timer resets
   * Alert turns OFF

---

### **2. Emergency Mode**

* Activated by pressing PB12
* Toggles a flag `Emr_En`
* While emergency mode is active:

  * Power to both segments stays ON
  * Emergency LED stays ON
  * Overload protection is temporarily bypassed
  * After the emergency window ends, system returns to normal logic

---

## **Timer Configuration**

* TIM2 → Segment 1 overload timeout
* TIM3 → Segment 2 overload timeout
* Prescaler: 36000
* Auto-reload: 8000
* Total delay ≈ 8 seconds

Can be modified for longer shutdown thresholds (e.g., 120 seconds).

---

## **How to Build and Flash**

1. Open project in **Keil uVision** 
2. Configure target as **STM32F103C8** (or compatible).
3. Compile to generate `.hex` or `.bin`.
4. Flash using:
   * ST-Link V2
   * STM32CubeProgrammer
   * OpenOCD

---

## **License**

MIT License — Open to modify and use.

---




