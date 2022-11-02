/* 
 * Robot Jet Action
 * Commodore 64 Arcade-like game
 * Development started on 2020.04.15 
 * by Tomasz 'Carrion' Mielnik / Bonzai / C64Portal.pl 
 *
 * main.c
 * Main module of the game - start point to compile everything
 *
 *
 * IMPORTANT Notice: Use only KickC v0.82 to compile it!
 * It won't compile with later KickC versions.
 */

#pragma link("main.ld")	// config file for linker

#define _MAIN 

#include <c64.h>
#include <keyboard.h> 
#include <print.h>
#include <division.h>
 
// order of the inluded files matters. 
#include "nmi.c"
#include "title.c"

byte* MAPS 			= $9230;	
byte* CHARMAPS		= $5a00;
byte* COLORMAPS		= $6200;

#include "levels.c"		// levels.c need above variables

byte* CHARS_DATA    = $4000;
byte* SCREEN   		= $4800;
byte* SPRITES_DATA  = $4c00;
byte* DECRUNCHER	= $c900;
byte* SOUNDFX		= $9100;
byte* MUSIC 		= $7000;

byte* levelDescriptor = $47d0;

#define ENDPART_CONST 0xe000
byte* ENDPART		= ENDPART_CONST;

const byte MUSIC_TITLE    = 0 ;
const byte MUSIC_WORLDMAP = 1 ;
const byte MUSIC_GETREADY = 6 ;
const byte MUSIC_GAMEOVER = 7 ;

byte* SPR1_PTR = SCREEN+(40*25)+16;

byte* ANIM_WATER   = $4200;
byte* ANIM_COIN    = $4200+64;
byte* ANIM_STAR    = $4200+64+64;
byte* ANIM_BG      = $4200+64+64+64;

byte* ANIMATIONS = $4200;

byte* CHAR_COLORS	= $6100;     // <---------- char color attribs are depacked here

const byte TILE_FLOOR = $05; 
const byte TILE_KEY = $01;
const byte TILE_LIFE = $3e;
const byte TILE_FUEL = 63;
const byte TILE_BLACK = $ff;
const byte TILE_WATER = 60;
const byte TILE_BG = 59;
const byte TILE_HIT1 = $2;
const byte TILE_HIT2 = $3;
const byte TILE_HIT3 = $4;

const byte SPRITE_EXIT = 53;

//----------- Game States --------------

const byte GAME_TITLESCREEN =1;
const byte GAME_PLAY 		=2;
const byte GAME_GAMEOVER 	=3;
const byte GAME_DIED 		=4;
const byte GAME_PAUSE 		=5;
const byte GAME_IDLE 		=6;
const byte GAME_NEXTLEVEL 	=7;
const byte GAME_WORLDMAP 	=8;
const byte GAME_NEXTWORLD 	=9;
const byte GAME_CONGRATULATIONS	=10;
const byte GAME_GETREADY	= 20;
const byte GAME_GAMEMAP		= 21;
const byte GAME_GAMEOVERWAIT 	=31; // ugly hack for a game over screen  


//----------- Go directions  ------------

volatile byte goDirection=0;
const byte GO_NONE  =0;
const byte GO_LEFT  =1;
const byte GO_RIGHT =2;
const byte GO_UP    =3;
const byte GO_DOWN  =4;

volatile byte gameStatus = GAME_TITLESCREEN;

volatile byte keys =0 ;
volatile byte minKeys =0 ;
volatile byte fuel ;
volatile byte lifes = 2;
volatile word score =0;

volatile byte playerX , playerY;
volatile byte* coll = $3f0;
volatile byte* enemyCollision = $380;
volatile byte plrTimer1 = 0; // a timer after which we change player sprite to front facing

volatile byte[3] joy = {0,0,0}; 	// used to store joy values for left, right & fire

volatile byte frameFire = 0;
volatile byte frameGoLeft = 0;
volatile byte frameGoRight = 0;

volatile byte px; 	// player global coordinates
volatile byte py;
volatile word xy;   // player corrds recalculated to map chars positions

volatile bool levelRestart;
volatile bool musicRestart;
volatile byte currentMusic = 2;
volatile byte gravity ; 

volatile byte currentLevel = 0;
volatile byte currentLevelWorld = 0;
volatile byte currentWorld = 0;

volatile bool isWorldMapDisplayed = false;
volatile bool gameCanBeStarted = false;
volatile bool bossLevel = false;
volatile bool perfectEnabled = false;
volatile bool perfectHandled ;
volatile bool perfectScoreAdded = false;
volatile byte perfectTimer = 100;
volatile byte perfectColorFrame = 0;

volatile byte isPerfect ;
volatile byte isPortalOpen ;
volatile byte allWorlds =0;
volatile bool gameComplete=false;

volatile word highScore1 = 250;
volatile word highScore2 = 100;
volatile word highScore3 = 10;

// Look Up Table for multiplying by 40
volatile word[25] mulLUT = kickasm {{
	.for (var i=0; i<25; i++){
		.word i*40;
	}
}};

// tables for animframes and colors flash
byte[8] framesAnimFire  = {0,0,1,1,2,2,3,3};
byte[19] colorFlash     = {0,6,$b,4,$e,3,$d,1,1,1,$d,$d,3,3,$d,$d,1,1,1};
byte[12] framesAnimGoLeft = {0,0,0,2,2,2,4,4,4,2,2,2};

//----------------------------------------------------------------
//----------------------------------------------------------------

