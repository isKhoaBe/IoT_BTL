#include "temp_humi_monitor.h"

/**
 * Task 3: Temperature and Humidity Monitoring with LCD Display
 * 
 * This task reads sensor data and updates the shared data structure.
 * It works in conjunction with lcd_display_task which displays the data
 * with three different states:
 * 1. NORMAL: All readings within comfortable range
 * 2. WARNING: Some readings approaching critical levels
 * 3. CRITICAL: Readings at dangerous levels
 * 
 * All data sharing is done through semaphore-protected functions.
 */

void temp_humi_monitor(void *pvParameters){
    // Wait for shared data structures to be ready
    vTaskDelay(pdMS_TO_TICKS(500));
    
    Serial.println("[SENSOR] Task starting - Initializing DHT20...");
    
    DHT20 dht20;
    
    // Initialize I2C and sensor
    Wire.begin(11, 12);
    vTaskDelay(pdMS_TO_TICKS(100));  // Allow I2C to stabilize
    
    dht20.begin();
    vTaskDelay(pdMS_TO_TICKS(500));  // Allow sensor to stabilize
    
    Serial.println("[SENSOR] DHT20 initialized successfully");
    
    // Local variables for readings
    float temperature = 0.0;
    float humidity = 0.0;
    
    while (1){
        // Read sensor data
        dht20.read();
        temperature = dht20.getTemperature();
        humidity = dht20.getHumidity();
        
        // Check if any reads failed
        if (isnan(temperature) || isnan(humidity)) {
            Serial.println("[SENSOR] ERROR: Failed to read from DHT sensor!");
            temperature = 25.0;  // Default safe values
            humidity = 50.0;
        }
        
        // Update shared data using semaphore-protected function
        setSensorData(temperature, humidity);
        
        // Print the results to serial monitor
        Serial.print("[SENSOR] Temp: ");
        Serial.print(temperature);
        Serial.print("C, Humidity: ");
        Serial.print(humidity);
        Serial.println("%");
        
        // Read sensor every 5 seconds
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

/**
 * LCD Display Task
 * 
 * This task displays sensor data on the LCD with different display states:
 * - NORMAL: Standard display with readings
 * - WARNING: Blinking display to indicate caution
 * - CRITICAL: Rapid blinking with alert message
 */

void lcd_display_task(void *pvParameters){
    // Wait for system to stabilize before initializing LCD
    vTaskDelay(pdMS_TO_TICKS(2000));
    
    Serial.println("[LCD] Task starting - Initializing display...");
    
    // Initialize LCD
    LiquidCrystal_I2C lcd(LCD_ADDRESS, LCD_COLUMNS, LCD_ROWS);
    lcd.begin();  // Use begin() instead of init()
    lcd.backlight();
    lcd.clear();
    
    lcd.setCursor(0, 0);
    lcd.print("System Ready");
    vTaskDelay(pdMS_TO_TICKS(1000));
    lcd.clear();
    
    Serial.println("[LCD] Display initialized successfully");
    
    // Local variables for sensor data
    float temperature = 0.0;
    float humidity = 0.0;
    DisplayState_t currentState = DISPLAY_NORMAL;
    DisplayState_t previousState = DISPLAY_NORMAL;
    
    while(1) {
        // Read sensor data using semaphore-protected function
        getSensorData(&temperature, &humidity);
        
        // Determine display state based on sensor readings
        bool isCritical = (temperature < TEMP_CRITICAL_LOW || 
                          temperature > TEMP_CRITICAL_HIGH ||
                          humidity < HUMIDITY_CRITICAL_LOW || 
                          humidity > HUMIDITY_CRITICAL_HIGH);
                          
        bool isWarning = (!isCritical) && 
                        (temperature < TEMP_WARNING_LOW || 
                         temperature > TEMP_WARNING_HIGH ||
                         humidity < HUMIDITY_WARNING_LOW || 
                         humidity > HUMIDITY_WARNING_HIGH);
        
        // Set display state
        if (isCritical) {
            currentState = DISPLAY_CRITICAL;
        } else if (isWarning) {
            currentState = DISPLAY_WARNING;
        } else {
            currentState = DISPLAY_NORMAL;
        }
        
        // Log state changes
        if (currentState != previousState) {
            Serial.print("[LCD] State: ");
            if (currentState == DISPLAY_NORMAL) Serial.println("NORMAL");
            else if (currentState == DISPLAY_WARNING) Serial.println("WARNING");
            else Serial.println("CRITICAL");
            previousState = currentState;
        }
        
        // Update display based on state (NO BLINKING - LCD always on)
        lcd.backlight();  // Keep backlight always on
        lcd.clear();
        
        switch(currentState) {
            case DISPLAY_NORMAL:
                // Normal display - standard readings
                lcd.setCursor(0, 0);
                lcd.print("Temp: ");
                lcd.print(temperature, 1);
                lcd.print("C");
                lcd.setCursor(0, 1);
                lcd.print("Humi: ");
                lcd.print(humidity, 1);
                lcd.print("%");
                break;
                
            case DISPLAY_WARNING:
                // Warning state - display with warning indicator
                lcd.setCursor(0, 0);
                lcd.print("!WARNING!");
                lcd.setCursor(0, 1);
                lcd.print("T:");
                lcd.print(temperature, 1);
                lcd.print(" H:");
                lcd.print(humidity, 1);
                break;
                
            case DISPLAY_CRITICAL:
                // Critical state - display with critical indicator
                lcd.setCursor(0, 0);
                lcd.print("!!!CRITICAL!!!");
                lcd.setCursor(0, 1);
                    lcd.print("T: ");
                    lcd.print(temperature, 1);
                    lcd.print("H: ");
                    lcd.print(humidity, 1);
                break;
        }
        
        // Update display every 1 second
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}