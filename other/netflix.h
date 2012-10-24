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
#define NMOVIES (17770)
#define NUSERS (480189)
#define NPROBE (1408395)
#define NTRAIN (99072112)
#define NQUALIFY (2817131)
#define NMOVIES_QUALIFY (17470) // not all movies appear in qualify
#define NUSERS_QUALIFY (478615)
#define NQUALIFY_SIZE (2852071)		

#define NENTRIES (103297638) // Total number of entries (training+probe+qualify)
#define MOVIE_LUSERMASK (19)
#define MOVIE_USERMASK (0x7ffff) // (1<<MOVIE_LUSERMASK)-1
#define MOVIE_LDAY (22)
#define USER_LMOVIEMASK (15)
#define USER_LDAY (18)
#define USER_MOVIEMASK (0x7fff) // (1<<USER_LMOVIEMASK)-1
#define CINEMATCH_RMSE (0.9514)
#define AVG_SCORE (2.603304)
#define MIN_DAY (680)
#define MAX_DAY (2922)
#define MAX_ENTERIES_PER_MOVIE (232944) // 0x38df0 < 1<<18

static char *movieidx_path="data/movie_index.bin";
static char *movieent_path="data/movie_entry.bin";
static char *moviedate_path="data/movie_date.bin";