void main (){

    asm { sei }
    *CIA1_INTERRUPT = CIA_INTERRUPT_CLEAR;
    *VIC_CONTROL &=$7f;
    *RASTER = $32;
    *IRQ_ENABLE = IRQ_RASTER;      
    *PROCPORT = $35;
    *HARDWARE_IRQ = &irqTitleScreen;
    NMIhandle ();
    asm { cli }

    // use kickc build in function from library
	keyboard_init();	// FIXME: level skipper to be kept or not?


	// ------------- Main Loop ------------
	// This is a "game state" engine
	// the game states are handled here

	while (true){
		if (gameStatus == GAME_TITLESCREEN){
			initTitleScreen ();	
			gameStatus = GAME_IDLE;
			textScreenTimer = 0;
			currentLevelWorld = 0;
			currentLevel =0;
			currentWorld =0;			
			textPos=0;

			cleanScrollLine(); // on the map
			worldDone[0]=0; // set all worlds as not done
			worldDone[1]=0;
			worldDone[2]=0;
			worldDone[3]=0;
			worldDone[4]=0;

			asm{
				lda #MUSIC_TITLE
				jsr $7000
			}
		}

		if (gameStatus == GAME_NEXTLEVEL){
			if (currentLevelWorld == world[ currentWorld ].levels){
				gameStatus = GAME_NEXTWORLD;
			}
			else{
				currentLevel ++;
				currentLevelWorld ++;
				gameStatus = GAME_GETREADY;
				*HARDWARE_IRQ = &irqTextScreen;
				textScreenTimer = 0;
				currentMusic ++;
				if (currentMusic==6) currentMusic =2;
				asm{
					lda #MUSIC_GETREADY		
					jsr $7000
				}
			}

			levelRestart = true;

		}

		if (gameStatus == GAME_NEXTWORLD){
			currentLevel += world[ currentWorld ].levels;

  			worldDone[currentWorld]=1;

			currentWorld ++;
			currentLevel ++;
			currentLevelWorld = 0;
			gameStatus = GAME_WORLDMAP;
			*HARDWARE_IRQ = &irqWorldMapScreen;
			worldmapTimer = 0;
			
			textPos=0;
			cleanScrollLine();

			levelRestart = true;
			musicRestart = true;
		}

		if (gameStatus == GAME_PLAY){
			if (levelRestart) {
				*D011 = 0;
				initLevel ();
				musicRestart = false;

				levelRestart = false;
				textScreenTimer = 0;
				*D011 = %01011011;
				isWorldMapDisplayed = false;
			}
		}

		if (gameStatus == GAME_GETREADY){
			TEXT_SCREEN[40*11+19] = $30+currentWorld +1;
			COLS [40*11+19] = $03;
			TEXT_SCREEN[40*11+20] = '.';
			COLS [40*11+20] = $03;
			TEXT_SCREEN[40*11+21] = $30+currentLevelWorld +1;
			COLS [40*11+21] = $03;

			for (byte i=0; i<40; i++) {
				TEXT_SCREEN[i+(40*10)]=getReadyText[i];
				COLS       [i+(40*10)]=$03;
			}
		}

		if (gameStatus == GAME_GAMEOVER){
			for (byte i=0; i<40*3; i++) {
				TEXT_SCREEN[i+(40*10)]=gameoverText[i];
				COLS [i+(40*10)]=$03;
			}		
			print_set_screen (TEXT_SCREEN+24+(40*12));
			print_uint_decimal (score);
			lastScoreLine = 0;
			updateHighScore ();
			gameStatus = GAME_GAMEOVERWAIT;
		}

		if (gameStatus == GAME_WORLDMAP){

			if (!isWorldMapDisplayed){
				*D011 = 0;
				for (word i=0; i<40*22; i++) {	
					byte c = WORLDMAP_SCREEN[40+i];
					COLS [40+i] = WORLDMAP_COLORS[c];
				}
				*D011 = %00011011;
				isWorldMapDisplayed = true;
				
				for (byte b=0; b<40; b++){
					COLORRAM[40*23+b]=0;
					COLORRAM[40*24+b]=3;
				}

				asm{
					lda #MUSIC_WORLDMAP
					jsr $7000
				}

				// grey out the map of the world that was already done!
				if (worldDone[0]==1) cleanWorldOnMap ((40*10)+0, 10,9);
				if (worldDone[1]==1) cleanWorldOnMap ((40*7 )+10, 8,10);
				if (worldDone[2]==1) cleanWorldOnMap ((40*10)+18, 5,10);
				if (worldDone[3]==1) cleanWorldOnMap ((40*9 )+22, 8,8);
				if (worldDone[4]==1) cleanWorldOnMap ((40*8 )+30, 9,10);
			}

			allWorlds = worldDone[0]+worldDone[1]+worldDone[2]+worldDone[3]+worldDone[4];
			if (allWorlds==5) {
				gameComplete = true;
				if (worldmapTimer>350) goToEndPart ();
			}
		}
	}
}

