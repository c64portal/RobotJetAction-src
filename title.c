/* 
 * Robot Jet Action
 * Commodore 64 Arcade-like game
 * Development started on 2020.04.15 
 * by Tomasz 'Carrion' Mielnik / Bonzai / C64Portal.pl 
 *
 * title.c
 * Title screens, maps and text screens
 * irq procedures.
 *
 * In general in this file we have everything related to title screen, 
 * map screen and text screens between levels + highscore
 */

#define _TITLE

byte* SPRITES_TITLEDATA	= $2f00;

byte* TITLE_SCREEN=$3000;
byte* TITLE_SCREEN2=$3400;
byte* TITLE_CHARS=$3800;
byte* TITLESPR_PTR  = TITLE_SCREEN+(40*25)+16;
byte* TITLESPR_PTR2 = TITLE_SCREEN+$400+(40*25)+16;

byte* TEXT_SCREEN = $0400;
byte* TEXT_PTR = TEXT_SCREEN+(40*25)+16;

byte* PERFECT_SPRITES = 0x6500;
byte* WORLDMAP_SPRITES =0x6500;
byte* WORLDMAP_CHARS  = 0x6800;
byte* WORLDMAP_COLORS = 0x6b80;
byte* WORLDMAP_SCREEN = 0x6c00;
byte* WORLDMAP_PTR = WORLDMAP_SCREEN+(40*25)+16;

volatile word screensTimer =0;
volatile word worldmapTimer =0;
volatile byte textScreenTimer = 0;

volatile byte textPos=0;
volatile byte scroll = 7;
volatile byte tfr = 0;

volatile byte[40] tframes = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1};

volatile byte endSin = 0;

extern byte[256] sinus1;

//used for joy move on the map
// cw current world position
// cworld - cw*3 for sprites
// cwx, cwy delay counters for moving left or right
volatile byte cw =0;
volatile byte cworld =0;
volatile byte cwx =0;
volatile byte cwy =0;

// x,y coords of the sprite on the worldmap
export byte[40] mapSpritePos1 = kickasm {{
	.byte 60, 105,0
	.byte 122, 80,0
	.byte 176,125,0
	.byte 226, 108,0
	.byte 35,  100,$ff
	.byte 176,70,0
}};

// set to 1 when the world is done.
export byte[5] worldDone = {0,0,0,0,0};

volatile export byte lastScoreLine = 0;
volatile export byte scoreColorFlash = 1;

//-------------------------------------------------------------------------
// Game Title Init. Used in the Game State engin in main loop
//-------------------------------------------------------------------------

void initTitleScreen(){

	*BG_COLOR      = BLACK;
	*BORDER_COLOR  = BLACK;
	*D016 		= %11011000;
	*D011 		= %00011011;

	*SPRITES_MC = %01111010;
	*SPRITES_MC1 = $05;
	*SPRITES_MC2 = $01;
	VICII->SPRITE0_COLOR = BLACK;
	VICII->SPRITE1_COLOR = PINK;
	VICII->SPRITE2_COLOR = $0f;

	for (byte i=0; i<$ff; ++i){
		TEXT_SCREEN[i+0] = $20;
		TEXT_SCREEN[i+$100] = $20;
		TEXT_SCREEN[i+$200] = $20;
		TEXT_SCREEN[i+$300] = $20;
	}

	asm {
		lda #MUSIC_TITLE
		jsr MUSIC
	}

	// reset sprite position on map
	cw = 0;

	*HARDWARE_IRQ = &irqTitleScreen;
}

//-------------------------------------------------------------------------
// Title Screens IRQ 
// some parts of this irq handler should be moved to game state engine in main loop
// but I started coding it this way and later I was lazy to move it to proper place
// but hey ... it works ;)
//-------------------------------------------------------------------------

