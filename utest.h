/*
########################################################################
#  Netflix Prize Tools
#  Copyright (C) 2007-8 Ehud Ben-Reuven
#  udi@benreuven.com
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation version 2.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
########################################################################
*/
extern int useridx[NUSERS][4];
extern unsigned int userent[NENTRIES];
extern float err[NENTRIES];
extern int aopt;
extern int dontclip;
#define UNTRAIN(u)  (aopt?(useridx[u][1]+useridx[u][2]):(useridx[u][1]))
#define UNALL(u)    (aopt?(useridx[u][1]+useridx[u][2]+useridx[u][3]):(useridx[u][1]+useridx[u][2]))
#define UNTOTAL(u)  (useridx[u][1]+useridx[u][2]+useridx[u][3])
extern char *useridx_path;
extern char *userent_path;
extern char *fname_rmovie;
extern int load_model;
extern int save_model;