//----------------------------------------------------------------
// Main Game IRQ
// All the gameplay happens here
//----------------------------------------------------------------
interrupt(hardware_all) void irqGame() {

	// level skipper used for debugging and testing purposes
	if ( keyboard_key_pressed (KEY_ARROW_LEFT) !=0) gameStatus = GAME_NEXTLEVEL;
	if ( keyboard_key_pressed (KEY_RUNSTOP) !=0) gameStatus = GAME_NEXTWORLD;
	asm { jsr MUSIC+3 }  // play music

	if (gameStatus == GAME_PLAY){
		if (gameCanBeStarted){

			collectKeys(); 
			collectLife();

			// why 4 times? almost the same function?
			// read more later
			if (!perfectEnabled){
				handleEnemy1 (levelDescriptor[06] ,  
						  levelDescriptor[07], 
						  levelDescriptor[08],
						  levelDescriptor[09] ,
						  levelDescriptor[10], 
						  levelDescriptor[11]);

				handleEnemy2 (levelDescriptor[15] ,  
						  levelDescriptor[16], 
						  levelDescriptor[17],
						  levelDescriptor[18] ,
						  levelDescriptor[19], 
						  levelDescriptor[20]	);

				handleEnemy3 (levelDescriptor[24] ,  
						  levelDescriptor[25], 
						  levelDescriptor[26],
						  levelDescriptor[27] ,
						  levelDescriptor[28], 
						  levelDescriptor[29]	);

				handleEnemy4 (levelDescriptor[33] ,  
						  levelDescriptor[34], 
						  levelDescriptor[35],
						  levelDescriptor[36],
						  levelDescriptor[37], 
						  levelDescriptor[38]	);
				
				animEnemies ();
				checkCollisions();
			}
			else{
				// FIXME: Maybe change PERFECT 
				// to new games status? like GAME_PERFECT?
				
				if (!perfectScoreAdded){	// no sprite-sprite collisions when PERFECT logo is on
					score += 100;
					perfectScoreAdded = true;
					updateScore ();
				}

				if (perfectTimer ==0){
					
					// perfectEnabled = false;
					moveOutSprites();
					animEnemies ();
					checkCollisions();
					perfectHandled = true;
					VICII->SPRITES_MC=%01111010;
					
				}
				else{
					perfectTimer --;
				}

				if (perfectColorFrame == 24){
					perfectColorFrame = 0;
				}
				else {
					VICII->SPRITE3_COLOR = perfectColors[ perfectColorFrame ];
					VICII->SPRITE4_COLOR = perfectColors[ perfectColorFrame ];
					VICII->SPRITE5_COLOR = perfectColors[ perfectColorFrame ];
					VICII->SPRITE6_COLOR = perfectColors[ perfectColorFrame ];
					perfectColorFrame ++;
					
					// back to normal sprites size after PERFECT displayed
					asm{	
						lda #$00
						sta $d017
						sta $d01d
					}
				}
			}

			animChar (ANIM_COIN , TILE_KEY, 6,0);
			animChar (ANIM_STAR , TILE_HIT1, 6,1);
			animChar (ANIM_WATER , TILE_WATER, 6,2);
			animChar (ANIM_BG , TILE_BG, 6,3);
			
			// sometimes pure assembler is quicker :) to write
			asm {
				lda $d01e 
				sta enemyCollision 
			}

			calculateXY ();	
			handlePlayer ();
			getCollisions ();
			handleFuel(fuel);
			checkAllKeys ();
		}

	}
	else if (gameStatus == GAME_DIED){

		playSFX_Death ();

		if (lifes==0){
			gameStatus=GAME_GAMEOVER;
			textScreenTimer = 0;
			*HARDWARE_IRQ = &irqTextScreen;
			asm{
				lda #MUSIC_GAMEOVER
				jsr $7000
			}
		}

		if (frameFire<=19) {  // colors flash
			frameFire++;
		}
		else{
			gameCanBeStarted = false;
			levelRestart = true;
			musicRestart = false;
			gameStatus = GAME_PLAY;
			lifes --;
		}
		VICII->SPRITE0_COLOR = colorFlash[frameFire];
	}

	
	// reset sprite pose to default front facing
	if (plrTimer1 == 50) {
		plrTimer1 =0;
		SPR1_PTR[1] = ($0c00/64); 
		SPR1_PTR[0] = ($0c00/64)+1; 
	}
	else plrTimer1 ++;

    *IRQ_STATUS = IRQ_RASTER;
}

//------------------------------------------------------------------

void animGoLeft (){

	if (frameGoLeft<11) frameGoLeft++;
	else frameGoLeft=0;

	SPR1_PTR[1] = ($0c00/64)+8+framesAnimGoLeft [frameGoLeft]; 
	SPR1_PTR[0] = ($0c00/64)+9+framesAnimGoLeft [frameGoLeft]; 
}

void animGoRight (){

	if (frameGoRight<11) frameGoRight++;
	else frameGoRight=0;

	SPR1_PTR[1] = ($0c00/64)+8+6+framesAnimGoLeft [frameGoRight]; 
	SPR1_PTR[0] = ($0c00/64)+9+6+framesAnimGoLeft [frameGoRight]; 
}


volatile bool canPlay = false;

//------------------------------------------------------------------

// read joystick and place in an array we use later
// joy[] is the array with 3 joy reads: left, right and fire
// I do this in asm as it was much quicker for me to write back then
void handleJoyAsm (){
	kickasm (uses joy) {{
		lda $dc00
		and #%00000100
		sta joy+0

		lda $dc00
		and #%00001000
		sta joy+1

		lda $dc00
		and #%00010000
		sta joy+2
	}}
}

// this is where we handle all player's movement
void handlePlayer (){

	handleJoyAsm ();	// read Joystick with assembly code

	if ( joy[1]==0 ) { 			// left
		if (playerX<255) {
			playerX ++;
			animGoLeft ();
			goDirection=GO_LEFT;
		}
	}
	
	if ( joy[0]==0 ) {  		// right
		if (playerX>=32) {
			playerX --;
			animGoRight ();
			goDirection=GO_RIGHT;
		}
	}

	if ( joy[2]==0 && fuel>10 ) { 		// fire - we fly up

		if (canPlay) {
			playSFX_JetPackOn();
			canPlay = false;
		}

		gravity = 1;	// reset the gravity to 1.

		if (playerY>=56){
			playerY --;
			goDirection = GO_UP;
			SPR1_PTR[1] = ($0c00/64)+2;
			SPR1_PTR[0] = ($0c00/64)+3;
			*SPRITES_ENABLE =  %11111111;
			fuel--;
			if (fuel<=10) {
				fuel = 0; // zerujemy aby przerwa między ładowaniami była większa.
				playSFX_JetPackOff();
			}
		}

		
		// you can comment next 3 lines to make game a bit easier
		// what it is to make player fall after hitting the roof.

		if (playerY==56){
			fuel = 0; 				// zero fuel when hit the roof 
			playSFX_JetPackOff();
		}
	}	
	else  			//  no fly - check if we fall down?
	{
		canPlay = true;
		*SPRITES_ENABLE =  %11111011;

		if (coll[9] !=TILE_FLOOR && coll[10] !=TILE_FLOOR ) {
				goDirection = GO_DOWN;
				playerY += gravity/4;    		// calculate fractions of gravity and sum up
				if (gravity<16) gravity ++;
		}
		else if (coll[10]!=TILE_FLOOR && coll[11] !=TILE_FLOOR ) {
				goDirection = GO_DOWN;
				playerY+=gravity/4;
				if (gravity<16) gravity ++;
		}
		else{ 				// if we stand on the floor then reset the gravity to 1
			gravity = 0;	
			goDirection=GO_NONE;
		}
	}

	SPRITES_XPOS[0] = playerX;
	SPRITES_YPOS[0] = playerY;
	SPRITES_XPOS[2] = playerX;
	SPRITES_YPOS[2] = playerY;
	SPRITES_XPOS[4] = playerX;
	SPRITES_YPOS[4] = playerY+22;
}


