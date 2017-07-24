#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/timeb.h>
#include <curses.h>
#include <fcntl.h>
#include <linux/joystick.h>

#include "servo.h"
#include "libsoc_gpio.h"

#define JS_DEV			"/dev/input/js%d"

#define MOTOR_A_IO		11
#define MOTOR_B_IO		105
#define MOTOR_A_DIR_IO		104
#define MOTOR_B_DIR_IO		106

servo servo_x, servo_y;
gpio *motor_a, *motor_b, *motor_a_dir, *motor_b_dir;

int process_event(struct js_event e)
{
        int a_val = 0, a_num = 0;
        int b_val = 0, b_num = 0;

	if (e.type == JS_EVENT_INIT) {
		b_val = e.value;
		b_num = e.number;
	} else if (e.type == JS_EVENT_BUTTON) {
		b_val = e.value;
		b_num = e.number;
	} else if (e.type == JS_EVENT_AXIS) {
		a_val = e.value;
		a_num = e.number;
	}

	if (b_num == 0) {
		libsoc_gpio_set_level(motor_a_dir, LOW);
		libsoc_gpio_set_level(motor_b_dir, LOW);

		libsoc_gpio_set_level(motor_a, b_val);
		libsoc_gpio_set_level(motor_b, b_val);
	}

	if (b_num == 1) {
		libsoc_gpio_set_level(motor_a_dir, LOW);
		libsoc_gpio_set_level(motor_b_dir, HIGH);

		libsoc_gpio_set_level(motor_a, b_val);
		libsoc_gpio_set_level(motor_b, b_val);
	}

	if (b_num == 2) {
		libsoc_gpio_set_level(motor_a_dir, HIGH);
		libsoc_gpio_set_level(motor_b_dir, HIGH);

		libsoc_gpio_set_level(motor_a, b_val);
		libsoc_gpio_set_level(motor_b, b_val);
	}

	if (b_num == 3) {
		libsoc_gpio_set_level(motor_a_dir, HIGH);
		libsoc_gpio_set_level(motor_b_dir, LOW);

		libsoc_gpio_set_level(motor_a, b_val);
		libsoc_gpio_set_level(motor_b, b_val);
	}

	if (a_num == 1) {
		//printf ("a_num:%d Angle:%d a_val:%d\n", a_num, ((180 * (a_val + 32767)) / 65534), a_val);
		s_set_angle(&servo_x, ((180 * (a_val + 32767)) / 65534));
	}

	if (a_num == 2) {
		//printf ("a_num:%d Angle:%d a_val:%d\n", a_num, ((180 * (a_val + 32767)) / 65534), a_val);
		s_set_angle(&servo_y, ((180 * (a_val + 32767)) / 65534));
	}

    return 0;
}

void servo_init() {
	/* Select Colibri standard PWM_B  */
	servo_x.chip = 1;
	servo_x.channel = 0;
	servo_x.period = 20000000;
	s_init(&servo_x);

	/* Select Colibri standard PWM_C  */
	servo_y.chip = 2;
	servo_y.channel = 0;
	servo_y.period = 20000000;
	s_init(&servo_y);

	/* Settle at initial positions */
	s_set_angle(&servo_x, 90);
	s_set_angle(&servo_y, 90);
}

void motor_init(void)
{
	motor_a		=	libsoc_gpio_request(MOTOR_A_IO, LS_GPIO_GREEDY);
	motor_b		=	libsoc_gpio_request(MOTOR_B_IO, LS_GPIO_GREEDY);
	motor_a_dir	=	libsoc_gpio_request(MOTOR_A_DIR_IO, LS_GPIO_GREEDY);
	motor_b_dir	=	libsoc_gpio_request(MOTOR_B_DIR_IO, LS_GPIO_GREEDY);

	libsoc_gpio_set_direction(motor_a, OUTPUT);
	libsoc_gpio_set_direction(motor_b, OUTPUT);
	libsoc_gpio_set_direction(motor_a_dir, OUTPUT);
	libsoc_gpio_set_direction(motor_b_dir, OUTPUT);

	libsoc_gpio_set_level(motor_a, LOW);
	libsoc_gpio_set_level(motor_b, LOW);
	libsoc_gpio_set_level(motor_a_dir, LOW);
	libsoc_gpio_set_level(motor_b_dir, LOW);
}

int main (int atgc, char **argv)
{
	int fd = 0;
	char js_dev[32] = {0};
	int js_dev_instance = 0;
	struct js_event e;

	motor_init();
	servo_init();

	sprintf(js_dev, JS_DEV, js_dev_instance);

	fd = open(js_dev, O_RDONLY);
	if (fd < 0) {
		printf("Open failed!\n");
		return -1;
	}

	while (1) {
		while (read (fd, &e, sizeof(e)) > 0)
			process_event (e);
    }

    return 0;
}
