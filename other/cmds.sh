#!/bin/bash

# 3) Run "python countusers.py" to generate the file "data/users" which converts
# a compressed user representation (19bits) to their values in netflix files
# (21 bits with gaps). This code also performs a sanity check on the data.

# 4) Run "python qualify2bin.py" to convert the qualifying file
# (input/qualifying.txt) to a binary file in a binary format (data/qualify.bin)
# which will be latter used to generate the submission to netflix.

# 5) Run "python netflix2moviebin.py" this will generate a binary representation
# of the entire data (training, probe and qualifying) in data/movie_entry.bin
# The file is organized according to movies and the start of each movie is stored
# at data/movie_index.bin. The size of movie_entry file was reduced by throwing
# some information from the date of each entry, however the exact date information
# is kept at data/movie_date.bin

# 6) Make (run "make moviebin2userbin") and run moviebin2userbin. This will read
# the files generated in step 5 and will generate new file: data/user_entry.bin
# which keeps the same information ordered according to users. The location
# of each user in the file is kept at data/user_index.bin . Note that in this
# format the entire information on date is kept.

# 7) Make utest0b1 and use it to remove the baseline from the training data
# using:
# utest0b1 -se data/b.bin
# NOTE: All the commands that start with "uXXX" have several flags, see utest.c.
# NOTE: These commands generate a log file at data/log.txt
# NOTE: The "-se data/b.bin" indicates where to store the resulting errors.

#./utest0b1 -se data/b.bin


# The above utest0b1 command generates the base line errors as described in
# Improved Neighborhood-based Collaborative Filtering by the BellKor team.
# Their baseline was improved by adding the following steps:
# * For each user entry, remove correlation with the number of other entries that
# same user made on the day.
# * Perform a final pass in which the global average error is removed.

# 8) Repeat the run to generate a base line for both training and probe data:
# utest0b1 -a -se data/xb.bin
# NOTE: The "-a" flag controls if the run is on the training or on all
# (training+probe) data.

#./utest0b1 -a -se data/xb.bin


# 9) Make and run SVD with weights on both training and all data:
# usvdbkw1 -le data/b.bin    -l 600 -se data/usvdbkw1-b-l600.bin
# usvdbkw1 -le data/xb.bin -a -l 600 -se data/usvdbkw1-xb-l600.bin
# NOTE: The "-l 600" indicates that 600 features are computed.
# NOTE: The "-le data/..." indicates from where to read the inital errors.
# The program usvdbkw1 performs SVD Factorization as described in section 4.3 of
# "Modeling Relationships at Multiple Scales to Improve Accuracy of Large
# Recommender Systems" again by the BellKor team. Note that their method was a 
# order of magnitude faster than Simon's Funck method.
# Their method was augmented by adding a linear time weights to the computation.

#./usvdbkw1 -le data/b.bin    -l 600 -se data/usvdbkw1-b-l600.bin

./usvdbkw1 -le data/xb.bin -a -l 600 -se data/usvdbkw1-xb-l600.bin


# 10) I found that running the first step (removing movie average) of the baseline
# helped at this stage:
# utest0b1 -l 1    -le data/usvdbkw1-b-l600.bin -se data/7.bin
# utest0b1 -l 1 -a -le data/usvdbkw1-xb-l600.bin -se data/x7.bin
# NOTE: how "-l 1" is used to control what base-line steps are performed.

./utest0b1 -l 1    -le data/usvdbkw1-b-l600.bin -se data/7.bin

./utest0b1 -l 1 -a -le data/usvdbkw1-xb-l600.bin -se data/x7.bin


# 11) The x7.bin gave a qualified RMSE of 0.9064. This can be repeated as follows
# utest0b1 -l 0 -le data/x7.bin -sq data/t.txt
# The file "data/t.txt" is the wanted result, you can check its validity using
# netflix script by running "check_format data/t.txt"
# You should gzip the result ("gzip data/r.txt") and compute MD5 hash of it
# ("md5sum data/t.txt.gz") and now you are ready to post the result to netflix.

./utest0b1 -l 0 -le data/x7.bin -sq data/t.txt


# 12) In order to improve this result make and run the NSVD1 method as described
# in section 3.8 of "Improving regularized singular value decomposition for
# collaborative filtering" by Arik Paterek
# utest10 -le data/b.bin     -l 30 -se data/u10-a-le-b-l-30.bin
# utest10 -le data/xb.bin -a -l 30 -se data/u10-a-le-xb-l-30.bin

./utest10 -le data/b.bin     -l 30 -se data/u10-a-le-b-l-30.bin

./utest10 -le data/xb.bin -a -l 30 -se data/u10-a-le-xb-l-30.bin


# 13) Again I found an improvement in removing the overal average.
# utest0b1 -l 1 -le data/u10-a-le-b-l-30.bin -bl 1  -se data/10
# utest0b1 -l 1 -a -le data/u10-a-le-xb-l-30.bin -bl 1  -se data/x10

./utest0b1 -l 1 -le data/u10-a-le-b-l-30.bin -bl 1  -se data/10

./utest0b1 -l 1 -a -le data/u10-a-le-xb-l-30.bin -bl 1  -se data/x10


# 13) At this point you are ready to find out what are the best mixture weights
# for combining 10 with 13 when using the training data.
# utest0b1 -l 0 -le data/7.bin -le data/10
# The weights that are displayed and should be used on the entire data,
# for example:
# utest0b1 -l 0 -lew 0.765888 -lew 0.231938 -lew -0.001085 -le data/x7.bin -le data/x10 -sq data/t.txt
# You can now process t.txt as described in step 11

./utest0b1 -l 0 -le data/7.bin -le data/10

./utest0b1 -l 0 -lew 0.765888 -lew 0.231938 -lew -0.001085 -le data/x7.bin -le data/x10 -sq data/t.txt