interrupt(hardware_all) void irqTitleScreen() {

	asm {
		lda $dc00
		and #%00010000
		sta joy+2
	}

	kickasm {{
		lda #$32
		cmp $d012
		bne *-3
		lda #$0c
		sta $d023
		lda #$0b
		sta $d022 

		lda #$68
		cmp $d012
		bne *-3
		lda #$0e
		sta $d023
		lda #$06
		sta $d022

		jsr MUSIC+3
	}}

	if (screensTimer <250){					// show title screen

		// *BG_COLOR1 = $06;
		// *BG_COLOR2 = $0e;

		for (byte i=0; i<40*5 ; ++i){
			COLS[i]= $0b;
		}
		for (byte i=0; i<40 ; ++i){
			COLS[40*5+i]= $09;
			COLS[40*6+i]= $0f;
		}
		for (byte i=0; i<40*2 ; ++i){
			COLS[40*1+i]= $09;
		}
		for (byte i=0; i<40*2 ; ++i){
			COLS[40*7+i]= $0b;
		}
		for (byte i=0; i<40 ; ++i){
			COLS[i + (40*13)]= $03;
			COLS[i + (40*14)]= $03;
			COLS[i + (40*16)]= $05;
			COLS[i + (40*17)]= $05;
			COLS[i + (40*19)]= $07;
			COLS[i + (40*20)]= $07;
			COLS[i + (40*21)]= $07;

			COLS[i + (40*9)]= $0c;
		}

		TITLESPR_PTR[1] = ($2f00/64) + (tframes[tfr] * 2);
		TITLESPR_PTR[0] = ($2f00/64)+1 + (tframes[tfr] * 2);
		*VIC_MEMORY = toD018( TITLE_SCREEN, TITLE_CHARS);
		CIA2->PORT_A = toDd00 (TITLE_CHARS);
		*SPRITES_ENABLE = $03;
		SPRITES_XPOS[0] = 175;
		SPRITES_YPOS[0] = 122;
		SPRITES_XPOS[2] = 175;
		SPRITES_YPOS[2] = 122;
		screensTimer ++;
	}	
	else{							// switch to high score screen

			*BG_COLOR1 = $09;
			*BG_COLOR2 = $0a;

		// color mem setup done in irq - lame but it stayed from the begining
		// and i havent time to fix it and move to game state engine in main loop :)
		for (byte i=0; i<40 ; ++i){ 
			COLS[40*1+i]= $07;
			COLS[40*2+i]= $07;
			COLS[40*3+i]= $07;

			COLS[40*6+i]= $04;
			COLS[40*8+i]= $04;
			COLS[40*10+i]= $04;
			COLS[40*12+i]= $04;

			COLS[40*13+i]= $0f;
			COLS[40*14+i]= $0f;
			COLS[40*15+i]= $0f;
			COLS[40*16+i]= $0f;
			COLS[40*17+i]= $0f;

			COLS[40*18+i]= $02;
			COLS[40*20+i]= $02;
			COLS[40*22+i]= $02;
		}

		printHighScores ();
		flashLatestScore ();

		TITLESPR_PTR2[1] = ($2f00/64) + (tframes[tfr] * 2);
		TITLESPR_PTR2[0] = ($2f00/64)+1 + (tframes[tfr] * 2);
		*VIC_MEMORY = toD018( TITLE_SCREEN+$400, TITLE_CHARS);
		CIA2->PORT_A = toDd00 (TITLE_CHARS);
		*SPRITES_ENABLE = $03;
		SPRITES_XPOS[0] = 175;
		SPRITES_YPOS[0] = 160+16;
		SPRITES_XPOS[2] = 175;
		SPRITES_YPOS[2] = 160+16;
		screensTimer++;
	}

	if (screensTimer==550) screensTimer =0;   // restart counter after 11 seconds

	if (tfr!=39) tfr++;		// tfr is a sprite animation frame index.
	else tfr=0;

	if (joy[2]==0){			// if joy pressed 
		lifes = 3;
		score =0;
		gameStatus=GAME_WORLDMAP;
		currentLevel =0 ;
		levelRestart = true;
		worldmapTimer =0;
		*HARDWARE_IRQ = &irqWorldMapScreen;

		kickasm (uses currentMusic){{
			lda currentMusic
			jsr MUSIC
		}}
	}

	*IRQ_STATUS = IRQ_RASTER;
}

//-------------------------------------------------------------------------
// Text Screen & GAME OVER Screen  IRQ 
//-------------------------------------------------------------------------

interrupt(hardware_all) void irqTextScreen() {

	*VIC_MEMORY = toD018( TEXT_SCREEN, TITLE_CHARS);
	CIA2->PORT_A = toDd00 (TITLE_CHARS);
	*SPRITES_ENABLE = 0;

	if (textScreenTimer==50*2 && gameStatus==GAME_GETREADY) {
		gameStatus = GAME_PLAY;
		*HARDWARE_IRQ = &irqGame;
		musicRestart = true;
	}

	if (textScreenTimer==50*5 && gameStatus==GAME_GAMEOVERWAIT) {
		*HARDWARE_IRQ = &irqTitleScreen;
		gameStatus = GAME_TITLESCREEN;
		screensTimer = 250;
	}

	textScreenTimer ++;
	asm { jsr MUSIC+3 }
	*IRQ_STATUS = IRQ_RASTER;
}