// a function to draw a fuel gauge on screen
void handleFuel (byte f){
	if (goDirection==GO_NONE){
		if (fuel>=0 && fuel<128) {
			fuel +=2;
		}
	}

	byte ff=f/8 ; 

	for (byte i=0; i<ff; i++){
		word m= mulLUT[i];
		SCREEN[33+40*8+m] = TILE_FUEL;
	}
	for (byte i=ff; i<16; i++){
		word m= mulLUT[i];
		SCREEN[33+40*8+m] = TILE_BLACK;
	}
}

// -----------------------------------------------------------------


// read enemies from level descriptors and place in starting positions
void levelEnemies (){

	SPRITES_XPOS[6] = levelDescriptor[04];
	SPRITES_YPOS[6] = levelDescriptor[05];
	SPR1_PTR[3] = ($0c00/64)+20 + levelDescriptor[03];

	SPRITES_XPOS[8] = levelDescriptor[13];
	SPRITES_YPOS[8] = levelDescriptor[14];
	SPR1_PTR[4] = ($0c00/64)+20 + levelDescriptor[15];

	SPRITES_XPOS[10] = levelDescriptor[22];
	SPRITES_YPOS[10] = levelDescriptor[23];
	SPR1_PTR[5] = ($0c00/64)+20 + levelDescriptor[21];

	SPRITES_XPOS[12] = levelDescriptor[31];
	SPRITES_YPOS[12] = levelDescriptor[32];
	SPR1_PTR[6] = ($0c00/64)+20 + levelDescriptor[30];
}

// these are indexes in sinus/cosinus tables 
// sinuses LUT are read & used to move the enemies
volatile byte cx1=0;
volatile byte cy1=0;
volatile byte cx2=0;
volatile byte cy2=0;
volatile byte cx3=0;
volatile byte cy3=0;
volatile byte cx4=0;
volatile byte cy4=0;

// next 4 routines are the hungry rastertime eaters :D
// they are used to move enemies and they look lame like this beacuse I was learning the capabilities
// of KickC. 
// I had big problems with using an array of levels structures + some others I cant remember now
// but it ended as these 4 procedures instead of 1. 
// one of the reasons is that one universal procedure generated huge and buggy (IMO) asm code.
// but I can't tell now if it was issues with compiler or my lack of knowledge back then.
// after a years of using kickC I would code it differently now.

// this prcoedure reads sinus table for X and Y pos. you can specify the speed and aplitude divider of this sinus
// thie procedure params are:
// tX - number of sinus table to use for X
// speed1 - how fast to read the sin LUT table for X
// amp1 - divider of the amplitude for X
// the rest arguments are the same but for Y coords
void handleEnemy1 (byte tX, byte speed1, byte amp1, byte tY, byte speed2,byte amp2){
	
	byte* tableX = movs[tX] ; // pointers to the selected sin/cos LUT
	byte* tableY = movs[tY] ; 
	byte ll = currentLevel ; 
	cx1+=speed1;	// increase the pointer to next value in sin LUT table
	cy1+=speed2;
	byte tx = div8u( tableX[cx1] , amp1);	// divisions could be avoided but I was lazy
	byte ty = div8u( tableY[cy1] , amp2);
	SPRITES_XPOS[6] = levelDescriptor[04] + tx;	// place the sprite
	SPRITES_YPOS[6] = levelDescriptor[05] + ty;
}

void handleEnemy2 (byte tX, byte speed1, byte amp1, byte tY, byte speed2,byte amp2){

	byte* tableX = movs[tX] ; 
	byte* tableY = movs[tY] ; 
	byte ll = currentLevel ; 
	cx2+=speed1;
	cy2+=speed2;
	byte tx = div8u( tableX[cx2] , amp1);
	byte ty = div8u( tableY[cy2] , amp2);
	SPRITES_XPOS[8] = levelDescriptor[13] + tx;
	SPRITES_YPOS[8] = levelDescriptor[14] + ty;
}

void handleEnemy3 (byte tX, byte speed1, byte amp1, byte tY, byte speed2,byte amp2){

	byte* tableX = movs[tX] ; 
	byte* tableY = movs[tY] ; 
	byte ll = currentLevel ; 
	cx3+=speed1;
	cy3+=speed2;
	byte tx = div8u( tableX[cx3] , amp1);
	byte ty = div8u( tableY[cy3] , amp2);
	SPRITES_XPOS[10] = levelDescriptor[22] + tx;
	SPRITES_YPOS[10] = levelDescriptor[23] + ty;
}

void handleEnemy4 (byte tX, byte speed1, byte amp1, byte tY, byte speed2,byte amp2){
	byte* tableX = movs[tX] ; 
	byte* tableY = movs[tY] ; 
	byte ll = currentLevel ; 
	cx4+=speed1;
	cy4+=speed2;
	byte tx = div8u( tableX[cx4] , amp1);
	byte ty = div8u( tableY[cy4] , amp2);
	SPRITES_XPOS[12] = levelDescriptor[31] + tx;
	SPRITES_YPOS[12] = levelDescriptor[32] + ty;
}


// -----------------------------------------------
// this function handles the collection of an item
// inputs: 
// at - position on screen map where the collectable is
// ch - no idea :)
void handleKeyCollect (word at,byte ch){

	ch = ch & %11000000;

	if (keys>0) {
		keys--;
		if (minKeys>0) minKeys--;

		score = score + 5;
		SCREEN [at] = ch;
		
		if (minKeys==0 && isPortalOpen!=1) {
			playSFX_PortalOpen ();
			isPortalOpen = 1;
		}
		if (keys==0) {
			isPerfect=1;
			playSFX_Perfect ();
		}

		playSFX_Collect();
				
		updateScore ();
	}
}

