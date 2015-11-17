#include <math.h>

// Define which pin we're using to accept the signal
#define RCPIN 1
 
// We have to declare as volatile as this will be modified
// by the interrupt routine, and read outside of it
volatile uint16_t channel;
uint16_t channel_start;
uint16_t r;
 
void setup() {
    Serial.begin(115200);
    Serial.println("Starting...");
 
    // Mark the pin as an INPUT
    pinMode(RCPIN, INPUT);
 
    // Attach an interrupt handler to be called whenever
    // the pin changes from LOW to HIGH or vice versa
    attachInterrupt(RCPIN, RCchannel, CHANGE);
}
 
void loop() {
    // Output the channel value
    char buffer[7]; 
    sprintf(buffer, "%d\n", channel);
    Serial.print(buffer);   
    delay(2);
}
 
void RCchannel() {
    // If the pin is HIGH, start a timer
    if (digitalRead(RCPIN) == HIGH) {
        channel_start = micros();
    } else {
        // The pin is now LOW so output the difference
        // between when the timer was started and now
        channel = (uint16_t) (micros() - channel_start);
    }
}
