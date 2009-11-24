/*============================================================================

    This file is part of Lithium II Mod for Quake II
    Copyright (C) 1997, 1998, 2009 Matthew A. Ayres

    Lithium II Mod is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Lithium II Mod is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Lithium II Mod.  If not, see <http://www.gnu.org/licenses/>.

    Quake II is a trademark of Id Software, Inc.

    Code by Matt "WhiteFang" Ayres, matt@lithium.com

============================================================================*/

#include "g_local.h"

#define MAX_MAPQUEUE 256
#define MAX_MAPNAME 16

lvar_t *mapqueue, *map_random;
cvar_t *mapqueue_pos;
lvar_t *use_mapqueue;

qboolean resetqueue = false;

char *override_map = NULL;
extern qboolean admin_override;

char start_cfg[64] = "";
char end_cfg[64] = "";

void Mapqueue_InitGame(void) {
	use_mapqueue = lvar("use_mapqueue", "1", "^", VAR_USE);
	mapqueue = lvar("mapqueue", "maps.lst", 0, VAR_OTHER);
	mapqueue_pos = gi.cvar("mapqueue_pos", "0", 0);
	map_random = lvar("map_random", "0", "^", VAR_OTHER);
}

void Mapqueue_InitLevel(void) {
	if(strlen(start_cfg)) {
		gi.AddCommandString(va("exec %s\n", start_cfg));
		strcpy(start_cfg, "");
	}
}

void Mapqueue_Override(char *mapname) {
	static char static_mapname[MAX_MAPNAME];
	strcpy(static_mapname, mapname);
	override_map = static_mapname;
}

edict_t *Mapqueue_EndDMLevel(void) {
	edict_t *ent;
	char *map;

	if(strlen(end_cfg)) {
		gi.AddCommandString(va("exec %s\n", end_cfg));
		strcpy(end_cfg, "");
	}

	if(override_map) {
		map = override_map;
		override_map = NULL;
		admin_override = false;
		resetqueue = true;
	}
	else {
		map = Mapqueue_GetMapName();
		if(!map)
			return NULL;
	}

	ent = G_Spawn();
	ent->classname = "target_changelevel";
	ent->map = map;
	
	return ent;
}

extern cvar_t *first;

void Mapqueue_SplitLine(char *buf, char *map, char *desc, char *start, char *end) {
	char *c, tmp[256];

	if(desc)
		strcpy(desc, "");
	if(start)
		strcpy(start, "");
	if(end)
		strcpy(end, "");

	c = strchr(buf, ' ');
	if(!c)
		c = strchr(buf, '\t');
	if(!c) {
		if(map)
			strcpy(map, buf);
		return;
	}
	if(map) {
		strncpy(map, buf, c - buf);
		map[c - buf] = 0;
	}

	c = strchr(buf, '\"');
	if(!c)
		return;
	strcpy(tmp, c + 1);
	c = strchr(tmp, '\"');
	if(desc) {
		strncpy(desc, tmp, c - tmp);
		desc[c - tmp] = 0;
	}

	strcpy(tmp, c + 2);
	if(start && end)
		sscanf(tmp, "%s %s", start, end);
}

char *Mapqueue_GetMapName(void) {
	static char map[MAX_MAPNAME];
	static char lastmapqueue[MAX_MAPNAME] = "";
	static char lastmap[MAX_MAPNAME] = "";
	static int maps = 0;
	static int lastmaps = -1;
	static int random[MAX_MAPQUEUE];

	FILE *file;
	char *c, buf[256];
	int i, count, pos;
	qboolean newlist = false;

	if(!use_mapqueue->value || !strlen(mapqueue->string))
		return NULL;

	file = fopen(file_gamedir(mapqueue->string), "rt");
	if(!file)
		return NULL;

	maps = 0;
	while(fgets(buf, 255, file)) {
		if(!strip(buf))
			continue;
		maps++;
	}
	fseek(file, 0, SEEK_SET);

	if(!first->value && (resetqueue || strcmp(level.mapname, lastmap) || maps != lastmaps || strcmp(mapqueue->string, lastmapqueue)))
		newlist = true;

	strcpy(lastmapqueue, mapqueue->string);
	strcpy(lastmap, level.mapname);
	lastmaps = maps;

	if(newlist) {
		count = 0;
		while(fgets(buf, 255, file)) {
			if(!strip(buf))
				continue;

			c = strchr(buf, ' ');
			if(c)
				*c = 0;

			if(resetqueue && !strcmp(level.mapname, buf)) {
				mapqueue_pos->value = count + 1;
				break;
			}

			count++;
		}
		fseek(file, 0, SEEK_SET);
	}

	if(mapqueue_pos->value >= maps)
		mapqueue_pos->value = 0;

	if(newlist || mapqueue_pos->value == 0) {
		int a, b, t;
		srand((unsigned)time(NULL));

		if(maps > MAX_MAPQUEUE)
			maps = MAX_MAPQUEUE;
		for(i = 0; i < maps; i++)
			random[i] = i;
		for(i = 0; i < maps * 10; i++) {
			a = rand() % maps;
			b = rand() % maps;
			t = random[a];
			random[a] = random[b];
			random[b] = t;
		}
	}

	if(map_random->value)
		pos = random[(int)mapqueue_pos->value];
	else
		pos = mapqueue_pos->value;

	count = 0;
	while(fgets(buf, 255, file)) {
		if(!strip(buf))
			continue;

		Mapqueue_SplitLine(buf, map, NULL, NULL, NULL);

		if(count == pos)
			break;

		count++;
	}

	fclose(file);

	resetqueue = false;
	mapqueue_pos->value++;

	if(!strlen(map))
		return NULL;

	// VWep has an index overflow issue with city3
	if(use_vwep->value && !Q_stricmp(map, "city3") && maps > 1)
		return Mapqueue_GetMapName();

	return map;
}

void Mapqueue_Menu(edict_t *ent, char *cmd) {
	FILE *file;
	static char c1[MAX_MAPQUEUE][32], c2[MAX_MAPQUEUE][MAX_MAPNAME + 8];
	char buf[256], map[MAX_MAPNAME], desc[32];
	int i = 0;

	file = fopen(file_gamedir(mapqueue->string), "rt");
	if(!file)
		return;

	while(fgets(buf, 256, file)) {
		if(!strip(buf))
			continue;

		Mapqueue_SplitLine(buf, map, desc, NULL, NULL);

		if(strlen(desc))
			strcpy(c1[i], desc);
		else
			strcpy(c1[i], map);
		sprintf(c2[i], "%s %s", cmd, map);

		Menu_AddLine(ent, MENU_CMD, 0, c1[i], c2[i]);

		i++;
	}		
	fclose(file);
}

qboolean Mapqueue_Valid(char *filename, char *mapname) {
	FILE *file;
	char buf[256];
	char map[MAX_MAPNAME];

	file = fopen(filename, "rt");
	if(!file)
		file = fopen(file_gamedir(mapqueue->string), "rt");
	if(!file)
		return false;

	while(fgets(buf, 256, file)) {
		if(!strip(buf))
			continue;

		Mapqueue_SplitLine(buf, map, NULL, NULL, NULL);
		if(!stricmp(mapname, map)) {
			fclose(file);
			return true;
		}
	}
	fclose(file);

	return false;
}