//this updates the score result on the screen
//uses kickc built in procedures
void updateScore() {
	print_set_screen (SCREEN+35+(40*4));
	print_uint_decimal (score);
	SCREEN[36] = 0;
	print_set_screen (SCREEN+35);
	print_uint_decimal (keys);
}

// check collisions for all of the 9 chars around the player for keys/diamonds
byte collectKeys (){

	byte[9] c = {0,0,0,0,0,0,0,0,0};

	for (byte i=0; i<9; i++){
		c[i] = coll[i] & %00111111;
	}

	if (c[0]==TILE_KEY)	{ handleKeyCollect (xy,    coll[0] ); return 0; }
	if (c[1]==TILE_KEY)	{ handleKeyCollect (xy+1,  coll[1] ); return 0; } 		
	if (c[2]==TILE_KEY)	{ handleKeyCollect (xy+2,  coll[2] ); return 0; }		
	if (c[3]==TILE_KEY)	{ handleKeyCollect (xy+40, coll[3] ); return 0; }		
	if (c[4]==TILE_KEY)	{ handleKeyCollect (xy+41, coll[4] ); return 0; }		
	if (c[5]==TILE_KEY)	{ handleKeyCollect (xy+42, coll[5] ); return 0; }		
	if (c[6]==TILE_KEY)	{ handleKeyCollect (xy+80, coll[6] ); return 0; }		
	if (c[7]==TILE_KEY)	{ handleKeyCollect (xy+81, coll[7] ); return 0; }		
	if (c[8]==TILE_KEY)	{ handleKeyCollect (xy+82, coll[8] ); return 0; }		

	return 1;
}

// name says it all. 
// this procedure handles the collection of extra life
// and prints on the screen
void handleLifeCollect (word at,byte ch){

	ch = ch & %11000000;

	if (lifes<200) { 			// max 9 lifes
		score = score + 50;
		lifes++;
		SCREEN [at] = ch;

		playSFX_ExtraLife();

		print_set_screen (SCREEN+35+(40*4));
		print_uint_decimal (score);
		print_set_screen (SCREEN+35+(40*2));
		print_uint_decimal (lifes);

		extraLifes [currentWorld] = 1;

	}
}

// check if collision fith extra life (heart) happened 
byte collectLife (){

	byte[9] c = {0,0,0,0,0,0,0,0,0};

	for (byte i=0; i<9; i++){
		c[i] = coll[i] & %00111111;
	}

	if (c[0]==TILE_LIFE) { handleLifeCollect (xy,    coll[0] ); return 0; }
	if (c[1]==TILE_LIFE) { handleLifeCollect (xy+1,  coll[1] ); return 0; } 		
	if (c[2]==TILE_LIFE) { handleLifeCollect (xy+2,  coll[2] ); return 0; }		
	if (c[3]==TILE_LIFE) { handleLifeCollect (xy+40, coll[3] ); return 0; }		
	if (c[4]==TILE_LIFE) { handleLifeCollect (xy+41, coll[4] ); return 0; }		
	if (c[5]==TILE_LIFE) { handleLifeCollect (xy+42, coll[5] ); return 0; }		
	if (c[6]==TILE_LIFE) { handleLifeCollect (xy+80, coll[6] ); return 0; }		
	if (c[7]==TILE_LIFE) { handleLifeCollect (xy+81, coll[7] ); return 0; }		
	if (c[8]==TILE_LIFE) { handleLifeCollect (xy+82, coll[8] ); return 0; }		

	return 1;
}

// check collisions 
void checkCollisions (){

	// player collides with kiling tile?
	// pretty lame code here... but it works ;)
	// now I think I could actually easily make it shorter but it ws to close to
	// game release date and I didnt wanted to make significant changes.
	if ((coll[1] & %00111111)==TILE_HIT1) gameStatus = GAME_DIED;
	if ((coll[3] & %00111111)==TILE_HIT1) gameStatus = GAME_DIED;
	if ((coll[4] & %00111111)==TILE_HIT1) gameStatus = GAME_DIED;
	if ((coll[5] & %00111111)==TILE_HIT1) gameStatus = GAME_DIED;
	if ((coll[6] & %00111111)==TILE_HIT1) gameStatus = GAME_DIED;
	if ((coll[7] & %00111111)==TILE_HIT1) gameStatus = GAME_DIED;
	if ((coll[8] & %00111111)==TILE_HIT1) gameStatus = GAME_DIED;
	if ((coll[1] & %00111111)==TILE_HIT2) gameStatus = GAME_DIED;
	if ((coll[3] & %00111111)==TILE_HIT2) gameStatus = GAME_DIED;
	if ((coll[4] & %00111111)==TILE_HIT2) gameStatus = GAME_DIED;
	if ((coll[5] & %00111111)==TILE_HIT2) gameStatus = GAME_DIED;
	if ((coll[6] & %00111111)==TILE_HIT2) gameStatus = GAME_DIED;
	if ((coll[7] & %00111111)==TILE_HIT2) gameStatus = GAME_DIED;
	if ((coll[8] & %00111111)==TILE_HIT2) gameStatus = GAME_DIED;
	if ((coll[1] & %00111111)==TILE_HIT3) gameStatus = GAME_DIED;
	if ((coll[3] & %00111111)==TILE_HIT3) gameStatus = GAME_DIED;
	if ((coll[4] & %00111111)==TILE_HIT3) gameStatus = GAME_DIED;
	if ((coll[5] & %00111111)==TILE_HIT3) gameStatus = GAME_DIED;
	if ((coll[6] & %00111111)==TILE_HIT3) gameStatus = GAME_DIED;
	if ((coll[7] & %00111111)==TILE_HIT3) gameStatus = GAME_DIED;
	if ((coll[8] & %00111111)==TILE_HIT3) gameStatus = GAME_DIED;

	// Sprites collisions. Player vs the 4 enemies
	if (isPerfect==0){
		if  (enemyCollision[0] == %00001011 ) gameStatus = GAME_DIED;
		if  (enemyCollision[0] == %00010011 ) gameStatus = GAME_DIED;
		if  (enemyCollision[0] == %00100011 ) gameStatus = GAME_DIED;
		if  (enemyCollision[0] == %01000011 ) gameStatus = GAME_DIED;
	}

	// if player colided with the last 7th sprite it means we exit the level
	// 7th sprite is on screen only if all colectables are collected
	if  (enemyCollision[0] == %10000011 ) gameStatus = GAME_NEXTLEVEL;

}

