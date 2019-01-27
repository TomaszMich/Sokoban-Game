#include <stdio.h>
#include <string.h>

extern "C" {
#include"./sdl-2.0.7/include/SDL.h"
#include"./sdl-2.0.7/include/SDL_main.h"
}

#define TRUE            1
#define FALSE           0
#define SCREEN_WIDTH	500
#define SCREEN_HEIGHT	560
#define MARGIN_LEFT     80
#define MARGIN_TOP      100
#define SIZE_OF_GAME_X  8
#define SIZE_OF_GAME_Y  9
#define BLOCK           48

// draw a text txt on surface screen, starting from the point (x, y)
// charset is a 128x128 bitmap containing character images
void DrawString(SDL_Surface *screen, int x, int y, const char *text,
                SDL_Surface *charset) {
	int px, py, c;
	SDL_Rect s, d;
	s.w = 8;
	s.h = 8;
	d.w = 8;
	d.h = 8;
	while(*text) {
		c = *text & 255;
		px = (c % 16) * 8;
		py = (c / 16) * 8;
		s.x = px;
		s.y = py;
		d.x = x;
		d.y = y;
		SDL_BlitSurface(charset, &s, screen, &d);
		x += 8;
		text++;
		};
	};


// draw a surface sprite on a surface screen in point (x, y)
// (x, y) is the center of sprite on screen
void DrawSurface(SDL_Surface *screen, SDL_Surface *sprite, int x, int y) {
	SDL_Rect dest;
	dest.x = x - sprite->w / 2;
	dest.y = y - sprite->h / 2;
	dest.w = sprite->w;
	dest.h = sprite->h;
	SDL_BlitSurface(sprite, NULL, screen, &dest);
	};


// draw a single pixel
void DrawPixel(SDL_Surface *surface, int x, int y, Uint32 color) {
	int bpp = surface->format->BytesPerPixel;
	Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;
	*(Uint32 *)p = color;
	};



// draw a vertical (when dx = 0, dy = 1) or horizontal (when dx = 1, dy = 0) line
void DrawLine(SDL_Surface *screen, int x, int y, int l, int dx, int dy, Uint32 color) {
	for(int i = 0; i < l; i++) {
		DrawPixel(screen, x, y, color);
		x += dx;
		y += dy;
		};
	};


// draw a rectangle of size l by k
void DrawRectangle(SDL_Surface *screen, int x, int y, int l, int k,
                   Uint32 outlineColor, Uint32 fillColor) {
	int i;
	DrawLine(screen, x, y, k, 0, 1, outlineColor);
	DrawLine(screen, x + l - 1, y, k, 0, 1, outlineColor);
	DrawLine(screen, x, y, l, 1, 0, outlineColor);
	DrawLine(screen, x, y + k - 1, l, 1, 0, outlineColor);
	for(i = y + 1; i < y + k - 1; i++)
		DrawLine(screen, x + 1, i, l - 2, 1, 0, fillColor);
	};

// free all surfaces, arrays
void FreeAll(SDL_Surface *charset, SDL_Surface *screen, SDL_Surface *mario, SDL_Surface *crate, SDL_Surface *wall, SDL_Surface *goal, SDL_Texture *scrtex, SDL_Window *window, SDL_Renderer *renderer, char **table, char **goals)
{
	for (int i = 0; i < SIZE_OF_GAME_Y; i++)
	{
		delete[] table[i];
		delete[] goals[i];
	}
	delete[] table;
	delete[] goals;

	if (charset != NULL)
		SDL_FreeSurface(charset);
	if (mario != NULL)
		SDL_FreeSurface(mario);
	if (crate != NULL)
		SDL_FreeSurface(crate);
	if (wall != NULL)
		SDL_FreeSurface(wall);
	if (goal != NULL)
		SDL_FreeSurface(goal);

	SDL_FreeSurface(screen);
	SDL_DestroyTexture(scrtex);
	SDL_DestroyWindow(window);
	SDL_DestroyRenderer(renderer);
	SDL_Quit();
}

//create table array
char** CreateTable()
{
	char** table = new char*[SIZE_OF_GAME_Y];
	for (int i = 0; i < SIZE_OF_GAME_Y; i++)
	{
		table[i] = new char[SIZE_OF_GAME_X];
	}
	return table;
}

//create array with info where goals are alocated
char** CreateGoalsArray(char **table)
{
	char** goals = new char*[SIZE_OF_GAME_Y];
	for (int i = 0; i < SIZE_OF_GAME_Y; i++)
	{
		goals[i] = new char[SIZE_OF_GAME_X];
	}

	for (int i = 0; i < SIZE_OF_GAME_Y; i++)
	{
		for (int j = 0; j < SIZE_OF_GAME_X; j++)
		{
			if (table[i][j] == 'g')
				goals[i][j] = 'g';
		}
	}

	return goals;
}

