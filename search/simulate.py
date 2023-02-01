#!/usr/bin/env python3
# 63G
# names ======================================================================
# hotels: all hotels
# lhtls: all hotels a user has seen, list
# lhotels: all hotels a user has seen, array
# lhotel: (4) hotels from the last session

from pylab import *
from mpmath import *
from math import *
from scipy.stats import norm
from scipy.stats import beta
import statsmodels.api as sm
import random


def screen_hotels(hotels, p0min, p0max, nhotel):
	idx = where((hotels[:, 0] >= p0min) & (hotels[:, 0] <= p0max))[0]
	random.shuffle(idx)
	return idx[:nhotel]

def calc_pstar(lhotels, theta, sbar_reci):
	pp, qq = lhotels[:,0], lhotels[:,1]
	model = sm.OLS(qq, sm.add_constant(pp)).fit()
	gamma,gammase = model.params[1], model.bse[1]
	const = tan(theta)*sbar_reci
	pstar = gamma*const
	return pstar, const, gamma, gammase

def search_action(pstar, p0min, p0max):
	if pstar > p0max:
		pmin, pmax = p0min + random.randint(0, 100), p0max + random.randint(0, 100)
		action = '1'
	elif pstar < p0min:
		pmin, pmax = p0min - random.randint(0, 100), p0max - random.randint(0, 100)
		action = '-1'
	else:
		pmin, pmax = p0min, p0max
		action = '0'
	if pmax - pmin < 20: 
		pmax = pmin + 20
	return pmin, pmax, action
	
def optimal_hotel(hotels, theta):
	nhotel = hotels.shape[0]
	pp, qq = hotels[:,0], hotels[:,1]
	lutility = []
	for h in range(nhotel):
		utility = (qq[h]/stdq)*sin(theta) - (pp[h]/stdp)**2*cos(theta)
		lutility.append(utility)
	idx_umax = argmax(lutility)
	return pp[idx_umax], qq[idx_umax]

if __name__=='__main__':
	# set users ==============================================================
	# ltheta = beta.rvs(a=2, b=2, size=20)*pi/2 
	ltheta = np.random.uniform(0, pi/2, 20)

	# set sessions ===========================================================
	nsession, nhotel = 10, 4

	# set hotels =============================================================
	source, today = 'linr', '0707'
	cols = range(1,3)
	if source=='hotel':
		stdp, stdq = 199, 1.62
		cols = range(2,4)
	elif source=='linr':
		stdp, stdq = 602.5, 2.521
	elif source=='nonlinr':
		stdp, stdq = 602.5, 3.3453
	sbar_reci = stdp**2/(2*stdq)

	datapre = './data/' + today + '/'
	plotpre = './plot/' + today + '/'
	in_file = './data/pq_' + source + '.txt'
	out_file = datapre + 'simulate_' + source + '.txt'
	out_theta = datapre + 'simulate_' + source + 'theta.txt'

	hotels = genfromtxt(in_file, delimiter='\t', skip_header=1, usecols=cols)
	hotelid = genfromtxt(in_file, delimiter='\t', skip_header=1, usecols=0, dtype=str)

	# col names ==============================================================
	f = open(out_file, 'a')
	f.write('%s\t%s\t%s\t%s\t' % ('uid', 'theta', 'sid', 'price'))
	f.write('%s\t%s\t%s\t%s\t' % ('quality', 'this_action', 'p0min', 'p0max'))
	f.write('%s\t%s\t%s\t%s\t' % ('hid', 'sbar_reci', 'tan_theta', 'const'))
	f.write('%s\t%s\t%s\t%s\t%s\n' % ('gamma', 'gammase', 'pstar', 'opt_p', 'opt_q'))
	ft = open(out_theta, 'a')
	# ========================================================================

	n = 1
	for theta in ltheta:
		print(n)
		ft.write('%f\n' % theta)

		p0min, p0max = random.randint(0, 400), random.randint(400, 600)
		if p0max - p0min > 200:
			p0min = p0max - 200
		action = '9'
		lhids = []
		lhtls = []
		lhotelids = []
		lhotels = []
		opt_p, opt_q = optimal_hotel(hotels, theta)
		for i in range(nsession):
			print(p0min, p0max)
			idx_hid = screen_hotels(hotels, p0min, p0max, nhotel)
			lhid = hotelid[idx_hid]
			lhids.append(lhid)
			lhotelids = array(lhid)
			lhotel = hotels[idx_hid]
			lhtls.append(lhotel)
			lhotels = array(lhtls).reshape((i+1)*nhotel, -1)

			pstar, const, gamma, gammase = calc_pstar(lhotels, theta, sbar_reci)
			
			for j in range(nhotel):
				f.write('%d\t' % n) # user ID
				f.write('%f\t' % theta) # theta
				f.write('%d\t' % i) # session ID
				f.write('%s\t' % lhotel[j,0]) # price
				f.write('%s\t' % lhotel[j,1]) # quality
				f.write('%s\t' % action) # current action after the last session 
				f.write('%s\t' % p0min) # current lower bound of prange
				f.write('%s\t' % p0max) # current upper bound of prange
				f.write('%s\t' % lhid[j]) # hotel ID
				f.write('%f\t ' % sbar_reci) # sbar reciprocal 
				f.write('%f\t ' % tan(theta)) # tan theta
				f.write('%s\t' % const) # the constant part of pstar besides gamma
				f.write('%f\t' % gamma) # gamma of the pstar equation
				f.write('%f\t' % gammase) # gammase of the pstar equation
				f.write('%f\t ' % pstar) # projected optimal price based on given info
				f.write('%s\t' % opt_p) # optimal price within all hotels in the dataset
				f.write('%s\n' % opt_q) # optimal quality within all hotels
			p0min, p0max, action = search_action(pstar, p0min, p0max)
		n += 1
	f.close()
	ft.close()

