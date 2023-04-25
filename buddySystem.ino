#include <MQTT.h>

const int trigPin = D2; // Trigger pin
const int echoPin = D3; // Echo pin
const int ledPin = D4;

double duration, distance;
const double speed_of_sound = 0.0343; // Speed of sound at 20°C

unsigned long previousMillis = 0; // Variable to store the previous millis value
unsigned long interval = 5000; // Interval in milliseconds for publishing messages

bool ignoreMessage;
int ledFlashCount = 0; // Counter for LED flash count

// MQTT client instance
MQTT client("broker.emqx.io", 1883, callback);

// Callback function for MQTT messages.
void callback(char *topic, uint8_t *payload, unsigned int length)
{
    // Check if the received topic is the same as the topic being sent
    if (String(topic) == "SIT210/wave" && String((const char*)payload, length) != "Name") 
    {
        // Write received messages from the EMQX broker to serial monitor.
        Serial.println("Message received:");
        Serial.print("Topic: ");
        Serial.println(topic);
        Serial.print("Payload: ");
        Serial.write(payload, length);
        Serial.println();
        
        // Flash the LED 3 times
        for (int i = 0; i < 3; i++)
        {
            digitalWrite(ledPin, HIGH);
            delay(500);
            digitalWrite(ledPin, LOW);
            delay(500);
        }
    }
}

void setup()
{
    Serial.begin(9600);
    
    // Initialize pins
    pinMode(trigPin, OUTPUT);
    pinMode(echoPin, INPUT);
    pinMode(ledPin, OUTPUT);
    
    // Connect to MQTT broker and provide a unique client ID.
    client.connect("mqttx_f13c10c0" + String(Time.now()));
    
    if (client.isConnected())
    {
        Serial.println("Connected to MQTT broker");
        // Subscribe to "SIT210/wave"
        client.subscribe("SIT210/wave");
    }
    else
    {
        Serial.println("Failed to connect to MQTT broker");
    }
    
    ignoreMessage = false;
}

void loop()
{
    if (client.isConnected())
    {
        // MQTT client loop for handling incoming messages.
        client.loop();
    }
    
    // Measure distance with ultrasonic sensor.
    distance = getDistance();
    
    // If distance is less than 10 cm, publish a message with "Katrina" as payload.
    if (distance < 10 && !ignoreMessage)
    {
        // Publish to "SIT210/wave" with "Name" as payload.
        client.publish("SIT210/wave", "Katrina", 0);
        ignoreMessage = true;        // Set ignoreMessage flag to true
        previousMillis = millis();   // Store the current millis value
    }
    
    // Check if it's time to reset the ignoreMessage flag
    if (ignoreMessage && (millis() - previousMillis >= interval))
    {
        ignoreMessage = false; // Reset ignoreMessage flag to false
    }
}

// Calculate distance measured by ultrasonic sensor.
double getDistance()
{
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    
    digitalWrite(trigPin, LOW);
    duration = pulseIn(echoPin, HIGH);
    
    distance = (duration * speed_of_sound) / 2;
    return (distance);
}
