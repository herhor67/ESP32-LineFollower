
#pragma once

// #include <stdio.h>
#include <driver/gpio.h>
#include <driver/ledc.h>
#include <esp_timer.h>
#include <esp_err.h>

using note_t = uint_fast16_t; // uint_fast16_t uint32_t
using duration_t = uint32_t;

enum Note : note_t
{
	A0 = 27,
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
	DS8 = 4978,

	ST1 = A3,
	ST15 = AS3,
	ST2 = B3,

	ST3 = C4,
	ST35 = CS4,
	ST4 = D4,
	ST45 = DS4,
	ST5 = E4,

	ST6 = F4,
	ST65 = FS4,
	ST7 = G4,
	ST75 = GS4,
	ST8 = A4,
	ST85 = AS4,
	ST9 = B4,

	ST10 = C5,
	ST105 = CS5,
	ST11 = D5,
	ST115 = DS5,
	ST12 = E5,
};

enum Duration : duration_t
{
	second = 1000 * 1000,

	x8 = second * 32,
	x4 = second * 16,
	x2 = second * 8,
	x1 = second * 4,
	o1 = second * 4,
	o2 = second * 2,
	o4 = second,
	o8 = second / 2,
	o16 = second / 4,
	o32 = second / 8,
	o64 = second / 16,
	o128 = second / 32,
	o256 = second / 64,

	octuplenote = x8,
	largenote = x8,
	maximanote = x8,
	quadruplenote = x4,
	longanote = x4,
	doublenote = x2,
	wholenote = x1,
	halfnote = o2,
	quarternote = o4,
	eighthnote = o8,
	sixteenthnote = o16,
	thirtysecondnote = o32,
	sixtyfourthnote = o64,
	hundredtwentyeighthnote = o128,
	twohundredfiftysixthnote = o256,
};

class Buzzer
{
	static constexpr ledc_timer_bit_t DUTYRES = LEDC_TIMER_12_BIT;
	static constexpr ledc_mode_t LEDCMODE = LEDC_LOW_SPEED_MODE;
	static constexpr uint32_t DUTY = 1 << (DUTYRES - 1); // Set DUTY to as close to 50% as possible ((1 << (DUTYRES)) - 1) / 2

	gpio_num_t pin = GPIO_NUM_NC;
	ledc_timer_t timer;
	ledc_channel_t channel;

	esp_timer_handle_t periodic_timer;

	uint64_t tempo = 60 * static_cast<duration_t>(Duration::o4);

	bool midsong = false;
	bool paused = false;
	size_t currentNote = 0;
	uint64_t remainingTime = -1;

public:
	note_t *notes = nullptr;
	duration_t *durations = nullptr;
	size_t length = 0;
	bool repeat = false;

	Buzzer(gpio_num_t, ledc_timer_t = static_cast<ledc_timer_t>(LEDC_TIMER_MAX - 1), ledc_channel_t = static_cast<ledc_channel_t>(LEDC_CHANNEL_MAX - 1));
	~Buzzer();

	void setTempo(uint32_t, Duration);

	void loadSong(note_t *, duration_t *, size_t);
	void volumeOFF();
	void volumeON();

	void start(bool = false);
	void pause();
	void resume();
	void end();
	void restart();

private:
	bool checkCurrentNote();
	void playCurrentNote();

	static void callback(void *);
};