//-------------------------------------------------------------------------
// World Map IRQ 
//-------------------------------------------------------------------------

interrupt(hardware_all) void irqWorldMapScreen() {

	kickasm {{	// sometimes asm is faster to write that C :) and in this case more clear I guess
		lda #$32
		cmp $d012
		bne *-3
		lda #$c8
		sta $d016
	}}	

	*VIC_MEMORY = toD018( WORLDMAP_SCREEN, WORLDMAP_CHARS);
	CIA2->PORT_A = toDd00 (WORLDMAP_CHARS);

	WORLDMAP_PTR[1] = ($0c00/64)+0 ;
	WORLDMAP_PTR[0] = ($0c00/64)+1 ;
	*SPRITES_ENABLE = %00000111;
	*SPRITES_MC = %01110010;

	cworld = cw *3;	

	//we check it here again because when checked outside irq we get some race conditions with 
	// scroll text pointer being cleard
	// at least that's my hypothesis :)
	// if done here it works OK with the "well done scroll"
	byte wellDoneScroll = worldDone[0]+worldDone[1]+worldDone[2]+worldDone[3]+worldDone[4];

	
	// this is handling of the sprite movement and world choose on the worldmap
	if (wellDoneScroll!=5) {

		handleJoyAsm ();	// read joy

		if ( joy[1]==0 ) {  
			cwx ++;
		}
		if ( joy[0]==0 ) { 
			cwy ++;
		}

		if (cwx==7){
			cwx=0; cwy=0; cw++;
			clearScrollLine (); 
			textPos=0;
		}
		if (cwy==7){
			cwx=0; cwy=0; cw--;
			clearScrollLine ();
			textPos=0;
		}
		if (cw==5) {
			cw=0; cwx=0; cwy=0;
			clearScrollLine ();
			textPos=0;
		}
		if (cw==255) {
			cw=4; cwx=0; cwy=0;
			clearScrollLine ();
			textPos=0;
		}
		SPRITES_XPOS[0] = mapSpritePos1[ cworld   ];
		SPRITES_YPOS[0] = mapSpritePos1[ cworld+1 ];
		SPRITES_XPOS[2] = mapSpritePos1[ cworld   ];
		SPRITES_YPOS[2] = mapSpritePos1[ cworld+1 ];
		SPRITES_XPOS[4] = mapSpritePos1[ cworld   ];
		SPRITES_YPOS[4] = mapSpritePos1[ cworld+1 ]+21;
		*SPRITES_XMSB   = mapSpritePos1[ cworld+2 ];
	}
	else{ 
		// game is finished - animate sprite on the map
		byte s = sinus1 [ endSin++ ];
		byte ys = (s>>2 )+65;
		byte xs =170;
		SPRITES_XPOS[0] = xs;
		SPRITES_YPOS[0] = ys;
		SPRITES_XPOS[2] = xs;
		SPRITES_YPOS[2] = ys;
		SPRITES_XPOS[4] = xs;
		SPRITES_YPOS[4] = ys+21;	
		cw=5;	
	}

	VICII->SPRITE3_COLOR = 1;
	kickasm {{
		lda #$eb
		cmp $d012
		bne *-3
		lda #$00
		sta $d015
	}}

	VICII->CONTROL2 = scroll;
	*VIC_MEMORY = toD018( TITLE_SCREEN, TITLE_CHARS);
	CIA2->PORT_A = toDd00 (TITLE_CHARS);

	asm { jsr MUSIC+3 }

	// ----------------------------------------- SCROLL taken from KickC examples

	byte *ptr = scrollptrs[ cw ];

	byte* scrollLine = TITLE_SCREEN+(23*40)+40;
	scroll --;
	if (scroll==$ff){
		scroll=7;
		byte c = ptr[textPos++];
		if (c!=0) scrollLine[39]=c;
		else textPos=0;

	    for(byte i=0;i!=39;i++) scrollLine[i]=scrollLine[i+1];
	}
    // -----------------------------------------  SCROLL end


	if (worldmapTimer>=50) { // -------- Wait 1 sec for pressing fire on the map

		if (joy[2]==0){	
			if (worldDone[cw]==0){
				textScreenTimer =0;
				currentWorld = cw;
				currentLevel = cw*7;
				*HARDWARE_IRQ = &irqTextScreen;
				gameStatus = GAME_GETREADY;
				asm{
					lda #MUSIC_GETREADY
					jsr $7000
				}
			}
		}
	}

	worldmapTimer++;

	if (frameFire<7) frameFire++;
	else frameFire=0;
	
	WORLDMAP_PTR[2] = ($0c00/64)+4+framesAnimFire[frameFire]; // fire sprite		
	WORLDMAP_PTR[3] = ($0c00/64)+4+framesAnimFire[frameFire]; // fire sprite		

	*IRQ_STATUS = IRQ_RASTER;
}


