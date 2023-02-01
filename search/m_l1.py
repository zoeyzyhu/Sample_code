#! /usr/bin/env python2.7

from pylab import *
from sklearn import linear_model, svm
from sklearn.linear_model import LogisticRegression
import argparse

parser = argparse.ArgumentParser()
parser.add_argument('-i', 
  help = 'input file'
  )
parser.add_argument('-l', 
  help = 'label file for explanatory variables'
  )
parser.add_argument('--C',
    default = '0.1',
    help = 'regularization')
parser.add_argument('--crossv',
    default = 'no',
    help = 'whether use cross validation')
args = vars(parser.parse_args())


def LoadData(filename):
    raw = genfromtxt(filename, delimiter=' ', skip_header=0)
    data = raw[:,:-1]
    y = raw[:, -1]
    return data, y

def Partition(data, label, nfold):      
    if nfold <= 1:
        print 'nfold has to be an int larger than 1!'
        exit(0)
    nlen = len(label)  
    # get a set of random index
    prange = permutation(range(nlen))  
    # get the start/end index e.g.[0,3,6,9]
    istart = range(0, nlen, nlen / nfold)  
    data_list = []
    label_list = []
    for i in range(nfold - 1):
        # get index of first n-1 random partitions e.g.prange[3:6]    
        pdex = prange[istart[i]: istart[i + 1]] 
        # append the random partition
        data_list.append(data[pdex, :]) 
        label_list.append(label[pdex])
    # get index of the last random partition
    pdex = prange[istart[i + 1]:]
    # and append
    data_list.append(data[pdex, :]) 
    label_list.append(label[pdex])
    # a list of list, outer list has nfold elements
    return data_list, label_list 

def CombinePartitions(data_list, label_list):  
    # check the partition module
    return concatenate(data_list), concatenate(label_list)

def Accuracy(value, label):
    length = len(label)
    correct = 0
    for i, a in enumerate(value):  
        if a >= 0.5 and label[i] > 0.5: 
        #-- >0.5 instead of ==1.0, in case of data type error
            correct += 1
        if a < 0.5 and label[i] < 0.5:
            correct += 1
    return 1.0 * correct / length 
    #-- multiple result by 1.0 to avoid '3/5 = 0'

def LogReg(data, label, regularization, vali_data = None, vali_label = None):  
#-- input = None is default value, for backward compatibility
    reg = LogisticRegression(penalty = 'l1', C = regularization,
        multi_class='multinomial',solver ='saga', max_iter=10000)
    reg.fit(data, label)
    value1 = reg.predict(data)
    score1 = Accuracy(value1,label)

    if (vali_data is None and vali_label is None):
        return score1, reg.classes_, reg.coef_
    else:
        value2 = reg.predict(vali_data)
        score2 = Accuracy(value2, vali_label)
        return score1, score2, reg.classes_, reg.coef_

def CrossValidation(data_list, label_list, regularization):
    train_score = []
    test_score = []
    reg_class = []
    reg_coef = []
    for i in range(len(data_list)):
        data_copy, label_copy = list(data_list), list(label_list)  
        #-- to really copy a list in python, use list(list)
        del data_copy[i]
        del label_copy[i]
        data, label = CombinePartitions(data_copy, label_copy)
        score1, score2, regclass, regcoef = LogReg(data, label, regularization, data_list[i], label_list[i])
        train_score.append(score1)
        test_score.append(score2)
        reg_class.append(regclass)
        reg_coef.append(regcoef)
    return train_score, test_score, reg_class, reg_coef




if __name__=='__main__':
    # seed(0)

    dt, label = LoadData('%s' % args['i'])
    vname = genfromtxt('%s' % args['l'], dtype = str, delimiter = ',')
    #data = dt[:,1:-4] 
    #varname = vname[1:-4]

    #-- sesion to clicks (2:28)
    dt1 = dt[:,1:28]
    vname1 = vname[1:28]
    #-- speed (30)
    dt2 = dt[:,29]
    vname2 = vname[29]
    #-- sd_priceqryL to sd_rangepriceqry (33:40)
    dt3 = dt[:,32:40]
    vname3 = vname[32:40]
    #-- concatenate data parts
    data = concatenate((dt1, dt2[:,None], dt3), axis=1)
    varname = concatenate((vname1, vname2, vname3), axis = None) 
    
    print data.shape, varname.shape
    regularization = float(args['C'])
    print 'C = ' + str(regularization)
    nfold = 3
    print 'Cross Validation: ' + args['crossv'].title()

    if args['crossv'] == 'no':
        score1, regclass, regcoef = LogReg(data, label, regularization)
        print 'Accuracy = ' + str(score1)
        print('%25s%10s%10s%10s' %('Type', regclass[0], regclass[1], regclass[2]))
        for j in range(len(regcoef[0])):
            print('%25s%10f%10f%10f' %(varname[j], regcoef[0][j], regcoef[1][j], regcoef[2][j]))

    else:
        data_list, label_list = Partition(data, label, nfold)
        train_score, test_score, regclass, regcoef = \
        CrossValidation(data_list, label_list, regularization)
        print 'Train Score:\n ' + str(train_score)
        print 'Test Score:\n ' + str(test_score)
        print 'Average Train Score = ' + str(mean(train_score))
        print 'Average Test Score = ' + str(mean(test_score))
        for i in range(nfold):
            print 'Iteration ' + str(i + 1)
            print('%25s%10s%10s%10s' %('Type', regclass[i][0], regclass[i][1], regclass[i][2]))
            for j in range(len(regcoef[0][0])):
                print('%25s%10f%10f%10f' %(varname[j], regcoef[i][0][j], regcoef[i][1][j], regcoef[i][2][j]))
