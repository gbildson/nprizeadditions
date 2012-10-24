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
# NOTE: this program uses the users file which was generated with countusers.py
#
# Convert all netflix data (training, probe and qualify) into movie oriented
# binary files: index, entries, date
#
# the index file contains for each movie its start location in the data followed
# by a triplet of number of  training, probe and qualify entries.
#
# The date file stores in a binary short number the exact date:
# day (5bits),month (4bits),year
#
# The entry file contain the following in a binary int:
# user index (19bits), rank or 7 if not ranked (3bits), year(3bits),month(4bits),day/4
#
import os
# input files
training_path="../training_set/"
users_path="data/users"
qualify_path="../qualifying.txt"
probe_path="../probe.txt"
NMOVIES=17770

# output files
movieidx_path="data/movie_index.bin"
movieent_path="data/movie_entry.bin"
moviedate_path="data/movie_date.bin"

def int2bin(data):
    return chr(data&255)+chr((data>>8)&255)+chr((data>>16)&255)+chr((data>>24)&255)
def int2sbin(data):
    return chr(data&255)+chr((data>>8)&255)
def date2sbin(year,month,day):
    return int2sbin((year<<(5+4))|(month<<5)|day)
def entry2bin(iuser,rank,year,month,day):
    return int2bin(iuser|rank<<19|year<<(19+3)|month<<(19+3+3)|(day>>2)<<(19+3+3+4))

# read userid to user index table
users={}
fp=open(users_path)
iuser=0
for user in fp:
    user=user.strip()
    users[user]=iuser
    iuser+=1
fp.close()
print "Users",iuser

# read qualifying entries, dates
qualifyent=[]
qualifydat=[]
for movie in range(NMOVIES):
    qualifyent.append('')
    qualifydat.append('')
fp=open(qualify_path)
count=0
for line in fp:
    line=line.strip()
    if line.endswith(':'):
        movie=int(line[:-1])-1
    else:
        count+=1
        sline=line.split(',')
        user,date=sline
        iuser=users[user]
        rank=7 # This signals that there is no known rank for this entry
        sdate=date.split('-')
        year,month,day=sdate
        year=int(year)-1998
        month=int(month)-1
        day=int(day)-1
        qualifydat[movie]+=date2sbin(year,month,day)
        qualifyent[movie]+=entry2bin(iuser,rank,year,month,day)
fp.close()
print "Qualify",count

# read probe entries
probe=[]
for movie in range(NMOVIES):
    probe.append({})
fp=open(probe_path)
count=0
for line in fp:
    line=line.strip()
    if line.endswith(':'):
        movie=int(line[:-1])-1
    else:
        count+=1
        iuser=users[line]
        probe[movie][iuser]=1
fp.close()
print "Probe",count

# read the training files and write idx, date and entries files
fpidx=open(movieidx_path,"wb")
fpent=open(movieent_path,"wb")
fpdat=open(moviedate_path,"wb")
movieidx=0
for movie in range(NMOVIES):
    if movie % 10==0: print movie
    fp=open(training_path+'mv_'+'%07d'%(movie+1)+'.txt')
    fmovie = None
    trainent=''
    traindat=''
    probeent=''
    probedat=''
    for line in fp:
        line=line.strip()
        if not fmovie:
            assert line.endswith(':')
            fmovie=int(line[:-1])
            assert fmovie-1==movie
            continue
        sline=line.split(',')
        user,rank,date=sline
        iuser=users[user]
        rank=int(rank)-1
        sdate=date.split('-')
        year,month,day=sdate
        year=int(year)-1998
        month=int(month)-1
        day=int(day)-1
        if(iuser in probe[movie]):
            probedat+=date2sbin(year,month,day)
            probeent+=entry2bin(iuser,rank,year,month,day)
        else:
            traindat+=date2sbin(year,month,day)
            trainent+=entry2bin(iuser,rank,year,month,day)
    fp.close()

    fpidx.write(int2bin(movieidx))
    ntrain=len(trainent)/4
    nprobe=len(probeent)/4
    nqualify=len(qualifyent[movie])/4
    movieidx+=ntrain+nprobe+nqualify
    fpidx.write(int2bin(ntrain))
    fpidx.write(int2bin(nprobe))
    fpidx.write(int2bin(nqualify))
    
    fpent.write(trainent) 
    fpent.write(probeent)
    fpent.write(qualifyent[movie])
    
    fpdat.write(traindat) 
    fpdat.write(probedat)
    fpdat.write(qualifydat[movie])
fpidx.close()
fpent.close()
fpdat.close()
