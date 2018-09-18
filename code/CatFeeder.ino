SYSTEM_THREAD(ENABLED);
STARTUP(System.enableFeature(FEATURE_RESET_INFO));

#include "clickButton.h"

#define BTN_RED D1
#define BTN_GREEN D3

#define BTN_POWER_DATA A3
#define BTN_POWER_GND A2

#define LED_PIN A0
#define LED_GND A1

#define SERVO D0

Servo myservo;

ClickButton buttonRed(BTN_RED, HIGH, false);
ClickButton buttonGreen(BTN_GREEN, HIGH, false);

bool isFeeding = false;
volatile bool isActive = false;

#define LED_BLINKSPEED 500
int lastLedAction = 0;
int ledState = LOW;

#define FEEDING_DURATION_NORMAL 3*1000
#define FEEDING_DURATION_INITIAL 6*1000
int feedingStart = 0;
int feedingDuration = 0;

#define FEEDING_TIME1_H 6
#define FEEDING_TIME1_M 0
#define FEEDING_TIME1_S 0

#define FEEDING_TIME2_H 12
#define FEEDING_TIME2_M 0
#define FEEDING_TIME2_S 0

#define FEEDING_TIME3_H 18
#define FEEDING_TIME3_M 30
#define FEEDING_TIME3_S 0

int feedingTime1 = FEEDING_TIME1_H*60*60 + FEEDING_TIME1_M*60 + FEEDING_TIME1_S;
int feedingTime2 = FEEDING_TIME2_H*60*60 + FEEDING_TIME2_M*60 + FEEDING_TIME2_S;
int feedingTime3 = FEEDING_TIME3_H*60*60 + FEEDING_TIME3_M*60 + FEEDING_TIME3_S;

#define SLEEP_DELAY 1*1000*60
int awakeSince = 0;
// should we deep sleep after the feeding?
bool sleepMode = true;

// Sync Time
#define SYNC_INTERVAL 4*60*60
int lastSync = 0;
int timezone = 1;

void setup() {
    awakeSince = millis();
    //Serial.begin(9600);

    // Load timezone from EEPROM or save it if necessary
    uint8_t timezone_value;
    EEPROM.get(0, timezone_value);
    if(timezone_value == 255) {
        EEPROM.put(0, timezone);
    }else{
        timezone = timezone_value;
    }
    Time.zone(timezone);

    pinMode(BTN_RED, INPUT);
    pinMode(BTN_GREEN, INPUT);
    pinMode(WKP, INPUT);

    pinMode(LED_GND, OUTPUT);
    digitalWrite(LED_GND, LOW);
    pinMode(LED_PIN, OUTPUT);
    setLed(LOW);

    pinMode(BTN_POWER_GND, OUTPUT);
    digitalWrite(BTN_POWER_GND, LOW);
    pinMode(BTN_POWER_DATA, INPUT_PULLUP);
    attachInterrupt(BTN_POWER_DATA, activate, CHANGE);

    activate();

    // Wake Up from sleep?
    // we can check the button immediately because SYSTEM_THREAD is used
    if(System.resetReason() == RESET_REASON_POWER_MANAGEMENT && isActive){

        // Wake by button press
        if(digitalRead(BTN_GREEN) == HIGH){
            feed(FEEDING_DURATION_NORMAL);
        }
        if(digitalRead(BTN_RED) == HIGH){
            feed(FEEDING_DURATION_INITIAL);
        }

        // is feeding time?
        // probably we need some time to wake up, so it could be some seconds later
        // so we have a small range of 5 seconds which is assumed to be the feeding time
        if(isFeedingTime(5)){
            feed(FEEDING_DURATION_NORMAL);
        }
    }

    Particle.function("feed", feedCloud);
    Particle.function("stopFeeding", stopFeedingCloud);
    Particle.function("setTimeZone", setTimeZone);
}

