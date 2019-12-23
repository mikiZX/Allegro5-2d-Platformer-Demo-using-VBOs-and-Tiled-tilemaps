/*
VBO & Tilemap 
*/

#include <stdio.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_opengl.h>
#include <allegro5/allegro_native_dialog.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
#include "main2.h"


int  load_tilemap_layer(TILEMAP_LAYER *tml, char *map_filename, char *bitmap_filename)
{
			// get map size (w & h)
			int col, row;
			col = row = 0;

			FILE *fp = fopen(map_filename, "r");
			if (!fp) {  // validate file open for reading 
				fprintf(stderr, "error: file open failed '%s'.\n", map_filename);
				return -1;
			}

			// calculate dimensions of Tiled CSV map
			char repeat = 1;
			int len = 0;
			do
			{
				char fc = fgetc(fp);
				if (fc == ',') len++;
				else if (fc == 10) { col = len+1; len = 0; row++; }
				if (fc == EOF) repeat = 0;

			} while (repeat);

			tml->map_w = col;
			tml->map_h = row;

			rewind(fp);

			// reserve memory space for our tilemap
			tml->map = (int **)malloc(row * sizeof(int *));
			if (!tml->map) {
				printf("error mem\n"); al_uninstall_system(); exit(-1);
			}
			for (int i = 0; i < row; i++)
			{
				tml->map[i] = (int*)malloc(col * sizeof(int));
				if (!tml->map[i]) {
					printf("error mem\n"); al_uninstall_system(); exit(-1);
				}

			}

			int *temp_map = malloc(col*row * sizeof(int));
			int *writepos;
			writepos = temp_map;

			// load Tiled CSV map
			repeat = 1;
			len = 1;
			int val = 0;
			do
			{
				char fc = fgetc(fp);
				if (fc=='-') { fc = fgetc(fp); fc = fgetc(fp); }  // skip empty tile
				if (fc == 13 || fc == 10 || fc == ',' || fc==EOF)
					val = val;
				else
					val = val * 10 + fc - '0';
				if (fc == ',' || fc==EOF || fc==10) { *writepos = val; val = 0; writepos++; }
				if (fc == EOF) repeat = 0;
			} while (repeat);

			int *readpos;
			readpos = temp_map;
			for (int j = 0; j < row; j++)
				for (int i = 0; i < col; i++)
				{
 					tml->map[j][i] = *readpos;
					readpos++;
				}

			//free(temp_map);

			if (fp) fclose(fp);           // close file

			// load bmp
			tml->bmp = al_load_bitmap(bitmap_filename);
			if (!tml->bmp) return -1;

			tml->tilesPerRow = al_get_bitmap_width(tml->bmp) / tileSize;

			// create vbo
			tml->vbo = malloc(col*row * 4 * sizeof(ALLEGRO_VERTEX));  // 4 vertices per tile
			if (!tml->vbo) return -1;
			ALLEGRO_VERTEX *vertex_writepos;
			vertex_writepos = tml->vbo;

			for (int j = 0; j < row; j++)
			{
				for (int i = 0; i < col; i++)
				{
					int tile_num = tml->map[j][i];
					if (tile_num)
					{
						vertex_writepos->color = al_map_rgb(200, 200, 200);
						vertex_writepos->u = (tile_num % tml->tilesPerRow)*tileSize;
						vertex_writepos->v = (tile_num / tml->tilesPerRow)*tileSize;
						vertex_writepos->x = i;
						vertex_writepos->y = j;
						vertex_writepos->z = -0.0;
						vertex_writepos++;

						vertex_writepos->color = al_map_rgb(200, 200, 200);
						vertex_writepos->u = (tile_num % tml->tilesPerRow)*tileSize + tileSize;
						vertex_writepos->v = (tile_num / tml->tilesPerRow)*tileSize;
						vertex_writepos->x = i + 1;
						vertex_writepos->y = j;
						vertex_writepos->z = -0.0;
						vertex_writepos++;

						vertex_writepos->color = al_map_rgb(200, 200, 200);
						vertex_writepos->u = (tile_num % tml->tilesPerRow)*tileSize + tileSize;
						vertex_writepos->v = (tile_num / tml->tilesPerRow)*tileSize + tileSize;
						vertex_writepos->x = i + 1;
						vertex_writepos->y = j + 1;
						vertex_writepos->z = -0.0;
						vertex_writepos++;

						vertex_writepos->color = al_map_rgb(200, 200, 200);
						vertex_writepos->u = (tile_num % tml->tilesPerRow)*tileSize;
						vertex_writepos->v = (tile_num / tml->tilesPerRow)*tileSize + tileSize;
						vertex_writepos->x = i;
						vertex_writepos->y = j + 1;
						vertex_writepos->z = -0.0;
						vertex_writepos++;
					}
				}
			}
			tml->vbo_gpu = al_create_vertex_buffer(0, tml->vbo, col*row*4, ALLEGRO_PRIM_BUFFER_DYNAMIC);

			// create ibo
			tml->ibo = malloc(col*row * 6 * sizeof(int));
			if (!tml->ibo) return -1;
			int *ibo_writepos;
			ibo_writepos = tml->ibo;

			for (int j = 0; j < row; j++)
			{
				for (int i = 0; i < col; i++)
				{
					for (int h = 0; h < 6; h++)
					{
						*ibo_writepos = 4*(i+j*col)+sprite_ibo[h];
						ibo_writepos++;
					}
				}
			}
			tml->ibo_gpu = al_create_index_buffer(4, tml->ibo, col*row*6, ALLEGRO_PRIM_BUFFER_DYNAMIC);

			return 0;
}