// quite importnt procedure
// xy - is a global variable to store player position recalculated to screen address position
// px,py player sprite positions recalculated to screen chars corrdinates (player pos div by 8)
void calculateXY (){
	px = (playerX - 24)/8;	 
	py = (playerY - 50)/8;
	xy = mulLUT[py];
	xy = xy +px;
}

// read 9 chars of possible colision around player - used for collectables and hits
// read additional 3 chars for collisions with floors 
// coll[] is global array where I store collision tiles (chars) indexes
void getCollisions() {

	coll[0] = SCREEN[xy] ;
	coll[1] = SCREEN[xy+1];
	coll[2] = SCREEN[xy+2] ;

	coll[3] = SCREEN[xy+40] ;
	coll[4] = SCREEN[xy+40+1];
	coll[5] = SCREEN[xy+40+2] ;

	coll[6] = SCREEN[xy+80] ;
	coll[7] = SCREEN[xy+80+1];
	coll[8] = SCREEN[xy+80+2] ;

	// separate line for floor checks only
	byte px1 = (playerX - 24)/8;
	byte py1 = (playerY - 50+6)/8;
	// word xy1 = mul8u(py1,40) + px1;
	word xy1 = mulLUT[py1];
	xy1 += px1;

	coll[9 ] = SCREEN[xy1+80]   & %00001111;
	coll[10] = SCREEN[xy1+80+1] & %00001111;
	coll[11] = SCREEN[xy1+80+2] & %00001111;

}

export byte[5] extraLifes = {0,0,0,0,0};

// this fixes a bug we catched almost few days before release.
// it's ugly fix to clear extralife heart on the screen.
//
// also, we try not to clear the heart at the right panel, where the left lifes info is :)
// i!=113  !!!?!? WTF? ;)
void fixExtraLife (){

	if (extraLifes[currentWorld]==1){
		for (word i=0; i<40*24; i++){
			byte c = SCREEN[i] ;
			if ((c & 0b00111111 )==TILE_LIFE && i!=113) SCREEN[i]=c & 0b11000000;	
		}
	}
}

// this is a level init procedure.
// it is called at game start, at each level or when player died
// it reads level params from level descriptor.
// currentlevel and currentworld vars are used for this
void initLevel (){

	gameCanBeStarted = false;

	isPortalOpen = 0;
	isPerfect =0;
	perfectScoreAdded = false;

	// buildScreen ( currentLevel, CHAR_COLORS); // FIXME: arguments are not needed here
	buildScreen ( ); 

	fixExtraLife ();

	levelEnemies ();

	*SPRITES_MC = %01111010;
	*SPRITES_MC1 = world[currentWorld].spritecolor;
	*SPRITES_MC2 = $01;

	playerX = levelDescriptor[0];	// start pos of the player. see levels.c file
	playerY = levelDescriptor[1];

	SPRITES_XPOS[0] = playerX;
	SPRITES_YPOS[0] = playerY;
	SPRITES_XPOS[2] = playerX;
	SPRITES_YPOS[2] = playerY;
	SPRITES_XPOS[4] = playerX;
	SPRITES_YPOS[4] = playerY+22;

	*SPRITES_XMSB   = 0;

	// exit sprite
	SPRITES_XPOS[14] = 0;
	SPRITES_YPOS[14] = 255;
	VICII->SPRITE7_COLOR = WHITE;

	VICII->SPRITE0_COLOR = BLACK;
	VICII->SPRITE1_COLOR = PINK;
	VICII->SPRITE2_COLOR = 7;
	VICII->SPRITE3_COLOR = levelDescriptor[39];
	VICII->SPRITE4_COLOR = levelDescriptor[39];
	VICII->SPRITE5_COLOR = levelDescriptor[39];
	VICII->SPRITE6_COLOR = levelDescriptor[39];

	gravity=1;
	fuel = 128;

	gameStatus = GAME_PLAY;

	// clean anim frames
	eaFrames=0;
	eaFramesSkip=0;

	// zero the enemy sinus movements indexes in LUT tables
	cx1 = 0; cx2 =0; cx3=0; cx4=0;
	cy1 = 0; cy2 =0; cy3=0; cy4=0;

	for (byte i=0; i<4;i++){
		animCharFrameSkip[i]=0;
		animCharFrames[i]=0;
	}

#ifdef _DEBUG	// some leftovers :)
	lifes=99;
#endif

	// TODO: Add lifes check no more than 9 ??? 
	// TODO: Add pts no more than 99999 ???
	// I never actually added these. 
	// So you can actually overflow the initeger points variable
	// but in normal circumstances the max nr of points is around 6500 so it doesnt mater 

	*VIC_MEMORY  = toD018(SCREEN,CHARS_DATA);
	CIA2->PORT_A = toDd00 (CHARS_DATA);
	*BG_COLOR      = world[currentWorld].bgcolor;
	*BORDER_COLOR  = BLACK;
	*BG_COLOR1		= world[currentWorld].color1;
	*BG_COLOR2		= world[currentWorld].color2;
	*BG_COLOR3		= world[currentWorld].color3;
	*D016		= $c8;
	*D011 		= %01011011;

	SPR1_PTR[7] = ($0c00/64)+SPRITE_EXIT;
	SPR1_PTR[1] = $0c00/64;
	SPR1_PTR[0] = ($0c00/64)+1;

	keys = levelDescriptor[2];
	minKeys = levelDescriptor[42];

	// clear all player's collisions with chars around him
	coll[0]  = 0;
	coll[1]  = 0;
	coll[2]	 = 0;
	coll[3]	 = 0;
	coll[4]	 = 0;
	coll[5]	 = 0;
	coll[6]	 = 0;
	coll[7]	 = 0;
	coll[8]	 = 0;
	coll[9]  = 0;
	coll[10] = 0;
	coll[11] = 0;

	// next few lines is a proof that sometimes it's quicker to write this in asm 
	enemyCollision[0]=0;
	asm {
		lda $d01e
	}

	if (currentLevelWorld == world[ currentWorld ].levels){
		kickasm {{
			lda #%01111000  // expand sprites
			sta $d01d
			sta $d017
			lda #$08      // if boss level play tune #8
			jsr $7000     // music is at $7000
		}}
	}
	else {
		kickasm(uses currentMusic){{    // IMO this parameter is not needed
			lda #%00000000
			sta $d01d
			sta $d017
		}}

		if (musicRestart){
			kickasm {{
				lda currentMusic
				jsr $7000
			}}
			
			extraLifes[0] =0;
			extraLifes[1] =0;
			extraLifes[2] =0;
			extraLifes[3] =0;
			extraLifes[4] =0;
		}
	}

	// here we use kickc build in finctions to print lifes, score and keys
	print_set_screen (SCREEN+35+(40*2));
	print_uint_decimal (lifes);
	print_set_screen (SCREEN+35+(40*4));
	print_uint_decimal (score);
	print_set_screen (SCREEN+35);
	print_uint_decimal (keys);

	perfectEnabled = false;
	perfectTimer = 50;
	perfectHandled = false;
	gameCanBeStarted = true;

	*HARDWARE_IRQ = &irqGame;	// set interrupts to this function and lets play the game
}

