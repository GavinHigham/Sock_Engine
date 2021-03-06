#include <stdio.h>
#include <stdbool.h>
#include <signal.h>
#include <string.h>
//#include <SDL2/SDL_opengl.h>
//Lua headers
#include <lua-5.3.5/src/lua.h>
#include <lua-5.3.5/src/lauxlib.h>
#include <lua-5.3.5/src/lualib.h>
//Project headers.
#include "graphics.h"
#include "init.h"
#include "math/utility.h"
#include "input_event.h"
#include "macros.h"
//Scenes
#include "space/space_scene.h"
#include "experiments/icosphere_scene.h"
#include "experiments/proctri_scene.h"
#include "experiments/spiral_scene.h"
#include "experiments/visualizer_scene.h"
//Tests
#include "test/test_main.h"
//Configuration
#include "configuration/lua_configuration.h"

static const int MS_PER_SECOND = 1000;
static const int FRAMES_PER_SECOND = 60; //Frames per second.
//static const int MS_PER_UPDATE = MS_PER_SECOND / FRAMES_PER_SECOND;
static int wake_early_ms = 2; //How many ms early should the main loop wake from sleep to avoid oversleeping.
static bool testmode = false; //If true, skip creating the window and just run the tests.

//Average number of tight loop iterations. Global so it can be accessed from handle_event.c
int loop_iter_ave = 0;
SDL_Renderer *renderer = NULL;
SDL_Window *window = NULL;
Uint32 windowID = 0;
lua_State *L = NULL;
const char *luaconf_path = "conf.lua";
bool ffmpeg_recording = false;
FILE *ffmpeg_file;
int *ffmpeg_buffer = NULL;

//Callback for close button event.
int SDLCALL quit_event(void *userdata, SDL_Event *e)
{
	if (e->type == SDL_QUIT)
		*((sig_atomic_t *)userdata) = true;

	return 0;
}

void drain_event_queue()
{
	mousewheelreset();
	SDL_Event e;
	while (SDL_PollEvent(&e)) { //Exhaust our event queue before updating and rendering
		switch (e.type) {
		case SDL_KEYDOWN:               keyevent(e.key.keysym, (SDL_EventType)e.type); break;
		case SDL_KEYUP:                 keyevent(e.key.keysym, (SDL_EventType)e.type); break;
		case SDL_CONTROLLERAXISMOTION:  caxisevent(e); break;
		case SDL_JOYAXISMOTION:         jaxisevent(e); break;
		case SDL_JOYBUTTONDOWN:         jbuttonevent(e); break;
		case SDL_JOYBUTTONUP:           jbuttonevent(e); break;
		case SDL_MOUSEWHEEL:			mousewheelevent(e); break;
		case SDL_CONTROLLERDEVICEADDED: //Fall-through
		case SDL_JOYDEVICEADDED:        input_event_device_arrival(e.jdevice.which); break;
		case SDL_WINDOWEVENT:
			if (e.window.windowID == windowID) {
				switch (e.window.event)  {
				case SDL_WINDOWEVENT_SIZE_CHANGED:
					scene_resize(e.window.data1, e.window.data2);
					break;
				}
			}
		}
	}
}

//Signal handler that tells the renderer module to reload itself.
static void reload_signal_handler(int signo) {
	printf("Received SIGUSR1! Reloading shaders!\n");
	scene_reload();
	luaconf_run(L, luaconf_path);
}