/*************************************************************************************/
/*************************************************************************************/

void update_sprite_vbo(VBO_SPRITE *spr, ALLEGRO_COLOR rgb)
{
			float w, h;
			float x, y, z;
			x = spr->x;
			y = spr->y;
			z = spr->z;
			w = spr->w / 2;
			h = spr->h / 2;
			// fill in 4 vertex information (4 corners of a square) that will be used to draw our sprite bitmap
			spr->vbo[0].x = x - w * spr->scale;
			spr->vbo[0].y = y - h * spr->scale;
			spr->vbo[0].z = z;
			spr->vbo[0].u = 0;
			spr->vbo[0].v = 0;
			spr->vbo[0].color = rgb;
	   
			spr->vbo[1].x = x + w * spr->scale;
			spr->vbo[1].y = y - h * spr->scale;
			spr->vbo[1].z = z;
			spr->vbo[1].u = spr->w;
			spr->vbo[1].v = 0;
			spr->vbo[1].color = rgb;
	   
			spr->vbo[2].x = x + w * spr->scale;
			spr->vbo[2].y = y + h * spr->scale;
			spr->vbo[2].z = z;
			spr->vbo[2].u = spr->w;
			spr->vbo[2].v = spr->h;
			spr->vbo[2].color = rgb;
	   
			spr->vbo[3].x = x - w * spr->scale;
			spr->vbo[3].y = y + h * spr->scale;
			spr->vbo[3].z = z;
			spr->vbo[3].u = 0;
			spr->vbo[3].v = spr->h;
			spr->vbo[3].color = rgb;
}

/*************************************************************************************/
/*************************************************************************************/

int setup_vbo_sprite(VBO_SPRITE *spr, char* bitmap_filename)
{
			spr->bmp = al_load_bitmap(bitmap_filename);
			if (!spr->bmp) return -1;
			spr->vbo_gpu = al_create_vertex_buffer(0, spr->vbo, 4, ALLEGRO_PRIM_BUFFER_DYNAMIC);
			if (!spr->vbo_gpu) return -1;
			spr->ibo_gpu = al_create_index_buffer(4, &sprite_ibo, 6, ALLEGRO_PRIM_BUFFER_DYNAMIC);
			if (!spr->ibo_gpu) return -1;
			spr->w = al_get_bitmap_width(spr->bmp);
			spr->h = al_get_bitmap_height(spr->bmp);
			spr->scale = 1.0;
			return 0;
}

void draw_vbo_sprite(VBO_SPRITE *spr, float x, float y, float z, float scale)
{
			spr->x = x;
			spr->y = y;
			spr->z = z;
			spr->scale = scale;
			update_sprite_vbo(spr, al_map_rgb(200,200,200));
			// update our vbo buffer on the gpu with the new coordinates
			void* lock_mem = al_lock_vertex_buffer(spr->vbo_gpu, 0, 4, ALLEGRO_LOCK_WRITEONLY);
			memcpy(lock_mem, &spr->vbo, sizeof(ALLEGRO_VERTEX) * 4);
			al_unlock_vertex_buffer(spr->vbo_gpu);

			// now draw our sprite
			al_draw_indexed_buffer(spr->vbo_gpu, spr->bmp, spr->ibo_gpu, 0, 6, ALLEGRO_PRIM_TRIANGLE_LIST);
}

