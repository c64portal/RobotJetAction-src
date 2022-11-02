/* 
 * Robot Jet Action
 * Commodore 64 Arcade-like game
 * Development started on 2020.04.15 
 * by Tomasz 'Carrion' Mielnik / Bonzai / C64Portal.pl 
 *
 * nmi.c
 * RUNStop + Restore handler. Included by all screens. 
 */

#ifndef _NMI
#define _NMI

void NMIhandle (){
 
kickasm	{{
   	lda #<NMI
   	sta $fffa
   	lda #>NMI
   	sta $fffb
   	rts
 NMI:
	rti
	}}
}

#endif