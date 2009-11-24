/*====================================================

  Lithium II Mod for Quake II.
  Copyright (c) 1997-1998 Matt Ayres.
  All rights reserved.

  Distribution of this source file, by any means, is
  hereby forbidden.  You may not use this source file
  without permission.  Do not remove this notice.

  Code by Matt "WhiteFang" Ayres, matt@lithium.com
  Of Lithium Software, http://www.lithium.com

  Quake II is a trademark of Id Software, Inc.

====================================================*/

#include "g_local.h"

extern lvar_t *use_runes;

void CTFSetIDView(edict_t *ent);

void Lithium_SetStats(edict_t *self) {
	short *stats = self->client->ps.stats;
	int i;
	qboolean statbar;
	qboolean observe;

	edict_t *ent = self;

	if(self->client->chase_target)
		ent = self->client->chase_target;

	if(ent->safety_time)
		stats[1] = 0;

	stats[25] = ent->client->resp.score;

	if(self->hud == HUD_NONE && !ctf->value) {
		for(i = 0; i < 32; i++)
			if(i != STAT_FRAGS)
				stats[i] = 0;
		if(self->layout & LAYOUT_MENU)
			stats[STAT_LAYOUTS] = 1;
		return;
	}

	if(self->client->chase_target) {
		for(i = 0; i <= 15; i++)
			if(i != STAT_FRAGS)
				stats[i] = ent->client->ps.stats[i];
		stats[STAT_LAYOUTS] = 1;
	}

	statbar = !(self->layout & (LAYOUT_SCORES | LAYOUT_HSCORES | LAYOUT_MENU)) && !level.intermissiontime;
	observe = self->lithium_flags & LITHIUM_OBSERVER && !self->client->chase_target;

	if(observe)
		for(i = 0; i <= 12; i++)
			stats[i] = 0;

	if(self->hud == HUD_LITHIUM) {
		if(!level.intermissiontime) {
			// CTF uses 26 and 27 for team header
			// flashing during intermission
			stats[26] = (short)ent->fph;
			stats[27] = (short)countplayers();
			if(ent->board == SCORES_BYFPH)
				stats[28] = (short)ent->fph_place;
			else
				stats[28] = (short)ent->place;
		}

		stats[29] = 0;
		if(statbar && use_runes->value) {
			for(i = 0; i < NUM_RUNES; i++) {
				if(ent->rune == 1 << i)
					stats[29] = CS_HUDRUNE1 + i;
			}
			if(!stats[29])
				stats[29] = CS_HUDRUNE0;
		}
	}

	if(self->hud == HUD_AMMO && !ctf->value) {
		// ammo: 20-25
		stats[17] = ent->client->pers.inventory[ITEM_INDEX(FindItem("shells"))];
		stats[18] = ent->client->pers.inventory[ITEM_INDEX(FindItem("bullets"))];
		stats[19] = ent->client->pers.inventory[ITEM_INDEX(FindItem("grenades"))];
		stats[20] = ent->client->pers.inventory[ITEM_INDEX(FindItem("rockets"))];
		stats[21] = ent->client->pers.inventory[ITEM_INDEX(FindItem("cells"))];
		stats[22] = ent->client->pers.inventory[ITEM_INDEX(FindItem("slugs"))];
	}

	stats[30] = stats[31] = 0;
	if(!level.intermissiontime) {
		if(observe)
			stats[30] = CS_OBSERVING;
		else if(ent->safety_time)
			stats[30] = CS_SAFETY;
		stats[31] = statbar;
	}

	if(ctf->value)
		SetCTFStats(ent, stats);
}

//===============================
// statusbar

void StatusBar_UpdateAll(void) {
	int i;
	edict_t *ent;

	for(i = 0; i < game.maxclients; i++) {
		ent = g_edicts + 1 + i;
		if(!ent->inuse)
			continue;
		ent->lithium_flags |= LITHIUM_STATUSBAR;
	}
}

char *ammobar =
"if 31 "
"yt 40 xr -24 picn a_shells xr -74 num 3 17 "
"yt 64 xr -24 picn a_bullets xr -74 num 3 18 "
"yt 88 xr -24 picn a_grenades xr -74 num 3 19 "
"yt 112 xr -24 picn a_rockets xr -74 num 3 20 "
"yt 136 xr -24 picn a_cells xr -74 num 3 21 "
"yt 160 xr -24 picn a_slugs xr -74 num 3 22 "
"endif "
;