void init()
{
	tileSize = 32;

	if (load_tilemap_layer(&background, "a5_2d_platformer_background.csv", "TempleTIles.png") == -1) 
		{
		printf("error loading background tilemap layer"); al_rest(1000); al_uninstall_system(); exit(-1);
		} 
	if (load_tilemap_layer(&playfield, "a5_2d_platformer_gamefield.csv", "TempleTIles.png") == -1) 
		{
		printf("error loading gamefield tilemap layer"); al_rest(1000); al_uninstall_system(); exit(-1);
		}
	
	if (setup_vbo_sprite(&player, "player.png") == -1) 
		{ 
		printf("error loading player bitmap"); al_rest(1000); al_uninstall_system(); exit(-1);
		}

	playerX = 15;
	playerY = 15;
	standing_on_tile = 0;
	jumping = 0;
	jump_add = 0;

	cameraX = -10;
	cameraY = 10;
}

void map_draw()
{
	// draw ibo/vbo
	al_clear_to_color(al_map_rgb(16, 96, 127));
	float tmp_tileSize = tileSize;
	{
		al_identity_transform(&MainTrans);
		al_build_transform(&MainTrans, -(cameraX)*tmp_tileSize + windowW / 2, -cameraY * tmp_tileSize + windowH / 2, tmp_tileSize, tmp_tileSize, 0);
		al_use_transform(&MainTrans);
		al_draw_indexed_buffer(background.vbo_gpu, background.bmp, background.ibo_gpu, 0, background.map_h*background.map_w * 6, ALLEGRO_PRIM_TRIANGLE_LIST);
		al_draw_indexed_buffer(playfield.vbo_gpu, playfield.bmp, playfield.ibo_gpu, 0, playfield.map_h*playfield.map_w * 6, ALLEGRO_PRIM_TRIANGLE_LIST);
		draw_vbo_sprite(&player, playerX, playerY, 0, 0.01);
	}

}

void validate_new_player_coordinates()
{
if (new_player_x > playfield.map_w - 2) new_player_x = playfield.map_w - 2;
if (new_player_x < 2) new_player_x = 2;
if (new_player_y > playfield.map_h - 2) new_player_y = playfield.map_h - 2;
if (new_player_y < 2) new_player_y = 2;
}



