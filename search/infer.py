#!/usr/bin/env python3
# 70G

from pylab import *
from mpmath import *
from math import *
from scipy.stats import norm
from scipy.stats import beta
from decimal import Decimal
import statsmodels.api as sm
import numpy as np
import pandas as pd
import random
import pickle
import corner
import emcee


def CalcGamma(sdata, delta):
	pp, qq = sdata[:,3], sdata[:,4]
	model = sm.OLS(qq, sm.add_constant(pp)).fit()
	gamma, gammase = model.params[1], model.bse[1]
	return gamma, gammase + delta

def CalcProb(sdata, theta, sbar, p0min, p0max, delta):
	mu, sigma = CalcGamma(sdata, delta)
	upper_up = sbar*p0max/tan(theta)
	upper_dn = sbar*p0min/tan(theta)
	p_up = 1 - norm.cdf(upper_up, loc=mu, scale=sigma)
	p_dn = norm.cdf(upper_dn, loc=mu, scale=sigma)
	p_in = 1 - p_up - p_dn
	return p_up, p_dn, p_in, mu, sigma, upper_up, upper_dn


def LnProb(theta, udata, sbar, delta, a, b):
	if theta < 0 or theta > pi/2.: 
		return -inf
	else:
		lnprob = Decimal(pi/2.).ln() + Decimal(beta.pdf(theta[0]*2/pi, 2, 2)).ln()
	nrow, ncol = udata.shape
	i = 0
	sessions = []
	lsession = []
	while i < nrow:
		sid = udata[i,2]
		j = i
		while j < nrow and (udata[j,2]==sid or udata[j,5]==0):   
			j += 1
		session = udata[i:j,:]
		sessions.append(session.tolist())
		lsession = array(list(flatten(sessions))).reshape(-1, 8)
		
		if j < nrow:
			print(udata[j-1,0:2])
			action = udata[j,5]
			print(action)
			p0min, p0max = udata[j-1,6], udata[j-1,7]
			print(p0min, p0max)
			p_up, p_dn, p_in, gamma, se, upper_up, upper_dn = CalcProb(lsession, theta, sbar, p0min, p0max, delta)
			if action == -1:
				lnpr = Decimal(p_dn).ln()
			elif action == 1:
				lnpr = Decimal(p_up).ln()
			elif action == 0:
				lnpr = Decimal(p_in).ln()
			lnprob += lnpr

		i = j
	return lnprob 


def Estimate(data, nwalker, nstep, nburn, outfile, sbar, delta, a, b):
	nrows, ncols = data.shape
	i = 0
	out = open(outfile, 'wb')
	while i < nrows:
		uid = data[i,0]
		print(int(uid))

		j = i
		while j < nrows and data[j,0]==uid:
			j += 1
		udata = data[i:j,:]

		# emcee
		ndims, nwalkers, nsteps, nburns = 1, nwalker, nstep, nburn
		p0 = np.random.rand(nwalkers, ndims)
		
		sampler = emcee.EnsembleSampler(nwalkers, ndims, LnProb, 
			args = [udata[:, :8], sbar, delta, a, b], threads = 16)
		sampler.run_mcmc(p0, nsteps)
		sample = sampler.chain[:, nburns:, :].reshape((-1, ndims))
		
		pickle.dump(sample, out)
		
		i = j
	out.close()
		


if __name__=='__main__':
	source, today = 'linr', '0707'
	delta = 0.001
	if source=='hotel':
		stdp, stdq = 199, 1.62
		delta = 0
	elif source=='linr':
		stdp, stdq = 602.5, 2.521
	elif source=='nonlinr':
		stdp, stdq = 602.5, 3.3453
	sbar = 2*stdq/stdp**2
	a, b = 2, 2

	datapre = './data/' + today + '/'
	in_file = datapre + 'simulate_' + source + '.txt'
	out_file = datapre + 'mcmc_' + source + '.pk'
	data = genfromtxt(in_file, delimiter='\t', skip_header=1)
	nrows, ncols = data.shape
	
	ndim, nwalker, nstep, nburn = 1, 4, 1000, 50
	Estimate(data, nwalker, nstep, nburn, out_file, sbar, delta, a, b)