void LoadTableFromFile(char **table)
{
	FILE *file = fopen("newgame.txt", "r");
	char sign;
	for (int i = 0; i < SIZE_OF_GAME_Y; i++)
	{
		for (int j = 0; j < SIZE_OF_GAME_X; j++)
		{
			sign = fgetc(file);
			table[i][j] = sign;
		}
	}
	fclose(file);
}

//draw all object from table array into screen surface
void DrawObjects(char **table, char **goals, SDL_Surface *screen, SDL_Surface *mario, SDL_Surface *crate, SDL_Surface *wall, SDL_Surface *goal)
{
	int stepX = 0, stepY = 0;
	for (int i = 0; i < SIZE_OF_GAME_Y; i++)
	{
		stepX = 0;
		for (int j = 0; j < SIZE_OF_GAME_X; j++)
		{
			if (goals[i][j] == 'g') //goal
				DrawSurface(screen, goal, MARGIN_LEFT + stepX, MARGIN_TOP + stepY);
			if (table[i][j] == 'p') //player
				DrawSurface(screen, mario, MARGIN_LEFT + stepX, MARGIN_TOP + stepY);
			else if (table[i][j] == 'c') //crate
				DrawSurface(screen, crate, MARGIN_LEFT + stepX, MARGIN_TOP + stepY);
			else if (table[i][j] == 'w') //wall
				DrawSurface(screen, wall, MARGIN_LEFT + stepX, MARGIN_TOP + stepY);

			stepX += BLOCK;
		}
		stepY += BLOCK;
	}
}

//change position of the player and crate
void Movement(char **table, int moveOnX, int moveOnY)
{
	int oldX, oldY;

	//find current position of player *** BETTER WOULD BE TO STORE THIS POSITION OUTSITE OF THE FUNCTION
	for (int i = 0; i < SIZE_OF_GAME_Y; i++)
	{
		for (int j = 0; j < SIZE_OF_GAME_X; j++)
		{
			if (table[i][j] == 'p') //'p' as player
			{
				oldX = j;
				oldY = i;
			}
		}
	}

	//if next object isn't wall or crate, move player
	if (table[oldY + moveOnY][oldX + moveOnX] != 'w' && table[oldY + moveOnY][oldX + moveOnX] != 'c') //'w' as wall, 'c' as crate
	{
		table[oldY][oldX] = 'e'; //'e' as empty
		table[oldY + moveOnY][oldX + moveOnX] = 'p';
	}
	//if on destined position stands crate and next field is free, move player and the crate
	else if (table[oldY + moveOnY][oldX + moveOnX] == 'c' && table[oldY + 2 * moveOnY][oldX + 2 * moveOnX] != 'w' && table[oldY + 2 * moveOnY][oldX + 2 * moveOnX] != 'c')
	{
		table[oldY][oldX] = 'e';
		table[oldY + moveOnY][oldX + moveOnX] = 'p';
		table[oldY + 2 * moveOnY][oldX + 2 * moveOnX] = 'c';
	}

}

//check if level is complited
int WinDetection(char **table, char **goals)
{
	for (int i = 0; i < SIZE_OF_GAME_Y; i++)
	{
		for (int j = 0; j < SIZE_OF_GAME_X; j++)
		{
			if (goals[i][j] == 'g' && table[i][j] != 'c')
			{
				return FALSE;
			}
		}
	}
	return TRUE;
}

