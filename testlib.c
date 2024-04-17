#include "primlib.h"
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define WIDTH 10
#define HEIGHT 20
#define BLOCK_WIDTH ((gfx_screenWidth() / 2) / WIDTH)
#define BLOCK_HEIGHT (gfx_screenHeight() / HEIGHT)
#define FRAMES 80
#define POSSIBLE 1
#define IMPOSSIBLE 0
#define ROT_AXIS 2
#define PLACED_BLOCK 3
#define MOVING_BLOCK 1
#define QUIT 1

int score; 
int next_array[4][4];
int coords_next[4][4][2]; // just (x1, y1) for every square
int current_block;
int current_rot;
char pieces[7 /*kind */][4 /* rotation */][4][4] = {
	/* square */
	{{{2, 1, 0, 0}, {1, 1, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}},
	 {{2, 1, 0, 0}, {1, 1, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}},
	 {{2, 1, 0, 0}, {1, 1, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}},
	 {{2, 1, 0, 0}, {1, 1, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}},
	/* I */
	{{{1, 2, 1, 1}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}},
	 {{1, 0, 0, 0}, {2, 0, 0, 0}, {1, 0, 0, 0}, {1, 0, 0, 0}},
	 {{1, 1, 2, 1}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}},
	 {{1, 0, 0, 0}, {1, 0, 0, 0}, {2, 0, 0, 0}, {1, 0, 0, 0}}},
	/* L */
	{{{1, 0, 0, 0}, {2, 0, 0, 0}, {1, 1, 0, 0}, {0, 0, 0, 0}},
	 {{1, 2, 1, 0}, {1, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}},
	 {{1, 1, 0, 0}, {0, 2, 0, 0}, {0, 1, 0, 0}, {0, 0, 0, 0}},
	 {{0, 0, 1, 0}, {1, 2, 1, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}},
	/* L mirrored */
	{{{0, 1, 0, 0}, {0, 2, 0, 0}, {1, 1, 0, 0}, {0, 0, 0, 0}},
	 {{1, 0, 0, 0}, {1, 2, 1, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}},
	 {{1, 1, 0, 0}, {2, 0, 0, 0}, {1, 0, 0, 0}, {0, 0, 0, 0}},
	 {{1, 2, 1, 0}, {0, 0, 1, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}},
	/* N */
	{{{0, 1, 0, 0}, {2, 1, 0, 0}, {1, 0, 0, 0}, {0, 0, 0, 0}},
	 {{1, 2, 0, 0}, {0, 1, 1, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}},
	 {{0, 1, 0, 0}, {1, 2, 0, 0}, {1, 0, 0, 0}, {0, 0, 0, 0}},
	 {{1, 1, 0, 0}, {0, 2, 1, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}},
	/* N mirrored */
	{{{1, 0, 0, 0}, {2, 1, 0, 0}, {0, 1, 0, 0}, {0, 0, 0, 0}},
	 {{0, 2, 1, 0}, {1, 1, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}},
	 {{1, 0, 0, 0}, {1, 2, 0, 0}, {0, 1, 0, 0}, {0, 0, 0, 0}},
	 {{0, 1, 1, 0}, {1, 2, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}},
	/* T */
	{{{1, 0, 0, 0}, {2, 1, 0, 0}, {1, 0, 0, 0}, {0, 0, 0, 0}},
	 {{1, 2, 1, 0}, {0, 1, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}},
	 {{0, 1, 0, 0}, {1, 2, 0, 0}, {0, 1, 0, 0}, {0, 0, 0, 0}},
	 {{0, 1, 0, 0}, {1, 2, 1, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}}};
int game_array[HEIGHT][WIDTH];
int coords_table[HEIGHT][WIDTH][2]; // just (x1, y1) for every square

void draw_background()
{
	char string[10];
	gfx_filledRect(0, 0, gfx_screenWidth() - 1, gfx_screenHeight() - 1, BLACK);
	gfx_line(gfx_screenWidth() / 4, 0, gfx_screenWidth() / 4,
			 gfx_screenHeight(), CYAN);
	gfx_line(3 * gfx_screenWidth() / 4, 0, 3 * gfx_screenWidth() / 4,
			 gfx_screenHeight(), YELLOW);
	gfx_line(gfx_screenWidth() / 4, BLOCK_HEIGHT * 4, 3 * gfx_screenWidth() / 4, BLOCK_HEIGHT * 4, RED);
	sprintf(string, "%d", score);
	gfx_textout(gfx_screenWidth() - 50, 50, string ,RED);
}

void initial_setup()
{
	for (int x = 0; x < HEIGHT; x++) {
		for (int y = 0; y < WIDTH; y++) {
			coords_table[x][y][0] = gfx_screenWidth() / 4 + (y * BLOCK_WIDTH);
			coords_table[x][y][1] = x * BLOCK_HEIGHT;
		}
	}
	for (int x = 0; x < 4; x++) {
		for (int y = 0; y < 4; y++) {
			coords_next[x][y][0] = 75 + y * BLOCK_WIDTH;
			coords_next[x][y][1] = 50 + x * BLOCK_HEIGHT;
		}
	}
}

void assign_color(int number, int X, int Y)
{
	enum color block_color;
	if (number == MOVING_BLOCK) {
		block_color = GREEN;
	}
	else if (number == ROT_AXIS) {
		block_color = YELLOW;
	}
	else if (number == PLACED_BLOCK) {
		block_color = RED;
	}

	if (number != 0) {
		gfx_filledRect(coords_table[X][Y][0], coords_table[X][Y][1],
					   coords_table[X][Y][0] + BLOCK_WIDTH,
					   coords_table[X][Y][1] + BLOCK_HEIGHT, block_color);
		gfx_rect(coords_table[X][Y][0], coords_table[X][Y][1],
				 coords_table[X][Y][0] + BLOCK_WIDTH,
				 coords_table[X][Y][1] + BLOCK_HEIGHT, MAGENTA);
	}
}

void display_next()
{
	for (int X = 0; X < 4; X++) {
		for (int Y = 0; Y < 4; Y++) {
			enum color block_color;
			if (next_array[X][Y] == MOVING_BLOCK) {
				block_color = GREEN;
			}
			else if (next_array[X][Y] == ROT_AXIS) {
				block_color = YELLOW;
			}

			if (next_array[X][Y] != 0) {
				gfx_filledRect(coords_next[X][Y][0], coords_next[X][Y][1],
							   coords_next[X][Y][0] + BLOCK_WIDTH,
							   coords_next[X][Y][1] + BLOCK_HEIGHT,
							   block_color);
				gfx_rect(coords_next[X][Y][0], coords_next[X][Y][1],
						 coords_next[X][Y][0] + BLOCK_WIDTH,
						 coords_next[X][Y][1] + BLOCK_HEIGHT, MAGENTA);
			}
		}
	}
}

void draw()
{
	draw_background();
	for (int x = 0; x < HEIGHT; x++) {
		for (int y = 0; y < WIDTH; y++) {
			assign_color(game_array[x][y], x, y);
		}
	}
	display_next();
	gfx_updateScreen();
}

int get_next()
{
	int next_kind = rand() % 7;
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			next_array[i][j] = pieces[next_kind][0][j][i];
		}
	}
	return next_kind;
}

int get_block(int next_b)
{
	int counter = 0;
	for (int x = 0; x < HEIGHT; x++) {
		for (int y = 0; y < WIDTH; y++) {
			if (game_array[x][y] == 1) {
				counter += 1;
				break;
			}
		}
	}
	if (counter == 0) {
		current_rot = 0;
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {
				game_array[i][WIDTH / 2 - 1 + j] = pieces[next_b][0][i][j];
			}
		}
		current_block = next_b;
		next_b = get_next();
		return next_b;
	}
	return next_b;
}

void block_change()
{
	for (int x = HEIGHT - 1; x >= 0; x--) {
		for (int y = WIDTH - 1; y >= 0; y--) {
			if (game_array[x][y] == MOVING_BLOCK || game_array[x][y] == ROT_AXIS) {
				game_array[x][y] = PLACED_BLOCK;
			}
		}
	}
}

int check_move_down()
{
	int counter = 0;
	for (int x = HEIGHT - 1; x >= 0; x--) {
		for (int y = WIDTH - 1; y >= 0; y--) {
			if (game_array[x][y] == MOVING_BLOCK || game_array[x][y] == ROT_AXIS) {
				if (game_array[x + 1][y] == PLACED_BLOCK) {
					counter += 1;
				}
			}
		}
	}
	return counter;
}

int check_move_left()
{
	int counter = 0;
	for (int x = HEIGHT - 1; x >= 0; x--) {
		for (int y = 0; y < WIDTH; y++) {
			if (game_array[x][y] == MOVING_BLOCK || game_array[x][y] == ROT_AXIS) {
				if (game_array[x][y - 1] == PLACED_BLOCK) {
					counter += 1;
				}
			}
		}
	}
	return counter;
}

void move_left()
{
	int move_checker = 0;
	for (int x = 0; x < HEIGHT; x++) {
		if (game_array[x][0] == MOVING_BLOCK || game_array[x][0] == ROT_AXIS) {
			move_checker += 1;
		}
	}
	if (move_checker == 0) {
		int counter = 0;
		counter = check_move_left();
		if (counter == 0) {
			for (int x = HEIGHT - 1; x >= 0; x--) {
				for (int y = 0; y < WIDTH; y++) {
					if (game_array[x][y] == MOVING_BLOCK || game_array[x][y] == ROT_AXIS) {
						game_array[x][y - 1] = game_array[x][y];
						game_array[x][y] = 0;
					}
				}
			}
		}
	}
}

int check_move_right()
{
	int counter = 0;
	for (int x = HEIGHT - 1; x >= 0; x--) {
		for (int y = WIDTH - 1; y >= 0; y--) {
			if (game_array[x][y] == MOVING_BLOCK || game_array[x][y] == ROT_AXIS) {
				if (game_array[x][y + 1] == PLACED_BLOCK) {
					counter += 1;
				}
			}
		}
	}
	return counter;
}

void move_right()
{
	int check_move = 0;
	for (int x = 0; x < HEIGHT; x++) {
		if (game_array[x][WIDTH - 1] == MOVING_BLOCK || game_array[x][WIDTH - 1] == ROT_AXIS) {
			check_move += 1;
		}
	}
	if (check_move == 0) {
		int counter = 0;
		counter = check_move_right();
		if (counter == 0) {
			for (int x = HEIGHT - 1; x >= 0; x--) {
				for (int y = WIDTH - 1; y >= 0; y--) {
					if (game_array[x][y] == MOVING_BLOCK || game_array[x][y] == ROT_AXIS) {
						game_array[x][y + 1] = game_array[x][y];
						game_array[x][y] = 0;
					}
				}
			}
		}
	}
}

void step_down()
{
	for (int x = HEIGHT - 1; x >= 0; x--) {
		for (int y = WIDTH - 1; y >= 0; y--) {
			if (game_array[x][y] == MOVING_BLOCK || game_array[x][y] == ROT_AXIS) {
				game_array[x + 1][y] = game_array[x][y];
				game_array[x][y] = 0;
			}
		}
	}
}

void check_fullrows()
{
	for (int x = HEIGHT - 1; x >= 0; x--) {
		int rowsum = 0;
		for (int y = WIDTH - 1; y >= 0; y--) {
			if (game_array[x][y] == PLACED_BLOCK) {
				rowsum += 1;
			}
			if (rowsum == WIDTH) {
				score += WIDTH;
				for (int y = WIDTH - 1; y >= 0; y--) {
					game_array[x][y] = 0;
				}
				for (int z = x; z >= 0; z--) {
					for (int y = WIDTH - 1; y >= 0; y--) {
						if (game_array[z][y] == PLACED_BLOCK) {
							game_array[z + 1][y] = game_array[z][y];
							game_array[z][y] = 0;
						}
					}
				}
			}
		}
	}
}

void move_down()
{
	int counter = 0;
	int move_controller = 0;
	for (int x = HEIGHT - 1; x >= 0; x--) {
		for (int y = WIDTH - 1; y >= 0; y--) {
			counter = 0;
			if ((game_array[x][y] == MOVING_BLOCK || game_array[x][y] == ROT_AXIS) &&
				move_controller == 0) {
				if (x + 1 == HEIGHT) {
					block_change();
					score += 2;
				}
				counter = check_move_down();

				if (counter != 0 || x + 1 == HEIGHT) {
					block_change();
					score += 2;
				}
				else {
					step_down();
					move_controller += 1;
				}
			}
		}
	}
}

void rotate(int tet_x, int tet_y, int rota_x, int rota_y)
{
	current_rot += 1;
	if (current_rot == 4) {
		current_rot = 0;
	}
	for (int x = 0; x < HEIGHT; x++) {
		for (int y = 0; y < WIDTH; y++) {
			if (game_array[x][y] != 3) {
				game_array[x][y] = 0;
			}
		}
	}

	for (int x = 0; x < 4; x++) {
		for (int y = 0; y < 4; y++) {
			if ((tet_x + x < HEIGHT) && (tet_y + y < WIDTH)) {
				if (pieces[current_block][current_rot][x][y] != 0) {
					game_array[tet_x + x][tet_y + y] =
						pieces[current_block][current_rot][x][y];
				}
			}
		}
	}
}

void check_rotate()
{
	current_rot += 1;
	if (current_rot == 4) {
		current_rot = 0;
	}
	int game_x = 0;
	int game_y = 0;
	int rot_x = 0;
	int rot_y = 0;

	int posibility = POSSIBLE;
	for (int x = 0; x < HEIGHT; x++) {
		for (int y = 0; y < WIDTH; y++) {
			if (game_array[x][y] == ROT_AXIS) {
				game_x = x;
				game_y = y;
			}
		}
	}
	for (int x = 0; x < 4; x++) {
		for (int y = 0; y < 4; y++) {
			if (pieces[current_block][current_rot][x][y] == ROT_AXIS) {
				rot_x = x;
				rot_y = y;
			}
		}
	}

	if ((game_x - rot_x < 0) || (game_y - rot_y < 0)) {
		posibility = IMPOSSIBLE;
	}
	for (int x = 0; x < 4; x++) {
		for (int y = 0; y < 4; y++) {
			int test_piece = pieces[current_block][current_rot][x][y];
			if (test_piece != 0) {
				if ((game_x - rot_x + x >= HEIGHT) || (game_y - rot_y + y >= WIDTH)) {
					posibility = IMPOSSIBLE;
				}
			}
		}
	}
	for (int x = 0; x < 4; x++) {
		for (int y = 0; y < 4; y++) {
			if (pieces[current_block][current_rot][x][y] != 0) {
				int index_x = game_x - rot_x + x;
				int index_y = game_y - rot_y + y;
				if ((index_x >= 0 && index_x < HEIGHT) && (index_y >= 0 && index_y < WIDTH)){
					if (game_array[index_x][index_y] == PLACED_BLOCK) {
						posibility = IMPOSSIBLE;
					}
				}
				else{
					posibility = IMPOSSIBLE;
				}
			}
		}
	}
	current_rot -= 1;
	if (current_rot == -1) {
		current_rot = 3;
	}
	
	if (posibility == POSSIBLE) {
		rotate(game_x - rot_x, game_y - rot_y, rot_x, rot_y);
	}
}

void drop()
{
	for (int x = 0; x < HEIGHT; x++) {
		move_down();
	}
}

void game_over_screen()
{
	gfx_filledRect(0, 0, gfx_screenWidth() - 1, gfx_screenHeight() - 1, BLACK);
	gfx_textout(gfx_screenWidth()/2 - 25, gfx_screenHeight()/2, "game over", RED);
	gfx_updateScreen();
	SDL_Delay(3000);
}

int losscheck()
{
	int counter = 0;
	for (int x = 0; x < 4; x++) {
		for (int y = 0; y < WIDTH; y++) {
			if (game_array[x][y] == PLACED_BLOCK)
			{
				counter += 1;
			}
		}
	}
	return counter;
}

int controls()
{
	int key = gfx_pollkey();
	if (key == SDLK_ESCAPE) {
		return QUIT;
	}

	if (key == SDLK_LEFT) {
		move_left();
	}
	else if (key == SDLK_RIGHT) {
		move_right();
	}
	else if (key == SDLK_DOWN) {
		drop();
	}
	else if (key == SDLK_SPACE) {
		check_rotate();
	}

	return 0;
}

int main(int argc, char* argv[])
{
	if (gfx_init()) {
		exit(3);
	}
	srand(time(NULL));
	initial_setup();
	int next_block = get_next();

	for (int z = 1; 1; z++) {
		next_block = get_block(next_block);
		draw();

		int quit = controls();
		if (quit == QUIT)
		{
			return 0;
		}

		if (z % FRAMES == 0) {
			move_down();
			z = 1;
		}
		check_fullrows();
		int loss_q = losscheck();
		if (loss_q != 0)
		{
			break;
		}
		
	}
	game_over_screen();
	return 0;
}