char *bottombar =
"yb	-24 "

// ammo
"if 2 xv 100 anum xv 150 pic 2 endif "

// armor
"if 4 xv 200 rnum xv 250 pic 4 endif "

// selected item
"if 6 xv 296 pic 6 endif "

"yb	-50 "

// timer
"if 9 xv 246 num 2 10 xv 296 pic 9 endif "

//  help / weapon icon 
"if 11 xv 148 pic 11 endif "

// picked up item
"if 7 xv 0 pic 7 xv	26 yb -42 stat_string 8 endif "

// config string output (observing / safety msg)
"if 30 yb -16 xv 8 stat_string 30 endif "

// health
"if 1 yb -24 xv 0 hnum xv 50 pic 0 endif "
;

char *ctf_statusbar =
// tech
"yb -129 if 24 xr -26 pic 24 endif "

// red team
"yb -102 if 17 xr -26 pic 17 endif "
"xr -62 num 2 18 "
//joined overlay
"if 22 yb -104 xr -28 pic 22 endif "

// blue team
"yb -75 if 19 xr -26 pic 19 endif "
"xr -62 num 2 20 "
"if 23 yb -77 xr -28 pic 23 endif "

// have flag graph
"if 21 yt 26 xr -24 pic 21 endif "

// frags
"yt 0 xr -50 num 3 25 "

// runes
"if 29 "
"xr -36 yb -150 string Rune "
"xr -68 yb -142 stat_string 29 "
"endif "
;

int StatusBar_Update(edict_t *ent) {
	char statusbar[1400] = "";

	if(ent->layout & LAYOUT_MENU)
		return 0;

	if(!(ent->lithium_flags & LITHIUM_STATUSBAR))
		return 0;
	ent->lithium_flags &= ~LITHIUM_STATUSBAR;

	if(ent->hud > HUD_NONE)
		strcat(statusbar, bottombar);

	if(ent->hud == HUD_NORMAL || ent->hud == HUD_AMMO && !ctf->value)
		strcat(statusbar, "yt 0 xr -50 num 3 25 ");

	if(ent->hud == HUD_AMMO && !ctf->value)
		strcat(statusbar, ammobar);

	if(ent->hud == HUD_LITHIUM && !ctf->value) {
		strcat(statusbar,
			"if 31 "
			"xr -44 "
			"yt 16 string Frags "
			"yt 130 string Place "
			"xr -68 "
			"yt 54 string Frags/Hr "
			"yt 92 string \" Players\" "
			"yt 24 num 4 25 "
			"yt 62 num 4 26 "
			"yt 100 num 4 27 "
			"yt 138 num 4 28 "
			"endif "

			// runes
			"if 29 "
			"xr -36 yt 168 string Rune "
			"xr -68 yt 176 stat_string 29 "
			"endif "
			);
	}

	if(ctf->value)
		strcat(statusbar, ctf_statusbar);

	gi.WriteByte(0x0D);
	gi.WriteShort(5);
	gi.WriteString(statusbar);
	gi.unicast(ent, true);

	return strlen(statusbar);
}

char *GetMOTD(void) {
	int pos, line, lines;
	FILE *file;
	char *c, buf[256];
	char add[256] = "";
	static char motdstr[640];

	file = fopen(file_gamedir(motd->string), "rt");
	lines = 0;
	if(file) {
		while(fgets(buf, 256, file))
			lines++;
		fseek(file, 0, SEEK_SET);
	}

	if(lines)
		lines++;

	lines += 3;

	// insure MOTD will fit on 320x240 screens
//	if(lines > 18)
//		lines = 18;

	pos = -60 - lines * 8;

	strcpy(motdstr, "xl 8 ");

	line = 4;
	if(file) {
		while(fgets(buf, 256, file)) {
			c = strchr(buf, 0x0A);
			if(c) *c = 0;
			c = strchr(buf, 0x0D);
			if(c) *c = 0;

			// insure MOTD will fit on 320x240 screens
//			if(strlen(buf) > 32)
//				buf[32] = '\0';

			if(strlen(buf) && strlen(motdstr) < 512) {
				sprintf(add, "yb %d string \"%s\" ", pos, buf);
				strcat(motdstr, add);
			}
 
			pos += 8;

			line++;
			if(line == lines)
				break;
		}

		pos += 8;

		fclose(file);
	}

	sprintf(add,
		"yb %d string2 \"Lithium II Mod v%s\" "
		"yb %d string \"By Matt Ayres (WhiteFang)\" "
		"yb %d string \"http://www.lithium.com\" "
		, pos, lithium_version, pos + 8, pos + 16);

	strcat(motdstr, add);

	return motdstr;
}