/*************************************************************************************/
// our program starts here
// this part below is the first to be executed when the program starts
/*************************************************************************************/
int main(void)
{

	if (!al_init())
	{
		printf("Error A5 init\n");
		printf("error init\n"); al_uninstall_system(); exit(-1);
	}


	if (!al_init_primitives_addon())
	{
		printf("Error primitives\n"); al_uninstall_system(); exit(-1);
	}

	if (!al_init_image_addon())
	{
		printf("error image\n"); al_uninstall_system(); exit(-1);
	}

	if (!al_install_keyboard())
	{
		printf("error keyboard\n"); al_uninstall_system(); exit(-1);
	}

/*	if (!al_init_font_addon())														// not used in this demo
	{																				// not used in this demo
		printf("error font\n"); al_uninstall_system(); exit(-1);					// not used in this demo
	}																				// not used in this demo
																					// not used in this demo
	if (!al_init_ttf_addon())														// not used in this demo
	{																				// not used in this demo
		printf("error ttf\n"); al_uninstall_system(); exit(-1);						// not used in this demo
	}																				// not used in this demo
																					// not used in this demo
	if (!al_install_audio())														// not used in this demo
	{																				// not used in this demo
		printf("error audio\n"); al_uninstall_system(); exit(-1);					// not used in this demo
	}																				// not used in this demo
																					// not used in this demo
	if (!al_init_acodec_addon())													// not used in this demo
	{																				// not used in this demo
		printf("error acodec\n"); al_uninstall_system(); exit(-1);					// not used in this demo
	}																				// not used in this demo
																					// not used in this demo
	if (!al_install_mouse())														// not used in this demo
	{																				// not used in this demo
		printf("error mouse\n"); al_uninstall_system(); exit(-1);					// not used in this demo
	}																				// not used in this demo
*/
	al_set_new_bitmap_flags(ALLEGRO_MIN_LINEAR);
	al_set_new_display_flags(ALLEGRO_RESIZABLE | ALLEGRO_OPENGL);
	al_set_new_display_option(ALLEGRO_DEPTH_SIZE, 8, ALLEGRO_SUGGEST);
	al_set_new_display_option(ALLEGRO_VSYNC, 2, ALLEGRO_SUGGEST);
	// al_set_new_display_flags(ALLEGRO_FULLSCREEN);

	windowW = 1024;
	windowH = 720;
	display = al_create_display(windowW, windowH);

	if (!display)
	{
		printf("Error creating display\n");
	}

	init();

	al_set_window_title(display, "Tilemap using VBO - Demo");

	ALLEGRO_EVENT_QUEUE *eventQueue = al_create_event_queue();

	refreshRate = 60;
	timer = al_create_timer(1.0 / refreshRate);
	al_register_event_source(eventQueue, al_get_keyboard_event_source());
//	al_register_event_source(eventQueue, al_get_mouse_event_source());				// not used in this demo
	al_register_event_source(eventQueue, al_get_display_event_source(display));
	al_register_event_source(eventQueue, al_get_timer_event_source(timer));

	al_start_timer(timer);
	al_flush_event_queue(eventQueue);

	ALLEGRO_EVENT event;
	char need_redraw = 0;

	while (1)
	{
		while (al_get_next_event(eventQueue, &event))
		{

			if (event.type == ALLEGRO_EVENT_TIMER)
			{
				cameraX = cameraX * 0.98 + 0.02*(playerX);
				cameraY = cameraY * 0.98 + 0.02*(playerY);

				double nowTime = al_get_time();
				deltaTime = nowTime - oldTime;
				oldTime = nowTime;

				// move our player and do basic collision testing
						// process Y axis movements
						new_player_y = playerY;        // new_player_y is out temporary 'work' variable
						// jumping
						if (jumping)
							{
							new_player_y -= jump_add; jump_add -= 0.02; 
							if (playfield.map[(int)(new_player_y - 0.5)][(int)new_player_x]) jump_add = -1.0;   // we've hit the ceiling
							if (jump_add < 0)
								{
								jumping = 0; jump_add = 0;
								}
							}
						else
							{
							new_player_y += 10 * deltaTime;
							}
						validate_new_player_coordinates();
						//standing on a tile
						if (playfield.map[(int)(new_player_y+0.5)][(int)new_player_x])
							{
							standing_on_tile = 1;
							playerY = (int)playerY + 0.5;
							}
						else
							{
							playerY = new_player_y;
							}

						// process X axis movements
						if (key[ALLEGRO_KEY_D])
						{
							new_player_x = playerX + 10 * deltaTime;
							validate_new_player_coordinates();
							if (!playfield.map[(int)new_player_y][(int)(new_player_x + 0.5)]) 
								playerX = new_player_x;
						}

						if (key[ALLEGRO_KEY_A])
						{
							new_player_x = playerX - 10 * deltaTime;
							validate_new_player_coordinates();
							if (!playfield.map[(int)new_player_y][(int)(new_player_x - 0.5)]) 
								playerX = new_player_x;
						}
						if (key[ALLEGRO_KEY_SPACE])
						{
							if (!jumping)
							{
								if (standing_on_tile)
									{
									jumping = 1; jump_add = 0.55; standing_on_tile = 0;
									}
							}
						}

				need_redraw^=1;
			}

			if (event.type == ALLEGRO_EVENT_KEY_DOWN)
			{

				for (int i = 0; i < ALLEGRO_KEY_MAX; i++)
					if (event.keyboard.keycode == i)
						key[i] = 1;
			}

			if (event.type == ALLEGRO_EVENT_KEY_UP)
			{

				for (int i = 0; i < ALLEGRO_KEY_MAX; i++)
					if (event.keyboard.keycode == i)
						key[i] = 0;
			}

			if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN || event.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP || event.type == ALLEGRO_EVENT_MOUSE_AXES)
			{

			}

			if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
			{
				al_stop_timer(timer);
				al_uninstall_system();
				exit(0);
			}

			if (event.type == ALLEGRO_EVENT_DISPLAY_RESIZE)
			{
				al_acknowledge_resize(display);
				windowW = al_get_display_width(display);
				windowH = al_get_display_height(display);
			}
		}


		if (key[ALLEGRO_KEY_ESCAPE])
		{
			al_stop_timer(timer);
			al_uninstall_system();
			exit(0);
		}

		if (need_redraw)
		{
			map_draw();
			al_identity_transform(&Identity);
			al_use_transform(&Identity);
			al_flip_display();
			need_redraw = 0;
		}
	}
}




