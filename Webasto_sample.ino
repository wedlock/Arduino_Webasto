#include "Arduino.h"
#include <math.h> // needed to perform some calculations
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <avr/wdt.h>

byte fanChar[ ] = {
B00000,
B10011,
B10100,
B01110,
B00101,
B11001,
B00000,
B00000 };
byte fuelChar[ ] = {
B11110,
B10010,
B10011,
B11111,
B11111,
B11111,
B11110,
B00000 };
byte waterChar[ ] = {
B00000,
B00100,
B01110,
B01110,
B11111,
B11111,
B01110,
B00000 };
byte exhaustChar[ ] = {
B01110,
B01110,
B01110,
B11111,
B01110,
B00100,
B00000,
B00000 };
byte w_pumpChar[ ] = {
B00000,
B00100,
B11110,
B11111,
B11110,
B00100,
B00000,
B00000 };

int fuel_pump_pin = 5;
int glow_plug_pin = 6;
int burn_fan_pin = 9;
int water_pump_pin = 10;
//int water_temp_pin = A0;
//int exhaust_temp_pin = A1;
int water_input_index = 0;
int exhaust_input_index = 1;
int button_pin = 3;

int temperature_target = 70;  // degres C
int run_pump_min = 30;  // minimal temperature for run water pump
int fan_speed;  // percent
int water_pump_speed;  // percent
int fuel_need;  // percent
int glow_time;  // seconds
int water_temp;  // degres C
int water_temp_sec[10];
int exhaust_temp;  // degres C
int exhaust_temp_sec[10];  // array of last 10 sec water temp, degres C
int shower_timeout;
int percent_map = 0;
int ignit_fail;
int seconds;

bool shower;
bool burn;
bool webasto_fail;
bool lean_burn;
bool overheating;
int burn_mode = 0;
bool check_pause;

int old_water_temp;
int old_exhaust_temp;
int old_fan_speed;
int old_fuel_need;
int old_fan;

int blink_LED = 13;
int blink_LED_status = 0;

LiquidCrystal_I2C lcd(0x3f, 16, 2);
DeviceAddress deviceAddress;
OneWire ds18x20[ ] = { A0, A1 };
const int oneWireCount = sizeof(ds18x20) / sizeof(OneWire);
DallasTemperature sensor[oneWireCount];

void setup() {
	webasto_fail = false;
	for (int i = 0; i < oneWireCount; i++) {
		sensor[i].setOneWire(&ds18x20[i]);
		sensor[i].begin();
		if (sensor[i].getAddress(deviceAddress, 0))
		    sensor[i].setResolution(deviceAddress, 9);
	}

//	lcd.createChar(0, waterChar);
//	lcd.createChar(1, exhaustChar);
//	lcd.createChar(2, fanChar);
//	lcd.createChar(3, fuelChar);
//	lcd.createChar(3, w_pumpChar);

	TCCR1B = (TCCR1B & 0b11111000) | 0x01;  // magic Fast PWM parameter

	pinMode(glow_plug_pin, OUTPUT);
	pinMode(fuel_pump_pin, OUTPUT);
	pinMode(burn_fan_pin, OUTPUT);
	pinMode(water_pump_pin, OUTPUT);
//	pinMode(water_temp_pin, INPUT);
//	pinMode(exhaust_temp_pin, INPUT);
	pinMode(button_pin, INPUT_PULLUP);  // important, this pulls de button pin to high, so when pressed it goes to low
	pinMode(blink_LED, OUTPUT);

	lcd.init();

	lcd.home();

	lcd.setCursor(0, 0);

	lcd.backlight();
	lcd.createChar(0, waterChar);
	lcd.createChar(1, exhaustChar);
	lcd.createChar(2, fanChar);
	lcd.createChar(3, fuelChar);
	lcd.createChar(4, w_pumpChar);

	check_pause = false;
	overheating = false;

	Serial.begin(9600);
}

void temp_data() {  // keeps the temp variables updated
	static unsigned long timer;
//	if (millis() < timer) {
//		timer = millis();
//	}
// call the get_temp function and smoothen the result
//	water_temp = (9 * water_temp + get_temp(water_temp_pin)) / 10;
//	exhaust_temp = (9 * exhaust_temp + get_temp(exhaust_temp_pin)) / 10;
	water_temp = get_temp(water_input_index);
	exhaust_temp = get_temp(exhaust_input_index);

	if (millis() > timer + 1000) {  // every sec
		timer = millis();

		for (int i = 9; i >= 1; i--) {  // updating the exhaust temperature history
			exhaust_temp_sec[i] = exhaust_temp_sec[i - 1];
			water_temp_sec[i] = water_temp_sec[i - 1];
		}
		exhaust_temp_sec[0] = exhaust_temp;  // add new temp value
		water_temp_sec[0] = water_temp;
	}
}