qboolean isnews = false;

char *GetNews(void) {
	int pos, line, lines;
	FILE *file;
	char *c, buf[256];
	char add[256];
	static char newsstr[1024];

	file = fopen(file_gamedir(news->string), "rt");

	isnews = false;

	if(!file)
		return "";

	lines = 0;
	while(fgets(buf, 256, file))
		lines++;
	fseek(file, 0, SEEK_SET);

	if(!lines)
		return "";

	isnews = true;

	pos = -60 - lines * 8;

	strcpy(newsstr, "xl 8 ");

	line = 1;
	while(fgets(buf, 256, file)) {
		c = strchr(buf, 0x0A);
		if(c) *c = 0;
		c = strchr(buf, 0x0D);
		if(c) *c = 0;

		if(strlen(buf) && strlen(newsstr) < 640) {
			sprintf(add, "yb %d string \"%s\" ", pos, buf);
			strcat(newsstr, add);
		}
 
		pos += 8;

		line++;
		if(line == lines)
			break;
	}
	strcat(newsstr, add);

	fclose(file);

	return newsstr;
}

char *GetCenterprint(edict_t *ent) {
	static char centerprint[1024];

	strcpy(centerprint, "");

	if(ent->centerprint && ent->centerprint2 && strlen(ent->centerprint2))
		strcpy(ent->centerprint, ent->centerprint2);

	if(ent->centerprint && strlen(ent->centerprint)) {
		int i, len, lines = 0;
		char line[20][40];
		char *c = ent->centerprint;
		char *d = c;

		while(*d) {
			d = strchr(c, '\n');
			if(!d)
				d = c + strlen(c);
			if(d) {
				len = d - c;
				if(len > 39)
					len = 39;
				strncpy(line[lines], c, len);
				line[lines][len] = '\0';
				lines++;
			}
			c = d + 1;
		}

		strcat(centerprint, "xv 0 ");
		for(i = 0; i < lines; i++) {
			if(strlen(line[i]))
				sprintf(centerprint + strlen(centerprint), "yv %d cstring \"%s\" ", (200 - lines * 8) / 2 + i * 8, line[i]);
		}
	}

	return centerprint;
}

int Layout_Update(edict_t *ent) {
	char string[1024] = "";
	int size;

	if(level.intermissiontime) {
		if(ent->layout_update) {
			size = Lithium_Scoreboard(ent, NULL);
			gi.unicast(ent, true);
			ent->layout_update = false;
			return size;
		}
		return 0;
	}

	if(!ent->layout_update) {
		if(ent->layout & LAYOUT_SCORES && !(level.framenum & 15)) {
			size = Lithium_Scoreboard(ent, ent->enemy);
			gi.unicast(ent, false);
			return size;
		}
		return 0;
	}
	ent->layout_update = false;

	if(ent->layout & LAYOUT_MENU)
		return Menu_Update(ent);

	if(ent->layout & LAYOUT_SCORES) {
		size = Lithium_Scoreboard(ent, ent->enemy);
		gi.unicast(ent, true);
		return size;
	}
	
	if(ent->layout & LAYOUT_HSCORES) {
		size = Highscores_FullScoreboard(ent);
		gi.unicast(ent, true);
		return size;
	}
	
	if(ent->layout & LAYOUT_CENTERPRINT)
		strcat(string, GetCenterprint(ent));

	if((level.time > ent->motd_time || !(ent->layout & LAYOUT_MOTD)) && ent->layout & LAYOUT_NEWS && isnews)
		strcat(string, GetNews());
	else if(ent->layout & LAYOUT_MOTD)
		strcat(string, GetMOTD());
	else {
		if(ent->layout & LAYOUT_CHASECAM && ent->client->chase_target)
			sprintf(string + strlen(string), "xv 2 yb -68 string2 \"Chasing %s\" ", ent->client->chase_target->client->pers.netname);

		if(ent->layout & LAYOUT_ID) {
			if(ent->id_ent) {
				if(ctf->value)
					sprintf(string + strlen(string), "xv 2 yb -60 string2 \"Viewing %s (%s)\" ", 
						ent->id_ent->client->pers.netname, Info_ValueForKey(ent->id_ent->client->pers.userinfo, "skin"));
				else
					sprintf(string + strlen(string), "xv 2 yb -60 string2 \"Viewing %s\" ", ent->id_ent->client->pers.netname);
			}
			else
				sprintf(string + strlen(string), " ");
		}
	}

	gi.WriteByte(svc_layout);
	gi.WriteString(string);
	gi.unicast(ent, true);

	return strlen(string);
}

