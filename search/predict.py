#!/usr/bin/env python3

from infer import *
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

def DistrAvg(sample):
	lpu, lpd, lpi = [], [], []
	for s in sample:
		probu, probd, probi, gamma, se, upper_up, upper_dn = CalcProb(session, s, sbar, p0min, p0max, delta)
		lpu.append(probu)
		lpd.append(probd)
		lpi.append(probi)
	pu, pd, pi = mean(lpu), mean(lpd), mean(lpi)
	return pu, pd, pi, gamma, se, upper_up, upper_dn

if __name__=='__main__':
	source, today = 'linr', '0709'
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
	out_file = datapre + 'predict_' + source + '.txt'
	data = genfromtxt(in_file, delimiter='\t', skip_header=1)
	nrows, ncols = data.shape
	ndim, nwalker, nstep, nburn = 1, 4, 1000, 50

	i = 0
	k = 0
	f = open(out_file, 'a')
	while i < nrows:
		uid, sid = data[i,0], data[i,2]
		uidp = data[i-1,0] if i!=0 else 999
		if uid!=uidp: k = i 

		# prepare session accumulatively
		j = i
		while j < nrows and data[j,0]==uid and data[j,2]==sid:
			j += 1
		session = data[k:j,:]
		print(k, j)

		# estimate theta
		dump_file = datapre + 'predict_' + source + str(int(uid)) + '.pk'
		Estimate(session, nwalker, nstep, nburn, dump_file, sbar, delta, a, b)
		fmc = open(dump_file, 'rb')
		sample_post = pickle.load(fmc)
		sample_prior = np.random.uniform(0, pi/2, 1000)
		tmean = mean(sample_post)

		# predict prior, post
		if j < nrows - 1:
			action = data[j,5]
			theta = data[j-1,1]
			p0min, p0max = data[j-1,6], data[j-1,7]
			gammau, gammase = data[j-1,12], data[j-1,13]
			pstar, optp, optq = data[j-1,14], data[j-1,15], data[j-1,16]
			lpu, lpd, lpi = [], [], []
			pu_prior, pd_prior, pi_prior, gamma, se, upper_up, upper_dn  = DistrAvg(sample_prior)
			pu_post, pd_post, pi_post, gamma, se, upper_up, upper_dn  = DistrAvg(sample_post)
			rd = random.random()
			if rd <= pd_post: 
				pred = -1
			elif rd > pd_post and rd < pi_post + pd_post:
				pred = 0
			else:
				pred = 1

			# calculate ratio
			pro_post, pro_prior = 1 - pi_post, 1 - pi_prior
			if action==1:
				pr_post = pu_post/pro_post
				pr_prior = pu_prior/pro_prior
				pr_ratio = pr_post/pr_prior
			elif action==-1:
				pr_post = pd_post/pro_post
				pr_prior = pd_prior/pro_prior
				pr_ratio = pr_post/pr_prior
			elif action==0:
				pr_post = pi_post/pro_post
				pr_prior = pi_prior/pro_prior
				pr_ratio = pr_post/pr_prior
			else:
				pr_post, pr_prior, pr_ratio = nan, nan, nan
				

			f.write('%d\t' % int(uid))
			f.write('%f\t' % theta)
			f.write('%f\t' % tmean)
			f.write('%d\t' % int(sid))
			f.write('%f\t' % pu_post)
			f.write('%f\t' % pd_post)
			f.write('%f\t' % pi_post)
			f.write('%d\t' % pred)
			f.write('%d\t' % action)
			f.write('%f\t' % gammau)
			f.write('%f\t' % gammase)
			f.write('%f\t' % gamma)
			f.write('%f\t' % se)
			f.write('%f\t' % upper_up)
			f.write('%f\t' % upper_dn)
			f.write('%d\t' % p0min)
			f.write('%d\t' % p0max)
			f.write('%f\t' % pstar)
			f.write('%f\t' % optp)
			f.write('%f\t' % optq)
			f.write('%f\t' % pu_prior)
			f.write('%f\t' % pd_prior)
			f.write('%f\t' % pi_prior)
			f.write('%f\t' % pr_post)
			f.write('%f\t' % pr_prior)
			f.write('%f\n' % pr_ratio)
		i = j
	f.close()