//--------------------------------------------------------------------
// high score functions


// very lame latest score "sorting" procedure
void updateHighScore (){

	if (score >= highScore3 && score<highScore2 ){
		highScore3 = score;
		lastScoreLine = 3;
	}
	if (score >= highScore2 && score<highScore1 ){
		highScore3 = highScore2;
		highScore2 = score;
		lastScoreLine = 2;
	}
	if (score >= highScore1 ){
		highScore3 = highScore2;
		highScore2 = highScore1; 
		highScore1 = score;
		lastScoreLine = 1;
	}
}

// prints top 3 scores using kickc standard lib functions
void printHighScores (){
	print_set_screen ( TITLE_SCREEN2+19+(40*8));
	print_uint_decimal (highScore1);
	print_set_screen ( TITLE_SCREEN2+19+(40*10));
	print_uint_decimal (highScore2);
	print_set_screen ( TITLE_SCREEN2+19+(40*12));
	print_uint_decimal (highScore3);
}

// flash latest entry on high score
void flashLatestScore (){
	word scline = 0;
	byte colorFlash[13] = {2,4,4,4,4,7,1,7,4,4,4,4,2};

	if (lastScoreLine!=0){
		if (lastScoreLine==1) scline = (40*8)+10;
		if (lastScoreLine==2) scline = (40*10)+10;
		if (lastScoreLine==3) scline = (40*12)+10;

		for (byte i=0; i<20 ; ++i){
			COLS[scline+i]= colorFlash[ scoreColorFlash ];
		}

		if (scoreColorFlash==12) scoreColorFlash=0;
		else scoreColorFlash++;
	}
}


//--------------------------------------------------------------------

// some help function to clean the scroll line on the map screen
void clearScrollLine (){
	for (byte i=0; i<40; i++){
		TITLE_SCREEN[(24*40)+i] = 0x20;
	}
}

//--------------------------------------------------------------------
// Load resources for title screens and map

kickasm (pc SPRITES_TITLEDATA){{
	.var  levelSprites2 = LoadBinary("data/sprites_title.bin")
	.fill levelSprites2.getSize(), levelSprites2.get(i)	
}}

//
// worldmap sprites + perfect sprites
// FIXME:  Remove robot anim and use the one from main sprites?
//
kickasm (pc PERFECT_SPRITES){{
	.var  perfectSprites = LoadBinary("data/perfect.bin")
	.fill perfectSprites.getSize(), perfectSprites.get(i)	
}}

kickasm (pc TITLE_CHARS){{
	.var  titleChar = LoadBinary("data/title_chars.bin")
	.fill titleChar.getSize(), titleChar.get(i)	
}}

kickasm (pc TITLE_SCREEN){{
	.var  titleScreen = LoadBinary("data/title_screen.bin")
	.fill titleScreen.getSize(), titleScreen.get(i)	
}}

kickasm (pc TITLE_SCREEN+$0400){{
	.var  titleScreen2 = LoadBinary("data/title_screen2.bin")
	.fill titleScreen2.getSize(), titleScreen2.get(i)	
}}

kickasm (pc WORLDMAP_CHARS){{
	.var  cw1 = LoadBinary("data/worldmap - Chars.bin")
	.fill cw1.getSize(), cw1.get(i)	
}}

kickasm (pc WORLDMAP_SCREEN){{
	.var  cw2 = LoadBinary("data/worldmap - Map (40x25).bin")
	.fill cw2.getSize(), cw2.get(i)	
}}

kickasm (pc WORLDMAP_COLORS){{
	.var  cw3 = LoadBinary("data/worldmap - CharAttribs.bin")
	.fill cw3.getSize(), cw3.get(i)	
}}