// decrunch level.
// levels are packed with ByteBoozer v2
void buildScreen (){
	
	word m=mapptrs[currentLevel]; // get current level number from maps pointers list

	kickasm (uses m) {{
		ldx m+1
		ldy m
		jsr decruncher.start 	// ByteBoozer decruncher is in asm in separate file
	}}

	word n=charptrs[currentWorld]; // get chars for current world and decrunch it

	kickasm (uses n) {{
		ldx n+1
		ldy n
		jsr decruncher.start
	}}

	word o=colorptrs[currentWorld]; // get char colors for current world and decrunch it

	kickasm (uses o) {{
		ldx o+1
		ldy o
		jsr decruncher.start
	}}

	byte* p=animptrs[currentWorld];	// char animations for current world. not crunched
	for (byte i=0; i<$ff; ++i){
		ANIMATIONS[i]=p[i];
	}
	ANIMATIONS[$ff]=p[$ff];

	byte col;
	for (word i=0; i<40*25; i++){	// fill the color map based on tiles(chars) on the screen
		col=SCREEN[i];
		COLS[i]=CHAR_COLORS[col];
	}
}



volatile byte eaFrames=0;
volatile byte eaFramesSkip=0;

// animate all 4 enemies on screen. all the time.
void animEnemies (){

	if (eaFramesSkip==3){	// next animation frame is changed every 4th frame
		eaFramesSkip=0;

		if (eaFrames<2){
			eaFrames++;
			SPR1_PTR[3]++;
			SPR1_PTR[4]++;
			SPR1_PTR[5]++;
			SPR1_PTR[6]++;
			SPR1_PTR[7]++;
		}
		else {
			SPR1_PTR[3]= ($0c00/64)+20 +levelDescriptor[3];
			SPR1_PTR[4]= ($0c00/64)+20 +levelDescriptor[12];
			SPR1_PTR[5]= ($0c00/64)+20 +levelDescriptor[21];
			SPR1_PTR[6]= ($0c00/64)+20 +levelDescriptor[30];	
			SPR1_PTR[7]= ($0c00/64)+SPRITE_EXIT ;	

			eaFrames =0;
		}
	}
	else eaFramesSkip ++;

	if (frameFire<7) frameFire++;
	else frameFire=0;
	
	SPR1_PTR[2] = ($0c00/64)+4+framesAnimFire[frameFire]; // fire sprite	
}



volatile byte[4] animCharFrameSkip = {0,0,0,0};
volatile byte[4] animCharFrames = {0,0,0,0};

// animated chars used for example for collectables or water like anims
void animChar (byte* animMem, word targetChar, byte numFrames, byte af){

	if (animCharFrameSkip[af]==3){
		animCharFrameSkip[af]=0;	

		word w = targetChar*8;

		for (byte i=0; i<8; i++){
			CHARS_DATA [w + i] = animMem [ animCharFrames[af]*8 +i];
		}
		if (animCharFrames[af]<=numFrames) { 
			animCharFrames[af]++ ;
		}
		else {
			animCharFrames[af]=0;
		}
	} 
	else animCharFrameSkip[af]++;
}


// check if we have all keys collected, display PERFECT and open exit
void checkAllKeys (){
	if (  minKeys == 0 ){
		// read the exit dor position from level descriptor
		SPRITES_XPOS[14] = levelDescriptor[40];
		SPRITES_YPOS[14] = levelDescriptor[41];
	}

	//show PERFECT!
	if (  keys == 0 && !perfectHandled ){
		SPR1_PTR[3] = ($2500/64)+0;
		SPR1_PTR[4] = ($2500/64)+1;
		SPR1_PTR[5] = ($2500/64)+2;
		SPR1_PTR[6] = ($2500/64)+3;
		VICII->SPRITES_MC = %00000010;
		VICII->SPRITE3_X=116;
		VICII->SPRITE4_X=116+24;
		VICII->SPRITE5_X=116+24+24;
		VICII->SPRITE6_X=116+24+24+24;
		VICII->SPRITE3_Y=136;
		VICII->SPRITE4_Y=136;
		VICII->SPRITE5_Y=136;
		VICII->SPRITE6_Y=136;

   		perfectEnabled= true;
	}
}

// move sprites off the screen
// used for clean up when changing game states
void moveOutSprites (){
		VICII->SPRITE3_X=80;
		VICII->SPRITE4_X=92+24;
		VICII->SPRITE5_X=106+24+24;
		VICII->SPRITE6_X=122+24+24+24;
		VICII->SPRITE3_Y=11;
		VICII->SPRITE4_Y=11;
		VICII->SPRITE5_Y=11;
		VICII->SPRITE6_Y=11;
}


