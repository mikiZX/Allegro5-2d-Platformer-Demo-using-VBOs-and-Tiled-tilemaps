
ALLEGRO_TRANSFORM MainTrans;
ALLEGRO_TRANSFORM Identity;

char key[ALLEGRO_KEY_MAX];				// buffer holding information regarding which keys are being held down

int tileSize;	// in pixels
int mapsizeW, mapsizeH;
float playerX, playerY, cameraX, cameraY;
int jumping, standing_on_tile;
float jump_add;
float new_player_x;
float new_player_y;


typedef struct TILEMAP_L {
	int** map;
	int map_w, map_h;
	ALLEGRO_VERTEX *vbo;					// pointer to memory space that holds information about tilemap vertices
	ALLEGRO_VERTEX_BUFFER *vbo_gpu;			// A5 interface to VBO buffer on the GPU

	ALLEGRO_BITMAP *bmp;
	int tileSize;
	int tilesPerRow;

	int* ibo;						// pointer to memory space that holds indices that define tile side
	ALLEGRO_INDEX_BUFFER* ibo_gpu;						// A5 GPU index buffer interface that holds indices that define tile top
} TILEMAP_LAYER;


typedef struct VBO_SPRITE_ {
	float x, y, z;
	float u, v;
	float w, h;
	float u2, v2;
	float scale;
	ALLEGRO_BITMAP *bmp;
	ALLEGRO_VERTEX vbo[4];					// pointer to memory space that holds information about tilemap vertices
	ALLEGRO_VERTEX_BUFFER *vbo_gpu;			// A5 interface to VBO buffer on the GPU
	ALLEGRO_INDEX_BUFFER* ibo_gpu;						// A5 GPU index buffer interface that holds indices that define the sprite
} VBO_SPRITE;

int sprite_ibo[6] = { 0, 1, 2,   2, 3, 0 };


TILEMAP_LAYER	background;
TILEMAP_LAYER	playfield;

VBO_SPRITE player;

typedef struct myVEC2i {
	int x, y;
} MYVEC2I;


int rangeW, rangeH;						// how much of the map is visible
int windowW, windowH;					// our window size
int refreshRate;						// refresh rate for the timer
ALLEGRO_DISPLAY* display;
ALLEGRO_TIMER *timer;

int panRate;							// movement speed

double deltaTime;
double oldTime;
