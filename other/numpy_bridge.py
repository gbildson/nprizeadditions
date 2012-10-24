""" load nprize's binary data for use in numpy
by brendan o'connor (anyall.org) who did *not* write the original C code
so no guarantees this is right ..."""

from numpy import loadtxt, array, fromfile, int32, float32, concatenate,zeros,cumsum

NMOVIES = (17770)
NUSERS = (480189)
NPROBE = (1408395)
NTRAIN = (99072112)
NQUALIFY = (2817131)
NMOVIES_QUALIFY = (17470) #// not all movies appear in qualify
NUSERS_QUALIFY = (478615)
NQUALIFY_SIZE = (2852071)		

NENTRIES = (103297638) #// Total number of entries (training+probe+qualify)

import sys, os.path
this_file = sys.modules[__name__].__file__
DATADIR = os.path.join(os.path.dirname(this_file), "data")

# same as C code's 'useridx' global variable
user_index = fromfile(DATADIR+"/user_index.bin", dtype=int32, count= 4 * NUSERS)
user_index = user_index.reshape((NUSERS,4))

# user_index says the offset, then the number of ratings from that user
# in the three different datasets.
# >>> pprint(user_index)
#
#           offset    # train    # probe   # qualifying
# array([[        0,        24,         2,         7],
#        [       33,       940,         2,         7],
#        [      982,       623,         3,         6],
#        ..., 
#        [103297494,        86,         4,         5],
#        [103297589,         9,         1,         8],
#        [103297607,        22,         3,         6]])


orig_user_ids = None
user_index_old_order = None

def lazy_load_old_orderings():
  global orig_user_ids,user_index_old_order
  if orig_user_ids is None:
    orig_user_ids = loadtxt(DATADIR+"/users", dtype=int32)
    user_index_old_order = user_index[order(orig_user_ids),]

def order(*seqs):
  # like R.  seqs are parallel
  inds = range(len(seqs[0]))
  inds.sort(key=lambda i: [x[i] for x in seqs])
  return array(inds)

try:
  from util import counter
except ImportError:
  counter = lambda x: x


def load_ratings(filename, train=True, probe=True, qualifying=False,  old_user_order=True):
  """ the -se and -le formats ('error' formats)
  e.g. if you follow the README, data/b.bin, data/xb.bin, etc. """

  lazy_load_old_orderings()

  assert not (qualifying and (not train or not probe)), "can only cumulatively do train, probe, qualifying."
  assert not (probe and not train), "can only cumulatively do train, probe"

  # set up user counts
  ui = user_index_old_order  if old_user_order else user_index
  count_cols = []
  if train: count_cols.append(1)
  if probe: count_cols.append(2)
  if qualifying: count_cols.append(3)
  user_counts = ui[:, count_cols].sum(axis=1)
  user_final_offsets = concatenate(([0], cumsum(user_counts)))
  #print user_counts
  #print user_final_offsets

  ret_size = 0
  if train: ret_size += NTRAIN
  if probe: ret_size += NPROBE
  if qualifying: ret_size += NQUALIFY
  ret = zeros(ret_size, dtype=float32)

  # load the raw rating data from disk
  ratings = fromfile(filename, dtype=float32, count=NENTRIES)
  if len(ratings)<NENTRIES: raise Exception("memory or i/o error?")

  # copy each segment from nprize ratings to our return ratings
  for u in xrange(ui.shape[0]):
    ret_o = user_final_offsets[u]
    ret[ ret_o  :  ret_o + user_counts[u] ] = \
        ratings[ ui[u,0]  :  ui[u,0] + user_counts[u] ]

  return ret

    



