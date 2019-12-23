
*************************************************
*************************************************
********* SIMPLE TILEMAP 2D PLATFORMER **********
***********   USING ALLEGRO5's VBO  *************
**************** and 'Tiled' map ****************
*************************************************

Files in this archive:
   a5_2d_platformer.tmx  ('Tiled' project file)
   a5.tsx  ('Tiled' tileset file)
   TempleTIles.png (background & playfield bitmap by Pyromantic)
   a5_2d_platformer_background.csv (background layer exported from 'Tiled' as CSV file)
   a5_2d_platformer_gamefield.csv  (gamefield layer exported from 'Tiled' as CSV file)
   main2.c, main2.h  ( C source code)
   player.png  (player bitmap)
   Read Me.txt  (this file)

'Tiled' is a map editor software available at: https://www.mapeditor.org/

Graphics for the 'Tiled' map found in this archive were created by: Pyromantic
and used here as per the author's license displayed at: https://opengameart.org/content/sunset-temple-tiles   (CC-BY 3.0, CC0)

The C source code contained within this archive is made available under CC0 license (without any guarantees/warranties, expressed or implied; as-is).

----
Usage: You will need to compile the source code (main2.c and main2.h) and link it with Allegro5 library as well as Allegro5 Add-on libraries called 'primitives_addon' and 'image_addon'.

The source code contains:
	-a basic loader for CSV layer file exported by 'Tiled' app
	-basic collision detection for 2d tilemap/platformer game (no sweeping)
	-VBO creation (each tilemap layer is stored as a single VBO/IBO to simplify the drawing)
	-simple VBO sprite creation and drawing example
