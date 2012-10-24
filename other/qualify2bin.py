#! /usr/bin/python
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
# Convert qualifying set to binary file
# NOTE: this program uses the users file which was generated with countusers.py
import os
bin_path="data/qualify.bin"
qualify_path="../qualifying.txt"
users_path="data/users"

def int2bin(data):
    return chr(data&255)+chr((data>>8)&255)+chr((data>>16)&255)+chr((data>>24)&255)

def binwrite(fp,movie,buf):
    if not buf: return
    header=int2bin(movie)+int2bin(len(buf)/4)
    fpbin.write(header+buf)

users={}
fp=open(users_path)
iuser=0
for user in fp:
    user=user.strip()
    users[user]=iuser
    iuser+=1
fp.close()
    
fpbin=open(bin_path,"wb")
fp=open(qualify_path)
buf=''
movie=None
for line in fp:
    line=line.strip()
    if line.endswith(':'):
        binwrite(fpbin,movie,buf)
        movie=int(line[:-1])-1
        buf=''
    else:
        sline=line.split(',')
        user,date=sline
        iuser=users[user]
        buf+=int2bin(iuser)
binwrite(fpbin,movie,buf)
fpbin.close()
fp.close()
