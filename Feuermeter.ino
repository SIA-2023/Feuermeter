// Welcome to the Feuermeter code

#include <IRremote.h>
#include <Servo.h>
#include <math.h>
//#include "omni_driver_v3.ino"
#include <LiquidCrystal.h>
#include <time.h>

// lcd pins
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

int IRRecv_pin = 0;  //Placeholder pin
IRrecv irrecv(IRRecv_pin);
decode_results IR_results;

int IRSend_pin = 1;  //plllacceeholdeerr
IRsend irsend;

Servo servo_y;
Servo servo_x;

int pos_x = 0;  // variable to store the servo x position
int pos_y = 0;  // variable to store the servo y position

bool direction_right = true;
bool direction_up = true;
static int angle_change_standard = 10;
static int angle_change_x = 10;
static int angle_change_y = 12 / angle_change_x;
static int maximum_angle = 120;
static int minimum_angle = 0;
bool check_up;
static int height_extinguisher = 20; // in cm
const int required_distance = 100;  //in cm



void setup() {
  Serial.begin(9600);
  // Es braucht einen Schaltplan für die pins
  pinMode(IRSend_pin, OUTPUT);
  pinMode(IRRecv_pin, INPUT);
  irrecv.enableIRIn();  //Aktiviert den Receiver

  IRsend irsend;

  servo_y.attach(2);  //placholder pin ig
  servo_x.attach(3);  //placeholder pin ig

  lcd.begin(16, 2);
}


enum class State {
  SEARCHING,
  APPROACHING,
  SETTING_UP,
  EXTINGUISHING,
  WAITING,
};
State state = State::SEARCHING;


// True if signal was received.
bool fire_found() {
  if (irrecv.decode(&IR_results)) {
    irrecv.resume();
    state = State::APPROACHING;
    return true;
  }
  return false;
}

void extinguish_flame() {
  // welchen code braucht der feuermeter sensor?
  cli();
  irsend.sendRC5(0x0, 8);  //code: 0x0, 8 bits
  sei();
}


// Method is up for debate
void search() {
  while (!fire_found()) {
    //Adjust x value
    if (pos_x < maximum_angle && direction_right) {
      pos_x += angle_change_x;
    } else if (pos_x < maximum_angle && !direction_right) {
      pos_x -= angle_change_x;
    }
    servo_x.write(pos_x);

    // Ensure physical limits are not exceeded
    if (pos_x == maximum_angle) {
      direction_right = false;
    } else if (pos_x == minimum_angle) {
      direction_right = true;
    }


    // Adjust y value
    if (pos_y < maximum_angle && direction_up && direction_right) {
      pos_y += angle_change_y;
    } else if (pos_y < maximum_angle && !direction_up && direction_right) {
      pos_y -= angle_change_y;
    }
    servo_y.write(pos_y);

    // Ensure physical limits are not exceeded
    if (pos_y == maximum_angle) {
      direction_up = false;
      // rotate(maximum_angle);, in deg
    } else if (pos_y == minimum_angle) {
      direction_up = true;
      // rotate(maximum_angle);, in deg
    }

    // Checks if the fire has been found at the end of each cycle
    if (fire_found() == true) {
      state = State::APPROACHING;
    }
  }
}


// Distance in centimetres, alpha in rad
int calc_ankathete(int alpha, int distance_to_fire) {
  return cos(alpha) * distance_to_fire;
}


// MOCKUP
void distance_drive(int distance, int degrees) {

}


// going to the fire
void approach(int current_distance) {
  //Distance and degrees, for some reasom

  if (current_distance > required_distance) {
    check_up = true;
  } else if (current_distance < required_distance) {
    check_up = false;
  } else if (current_distance == required_distance) {
    state = State::EXTINGUISHING;  // or announcing?
    return;
  }
  distance_drive(current_distance, 0);
  // Betriebsparameter(current_distance - required distance) should hopefully move the feuermeter the distance
}


void set_up_extinguish() {
  int temp_angle_change;

  if (check_up) {
    temp_angle_change = angle_change_standard;
  } else if (!check_up) {
    temp_angle_change = -angle_change_standard;
  }

  for (pos_y; pos_y <= maximum_angle || pos_y >= minimum_angle; pos_y += temp_angle_change) {
    servo_y.write(pos_y);

    if (fire_found()) {
      break;
    }
  }

  if (pos_y == maximum_angle && !fire_found() || pos_y == minimum_angle && !fire_found()) {
    // error!!
    state = State::SEARCHING;
    return;
  }

  int height = tan(pos_y) * required_distance;
  // ausgeben mit zusätzlich winkel(pos_y) und distanz zum feuer (immernoch:woher??)

  //Problem: Nur auf 10 grad präzise
}

void canon_reset() {
  pos_x = 0;
  servo_x.write(pos_x);
  pos_y = 0;
  servo_y.write(pos_y);
  // wait a bit

}


void extinguish(time_t time_beginning) {
  time_t time_current = second();

  int time_elapsed = seconds(time_current - time_beginning); 
  String time_elapsed_s = time_elapsed;

  if (time_elapsed < 10) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Löschvorgang - Pause");
    lcd.setCursor(0, 1);
    lcd.print("Vergangene Zeit: " + time_elapsed);
  }
  if (time_elapsed < 15 && time_elapsed > 10) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Löschvorgang - Löschen");
    lcd.setCursor(0, 1);
    lcd.print("Vergangene Zeit: " + time_elapsed);

    irsend.sendRC6(0, 1); // sollte das signal zum löschen sein
  }
  if (time_elapsed > 15) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Feuer gelöscht");
    canon_reset();

    State::WAITING;
  }
}


int lcd_write(String line_1, String line_2) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(line_1);
  lcd.setCursor(0, 1);
  lcd.print(line_2);
}


void output_data() {
  // winkel
  float current_distance = calc_ankathete(1, (pos_y / maximum_angle) * M_PI);
  float height = height_extinguisher +  (tan(pos_y) * required_distance);

  String line_1 = String("y/z: ", height, " / ", current_distance);
  String line_2 = String("Winkel: ", pos_y, "°");

  lcd_write(line_1, line_2);
}


void loop_feuer() {
  // 1 sollte entfernung sein
  float current_distance = calc_ankathete(1, (pos_y / maximum_angle) * M_PI);

  if (state == State::SEARCHING) {
    search();
  } else if (state == State::APPROACHING) {
    approach(current_distance);
    output_data();
  } else if (state == State::SETTING_UP) {
    set_up_extinguish();
    time_t time_beginning = second();
  } else if (state == State::EXTINGUISHING) {
    extinguish( time_beginning);
  } else if ( state == State::WAITING) {
    // reaktivationssignal nötig
  }
}

void loop() {
  loop_feuer();
}


// TODO: alle pins verbinden (wo sind die pins), auf Betriebsparameter warten :(
// entfernungssensor (wtf?)
// 30 deg drehen
// "maximale helligkeitsamplitude suchen"
// sobald entfernungssignal vorliegt: alle 0.5 s position feuer zu lk (winkel, entfernung und höhe)
// fire found: anzeigen, 10s pause (als pause angeben), 5s löschen (auch ausgeben), auf  nullpunkt zurück
// "IR4"


// IR testen, Entfernungssensoren verstehen und impelemntieren, search nichtmehr von while abhängig
