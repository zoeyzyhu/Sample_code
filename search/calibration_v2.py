#!/usr/bin/env python3.7

from pylab import *
import pandas as pd
from numpy.matlib import repmat
import emcee
import corner
import os
from multiprocessing import Pool
from schwimmbad import MPIPool
from sklearn.preprocessing import StandardScaler
from sklearn.impute import SimpleImputer

os.environ["OMP_NUM_THREADS"] = "1"


def lnprob(beta):
    lnprob = []
    num = feature.shape[0]
    i = 0
    while i < num:
        block = feature[i, :]
        j = i + 1
        while j < num and uid[j] == uid[i]:
            block = vstack((block, feature[j,:]))
            j += 1
        utility = []
        if len(block.shape) == 1:
            price = ft[-1]
            if abs(beta[0]) > 1000: return -inf
            quality = beta[0] + dot(ft[:-1], beta[1:])
            utility.append(fa*quality + ga*price)
        else:
            for ft in block:
                price = ft[-1]
                quality = beta[0] + dot(ft[:-1], beta[1:])
                utility.append(fa*quality + ga*price)
        utility = array(utility)
        u_diff = utility - utility[-1]
        lnprob.append(log(1/sum(exp(u_diff))))
        i = j

    return sum(lnprob)


if __name__ == '__main__':

    #=========================================================================
    # prepare data
    #=========================================================================

    file = './data/hotel_data_rand.csv'

    uid = genfromtxt(file, delimiter=',', skip_header=1, usecols=0, dtype='str')

    feature_raw = genfromtxt(file, delimiter=',', skip_header=1, usecols=range(3,9), dtype = float)
    imr = SimpleImputer(missing_values=-999, strategy='mean')
    imr = imr.fit(feature_raw)
    feature_imputed = imr.transform(feature_raw)
    feature = StandardScaler().fit_transform(feature_imputed)

    fa, ga = 1.1, 1.1
    
    #=========================================================================
    # MCMC
    #=========================================================================

    ndim, nwalker, nstep, nburn = 6, 12, 10000, 100
    p0 = rand(nwalker, ndim)

    with Pool() as pool:
        tic = time.time()
        sampler = emcee.EnsembleSampler(nwalker, ndim, lnprob, pool=pool)
        state = sampler.run_mcmc(p0, nstep, progress=True)
        toc = time.time() - tic
        print('MCMC runs for %d steps using %d chains and takes %s s.' % (nstep, nwalker, toc))

    #=========================================================================
    # Output
    #=========================================================================

    sample = sampler.chain[:,nburn:,:].reshape((-1, ndim))
    labels = ['Intercept', 'Star', 'RevScore', 'RevCount', 'RecLevel',
    'CoopLevel']

    # basic stat
    stat = ['Parameter', 'Mean', 'Std', '2.5%', '16%', '50%', '84%', '97.5%']
    sample_mean = mean(sample, axis=0)
    sample_std = std(sample, axis=0)
    sample_p = percentile(sample, [2.5, 16, 50, 84, 97.5], axis=0)
    result = vstack((labels, sample_mean, sample_std, sample_p)).transpose()
    result = pd.DataFrame(vstack((stat, result)))
    result.to_csv('fit_data1.csv', index=False)

    print(result) 

    # lnprobability
    print('min lnprob = ', sampler.lnprobability.flatten().min())
    print('max lnprob = ', sampler.lnprobability.flatten().max())
    
    # steps
    fig1, axes = plt.subplots(6, figsize=(10, 15), sharex=True)
    for i in range(ndim):
        ax = axes[i]
        ax.plot(sample[:,i], 'k', alpha=0.3)
        ax.set_xlim(0,len(sample))
        ax.set_ylabel(labels[i])
        ax.yaxis.set_label_coords(-0.1,0.5)
    axes[-1].set_xlabel("step number")
    savefig('fit_data1_series.png')

    # correlation
    fig = corner.corner(sample, labels = labels)
    savefig('fit_data1_corner.png')
