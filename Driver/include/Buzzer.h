
#pragma once

#include <stdio.h>
#include <driver/ledc.h>
#include <esp_timer.h>
#include <esp_err.h>

enum Note
{
	B0 = 31,
	C1 = 33,
	CS1 = 35,
	D1 = 37,
	DS1 = 39,
	E1 = 41,
	F1 = 44,
	FS1 = 46,
	G1 = 49,
	GS1 = 52,
	A1 = 55,
	AS1 = 58,
	B1 = 62,
	C2 = 65,
	CS2 = 69,
	D2 = 73,
	DS2 = 78,
	E2 = 82,
	F2 = 87,
	FS2 = 93,
	G2 = 98,
	GS2 = 104,
	A2 = 110,
	AS2 = 117,
	B2 = 123,
	C3 = 131,
	CS3 = 139,
	D3 = 147,
	DS3 = 156,
	E3 = 165,
	F3 = 175,
	FS3 = 185,
	G3 = 196,
	GS3 = 208,
	A3 = 220,
	AS3 = 233,
	B3 = 247,
	C4 = 262,
	CS4 = 277,
	D4 = 294,
	DS4 = 311,
	E4 = 330,
	F4 = 349,
	FS4 = 370,
	G4 = 392,
	GS4 = 415,
	A4 = 440,
	AS4 = 466,
	B4 = 494,
	C5 = 523,
	CS5 = 554,
	D5 = 587,
	DS5 = 622,
	E5 = 659,
	F5 = 698,
	FS5 = 740,
	G5 = 784,
	GS5 = 831,
	A5 = 880,
	AS5 = 932,
	B5 = 988,
	C6 = 1047,
	CS6 = 1109,
	D6 = 1175,
	DS6 = 1245,
	E6 = 1319,
	F6 = 1397,
	FS6 = 1480,
	G6 = 1568,
	GS6 = 1661,
	A6 = 1760,
	AS6 = 1865,
	B6 = 1976,
	C7 = 2093,
	CS7 = 2217,
	D7 = 2349,
	DS7 = 2489,
	E7 = 2637,
	F7 = 2794,
	FS7 = 2960,
	G7 = 3136,
	GS7 = 3322,
	A7 = 3520,
	AS7 = 3729,
	B7 = 3951,
	C8 = 4186,
	CS8 = 4435,
	D8 = 4699,
	DS8 = 4978
};

struct Duration
{
	enum Drtn
	{
		whole = 1000000,

		octupl = whole * 8,
		large = octupl,
		maxima = octupl,
		quadrupl = whole * 4,
		longa = quadrupl,
		doubl = whole * 2,

		half = whole / 2,
		quarter = whole / 4,
		eighth = whole / 8,
		sixteenth = whole / 16,
		thirtysecond = whole / 32,
		sixtyfourth = whole / 64,
		hundredtwentyeighth = whole / 128,
		twohundredfiftysixth = whole / 256,

		o2 = half,
		o4 = quarter,
		o8 = eighth,
		o16 = sixteenth,
		o32 = thirtysecond,
		o64 = sixtyfourth,
		o128 = hundredtwentyeighth,
		o256 = twohundredfiftysixth,
	};
};

class Buzzer
{
	static constexpr ledc_timer_bit_t duty_res = LEDC_TIMER_5_BIT;
	static constexpr ledc_mode_t ledc_mode = LEDC_LOW_SPEED_MODE;
	static constexpr uint32_t duty = ((1 << duty_res) - 1) / 2; // Set duty to as close to 50% as possible

	ledc_timer_t timer = LEDC_TIMER_0;
	ledc_channel_t channel = LEDC_CHANNEL_0;
	gpio_num_t pin = GPIO_NUM_NC;

	esp_timer_handle_t periodic_timer;

	bool paused = true;
	size_t currentNote = 0;

public:
	using note_t = uint_fast16_t; // uint_fast16_t uint32_t
	using duration_t = uint32_t;

	note_t *notes = nullptr;
	duration_t *durations = nullptr;
	size_t length = 0;
	bool repeat = false;

	Buzzer(ledc_timer_t, ledc_channel_t, gpio_num_t);
	~Buzzer();
	void init();

	void loadSong(note_t *, duration_t *, size_t);
	void volumeOFF();
	void volumeON();
	void play(bool = false);

	void pause();
	void resume();

private:
	static void callback(void *);
};
