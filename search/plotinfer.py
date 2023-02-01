#!/usr/bin/env python3

from pylab import *
from math import *
import pickle
from scipy.stats import beta


if __name__=='__main__':
	source, today = 'linr', '0217'
	a, b = 2, 2

	datapre = './data/' + today + '/'
	plotpre = './plot/' + today + '/'
	in_simu = datapre + 'simulate_' +  source + '_' + today + '.txt'
	in_mcmc = datapre + 'mcmc_' + source + '_' + today + '.pk'
	in_theta = datapre + 'simulate_' + source + '_' + today + 'theta.txt'
	out_plot = plotpre + source + '_'
	ttheta = genfromtxt(in_theta)
	data = genfromtxt(in_simu, delimiter='\t', skip_header=1)
	nrows, ncols = data.shape

	f = open(in_mcmc, 'rb')
	i = 0
	while i < nrows:
		uid = data[i,0]
		print(int(uid))

		j = i 
		while j < nrows and data[j,0]==uid:
			j += 1
		udata = data[i:j,:]

		### plot inference
		sample = pickle.load(f)
		tmean, tstd = mean(sample, axis=0), std(sample, axis=0)
		ttrue = ttheta[int(uid)-1]
		xbeta = np.arange(0, pi/2, 0.01)
		ybeta = beta.pdf(xbeta*2/pi, a, b)

		fig, axs = subplots(2, 1, figsize = (15, 20))
		ax = axs[0]
		ax.set_xlim([0, pi/2])
		ax.hist(sample, bins=50, histtype='step', density=True)
		ax.plot(xbeta, ybeta, 'k--', label='prior')
		ax.axvline(x=ttrue, c='r', label='true theta')
		ax.axvline(x=tmean, c='b', label='mean')
		ax.tick_params(labelsize=17)

		# plot simulation
		session = udata[:,2]
		price, quality = udata[:,3], udata[:,4]
		pmin, pmax = udata[:,6], udata[:,7]
		opt_p, opt_q = udata[:,15], udata[:,16]

		ax = axs[1]
		points = ax.scatter(session, price, s=80, c=quality)
		cb = fig.colorbar(points)
		cb.set_label(label='Quality', size=20)
		cb.ax.tick_params(labelsize=15)
		ax.fill_between(session, pmin, pmax, alpha=0.5)
		ax.axhline(y=opt_p[0], color='r')
		ax.set_xlabel('Query', fontsize=20)
		ax.set_ylabel('Price', fontsize=20)
		ax.tick_params(labelsize=15)
		savefig('%s_%s.png' % (plotpre, uid))

		savefig(out_plot + str(int(uid)) + '.png')
		close()

		i = j
	f.close()
