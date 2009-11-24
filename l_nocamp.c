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

lvar_t *use_nocamp;
lvar_t *camp_threshold;
lvar_t *camp_time;
lvar_t *camp_warn;

void NoCampTimeChanged(void) {
	if(camp_time->value > 30)
		gi.cvar_set(camp_time->cvar->name, "30");
	if(camp_time->value < 0)
		camp_time->value = 0;
}

void NoCamp_InitGame(void) {
	use_nocamp = lvar("use_nocamp", "0", "^", VAR_USE);

	camp_threshold = lvar("camp_threshold", "250", "####", VAR_OTHER);
	camp_time = lvar("camp_time", "8.0", "##.#", VAR_OTHER);
	camp_warn = lvar("camp_warn", "5.0", "##.#", VAR_OTHER);

	camp_time->func = NoCampTimeChanged;
	NoCampTimeChanged();
}

void NoCamp_PutClientInServer(edict_t *ent) {
	gclient_t *client = ent->client;
	int i;

	client->decamp_time = level.time + 0.5;
	client->decamp_num = 0;
	client->decamp_vec[0] = ent->s.origin[0];
	client->decamp_vec[1] = ent->s.origin[1];
	client->decamp_vec[2] = ent->s.origin[2];
	for(i = 0; i < camp_time->value * 2; i++)
		client->decamp_move[i] = 100;
}

void NoCamp_ClientThink(edict_t *ent, usercmd_t *ucmd) {
	gclient_t *client = ent->client;
	int i, move, last_num, sec;

	if(!use_nocamp->value || !camp_threshold->value || !camp_time->value)
		return;
	if(ent->deadflag != DEAD_NO)
		return;
	if(ent->lithium_flags & LITHIUM_OBSERVER)
		return;

	if(level.time < client->decamp_time) {
		if(client->decamp_count) {
			if(client->decamp_count < 4 && client->decamp_fire && (ucmd->forwardmove || ucmd->sidemove))
				if(ent->centerprint && strlen(ent->centerprint))
					centerprintf(ent, "");

			if(ucmd->buttons & BUTTON_ATTACK)
				client->decamp_fire++;
		}
		return;
	}

	client->decamp_time = level.time + 0.5;

	last_num = client->decamp_num;

	client->decamp_move[client->decamp_num] = 
		abs(ent->s.origin[0] - client->decamp_vec[0]) +
		abs(ent->s.origin[1] - client->decamp_vec[1]) +
		abs(ent->s.origin[2] - client->decamp_vec[2]);

	client->decamp_vec[0] = ent->s.origin[0];
	client->decamp_vec[1] = ent->s.origin[1];
	client->decamp_vec[2] = ent->s.origin[2];

	client->decamp_num++;
	if(client->decamp_num == camp_time->value * 2)
		client->decamp_num = 0;
	client->decamp_move[client->decamp_num] = 0;

	move = 0;
	for(i = 0; i < camp_time->value * 2; i++)
		move += client->decamp_move[i];

	if(move > camp_threshold->value * (camp_time->value - 2) / 5) {
		if(client->decamp_count && client->decamp_fire) {
			centerprintf(ent, "");
			client->decamp_count = 0;
			client->decamp_fire = 0;
		}
		return;
	}

	if(!client->decamp_fire || ent->layout & (LAYOUT_SCORES | LAYOUT_MOTD | LAYOUT_MENU)) {
		if(!client->decamp_count)
			client->decamp_count++;
		return;
	}

	sec = camp_warn->value - client->decamp_count / 2 + 2;
	if(sec >= 0 && sec <= camp_warn->value && client->decamp_count > 3)
		centerprintf(ent, "No camping.  Get moving.\n%d seconds to comply.\n", sec);

	if(client->decamp_count > camp_warn->value * 2 + 5) {
		centerprintf(ent, "Killed for camping.");

		ent->health = 0;
		player_die(ent, ent, ent, 100000, vec3_origin);
		ent->deadflag = DEAD_DEAD;
		respawn(ent);
	}

	client->decamp_count++;
}

