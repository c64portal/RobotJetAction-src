/* 
 * Robot Jet Action
 * Commodore 64 Arcade-like game
 * Development started on 2020.04.15 
 * by Tomasz 'Carrion' Mielnik / Bonzai / C64Portal.pl 
 *
 * levels.c
 * Levels and Enemies definitions and descriptors
 */

// Levels description structure & offsets
// each level has it's own file with description acoording to this specs
//-----------------------------------------------------------------
// 00 	byte pStartX;
// 01  	byte pStartY;
// 02  	byte keys;
// 03  	byte e1_ptr;  //// ------------ 1 
// 04  	byte e1_x, 
// 05 	byte e1_y;
// 06  	byte e1_table1;
// 07  	byte e1_t1speed;
// 08  	byte e1_t1ampl;
// 09  	byte e1_table2;
// 10  	byte e1_t2speed;
// 11  	byte e1_t2ampl;
// 12 	byte e2_ptr;	//// ------------ 2
// 13 	byte e2_x, 
// 14 	byte e2_y;
// 15  	byte e2_table1;
// 16  	byte e2_t1speed;
// 17  	byte e2_t1ampl;
// 18  	byte e2_table2;
// 19  	byte e2_t2speed;
// 20  	byte e2_t2ampl;
// 21  	byte e3_ptr;	//// ------------ 3
// 22  	byte e3_x, 
// 23 	byte e3_y;
// 24 	byte e3_table1;
// 25  	byte e3_t1speed;
// 26  	byte e3_t1ampl;
// 27  	byte e3_table2;
// 28  	byte e3_t2speed;
// 29  	byte e3_t2ampl;
// 30  	byte e4_ptr;	//// ------------ 4
// 31  	byte e4_x, 
// 32 	byte e4_y;
// 33  	byte e4_table1;
// 34  	byte e4_t1speed;
// 35  	byte e4_t1ampl;
// 36  	byte e4_table2;
// 37  	byte e4_t2speed;
// 38  	byte e4_t2ampl;
// 39  	byte sprcolor;
// 40  	byte exitX;
// 41 	byte exitY;
// 42   byte minKeys;


//-- Movement paths ---------------------------------
// Look Up Tables for sinus/cosinus and linear enemy movements

export byte[256] sinus1 = kickasm {{
	.fill 256,round(80+80*sin(toRadians(i*360/256)))
}};

export byte[256] cosinus1 = kickasm {{
	.fill 256,round(80+80*cos(toRadians(i*360/256)))
}};

export byte[256] cosinus2 = kickasm {{
	.fill 256,round(120+(-120)*abs(cos(toRadians(i*360/256))) )
}};

export byte[256] line0 = kickasm {{
	.fill 256,0;
}};

export byte[256] line1 = kickasm {{
	.for (var i=0; i<$100; i++){
		.byte i
	} 
}};

export byte[256] line2 = kickasm {{
	.for (var i=0; i<$80; i++){
		.byte i
	}
	.for (var i=$80; i>0; i--){
		.byte i
	}
}};

export byte *movs[6] = {
	&line0, &line1, &line2, &sinus1, &cosinus1, &cosinus2
}; 

//------------------------------------------------------------
//------- Worlds params and colors ---------
struct worlds {
	byte levels;
	byte bgcolor;
	byte color1;
	byte color2;
	byte color3;
	byte spritecolor;
};

export struct worlds[5] world = kickasm {{
	.byte 7-1,0,6,4,$e,$5;
	.byte 7-1,0,9,2,8, $c;
	.byte 7-1,0,9,4,$e,$a;
	.byte 7-1,0,6,4,$a,$5;
	.byte 7-1,0,$b,5,3,$a;
}};

//-------- CHARMAPS ------------------------------------------------
// exported from Charpad and compressed with byteboozer
kickasm (pc CHARMAPS) {{
charmap1:
	.var  c1 = LoadBinary("data/w1/chars.bin.prg.b2",BF_C64FILE)
	.fill c1.getSize(), c1.get(i)	
}}

//-------- COLOR MAPS ------------------------------------------------
// exported from Charpad and compressed with byteboozer.
// Only character set for world 1(Monsterland) are distributed in this release.

kickasm (pc COLORMAPS) {{
colormap1:
	.var  cm1 = LoadBinary("data/w1/colors.bin.prg.b2",BF_C64FILE)
	.fill cm1.getSize(), cm1.get(i)	
}}

//------- ANIMATIONS  -----------------------
// Exported from Charpad not compressed.
// These are the anim frames for characters like collectables, water, etc
// Only animations for world 1(Monsterland) are distributed in this release.
// 
export byte[256] w1anim = kickasm {{
	.var w1a = LoadBinary("data/w1/w1_anims.bin")
	.fill w1a.getSize(), w1a.get(i)
}};


//-------- MAPS ------------------------------------------------

// Only maps for world 1(Monsterland) are distributed in this release.

kickasm (pc MAPS){{
map1:
	.var  m1 = LoadBinary("data/w1/l1.asm.prg.b2",BF_C64FILE)
	.fill m1.getSize(), m1.get(i)	
	 
map2:
	.var  m2 = LoadBinary("data/w1/l2.asm.prg.b2",BF_C64FILE)
	.fill m2.getSize(), m2.get(i)	
	 
map3:
	.var  m3 = LoadBinary("data/w1/l3.asm.prg.b2",BF_C64FILE)
	.fill m3.getSize(), m3.get(i)	
	 
map4:
	.var  m4 = LoadBinary("data/w1/l4.asm.prg.b2",BF_C64FILE)
	.fill m4.getSize(), m4.get(i)	
	 
map5:
	.var  m5 = LoadBinary("data/w1/l5.asm.prg.b2",BF_C64FILE)
	.fill m5.getSize(), m5.get(i)	
	 
map6:
	.var  m6 = LoadBinary("data/w1/l6.asm.prg.b2",BF_C64FILE)
	.fill m6.getSize(), m6.get(i)	
	 
map7:
	.var  m7 = LoadBinary("data/w1/l7.asm.prg.b2",BF_C64FILE)
	.fill m7.getSize(), m7.get(i)	

}}


//-------- Pointers ------------------------------------------------

#pragma data_seg(DataHigh)

// pointers to levels. Only World 1. 
// Repeated 5 times for 5 worlds

word[36] mapptrs = kickasm {{
	.word map1 ,map2 ,map3 ,map4 ,map5 ,map6 ,map7
	.word map1 ,map2 ,map3 ,map4 ,map5 ,map6 ,map7
	.word map1 ,map2 ,map3 ,map4 ,map5 ,map6 ,map7
	.word map1 ,map2 ,map3 ,map4 ,map5 ,map6 ,map7
	.word map1 ,map2 ,map3 ,map4 ,map5 ,map6 ,map7


	// The following are the maps of other worlds as used in final game.

	// .word map8 ,map9 ,map10,map11,map12,map13,map14	
	// .word map29,map30,map31,map32,map33,map34,map35
	// .word map22,map23,map24,map25,map26,map27,map28
	// .word map15,map16,map17,map18,map19,map20,map21
}}; 

//pointers to charactersets
export word[5] charptrs = kickasm {{
	.word charmap1, charmap1, charmap1, charmap1, charmap1
}};

// pointers to colors for each worlds characterset
export word[5] colorptrs = kickasm {{
	.word colormap1, colormap1, colormap1, colormap1, colormap1
}};

// pointers to each worlds animations
// done as a C array to show that it can be initiated like this - not only via kickasm {{ }}
export byte *animptrs[5] = {
	&w1anim, &w1anim, &w1anim, &w1anim, &w1anim
};


