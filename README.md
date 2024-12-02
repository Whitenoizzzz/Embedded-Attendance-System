# Attendance System 

This project implements a real-time clock system using an AVR microcontroller. It integrates multiple hardware modules including:

- **RTC (DS1307)** for precise timekeeping using the I2C protocol.
- **LCD Display (HD44780)** to display current time and attendance status.
- **Buttons** for user interaction to mark arrivals and departures.
- **EEPROM** for persistent data storage of timestamps.

### Features
- Displays real-time updates on an LCD.
- Tracks attendance with "Arrive" and "Leave" functionality.
- Stores attendance records in EEPROM for future retrieval.
- Offers reset functionality to clear memory and reset the clock.
