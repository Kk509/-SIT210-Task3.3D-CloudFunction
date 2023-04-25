#include <MQTT.h> 

const int trigPin = D2; // Trigger pin
const int echoPin = D3; // Echo pin
const int ledPin = D4; 

double duration, distance;
const double speed_of_sound = 0.0343; // Speed of sound at 20°C

bool ignoreMessage;

// MQTT client instance
MQTT client("broker.emqx.io", 1883, callback);

// Callback function for MQTT messages.
void callback(char* topic, uint8_t* payload, unsigned int length) 
{
    // The system should not react to messages that the 'wave' is sending.
    if (String(topic) == "SIT210/wave" && String((const char*)payload, length) == "Katrina") 
    {
        ignoreMessage = true; 
        return; // Exit the callback function.
    }
  
    // Write received messages from the EMQX broker to serial monitor.
    Serial.println("Message received:");
    Serial.print("Topic: ");
    Serial.println(topic);
    Serial.print("Payload: ");
    Serial.write(payload, length);
    Serial.println();
  
    // Flash LED 3 times.
    for (int i = 0; i < 3; i++) 
    {
        digitalWrite(ledPin, HIGH);
        delay(100);
        digitalWrite(ledPin, LOW);
        delay(100);
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
        // Publish to "SIT210/wave" 
        client.publish("SIT210/wave","hello world");
    } 
    else 
    {
        Serial.println("Failed to connect to MQTT broker");
    }
}

void loop() 
{
    if (client.isConnected())
    {
        // MQTT client loop for handling incoming messages.
        client.loop();
          
        // If ignore flag is set to true, reset it after a delay.
        if (ignoreMessage)
        {
            delay(5000); 
            ignoreMessage = false; 
        }
    }

    // Measure distance with ultrasonic sensor.
    distance = getDistance();
    
    // If distance is less than 10 cm, publish a message with a name.
    if (distance < 10) 
    {
        client.publish("SIT210/wave", "Katrina");
    }
}

// calculate distance measured by ultrasonic sensor.
double getDistance()
{
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    
    digitalWrite(trigPin, LOW);
    duration = pulseIn(echoPin, HIGH);
    
    distance = (duration*speed_of_sound)/2;
    return (distance);
}