// main
#ifdef __cplusplus
extern "C"
#endif
int main(int argc, char **argv) {
	int t1, t2, quit, frames, rc, win;
	double delta, worldTime, fpsTimer, fps;
	char **table = CreateTable(); //playfield
	SDL_Event event;
	SDL_Surface *screen, *charset;
	SDL_Surface *wall, *mario, *crate, *goal;
	SDL_Texture *scrtex;
	SDL_Window *window;
	SDL_Renderer *renderer;

	LoadTableFromFile(table); //filling table array with data about objects types

	char **goals = CreateGoalsArray(table);

	if(SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		printf("SDL_Init error: %s\n", SDL_GetError());
		return 1;
		}

	// tryb pe³noekranowy / fullscreen mode
//	rc = SDL_CreateWindowAndRenderer(0, 0, SDL_WINDOW_FULLSCREEN_DESKTOP,
//	                                 &window, &renderer);
	rc = SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, 0,
	                                 &window, &renderer);
	if(rc != 0) {
		SDL_Quit();
		printf("SDL_CreateWindowAndRenderer error: %s\n", SDL_GetError());
		return 1;
		};
	
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
	SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

	SDL_SetWindowTitle(window, "Sokoban game - Tomasz Michalski 171890");


	screen = SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, 32,
	                              0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);

	scrtex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
	                           SDL_TEXTUREACCESS_STREAMING,
	                           SCREEN_WIDTH, SCREEN_HEIGHT);


	// wy³¹czenie widocznoœci kursora myszy
	SDL_ShowCursor(SDL_DISABLE);

	// wczytanie obrazka cs8x8.bmp
	charset = SDL_LoadBMP("./cs8x8.bmp");
	if(charset == NULL) {
		printf("SDL_LoadBMP(cs8x8.bmp) error: %s\n", SDL_GetError());
		FreeAll(charset, screen, mario, crate, wall, goal, scrtex, window, renderer, table, goals);
		return 1;
		};
	SDL_SetColorKey(charset, true, 0x000000);

	//load image wall.bmp
	wall = SDL_LoadBMP("./wall.bmp");
	if (wall == NULL) {
		printf("SDL_LoadBMP(wall.bmp) error: %s\n", SDL_GetError());
		FreeAll(charset, screen, mario, crate, wall, goal, scrtex, window, renderer, table, goals);
		return 1;
	};

	mario = SDL_LoadBMP("./mario.bmp");
	if (wall == NULL) {
		printf("SDL_LoadBMP(mario.bmp) error: %s\n", SDL_GetError());
		FreeAll(charset, screen, mario, crate, wall, goal, scrtex, window, renderer, table, goals);
		return 1;
	};

	crate = SDL_LoadBMP("./crate.bmp");
	if (wall == NULL) {
		printf("SDL_LoadBMP(crate.bmp) error: %s\n", SDL_GetError());
		FreeAll(charset, screen, mario, crate, wall, goal, scrtex, window, renderer, table, goals);
		return 1;
	};

	goal = SDL_LoadBMP("./goal.bmp");
	if (wall == NULL) {
		printf("SDL_LoadBMP(goal.bmp) error: %s\n", SDL_GetError());
		FreeAll(charset, screen, mario, crate, wall, goal, scrtex, window, renderer, table, goals);
		return 1;
	};

	char text[128];
	int czarny = SDL_MapRGB(screen->format, 0x00, 0x00, 0x00);
	int zielony = SDL_MapRGB(screen->format, 0x00, 0xFF, 0x00);
	int czerwony = SDL_MapRGB(screen->format, 0xFF, 0x00, 0x00);
	int niebieski = SDL_MapRGB(screen->format, 0x11, 0x11, 0xCC);
	int background = SDL_MapRGB(screen->format, 233, 166, 99);

	t1 = SDL_GetTicks();

	frames = 0;
	fpsTimer = 0;
	fps = 0;
	quit = 0;
	worldTime = 0;

	while(!quit) {
		t2 = SDL_GetTicks();
		win = WinDetection(table, goals);

		// here t2-t1 is the time in milliseconds since
		// the last screen was drawn
		// delta is the same time in seconds
		delta = (t2 - t1) * 0.001;
		t1 = t2;

		//stop timer if level is complete
		if (!win)
			worldTime += delta;

		SDL_FillRect(screen, NULL, background);

		DrawObjects(table, goals, screen, mario, crate, wall, goal);

		fpsTimer += delta;
		if(fpsTimer > 0.5) {
			fps = frames * 2;
			frames = 0;
			fpsTimer -= 0.5;
			};

		if (win) //info text in case of winning
		{
			DrawRectangle(screen, 4, SCREEN_HEIGHT / 2, SCREEN_WIDTH - 8, 54, czerwony, niebieski);

			sprintf(text, "GRATULACJE WYGRALES!");
			DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, SCREEN_HEIGHT / 2 + 6, text, charset);

			sprintf(text, "Twoj czas to: %.1lf s", worldTime);
			DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, SCREEN_HEIGHT / 2 + 22, text, charset);

			sprintf(text, "Esc - wyjscie, n - nowa gra");
			DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, SCREEN_HEIGHT / 2 + 38, text, charset);

		}

		// tekst informacyjny / info text
		DrawRectangle(screen, 4, 4, SCREEN_WIDTH - 8, 36, czerwony, niebieski);

		sprintf(text, "Sokoban - projekt, czas trwania gry = %.1lf s  %.0lf klatek / s", worldTime, fps);
		DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 10, text, charset);

		sprintf(text, "Esc - wyjscie, n - nowa gra, \030\032\031\033 - sterowanie");
		DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 26, text, charset);

		SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
//		SDL_RenderClear(renderer);
		SDL_RenderCopy(renderer, scrtex, NULL, NULL);
		SDL_RenderPresent(renderer);

		// handling of events (if there were any)
		while(SDL_PollEvent(&event)) {
			switch(event.type) {
				case SDL_KEYDOWN:
					if (event.key.keysym.sym == SDLK_ESCAPE) quit = 1;
					else if (event.key.keysym.sym == SDLK_n) {
						LoadTableFromFile(table);
						worldTime = 0;
					}
					else if (event.key.keysym.sym == SDLK_UP && !win) Movement(table, 0, -1);
					else if (event.key.keysym.sym == SDLK_DOWN && !win) Movement(table, 0, 1);
					else if (event.key.keysym.sym == SDLK_RIGHT && !win) Movement(table, 1, 0);
					else if (event.key.keysym.sym == SDLK_LEFT && !win) Movement(table, -1, 0);
					break;
				case SDL_QUIT:
					quit = 1;
					break;
				};
			};
		frames++;
		};

	FreeAll(charset, screen, mario, crate, wall, goal, scrtex, window, renderer, table, goals);

	return 0;
	}