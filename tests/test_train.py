import sys
from numpy.testing import *
import numpy as N
from scipy.linalg import pinv, inv
import random

# TODO: right module and path handling
sys.path.append("python/")
from aureservoir import *


class test_train(NumpyTestCase):

    def setUp(self):
	
	# parameters
	self.size = 15
	self.ins = random.randint(1,5)
	self.outs = random.randint(1,5)
	self.conn = random.uniform(0.95,0.99)
	self.train_size = 22
	self.dtype = 'float64'
	
	# construct network
	if self.dtype is 'float32':
		self.net = SingleESN()
	else:
		self.net = DoubleESN()
	
	# set parameters
	self.net.setReservoirAct(ACT_LINEAR)
	self.net.setOutputAct(ACT_LINEAR)
	self.net.setSize( self.size )
	self.net.setInputs( self.ins )
	self.net.setOutputs( self.outs )
	self.net.setInitParam(CONNECTIVITY, self.conn)
	self.net.setInitParam(FB_CONNECTIVITY, 0.5)
	
	# debugging
	#print self.size, self.ins, self.outs, self.conn
	
	
    def _teacherForcing(self, indata, outdata, act=None):
	""" teacher forcing and collect internal states """
	
	steps = indata.shape[1]
	X = N.empty((self.size,steps),self.dtype)
	self.net.resetState()
	
	# get data to python
	W = N.empty((self.size,self.size),self.dtype)
	self.net.getW( W )
	Win = self.net.getWin()
	Wback = self.net.getWback()
	x = N.zeros((self.size),self.dtype)
	
	# recalc simulation algorithm
	for n in range(steps):
		# calc new network activation
		x = N.dot( W, x )
		x += N.dot( Win, indata[:,n] )
		if n > 0:
			x += N.dot( Wback, outdata[:,n-1] )
		# calc activation function
		if act:
			x = act( x )
		# we don't need output, simply store network states
		X[:,n] = x
	
	return X


    def testPI(self, level=1):
	""" test TRAIN_PI with zero input and feedback """
        
	# init network
	self.net.setSimAlgorithm(SIM_STD)
	self.net.setTrainAlgorithm(TRAIN_PI)
	self.net.init()
	
	# train network
	washout = 2
	# test with zero input:
	indata = N.zeros((self.ins,self.train_size),self.dtype)
	outdata = N.random.rand(self.outs,self.train_size) * 2 - 1
	indata = N.asfarray( indata, self.dtype )
	outdata = N.asfarray( outdata, self.dtype )
	self.net.train( indata, outdata, washout )
	wout_target = self.net.getWout().copy()
	
	# teacher forcing, collect states
	X = self._teacherForcing(indata,outdata)
	
	# restructure data
	M = N.r_[X,indata]
	M = M[:,washout:self.train_size].T
	T = outdata[:,washout:self.train_size].T
	
	# calc pseudo inverse: wout = pinv(M) * T
	wout = ( N.dot(pinv(M),T) ).T
	
	# normalize result for comparison
	wout = wout / abs(wout).max()
	wout_target = wout_target / abs(wout_target).max()
	assert_array_almost_equal(wout_target,wout,2)
	

    def testLS(self, level=1):
	""" test TRAIN_LS with noise input without feedback """
        
	# init network
	self.net.setInitParam(FB_CONNECTIVITY, 0)
	self.net.setSimAlgorithm(SIM_STD)
	self.net.setTrainAlgorithm(TRAIN_LS)
	self.net.init()
	
	# train network
	washout = 2
	# test with random input:
	indata = N.random.rand(self.ins,self.train_size) * 2 - 1
	outdata = N.random.rand(self.outs,self.train_size) * 2 - 1
	indata = N.asfarray( indata, self.dtype )
	outdata = N.asfarray( outdata, self.dtype )
	self.net.train( indata, outdata, washout )
	wout_target = self.net.getWout().copy()
	
	# teacher forcing, collect states
	X = self._teacherForcing(indata,outdata)
	
	# restructure data
	M = N.r_[X,indata]
	M = M[:,washout:self.train_size].T
	T = outdata[:,washout:self.train_size].T
	
	# calc pseudo inverse: wout = pinv(M) * T
	wout = ( N.dot(pinv(M),T) ).T
	
	# normalize result for comparison
	wout = wout / abs(wout).max()
	wout_target = wout_target / abs(wout_target).max()
	assert_array_almost_equal(wout_target,wout,1)


    def testRidgeRegression(self, level=1):
	""" test TRAIN_RIDGEREG with noise input without feedback """
        
	# init network
	tikfactor = 0.7;
	self.net.setInitParam(FB_CONNECTIVITY, 0)
	self.net.setInitParam(TIKHONOV_FACTOR, tikfactor)
	self.net.setSimAlgorithm(SIM_STD)
	self.net.setTrainAlgorithm(TRAIN_RIDGEREG)
	self.net.init()
	
	# train network
	washout = 2
	# test with random input:
	indata = N.random.rand(self.ins,self.train_size) * 2 - 1
	outdata = N.random.rand(self.outs,self.train_size) * 2 - 1
	indata = N.asfarray( indata, self.dtype )
	outdata = N.asfarray( outdata, self.dtype )
	self.net.train( indata, outdata, washout )
	wout_target = self.net.getWout().copy()
	
	# teacher forcing, collect states
	X = self._teacherForcing(indata,outdata)
	
	# restructure data
	S = N.r_[X,indata]
	S = S[:,washout:self.train_size].T
	T = outdata[:,washout:self.train_size].T
	
	# calc ridge regression
	wout = N.dot( N.dot( inv( N.dot(S.T,S) + (tikfactor**2) * \
	              N.eye(self.size+self.ins) ), S.T ), T ).T
	
	# normalize result for comparison
	wout = wout / abs(wout).max()
	wout_target = wout_target / abs(wout_target).max()
	assert_array_almost_equal(wout_target,wout,5)


    def testRidgeRegressionVsPI(self, level=1):
	""" TODO: tests if we get the same result with Ridge Regression and
	Pseudo Inverse method, if we set the regularization parameter to 0 """
        
	# init network
	self.net.setInitParam(FB_CONNECTIVITY, 0)
	self.net.setInitParam(TIKHONOV_FACTOR, 0)
	self.net.setSimAlgorithm(SIM_STD)
	self.net.setTrainAlgorithm(TRAIN_RIDGEREG)
	self.net.init()
	
	# generate data
	washout = 2
	# test with random input:
	indata = N.random.rand(self.ins,self.train_size) * 2 - 1
	outdata = N.random.rand(self.outs,self.train_size) * 2 - 1
	indata = N.asfarray( indata, self.dtype )
	outdata = N.asfarray( outdata, self.dtype )
	
	# train with ridge regression
	self.net.train( indata, outdata, washout )
	wout_ridge = self.net.getWout().copy()
	
	# make the same with PI training
	self.net.setTrainAlgorithm(TRAIN_PI)
	self.net.resetState()
	self.net.init()
	self.net.train( indata, outdata, washout )
	wout_pi = self.net.getWout().copy()
	
	#assert_array_almost_equal(wout_ridge,wout_pi,5)


    def testPISquare(self, level=1):
	""" test squared updates with TANH activation functions """
        
	# init network
	self.net.setReservoirAct(ACT_TANH)
	self.net.setOutputAct(ACT_TANH)
	self.net.setSimAlgorithm(SIM_SQUARE)
	self.net.setTrainAlgorithm(TRAIN_PI)
	self.net.init()
	
	# simulate network
	washout = 2
	indata = N.random.rand(self.ins,2*self.train_size) * 2 - 1
	outdata = N.random.rand(self.outs,2*self.train_size) * 2 - 1
	indata = N.asfarray( indata, self.dtype )
	outdata = N.asfarray( outdata, self.dtype )
	self.net.train( indata, outdata, washout )
	wout_target = self.net.getWout().copy()
	
	print "INPUTS: ", self.ins
	
	# teacher forcing, collect states
	X = self._teacherForcing(indata,outdata,N.tanh)
	
	# restructure data
	M = N.r_[X,indata,X**2,indata**2]
	M = M[:,washout:2*self.train_size].T
	T = outdata[:,washout:2*self.train_size].T
	
	# undo output activation
	T = N.arctanh( T )
	
	# calc pseudo inverse: wout = pinv(M) * T
	wout = ( N.dot(pinv(M),T) ).T
	
	# normalize result for comparison
	wout = wout / abs(wout).max()
	wout_target = wout_target / abs(wout_target).max()
	assert_array_almost_equal(wout_target,wout,2)


    def testRelaxationStages(self, level=1):
	""" relaxation stages for ESNs in generator mode """
        
	# init network
	self.net.setReservoirAct(ACT_TANH)
	self.net.setSimAlgorithm(SIM_STD)
	self.net.setTrainAlgorithm(TRAIN_PI)
	self.net.setInitParam(FB_CONNECTIVITY, 0.9)
	self.net.init()
	
	# copy network
	if self.dtype is 'float32':
		netB = SingleESN(self.net)
	else:
		netB = DoubleESN(self.net)
	
	# train network
	self.net.setInitParam(RELAXATION_STAGES, 1)
	washout = 2
	# test with zero input:
	indata = N.zeros((self.ins,self.train_size),self.dtype)
	outdata = N.random.rand(self.outs,self.train_size) * 2 - 1
	indata = N.asfarray( indata, self.dtype )
	outdata = N.asfarray( outdata, self.dtype )
	self.net.train( indata, outdata, washout )
	wout_target = self.net.getWout().copy()
	
	# recalc algorithm
	
	# 1. train initial output weights
	netB.train( indata, outdata, washout )
	
	# 2. calculate new teacher signal
	t_new = N.zeros( outdata.shape )
	t_new[:,0] = outdata[:,0]
	outtmp = N.empty( self.outs )
	netB.simulateStep( indata[:,0].flatten(), outtmp )
	netB.setLastOutput( outdata[:,0].flatten() )
	for n in range(1,indata.shape[1]):
		netB.simulateStep( indata[:,n].flatten(), outtmp )
		netB.setLastOutput( outdata[:,n].flatten() )
		t_new[:,n] = outtmp
	
	# 3. calculate weights with new teacher signal
	netB.train( indata, t_new, washout )
	
	# normalize result for comparison
	woutB = netB.getWout().copy()
	woutB = woutB / abs(woutB).max()
	wout_target = wout_target / abs(wout_target).max()
	assert_array_almost_equal(wout_target,woutB,2)


    def testStateCollection(self, level=1):
	""" test state collection """
        
	# init network
	self.net.setSimAlgorithm(SIM_STD)
	self.net.setTrainAlgorithm(TRAIN_PI)
	self.net.setInitParam(FB_CONNECTIVITY, 0.)
	self.net.init()
	
	# collect states
	washout = 3
	# test with zero input:
        X1 = N.zeros((self.train_size-washout,self.size), dtype=self.dtype)
	indata = N.random.rand(self.ins,self.train_size) * 2 - 1
	outdata = N.zeros((self.outs,self.train_size))
	self.net.collectStates( indata, X1, washout )
	
	# teacher forcing, collect states
	tmp = self._teacherForcing(indata,outdata).T
        X2 = tmp[3:,:]
        
        # test if states are the same
	assert_array_almost_equal(X1,X2)


if __name__ == "__main__":
    NumpyTest().run()