void loop() {
    adjustTime();

    // Go to sleep
    if(!isFeeding){
        sleepUntilNextFeed();
    }

    if(isActive){

        // Is now feeding time?
        if(!isFeeding && isFeedingTime(0)){
            feed(FEEDING_DURATION_NORMAL);
        }

       // Update Buttons

       // Red Button
       // When there is a feeding the feeding is stopped
       // When there is no feeding the initial feeding is started
       buttonRed.Update();
       if(buttonRed.clicks == 1){
           if(isFeeding){
               stopFeeding();
           }else{
               feed(FEEDING_DURATION_INITIAL);
           }
       }
       // Green Button
       // Single Click: Start feeding
       // Double Click: Start endless feeding
       buttonGreen.Update();
       if(buttonGreen.clicks == 1){
           feed(FEEDING_DURATION_NORMAL);
       }
       if(buttonGreen.clicks == 2){
           feed(0);
       }

       // avoiding delay
       // blink led while feeding
       if(isFeeding && (millis() - lastLedAction > LED_BLINKSPEED)){
           setLed(!ledState);
       }

       // stop feeding when there was a duration set
       if(isFeeding && feedingDuration > 0 && (millis() - feedingStart >= feedingDuration)){
           stopFeeding();
       }
    }

 }

void activate(){
    if(digitalRead(BTN_POWER_DATA) == LOW){
        isActive = true;
    }else{
        isActive = false;
        stopFeeding();
        setLed(LOW);
    }
}

void feed(int duration){
    Particle.publish("Feed");
    myservo.attach(SERVO);
    // small step backwards
    myservo.write(30);
    delay(500);
    // forward
    myservo.write(180);

    isFeeding = true;
    feedingStart = millis();
    feedingDuration = duration;

    // update awake since last feed
    awakeSince = millis();
}

void stopFeeding(){
    isFeeding = false;
    myservo.detach();
    setLed(LOW);
}

int setTimeZone(String zone){
    uint8_t timezone = zone.toInt();
    EEPROM.put(0, timezone);
    Time.zone(timezone);
    return 0;
}

void setLed(bool state){
    ledState = state;
    digitalWrite(LED_PIN, ledState);
    lastLedAction = millis();
}


int feedCloud(String dur){
    uint8_t duration = dur.toInt();
    if(duration > 0 && isActive){
      feed(duration*1000);
      return 0;
    }
    return 1;
}

int stopFeedingCloud(String none){
    stopFeeding();
    return 0;
}

void adjustTime(){
    if(WiFi.ready() && (lastSync + SYNC_INTERVAL < Time.now())) {
         Particle.syncTime();
         lastSync = Time.now();
     }
}

void sleepUntilNextFeed(){
    if (sleepMode == true && (millis() - awakeSince > SLEEP_DELAY)) {
        int secondsToSleep = 0;
        int secondOfDay = Time.local() % 86400;

        if(secondOfDay < feedingTime1){
            secondsToSleep = feedingTime1 - secondOfDay;
        }else if(secondOfDay < feedingTime2){
            secondsToSleep = feedingTime2 - secondOfDay;
        }else if(secondOfDay < feedingTime3){
            secondsToSleep = feedingTime3 - secondOfDay;
        }
        // Tomorrow
        else{
            secondsToSleep = 86400 + (feedingTime1 - secondOfDay);
        }

        //Serial.println("Sleeping");
        //Serial.println(secondsToSleep);
        Particle.publish("Sleeping", String(secondsToSleep));
        System.sleep(SLEEP_MODE_DEEP, secondsToSleep);;
    }
}

bool isFeedingTime(int precision){
    int secondOfDay = Time.local() % 86400;

    return (secondOfDay >= feedingTime1 && secondOfDay <= feedingTime1 + precision) ||
            (secondOfDay >= feedingTime2 && secondOfDay <= feedingTime2 + precision) ||
            (secondOfDay >= feedingTime3 && secondOfDay <= feedingTime3 + precision);
}
