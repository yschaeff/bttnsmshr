#include <Arduino.h>
#include "scheduler.h"
#include "tasks.h"
#include "Adafruit_NeoPixel.h"

#define HEARTBEAT_PERIOD 3000
#define HEARTBEAT_DELAY 40
#define HEARTBEAT_SPEED 5
#define HEARTBEAT_MAX 255.0
#define HEARTBEAT_OFFSET 1

/*
  Time of pulse: (max-offset)/speed * delay
  time off: period - 'time of pulse'
*/

void heartbeat(int pin, int level, int arg3)
{
    (void) arg3;
    float in = (level/255.0) * (2*PI);
    int out = sin(in-PI/2) * HEARTBEAT_MAX/2 + HEARTBEAT_MAX/2 + HEARTBEAT_OFFSET;
    analogWrite(pin, out);
    level += HEARTBEAT_SPEED;
    if (level+HEARTBEAT_SPEED > 255) {
        analogWrite(pin, HEARTBEAT_OFFSET);
        (void) schedule_insert(heartbeat, pin, 0, 0, millis()+HEARTBEAT_PERIOD);
    } else {
        (void) schedule_insert(heartbeat, pin, level, 0, millis()+HEARTBEAT_DELAY);
    }
}

void pulse_end(int pin, int level, int arg3)
{
    (void) arg3;
    digitalWrite(pin, level);
}
void pulse(int pin, int level, int duration)
{
    digitalWrite(pin, level);
    (void) schedule_insert(pulse_end, pin, !level, 0, millis()+duration);
}

void blink(int pin, int level, int duration)
{
    digitalWrite(pin, level);
    (void) schedule_insert(blink, pin, !level, duration, millis()+duration);
}

extern Adafruit_NeoPixel strip;
void
kit(int period, int startled, int nleds)
{
    float pos = cos((millis()*2*PI)/(float)period)/2+.5;
    float sigma = 4.0/nleds;
        Serial.println(pos);
    float e = 2.71828;
    for(int i=0; i<nleds; i++) {
        float br = 255 * pow(e, -.5*pow((i/(float)nleds-pos)/sigma, 2));
        strip.setPixelColor(i+startled, strip.Color((int)br, 0, 0));
    }
    strip.show();
    schedule_insert(kit, period, startled, nleds, millis()+30);
}

