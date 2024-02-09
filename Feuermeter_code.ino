// Welcome to the Feuermeter code

#include <IRremote.h>
#include <Servo.h>
#include <math.h>

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
int angle_change_value = 10;
bool check_up;



void setup() {
  Serial.begin(9600);
  // Es braucht einen Schaltplan für die pins
  pinMode(IRSend_pin, OUTPUT);
  pinMode(IRRecv_pin, INPUT);
  irrecv.enableIRIn();  //Aktiviert den Receiver

  servo_y.attach(2);  //placholder pin ig
  servo_x.attach(3);  //placeholder pin ig
}


// Announcing is up for debate
enum class State {
  SEARCHING,
  APPROACHING,
  SETTING_UP,
  ANNOUNCING,
  EXTINGUISHING,
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
    if (pos_x < 180 && direction_right) {
      pos_x += angle_change_value;
    } else if (pos_x < 180 && !direction_right) {
      pos_x -= angle_change_value;
    }
    servo_x.write(pos_x);

    // Ensure physical limits are not exceeded
    if (pos_x == 180) {
      direction_right = false;
    } else if (pos_x == 0) {
      direction_right = true;
    }


    // Adjust y value
    if (pos_y < 180 && direction_up) {
      pos_y += angle_change_value;
    } else if (pos_y < 180 && !direction_up) {
      pos_y -= angle_change_value;
    }
    servo_x.write(pos_y);

    // Ensure physical limits are not exceeded
    if (pos_y == 180) {
      direction_up = false;
    } else if (pos_y == 0) {
      direction_up = true;
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


int const required_distance = 100;  //in cm


// going to the fire
void approach() {
  // how to read distance from IR to fire?
  // Problem: What measures distance?
  int current_distance = calc_ankathete(1, (pos_y / 180) * M_PI);

  if (current_distance > required_distance) {
    check_up = true;
  } else if (current_distance < required_distance) {
    check_up = false;
  } else if (current_distance == required_distance) {
    state = State::EXTINGUISHING;  // or announcing?
    return;
  }

  // Betriebsparameter(current_distance - required distance) should hopefully move the feuermeter the distance
}


void set_up_extinguish() {
  int temp_angle_change;

  if (check_up) {
    temp_angle_change = angle_change_value;
  } else if (!check_up) {
    temp_angle_change = -angle_change_value;
  }

  for (pos_y; pos_y <= 180 || pos_y >= 0; pos_y += temp_angle_change) {
    servo_y.write(pos_y);

    if (fire_found()) {
      break;
    }
  }

  if (pos_y == 180 && !fire_found() || pos_y == 0 && !fire_found()) {
    // error!!
    state = State::SEARCHING;
    return;
  }

  int height = tan(pos_y) * required_distance;
  // ausgeben mit zusätzlich winkel(pos_y) und distanz zum feuer (immernoch:woher??)

  //Problem: Nur auf 10 grad präzise
}


void loop() {
  if (state == State::SEARCHING) {
    search();
  } else if (state == State::APPROACHING) {
    approach();
  } else if (state == State::SETTING_UP) {
    set_up_extinguish();
  } else if (state == State::ANNOUNCING) {
    // kdhb
  } else if (state == State::EXTINGUISHING) {
    // sdbghj
  }
}


// TODO: alle pins verbinden (wo sind die pins), auf Betriebsparameter warten :(