void control() {
	static unsigned long timer;
	if (millis() < timer) {
		timer = millis();
	}
	static bool pushed;
	static bool long_press;
	bool push;

	if (digitalRead(button_pin)) {
		push = true;
	} else {
		push = false;
	}

	if (!push && !pushed) {
		timer = millis();

	}

	if (push && !pushed) {
		timer = millis();
		pushed = true;
	}

	if (push && pushed && millis() > timer + 500) {
		if (!long_press) {  // when long press
			long_press = true;
			if (webasto_fail) {  // reset webasto fail if there has been a failure
				lcd.setCursor(0, 0);
				lcd.print("RESET     |");
				shower_timeout = 0;
				burn_mode = 0;
				burn = 0;
				glow_time = 0;
				shower = false;
				webasto_fail = false;
				check_pause = false;
				overheating = false;
			}
			if (!webasto_fail && burn_mode > 0) {  // fast stop
				lcd.setCursor(0, 0);
				lcd.print("FAST STOP |");
				burn_mode = 0;
				shower_timeout = 0;
				webasto_fail = false;
				shower = false;
				burn_mode = 0;
				burn = 0;
				glow_time = 0;
				check_pause = false;
				overheating = false;
			}
		}
	}

	if (pushed && !push) {  // if shower is off, turn it on with a 60 seconds timeout
		if (millis() > timer + 50 && !long_press) {  // when short press
			if (burn_mode > 0) {  // if shower is on, turn it off with a 15 second timeout
				lcd.setCursor(0, 0);
				lcd.print("TURN OFF  |");
				shower_timeout = 15;
				shower = false;
				burn_mode = 3;
				burn = 0;
				glow_time = 0;
				seconds = 0;
				check_pause = false;
				overheating = false;
			}
			if (burn_mode == 0) {
				lcd.setCursor(0, 0);
				lcd.print("START     |");
				shower = 1;
				shower_timeout = 99;
				check_pause = false;
				overheating = false;
			}
		}
		pushed = 0;
		long_press = 0;
	}
}

void shower_void() {

	static unsigned long secs_timer;
	if (millis() < secs_timer) {
		secs_timer = millis();
	}

	if (shower) {
		burn = 1;
		if (shower_timeout == 99) {
			shower_timeout = 60;
		}
		if (shower_timeout < 15) {  // helps to burn all the remaining fuel in the combustion chamber
			lean_burn = 1;
		} else {
			lean_burn = 0;
		}

	}

	else {  // reinitialize variables
		burn = 0;
		//seconds = 0;
		shower_timeout = 0;
		lean_burn = 0;
	}

	water_pump();  // calls the water_pump function
}

void printData(int time) {

	Serial.println(
	        "fail|igniF|shower|burn|b_mode|waterT |exhaustT|fanSpeed|fuel  |glowTime|Pump |Time");
	Serial.print("  ");
	Serial.print(webasto_fail);
	Serial.print(" | ");
	Serial.print(ignit_fail);
	Serial.print("   | ");
	Serial.print(shower);
	Serial.print("    | ");
	Serial.print(burn);
	Serial.print("  | ");
	Serial.print(burn_mode);
	Serial.print("    | ");
	Serial.print(water_temp);
	Serial.print(" | ");
	Serial.print(exhaust_temp);
	Serial.print("  | ");
	Serial.print(fan_speed);
	Serial.print("  | ");
	Serial.print(fuel_need);
	Serial.print(" |    ");
	Serial.print(glow_time);
	Serial.print("   | ");
	Serial.print(water_pump_speed);
	Serial.print("   | ");
	Serial.print(time);
	Serial.println();
}

