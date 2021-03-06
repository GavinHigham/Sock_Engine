#include "controllable.h"
#include "physical.h"
#include <stdio.h>
#include <math.h>

controllable_callback(noop_control)
{
	//Do nothing.
}

//TODO(Gavin): Move this to its own source file with other "ship" components
controllable_callback(ship_control)
{
	Physical ship = *entity->physical;
	input = controller_input_apply_threshold(input, 0.005);
	float speed = 10;

	/* TODO
	Jerk should be nonzero when a boost button is pressed.
	It adds to acceleration while the button is pressed, otherwise acceleration is normal.
	*/


	//Set the ship's acceleration using the controller axes.
	vec3 dir = mat3_multvec(ship.position.a, (vec3){
		 input.leftx,
		-input.lefty,
		buttons[INPUT_BUTTON_L2] - buttons[INPUT_BUTTON_R2]});//speed * (input.rtrigger - input.ltrigger)});

	if (buttons[INPUT_BUTTON_A])
		ship.acceleration.t += 300 * dir;
	else
		ship.acceleration.t = speed * dir;

	//Angular velocity is currently determined by how much each axis is deflected.
	float a1 = -input.rightx / 100;
	float a2 = input.righty / 100;
	ship.velocity.a = mat3_rot(mat3_rotmat(0, 0, 1, sin(a1), cos(a1)), 1, 0, 0, sin(a2), cos(a2));

	if (buttons[INPUT_BUTTON_Y])
		ship.velocity.a = (mat3)MAT3_IDENT;

	if (buttons[INPUT_BUTTON_X]) {
		ship.acceleration.t = (vec3){0, 0, 0};
		ship.velocity.t = (vec3){0, 0, 0};
	}

	{
		//This part should be spliced out into the physical component.
			//Add our acceleration to our velocity to change our speed.
			ship.velocity.t += ship.acceleration.t;

			//Rotate the ship by applying the angular velocity to the angular orientation.
			ship.position.a = mat3_mult(ship.position.a, ship.velocity.a);

			//Move the ship by applying the velocity to the position.
			if (buttons[INPUT_BUTTON_B])
				ship.position.t += (10000 * ship.acceleration.t);
			else
				ship.position.t += ship.velocity.t;

			bpos_split_fix(&ship.position.t, &ship.origin);
	}

	//Commented out because I keep hitting T by mistake.
	//Useful for debug teleportation.
	// if (key_state[SDL_SCANCODE_T]) {
	// 	printf("Ship is at ");
	// 	qvec3_print(ship.origin);
	// 	vec3_print(ship.position.t);
	// 	puts(", where would you like to teleport?");
	// 	float x, y, z;
	// 	int_fast64_t sx, sy, sz;
	// 	scanf("%lli %lli %lli %f %f %f", &sx, &sy, &sz, &x, &y, &z);
	// 	ship.position.t = (vec3){x, y, z};
	// 	ship.origin = (bpos_origin){sx, sy, sz};
	// 	bpos_split_fix(&ship.position.t, &ship.origin);
	// }

	*entity->physical = ship;
}