int main(int argc, char **argv)
{
	SDL_GLContext context = NULL;
	int error = 0;

	sig_atomic_t quit = false; //Use an atomic so that changes from another thread will be seen.

	if (argc > 1 && !strcmp(argv[1], "test"))
		testmode = true;

	int result = 0;
	if ((result = SDL_Init(SDL_INIT_EVERYTHING)) != 0) {
		fprintf(stderr, "SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
		error = -1;
		return result;
	}

	L = luaL_newstate();
	luaL_openlibs(L);
	luaconf_run(L, luaconf_path);
	char *screen_title = getglob(L, "screen_title", "Creative Title");
	bool fullscreen = getglobbool(L, "fullscreen", false);
	SDL_SetRelativeMouseMode(getglobbool(L, "grab_mouse", false));
	float screen_width = getglob(L, "screen_width", 800);
	float screen_height = getglob(L, "screen_height", 600);
	char *default_scene = getglob(L, "default_scene", "icosphere_scene");

	//Skip window creation, OpenGL init, and and GLEW init in test mode.
	if (!testmode) {
		window = SDL_CreateWindow(
		screen_title,
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		screen_width,
		screen_height,
		SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

		free(screen_title);

		if (window == NULL) {
			fprintf(stderr, "Window could not be created! SDL_Error: %s\n", SDL_GetError());
			error = -2;
			goto error;
		}

		if (gl_init(&context, window)) {
			fprintf(stderr, "OpenGL could not be initiated!\n");
			return -1;
		}
	}

	result = engine_init();
	if (result < 0) {
		fprintf(stderr, "Something went wrong in init_engine! Aborting.\n");
		return -1;
	}

	if (testmode) {
		error = test_main(argc, argv);
		goto error;
	}
	
	//When we receive SIGUSR1, reload the scene.
	if (signal(SIGUSR1, reload_signal_handler) == SIG_ERR) {
		fprintf(stderr, "An error occurred while setting a signal handler.\n");
	}

	struct game_scene *scenes[] = {
		&space_scene,
		&proctri_scene,
		&spiral_scene,
		&icosphere_scene,
		&visualizer_scene
	};

	//Scenes usually depend on OpenGL being init'd.
	char *chosen_scene = (argc > 1) ? argv[1] : default_scene;
	for (int i = 0; i < LENGTH(scenes); i++)
		if (!strcmp(chosen_scene, scenes[i]->name))
			scene_set(scenes[i]);

	free(default_scene);

	scene_resize(screen_width, screen_height);

	SDL_AddEventWatch(quit_event, &quit);

	if (fullscreen)
		SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);

	windowID = SDL_GetWindowID(window);
	Uint32 last_swap_timestamp = SDL_GetTicks();
	int loop_iter = 0;
	while (!quit) { //Loop until quit
		loop_iter++; //Count how many times we loop per frame.

		drain_event_queue();

		int frame_time_ms = MS_PER_SECOND/FRAMES_PER_SECOND;
		Uint32 since_update_ms = (SDL_GetTicks() - last_swap_timestamp);

		if (since_update_ms >= frame_time_ms - 1) {
				//Since user input is handled above, game state is "locked" when we enter this block.
				last_swap_timestamp = SDL_GetTicks();
		 		SDL_GL_SwapWindow(window); //Display a new screen to the user every 16 ms, on the dot.
		 		if (ffmpeg_recording && ffmpeg_buffer && ffmpeg_file) {
		 			int width = getglob(L, "screen_width", 800), height = getglob(L, "screen_height", 600);
			 		glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, ffmpeg_buffer);
			 		fwrite(ffmpeg_buffer, sizeof(int)*width*height, 1, ffmpeg_file);
			 		printf(".");
		 		}
				scene_update(1.0/FRAMES_PER_SECOND); //At 16 ms intervals, begin an update. HOPEFULLY DOESN'T TAKE MORE THAN 16 MS.
				scene_render(); //This will be a picture of the state as of (hopefully exactly) 16 ms ago.
				//Get a rolling average of the number of tight loop iterations per frame.
				loop_iter_ave = (loop_iter_ave + loop_iter)/2; //Average the current number of loop iterations with the average.
				loop_iter = 0;

				//Needs to be done before the call to SDL_PollEvent (which implicitly calls SDL_PumpEvents)
				//WARNING: This modifies the input state.
				//Done here because there are sometimes issues detecting "pressed" edges from rapid keypresses if it's placed after the SDL_Delay
				input_event_save_prev_key_state();
		} else if ((frame_time_ms - since_update_ms) > wake_early_ms) { //If there's more than wake_early_ms milliseconds left...
			SDL_Delay(frame_time_ms - since_update_ms - wake_early_ms); //Sleep up until wake_early_ms milliseconds left. (Busywait the rest)
		}
	}

	// From http://gameprogrammingpatterns.com/game-loop.html
	// Uint32 previous = SDL_GetTicks();
	// double lag = 0.0;
	// while (!quit)
	// {
	// 	Uint32 current = SDL_GetTicks();
	// 	Uint32 elapsed = current - previous;
	// 	previous = current;
	// 	lag += elapsed;

	// 	drain_event_queue();

	// 	while (lag >= MS_PER_UPDATE)
	// 	{
	// 		update(MS_PER_UPDATE/MS_PER_SECOND);
	// 		lag -= MS_PER_UPDATE;
	// 	}
	// 	render();
	// 	SDL_GL_SwapWindow(window);
	// }

error:
	scene_set(NULL);
	SDL_GL_DeleteContext(context);
	SDL_DestroyWindow(window);
	engine_deinit();
	return error;
}