void printLCD() {

	if (water_temp != old_water_temp) {
		lcd.setCursor(1, 1);
		lcd.print("   ");
	}
	if (exhaust_temp != old_exhaust_temp) {
		lcd.setCursor(5, 1);
		lcd.print("   ");
	}

	if (fan_speed != old_fan_speed) {
		lcd.setCursor(9, 1);
		lcd.print("   ");
	}
	if (fuel_need != old_fuel_need) {
		lcd.setCursor(13, 1);
		lcd.print("   ");
	}

	lcd.setCursor(0, 1);
	lcd.write(0);
	lcd.print(water_temp);

	lcd.setCursor(4, 1);
	lcd.write(1);
	lcd.print(exhaust_temp);

	lcd.setCursor(8, 1);
	lcd.write(2);
	lcd.print(fan_speed);

	lcd.setCursor(12, 1);
	lcd.write(3);
	lcd.print(fuel_need);

	lcd.setCursor(11, 0);
	if (check_pause) {
		lcd.print(21 - seconds);
		lcd.print(" ");
	} else {
		lcd.print("   ");
	}

//	lcd.write(0);
//	lcd.write(4);
//	lcd.print(seconds);
//	lcd.print("  ");

	if (webasto_fail) {
		lcd.setCursor(0, 0);
		lcd.print("FAIL FAIL |");
		if (blink_LED_status)
			lcd.backlight();
		else
			lcd.noBacklight();
	} else {
		lcd.backlight();
		switch (burn_mode) {
		case 0:
			lcd.setCursor(0, 0);
			lcd.print("IDLE      |");
			break;
		case 1:
			lcd.setCursor(0, 0);
			lcd.print("BURNING UP|");
			break;
		case 2:
			lcd.setCursor(0, 0);
			if (overheating)
				lcd.print("SLEEPING  |");
			else
				lcd.print("WORKING   |");
			break;
		case 3:
			lcd.setCursor(0, 0);
			lcd.print("BURN OFF  |");
			break;
		}
	}

	old_water_temp = (int) water_temp;
	old_exhaust_temp = (int) exhaust_temp;
	old_fan_speed = (int) fan_speed;
	old_fuel_need = (int) fuel_need;
}

void webasto_power() {  //calculate webasto power and set fan and fuel
	old_fan = fan_speed;
//	fan_speed = 100 - mapf(water_temp, 40, temperature_target + 10, 30, 80);  //water_temp,min water, max water, min fan, max fan
//	fuel_need = constrain(fan_speed + 20, 30, 80);
//	fan_speed = constrain(fan_speed + 10, 30, 80);

	fan_speed = 80;
	fuel_need = 50;

	if (water_temp < 30) {
		fan_speed = 60;
		fuel_need = 30;
	}

	if (water_temp > 60) {
		fan_speed = 70;
		fuel_need = 40;
	}

	if (water_temp > 65) {
		fan_speed = 60;
		fuel_need = 30;
	}

	if (old_fan != fan_speed) {
		seconds = 0;
		check_pause = true;
	}

}

void webasto() {  // this will handle the combustion
	static unsigned long timer;
	if (millis() < timer) {
		timer = millis();
	}
	static float temp_init_exhaust;
	static float temp_init_water;
	static int ignit_fail;

	if (millis() > timer + 1000) {  // every seconds, run this
		timer = millis();
		seconds++;  // increment the seconds counter
		//printData(seconds);
		printLCD();
		blink_LED_status = !blink_LED_status;
		digitalWrite(blink_LED, blink_LED_status);

	}

	if (!webasto_fail) {  // if everything's going fine

		if ((burn_mode == 0 || burn_mode == 3) && burn) {  // if the user wants a shower
			// initiate the start sequence
			burn_mode = 1;
			seconds = 0;
			temp_init_exhaust = exhaust_temp;// store the exhaust temperature before trying to start the fire
			temp_init_water = water_temp;
		}

		if ((burn_mode == 1 || burn_mode == 2) && !burn) {  // if the shower has ended
			burn_mode = 3;
			seconds = 0;
			ignit_fail = 0;
		}

		if (ignit_fail > 3) {  // if there was more than 3 attempts to start fire but all failed
			webasto_fail = true;
			burn_mode = 3;
			shower = 0;
			burn = 0;
			fan_speed = 50;
			water_pump_speed = HIGH;
		}

	} else {  // if there has been a major failure, stop everything
		shower = 0;
		burn = 0;
		ignit_fail = 0;
	}

	switch (burn_mode) {
	case 0: {  // everything is turned off in this mode
		fan_speed = 0;
		fuel_need = 0;
		glow_time = 0;
		lean_burn = 0;
	}
		break;

	case 1: {  // the fire starting sequence

		if (seconds < 5) {
			fuel_need = 0;
			fan_speed = 60;  //vent a chamber
		}

		if (seconds == 6) {
			fuel_need = 90;
			fan_speed = 40;
		}

		if (seconds == 8) {
			fuel_need = 0;  // stop and wait for it to spread all over the glow plug (important)
			// don't forget that the fan is still spinning as it wasn't turned off
		}

		if (seconds == 10) {
			glow_time = 12;  // switch on the glow plug for 12 seconds so it ignites the fuel
		}

		if (seconds == 15) {
			fuel_need = 30;
		}

		if (seconds > 17) {  // the glow plug has just been turn of (7+12=19)
			fan_speed = 50;  // get some more air and restart pumping fuel slowly
			fuel_need = 30;
		}

		if (seconds > 18) {
			if (exhaust_temp - temp_init_exhaust > 3) {  // exhaust temp raised a bit meaning fire has started
				burn_mode = 2;  // go to main burning mode and initialize variables
				seconds = 0;
				glow_time = 0;
				ignit_fail = 0;
			}
			if (water_temp - temp_init_water > 2) {  // water temp raised a bit meaning fire has started
				burn_mode = 2;  // go to main burning mode and initialize variables
				seconds = 0;
				glow_time = 0;
				ignit_fail = 0;
			}

			if (seconds > 30) {
				// the fire sequence didn't work, give it an other try
				burn_mode = 0;
				ignit_fail++;
			}
		}
		break;

		case 2:
		{
			webasto_power();  //calculate and set webasto power
			if (water_temp >= temperature_target) {  //if we reach max temp. make a sleep for webasto
				overheating = true;
			}
			if (overheating && (water_temp < temperature_target - 5)) {  //if temp is lower use start sequence
				overheating = false;
				burn_mode = 1;
			}
			if (overheating) {
				fan_speed = 20;
				fuel_need = 0;
			}
			if (!check_pause) {
				if (!overheating && (exhaust_temp - exhaust_temp_sec[9]) > 10) {  // if flame died
					burn_mode = 1;
					seconds = 0;
				}
			} else {
				if (seconds > 20) {  // wait 20 sec to establish exhaust temp after we change power
					check_pause = false;
				}
			}
		}
		if (lean_burn) {  // burn the remaining fuel in the chamber
			fuel_need = 0;
			fan_speed = 50;
		}
	}
		break;

	case 3: {  // snuff out the fire, with just a little air to avoid fumes and backfire

		fan_speed = 50;
		fuel_need = 0;
		glow_time = 0;

//		if (seconds > 30) {
//			fan_speed = 100;
//		}
		if (water_temp > water_temp_sec[9]) {  // if water temp is raising, slow down the fan
			fan_speed = fan_speed - 10;
			if (fan_speed < 20) {
				fan_speed = 20;
			}
		} else {
			fan_speed = fan_speed - 10;
			if (fan_speed > 100) {
				fan_speed = 100;
			}
		}

		if (seconds > 120 && exhaust_temp < exhaust_temp_sec[9]) {
			// turn everything off if temperature is not raising
			burn_mode = 0;
		}
	}
		break;
	}

// call every output function
	fuel_pump();
	burn_fan();
	glow_plug();
}

