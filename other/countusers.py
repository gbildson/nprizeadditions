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
#
# build the file "users" which translate users indexd from 0 to 480190-1 without
# gaps to user values used in the original netflix files.
import os
path="../training_set/"

fcount=0
users={}
minyear='9999'
for fname in os.listdir(path):
    if fcount % 10==0: print fcount
    if not (fname.startswith('mv_') and fname.endswith('.txt')):
        print 'Bad file name',fname
        continue
    fcount+=1
    movie=int(fname[3:-4])
    fp=open(path+fname)
    header = None
    for line in fp:
        line=line.strip()
        if not header:
            header=line.strip()
            if header!=str(movie)+':':
                print fname,'bad header',header
            continue
        sline=line.split(',')
        if len(sline)!=3:
            print fname,'bad line',line
            continue
        user,rank,date=sline
        user=int(user)
        if user<1 or user>2649429:
            print fname,'bad user',user
            continue
        users[user]=users.get(user,0)+1
        if rank<'1' or rank>'5':
            print fname,'bad rank',line
            continue
        sdate=date.split('-')
        if len(sdate)!=3:
            print fname,'bad date',line
            continue
        year,month,day=sdate
        if year<'1890' or year>'2005':
            print fname,'bad year',line
            continue
        if year<minyear:
            minyear=year
        if month<'01' or month>'12':
            print fname,'bad month',line
            continue
        if day<'01' or day>'31':
            print fname,'bad day',line
            continue
    fp.close()
print 'Oldest movie',minyear
print 'There are:'
print fcount,'movies'
fusers=open('data/users','w')
print len(users),'users:'
for user in users:
    print >>fusers,user
fusers.close()
