########################################################################
#  Netflix Prize Tools
#  Copyright (C) 2009 Greg Bildson
#  http://code.google.com/p/nprizeadditions/
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
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301,USA.
########################################################################

To run this package, you must first obtain the original nprize code and follow the first six 
steps there.  See the readme here for more information:
  http://code.google.com/p/nprize/

This package contains code for a pure restricted boltzmann machine (rbm.c), a 
conditional restricted boltzmann machine (rbmcond.c) and a simple baseline
calculator for use with the integrated model (ubest.c).

If you have trouble making this package due to not having the lapack libraries, you
can comment out the call to dposv in mix2.c and everything should compile.  Lapack
is really only required when blending with the full nprize package.

To run the pure rbm:
1) make rbm

In the next command, the "-l" parameter doesn't have its normal affect from the nprize code.
It just signals to run things once.  "-se " specifies the output residual file.  When I run this on linux,
I use nohup and keep things running in the background.  This should give you a probe RMSE of 0.918197.
2) ./rbm -l 1 -se data/r100_01.bin > rbm.log 2>&1

If you have the full nprize codebase, you can improve the output by removing the overall average 
from the result.  This should give you a probe RMSE of 0.915987.
3) ./utest0b1 -l 1 -le data/r100_01.bin -bl 1  -se data/rc100_01.bin

The conditional rbm can be run similarly.  
4) make rbmcond

5) ./rbmcond -l 1 -se data/rcond100_01.bin > rbmcond.log 2>&1

If you achieve a good result (published reports imply the low 0.91x or high 0.90x), you should be able
to improve this by again removing the overall average.  You need the full nprize package for this. The
parameters and code currently in use don't yet improve on the pure rbm values.

6) ./utest0b1 -l 1 -le data/rcond100_01.bin -bl 1  -se data/rccond100_01.bin

The important logging from these programs will be appended to data/log.txt


See the other subdirectory for an non-cleaned-up version of all the code that I used for the netflix prize.