void fuel_pump() {
	static unsigned long timer;
	if (millis() < timer) {
		timer = millis();
	}
	static bool pulsing;
	int pulse_lenght = 11;

	if (fuel_need > 0) {
		int impulse_delay = mapf(fuel_need, 1, 100, 1000, 90);

		if (pulsing == 0 && millis() > timer + impulse_delay) {
			timer = millis();
			pulsing = 1;
		}
		if (pulsing == 1 && millis() > timer + pulse_lenght) {
			timer = millis();
			pulsing = 0;
		}
	} else {
		pulsing = 0;
	}
	analogWrite(fuel_pump_pin, 255 * pulsing);
}

void burn_fan() {
// the webasto fan runs on 10v so we need to that into account : pwm average of 14v for 10v = 0.7, 255 * 0.7 = 179
	int percent_map = mapf(fan_speed, 0, 100, 0, 179);  // pwm average of 14v for 10v = 0.7, 255 * 0.7 = 179
	analogWrite(burn_fan_pin, percent_map);
}

void glow_plug() {  // just turn the plug on if glow_time > 0, and decrement glow_time every second
	static unsigned long timer;
	if (millis() < timer || glow_time == 0) {
		timer = millis();
	}

	if (millis() < timer + glow_time * 1000) {
		digitalWrite(glow_plug_pin, HIGH);
	} else {
		digitalWrite(glow_plug_pin, LOW);
		glow_time = 0;
	}
}

void water_pump() {
	water_pump_speed = LOW;

	if (water_temp > 20) {
		water_pump_speed = 100;
	}

	if (water_temp >= run_pump_min) {
		water_pump_speed = HIGH;
	}

	digitalWrite(water_pump_pin, water_pump_speed);
}

int get_temp(int index) {
	int t_temp;
	// DANGER. Dont wait for conversion makes sometimes wrong reads
	sensor[index].setWaitForConversion(false);
	sensor[index].requestTemperatures();
	sensor[index].setWaitForConversion(true);

	t_temp = sensor[index].getTempCByIndex(0);

	if (t_temp > 200 || t_temp < -100) {  //we check here a wrong read, and make a "correction"
		if (index == water_input_index) {
			t_temp = water_temp;
		} else {
			t_temp = exhaust_temp;
		}

	}

	return t_temp;

}

float mapf(float x, float in_min, float in_max, float out_min, float out_max) {
// the perfect map fonction, with constraining and float handling
	x = constrain(x, in_min, in_max);
	return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void loop() {  // runs over and over again, calling the functions one by one

	wdt_enable(WDTO_1S);  //watchdog
	temp_data();
	control();
	shower_void();
	webasto();
	wdt_reset();

}
