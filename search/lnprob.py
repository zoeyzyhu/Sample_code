#!/usr/bin/env python3

from pylab import *
from mpmath import *
from math import *
from scipy.stats import norm
from scipy.stats import beta
from decimal import Decimal
import statsmodels.api as sm
import random


def CalcGamma(sdata):
    pp, qq = sdata[:,3], sdata[:,4]
    model = sm.OLS(qq, sm.add_constant(pp)).fit()
    gamma, gammase = model.params[1], model.bse[1]
    return gamma, gammase

def CalcProb(sdata, theta, sbar, p0min, p0max):
    mu, sigma = CalcGamma(sdata)
    upper_up = sbar*p0max/tan(theta)
    upper_dn = sbar*p0min/tan(theta)
    p_up = 1 - norm.cdf(upper_up, loc=mu, scale=sigma)
    p_dn = norm.cdf(upper_dn, loc=mu, scale=sigma)

    const_reci = sbar/tan(theta)
    ## print #2 =============================================
    f.write('%f\t' % mu) # mean of coefficient
    f.write('%f\t' % sigma) # std of coefficient
    f.write('%f\t' % sbar) # sbar
    f.write('%f\t' % tan(theta)) # tan(theta)
    f.write('%f\t' % const_reci) # const
    f.write('%d\t' % p0min) # p0min
    f.write('%d\t' % p0max) # p0max
    f.write('%f\t' % upper_up) # integral upper limit of up
    f.write('%f\t' % upper_dn) # integral upper limit of dn
    f.write('%f\t' % p_up) # p_up
    f.write('%f\t' % p_dn) # p_dn
    # =======================================================

    return p_up, p_dn

def LnProb(udata, theta, sbar):
    nrow, ncol = udata.shape
    lnprob = 0
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
            ## print #1 =========================
            f.write('%d\t' % int(udata[i,0])) # user ID
            f.write('%f\t' % udata[i,1]) # theta_t
            f.write('%f\t' % theta) # theta_p
            f.write('%d\t' % int(sid)) # session ID
            f.write('%d\t' % i) # session first line
            # ========================================

            action = udata[j,5]
            p0min, p0max = udata[j-1,6], udata[j-1,7]
            p_up, p_dn = CalcProb(lsession, theta, sbar, p0min, p0max)
            if action == -1:
                lnpr = log(p_dn)
                lnprob += lnpr
            if action == 1:
                lnpr = log(p_up)
                lnprob += lnpr

            ## print #3 ====================================
            f.write('%d\t' % action) # search action
            f.write('%f\t' % lnpr) # lnprob of this session
            f.write('%f\n' % lnprob) # lnprob so far
            # ==============================================

        i = j
    return lnprob 


if __name__=='__main__':
    # pseudo_theta = [0.538241, 0.857717]
    # pseudo_theta = [0.1, 0.2]
    pseudo_theta = [0.3, 1.3]
    stdp, stdq = 199, 1.62
    sbar = 2*stdq/stdp**2
    tname = '_' + str(pseudo_theta[0]) + '_' + str(pseudo_theta[1])
    in_file = './data/simulate_hotel.txt'
    out_file = './data/infer_hotel' + tname + '.txt'
    data = genfromtxt(in_file, delimiter='\t', usecols=range(8))
    nrows, ncols = data.shape

    # col names ==============================================================
    f = open(out_file, 'a')
    f.write('%s\t%s\t%s\t%s\t' % ('uid', 'theta_t', 'theta_p', 'sid')) 
    f.write('%s\t%s\t%s\t%s\t' % ('s1line', 'gamma_mu', 'gamma_std', 'sbar'))
    f.write('%s\t%s\t%s\t%s\t' % ('tan_theta', 'const_reci', 'p0min', 'p0max'))
    f.write('%s\t%s\t%s\t%s\t' % ('uplimit_up', 'uplimit_dn', 'p_up', 'p_dn'))
    f.write('%s\t%s\t%s\n' % ('action', 'lnpr', 'lnprob')) 
    # ========================================================================

    i = 0

    while i < nrows:
        uid = data[i,0]
        j = i
        while j < nrows and data[j,0]==uid:
            j += 1
        udata = data[i:j,:]

        # emcee and write estimation
        theta = pseudo_theta[int(uid)-1]
        lnpr = LnProb(udata, theta, sbar)
        print('logprob: ', lnpr)

        i = j
    f.close()
    print(pseudo_theta)
