/*  Death Star Lamp
    by Bithead

    This sketch is for an Arduino Pro Mini

    This sketch relies on the Pololu DRV8834 stepper motor driver.
       It triggers a screw turn which rises/lowers the platform which
       opens/closes the deathstar.
       https://www.pololu.com/product/2134
    This sketch also relies on 6 strips of NeoPixels, with 10 LEDs each.
    This sketch also relies on 1 Sharp IR distance sensor.
       It is used as a "touchless" switch to trigger the animation.

    Pin     Function
    0,1     Serial
    2       Step Pin - Stepper motor
    3       Direction Pin - Stepper motor
    4       Stepper Motor Driver Board Sleep
    6       LED Data Pin
    A0      Distance Sensor Pin


 ************************************************************************************/
#include <Adafruit_NeoPixel.h>
#include <SharpDistSensor.h>

#define _stepPin 2
#define _dirPin 3
#define _nsleep 4
#define _distPin A0
#define _ledPin 6
#define _iSpeedMax 1500
#define _iSpeedMin 600
#define _iRamp 400
#define _iNumSteps 2000
#define _sensorCount 5
#define _sensorDist 800
#define _LEDCount 60
int direction = 1;                      //start closed
const byte medianFilterWindowSize = 1;  // Window size of the Sharp sensor median filter (odd number, 1 = no filtering)

SharpDistSensor sensor(_distPin, medianFilterWindowSize);
Adafruit_NeoPixel pixels(_LEDCount, _ledPin, NEO_GRB + NEO_KHZ800);

void setup() {
  pinMode(_stepPin, OUTPUT);
  pinMode(_dirPin, OUTPUT);
  pinMode(_nsleep, OUTPUT);
  pinMode(_distPin, INPUT);
  pinMode(_ledPin, OUTPUT);

  pixels.begin();  // INITIALIZE NeoPixel strip object (REQUIRED)
  pixels.clear();  // Turn OFF all pixels ASAP
  pixels.show();

  digitalWrite(_dirPin, HIGH);  //start closed, get ready to open
  digitalWrite(_nsleep, LOW);   //start asleep
  Serial.begin(9600);
}

void loop() {
  unsigned int distance = getDist();
  Serial.println(distance);
  if (distance <= _sensorDist) {
    if (direction == 1) {
      stripColor(pixels.Color(255, 0, 0));  //Red
    } else {
      stripColor(pixels.Color(0, 255, 0));  //Green
    }
    motorMove(direction);
    //Switch direction
    if (direction == 1) {
      direction = 0;
      fireAnimation(25);
      pixels.clear();
      pixels.show();
    } else {
      direction = 1;
      laserAnimation(pixels.Color(0, 255, 0), 50);  // Green
      pixels.clear();
      pixels.show();
    }
    delay(1000);  //Delay after move
  }

  delay(200);  //Delay between loops
}

int getDist() {
  uint16_t sum = 0;
  for (int count = 0; count <= _sensorCount; count++) {
    sum += sensor.getDist();
  }
  return sum / _sensorCount;
}

void motorMove(int Direction) {
  int iSpeed = _iSpeedMax;

  if (Direction == 1) {
    digitalWrite(_dirPin, HIGH);  //Go Up
    Serial.println("Going Up");
  } else {
    digitalWrite(_dirPin, LOW);  //Go Down
    Serial.println("Going Down");
  }
  digitalWrite(_nsleep, HIGH);  //Wake

  for (int x = 0; x <= _iNumSteps; x++) {
    if (x <= _iRamp) iSpeed = iSpeed - 4;              //ramp up
    if (x > _iNumSteps - _iRamp) iSpeed = iSpeed + 4;  //ramp down
    if (iSpeed > _iSpeedMax) iSpeed = _iSpeedMax;
    if (iSpeed < _iSpeedMin) iSpeed = _iSpeedMin;
    digitalWrite(_stepPin, HIGH);
    delayMicroseconds(iSpeed);  //lower value = faster pulse
    digitalWrite(_stepPin, LOW);
    delayMicroseconds(iSpeed);
    //Serial.println(iSpeed);
  }
  digitalWrite(_nsleep, LOW);  //Sleep
}

void stripColor(uint32_t color) {
  pixels.clear();
  for (int c = 0; c < _LEDCount; c++) {
    pixels.setPixelColor(c, color);  // Set pixel 'c' to value 'color'
  }
  pixels.show();  // Update strip with new contents
}

void fireAnimation(int loop) {
  pixels.setBrightness(100);
  for (int j = 0; j <= loop; j++) {
    int randDelay = random(20, 500);
    pixels.clear();
    for (int i = 0; i < _LEDCount; i += 2) {
      pixels.setPixelColor(i, pixels.Color(255, 0, 0));       //red
      pixels.setPixelColor(i + 1, pixels.Color(255, 75, 0));  //orange
    }
    pixels.show();
    delay(randDelay);
    pixels.clear();
    for (int i = 0; i < _LEDCount; i += 2) {
      pixels.setPixelColor(i + 1, pixels.Color(255, 0, 0));  //red
      pixels.setPixelColor(i, pixels.Color(255, 75, 0));     //orange
    }
    pixels.show();
    delay(randDelay);
  }
}

// Theater-marquee-style chasing lights. Pass in a color (32-bit value,
// a la strip.Color(r,g,b) as mentioned above), and a delay time (in ms)
// between frames.
void laserAnimation(uint32_t color, int wait) {
  pixels.setBrightness(255);
  for (int a = 0; a < 50; a++) {   // Repeat 50 times...
    for (int b = 0; b < 3; b++) {  //  'b' counts from 0 to 2...
      pixels.clear();              //   Set all pixels in RAM to 0 (off)
      // 'c' counts up from 'b' to end of strip in steps of 3...
      for (int c = b; c < _LEDCount; c += 3) {
        pixels.setPixelColor(c, color);  // Set pixel 'c' to value 'color'
      }
      pixels.show();  // Update strip with new contents
      delay(wait);    // Pause for a moment
    }
  }
}