// assembly code to play SFX
void playSFX_Collect (){
	kickasm {{
		lda #<collectkeysfx
		ldy #>collectkeysfx
		ldx #14
		jsr $7006	
	}}
}

void playSFX_JetPackOn (){
	kickasm {{
		lda #<jetpackstartsfx
		ldy #>jetpackstartsfx
		ldx #14
		jsr $7006	
	}}
}

void playSFX_JetPackOff (){
	kickasm {{
		lda #<jetpackoverheatsfx
		ldy #>jetpackoverheatsfx
		ldx #14
		jsr $7006	
	}}
}

void playSFX_Death (){
	kickasm {{
		lda #<deathsfx
		ldy #>deathsfx
		ldx #14
		jsr $7006	
	}}
}

void playSFX_PortalOpen (){
	kickasm {{
		lda #<portalopenfx
		ldy #>portalopenfx
		ldx #14
		jsr $7006	
	}}
}

void playSFX_Perfect (){
	kickasm {{
		lda #<perfectfx
		ldy #>perfectfx
		ldx #14
		jsr $7006	
	}}
}

void playSFX_ExtraLife (){
	kickasm {{
		lda #<extralifesfx
		ldy #>extralifesfx
		ldx #14
		jsr $7006	
	}}
}

//----------------------------//----------------------------
//                    R E S O U R C E S 
//----------------------------//----------------------------

//---------------------------- GFX  ------------------------

kickasm (pc SPRITES_DATA){{
	.var  levelSprites = LoadBinary("data/sprites.bin")
	.fill levelSprites.getSize(), levelSprites.get(i)	
}}

//--------AUDIO --------------------------------------------

kickasm (pc MUSIC){{
	.var music = LoadSid("audio/RobotJetAction_MSX.sid")
	.fill music.size, music.getData(i)
}}


kickasm (pc SOUNDFX ) {{ 

jetpackstartsfx:
.var jetpackstartsfximport = LoadBinary("audio/JetpackStart.snd")
.fill jetpackstartsfximport.getSize(), jetpackstartsfximport.get(i)

jetpackoverheatsfx:
.var jetpackoverheatsfximport = LoadBinary("audio/JetpackOverheat.snd")
.fill jetpackoverheatsfximport.getSize(), jetpackoverheatsfximport.get(i)

collectkeysfx:
.var collectkeysfximport = LoadBinary("audio/CollectKey.snd")
.fill collectkeysfximport.getSize(), collectkeysfximport.get(i)

deathsfx:
.var deathsfximport = LoadBinary("audio/Death.snd")
.fill deathsfximport.getSize(), deathsfximport.get(i)

portalopenfx:
.var portalopensfximport = LoadBinary("audio/PortalOpen.snd")
.fill portalopensfximport.getSize(), portalopensfximport.get(i)

// TODO: Fix missing sounds

perfectfx:
.var perfectsfximport = LoadBinary("audio/Perfect.snd")
.fill perfectsfximport.getSize(), perfectsfximport.get(i)

extralifesfx:
.var extralifesfximport = LoadBinary("audio/ExtraLife.snd")
.fill extralifesfximport.getSize(), extralifesfximport.get(i)

}}

// ==================================================================

//
// Here in the final release of the game the end-part was included as a binary
// It is not included in this distribution of source code.
// It's only kept as a placeholder to make the code compile.

kickasm (pc ENDPART_CONST){{
	.byte 0
	// .var endpartbin = LoadBinary("endpart.prg",BF_C64FILE)
	// .fill endpartbin.getSize(), endpartbin.get(i)
}}

// ==================================================================

// include the ByteBoozer2 decrunch sourcecode
kickasm  (pc DECRUNCHER){{
#import "decruncher.asm"
}}

// some code is pushed to upper addresses of the memory (here $c000)
// because the code didnt fit in the planned block starting at the $0801 
// we use linker file for the segment CodeHigh definiotn (see linking.ld file)
#pragma code_seg(CodeHigh)

// this is function used on the worlds map screen
// it clears the line where scroll is.
// used 
void cleanScrollLine () {
	byte* scrollLine = TITLE_SCREEN+(23*40)+40;
	for (byte i=0; i<40; i++){
		scrollLine[i]=$20;
	}
}

// put dark grey color for world that is already done
// inputs: 
// world - world to be greyed
// w,h - width, height of the reyed area.
void cleanWorldOnMap (word world, byte w, byte h){

	word l=0;
	byte j=0;

	while (j<h){
		for (byte i=0; i<w; i++){
			COLS[world+i+l]=0x0b; 	// set $0b which is dark grey color
		}
		l+=40;
		j++;
	}
}


// Here in final game comes the asm function that relocates the endpart binary (.prg) to $0801
// Endpart binary is not included in this distribution. So it's only a placeholder here.
// This game will stop (crash) here.
void goToEndPart(){

	while (true) { }

}

#pragma data_seg(DataHigh)

// color fade table for the PERFECT sprites 
volatile byte[] perfectColors = kickasm {{
	.byte $1,$1,$1
	.byte $d,$d,$d
	.byte $3,$3,$3
	.byte $e,$e,$e
	.byte $3,$3,$3
	.byte $d,$d,$d
	.byte $1,$1,$1
	.byte $1,$1,$1	
}};

//-------- TEXTS ------------------------------------------------

byte[40] getReadyText = kickasm {{
.text "           get ready for level          "
}};

byte[40] gameoverText = kickasm {{
.text "                game over               "
.text "                                        "
.text "            your score.                 "
}};

export byte[40] text0 = kickasm {{
	.text "monsterland planet...           @"
}};

export byte[40] text1 = kickasm {{
	.text "rick's planet...        @"
}};

export byte[40] text2 = kickasm {{
	.text "classics planet...        @"
}};

export byte[40] text3 = kickasm {{
	.text "bubble planet...        @"
}};

export byte[40] text4 = kickasm {{
	.text "flashback planet...        @"
}};

export byte[40] text5 = kickasm {{
	.text "well done robot!              @"
}};


export byte *scrollptrs[6] = {
	&text0, &text1, &text2, &text3, &text4, &text5
};