char ad[5][64] = { 
	"Get Qoole at www.qoole.com",
	"Quake II level editing made simple",
	"",
	"",
	""
};

char *Lithium_GetAd(int down) {
	int i;
	static char thead[320];
	strcpy(thead, "");
	for(i = 0; i < 5; i++)
		if(strlen(ad[i]))
			strcat(thead, va("xv 0 yv %d cstring \"%s\" ", down + i * 8, ad[i]));
	return thead;
}

void Lithium_SetAd(int num, char *str) {
	strcpy(ad[num], str);
}

int Lithium_Scoreboard(edict_t *ent, edict_t *killer) {
	char	entry[1024];
	char	string[1400] = "";
	int		stringlength;
	int		i, j;
	gclient_t	*cl;
	edict_t		*cl_ent;
	qboolean highlight;
	int down;

//ZOID
	if (ctf->value) {
		CTFScoreboardMessage (ent, killer);
		return countclients() * 50;
	}
//ZOID

	if(!ent->board) {
		DeathmatchScoreboardMessage(ent, killer);
		return countclients() * 50;
	}

	if(ent->menu)
		return 0;

	down = 90 - ent->lclient->board_show * 4;

	// print level name and exit rules
	string[0] = 0;

	stringlength = strlen(string);

	if(use_highscores->value)
		Highscores_Scoreboard(string, &down);

	if(ent->board == SCORES_BYFRAGS)
		strcat(string, va("yv %d xv 152 string2 Frags xv 208 string \"FPH Time Ping\" ", down));
	else if(ent->board == SCORES_BYFPH)
		strcat(string, va("yv %d xv 152 string \"Frags      Time Ping\" xv 208 string2 FPH ", down));
	
	strcat(string, "xv 0 ");
	for(i = 0; i < ent->lclient->board_show; i++) {
		j = ent->sel + i;
		if(j >= sorted_ents)
			break;

		if(ent->board == SCORES_BYFPH)
			cl_ent = fph_sorted_ent[j];
		else
			cl_ent = sorted_ent[j];

		if(!cl_ent->lclient)
			continue;

		cl = cl_ent->client;

		highlight = (!ent->client->chase_target && cl_ent == ent) || cl_ent == ent->client->chase_target;

		if(cl_ent->lithium_flags & LITHIUM_PLAYING)
			sprintf(entry, "yv %d string%s \"%2d %-16s %4d %4d %4d %4d\" ", i * 10 + down + 10, highlight ? "2" : "",
				j + 1, cl->pers.netname, cl->resp.score, cl_ent->fph, cl_ent->play_frames / 600, MIN(cl_ent->lclient->ping, 999));
		else {
			if(cl->chase_target)
				sprintf(entry, "yv %d string%s \"   %-16s  (chasing #%d)%s %4d\" ", i * 10 + down + 10, highlight ? "2" : "", 
					cl->pers.netname, cl->chase_target->place, cl->chase_target->place < 10 ? " " : "", MIN(cl_ent->lclient->ping, 999));
			else
				sprintf(entry, "yv %d string%s \"   %-16s  (observer)    %4d\" ", i * 10 + down + 10, highlight ? "2" : "", 
					cl->pers.netname, MIN(cl_ent->lclient->ping, 999));
		}

		strcat(string, entry);
	}

	if(level.intermissiontime) {
		strcat(string, Lithium_GetAd(down + ent->lclient->board_show * 10 + 32));
	}
	else {
		if(countclients() > ent->lclient->board_show)
			strcat(string, va("xv 0 yv %d cstring \"Use [ and ] to scroll scores\" ", down + ent->lclient->board_show * 10 + 16));
		strcat(string, va("xv 0 yv %d cstring \"Press 0 for Lithium II menu\" ", down + ent->lclient->board_show * 10 + 24));
	}

	gi.WriteByte(svc_layout);
	gi.WriteString(string);

	return strlen(string);
}
