/***************************************************************************/
/*!
 *  \file   aureservoir.i
 *
 *  \brief  SWIG interface to aureservoir library
 *
 *  \author Georg Holzmann, grh _at_ mur _dot_ at
 *  \date   Oct 2007
 *
 *   ::::_aureservoir_::::
 *   C++ library for analog reservoir computing neural networks
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License as published by the Free Software Foundation; either
 *   version 2.1 of the License, or (at your option) any later version.
 *
 ***************************************************************************/


/***************************************************************************/
// module docstring
%define DOCSTRING
"
aureservoir is an open-source (L-GPL) C++ library for analog
reservoir computing neural networks. The basic class is the ESN
(Echo State Network) class, which can be used in single or double
precision (SingleESN, DoubleESN).

Echo State Networks can be used with different initialization,
training and simulation algorithms - which can be choosen at runtime
from the definitions in the DATA section of the module documentation.
For more info on ESNs see docstrings of DoubleESN or SingleESN.

You can find autogenerated docstrings for most of the methods - for
more detailed documentation and more information see
http://aureservoir.sourceforge.net.

2007-2008, by
Georg Holzmann
grh _at_ mur _dot_ at
http://grh.mur.at
"
%enddef


/***************************************************************************/
// module definition

%module(docstring=DOCSTRING) aureservoir

%{
#define SWIG_FILE_WITH_INIT

#include "../aureservoir/aureservoir.h"
using namespace aureservoir;
%}

// numpy initialization
%include "numpy.i"
%init %{
import_array();
%}

// general exception handling for AUExcept
%exception
{
   try {
      $action
   } catch (AUExcept &e) {
      PyErr_SetString( PyExc_RuntimeError, e.what().c_str() );
      return NULL;
   }
}


/***************************************************************************/
// C++ arrays to numpy conversions

%apply (float *INPLACE_ARRAY2, int DIM1, int DIM2)
{  (float *inmtx, int inrows, int incols),
   (float *outmtx, int outrows, int outcols),
   (float *wmtx, int wrows, int wcols), 
   (float *amtx, int arows, int acols),
   (float *bmtx, int brows, int bcols) };

%apply (double *INPLACE_ARRAY2, int DIM1, int DIM2)
{  (double *inmtx, int inrows, int incols),
   (double *outmtx, int outrows, int outcols),
   (double *wmtx, int wrows, int wcols),
   (double *amtx, int arows, int acols),
   (double *bmtx, int brows, int bcols) };

%apply (float* INPLACE_ARRAY1, int DIM1)
{  (float *invec, int insize),
   (float *outvec, int outsize),
   (float *f1vec, int f1size),
   (float *f2vec, int f2size),
   (float *last, int size) };

%apply (double* INPLACE_ARRAY1, int DIM1)
{  (double *invec, int insize),
   (double *outvec, int outsize),
   (double *f1vec, int f1size),
   (double *f2vec, int f2size),
   (double *last, int size) };

%apply (float** ARGOUTVIEW_ARRAY1, int* DIM1)
{ (float **vec, int *length) };

%apply (double** ARGOUTVIEW_ARRAY1, int* DIM1)
{ (double **vec, int *length) };

%apply (float** ARGOUTVIEW_FARRAY2, int* DIM1, int* DIM2)
{ (float **mtx, int *rows, int *cols) };

%apply (double** ARGOUTVIEW_FARRAY2, int* DIM1, int* DIM2)
{ (double **mtx, int *rows, int *cols) };


/***************************************************************************/
// documentation

// use autodoc
%feature("autodoc", "1");

// include doxygen documentation
%include ESN.i

// so gehts (ohne namespace!):
// %feature("docstring", "TTTTTTTTTTTTTTTTTTTT")  ESN::simulate;


/***************************************************************************/
// class declarations
// NOTE: don't add throw(AUExcept) to the declarations !

template <typename T>
class ESN
{
 public:
  ESN();
  ESN(const ESN<T> &src);
  ~ESN();

  void init();
  void resetState();
  double adapt(T *inmtx, int inrows, int incols);
  inline void train(T *inmtx, int inrows, int incols,
                    T *outmtx, int outrows, int outcols,
                    int washout);
  inline void simulate(T *inmtx, int inrows, int incols,
                       T *outmtx, int outrows, int outcols);
  inline void simulateStep(T *invec, int insize, T *outvec, int outsize);
  void teacherForce(T *inmtx, int inrows, int incols,
                    T *outmtx, int outrows, int outcols);

  void setBPCutoff(T *f1vec, int f1size, T *f2vec, int f2size);
  void setIIRCoeff(T *bmtx, int brows, int bcols,
                   T *amtx, int arows, int acols, int series=1);

  void post();
  int getSize();
  int getInputs();
  int getOutputs();
  double getNoise();
  T getInitParam(InitParameter key);
  InitAlgorithm getInitAlgorithm();
  TrainAlgorithm getTrainAlgorithm();
  SimAlgorithm getSimAlgorithm();
  ActivationFunction getReservoirAct();
  ActivationFunction getOutputAct();

  void getWin(T **mtx, int *rows, int *cols);
  void getWback(T **mtx, int *rows, int *cols);
  void getWout(T **mtx, int *rows, int *cols);
  void getX(T **vec, int *length);
  void getW(T *wmtx, int wrows, int wcols);
  void getDelays(T *wmtx, int wrows, int wcols);
  void getReservoirDelays(T *wmtx, int wrows, int wcols);

  void setInitAlgorithm(InitAlgorithm alg=INIT_STD);
  void setTrainAlgorithm(TrainAlgorithm alg=TRAIN_LEASTSQUARE);
  void setSimAlgorithm(SimAlgorithm alg=SIM_STD);
  void setSize(int neurons=10);
  void setInputs(int inputs=1);
  void setOutputs(int outputs=1);
  void setNoise(double noise);
  void setInitParam(InitParameter key, T value=0.);
  void setReservoirAct(ActivationFunction f=ACT_TANH);
  void setOutputAct(ActivationFunction f=ACT_LINEAR);

  void setWin(T *inmtx, int inrows, int incols);
  void setW(T *inmtx, int inrows, int incols);
  void setWback(T *inmtx, int inrows, int incols);
  void setWout(T *inmtx, int inrows, int incols);
  void setX(T *invec, int insize);
  void setLastOutput(T *last, int size);
};

template <typename T>
class ArrayESN
{
 public:
  ArrayESN(const ESN<T> &model, int array_size);
  ~ArrayESN();

  void init();
  void train(T *inmtx, int inrows, int incols,
             T *outmtx, int outrows, int outcols, int washout);
  void simulate(T *inmtx, int inrows, int incols,
                T *outmtx, int outrows, int outcols);
  void teacherForce(T *inmtx, int inrows, int incols,
                    T *outmtx, int outrows, int outcols);

  int getArraySize();
  ESN<T> getNetwork(int index);
  void printWoutMean();
  void printWoutMax();
  void setNoise(double noise);
};

%template(DoubleESN) ESN<double>;
%template(SingleESN) ESN<float>;
%template(DoubleArrayESN) ArrayESN<double>;
%template(SingleArrayESN) ArrayESN<float>;


/***************************************************************************/
// additional enums

enum InitParameter
{
  CONNECTIVITY,     //!< connectivity of the reservoir weight matrix
  ALPHA,            //!< spectral radius of the reservoir weight matrix
  IN_CONNECTIVITY,  //!< connectivity of the input weight matrix
  IN_SCALE,         //!< scale input weight matrix random vaules
  IN_SHIFT,         //!< shift input weight matrix random vaules
  FB_CONNECTIVITY,  //!< connectivity of the feedback weight matrix
  FB_SCALE,         //!< scale feedback weight matrix random vaules
  FB_SHIFT,         //!< shift feedback weight matrix random vaules
  LEAKING_RATE,     //!< leaking rate for Leaky Integrator ESNs
  TIKHONOV_FACTOR,  //!< regularization factor for TrainRidgeReg
  DS_USE_CROSSCORR, //!< use simple cross-correlation for delay calculation
  DS_USE_GCC,       //!< use generalized cross-correlation for delay calculation
  DS_MAXDELAY,      //!< maximum delay for delay&sum readout
  DS_RESERVOIR_MAXDELAY, //!< maximum static delay in the dynamic reservoir
  DS_EM_ITERATIONS, //!< nr of EM iterations for delay+weight calculation
  DS_WEIGHTS_EM,    //!< use calculated weights from EM algorithm
  EM_VERSION,       //!< TESTING !!!! 
  IP_LEARNRATE,     //!< learnrate for Gaussian-IP reservoir adaptation
  IP_MEAN,          //!< desired mean for Gaussian-IP reservoir adaptation
  IP_VAR,           //!< desired variance for Gaussian-IP reservoir adaptation
  RELAXATION_STAGES, //!< relaxation stages in training algorithm
  DS_FORCE_MAXDELAY  //!< force a specific maxdelay without checks
};

enum InitAlgorithm
{
  INIT_STD
};

enum SimAlgorithm
{
  SIM_STD,     //!< standard simulation \sa class SimStd
  SIM_SQUARE,  //!< additional squared state updates \sa class SimSquare
  SIM_LI,      //!< simulation with leaky integrator neurons \sa class SimLI
  SIM_BP,      //!< simulation with bandpass neurons \sa class SimBP
  SIM_FILTER,  //!< simulation with IIR-Filter neurons \sa class SimFilter
  SIM_FILTER2, //!< IIR-Filter before nonlinearity \sa class SimFilter2
  SIM_FILTER_DS
};

enum TrainAlgorithm
{
  TRAIN_PI,        //!< offline, pseudo inverse based \sa class TrainPI
  TRAIN_LS,        //!< offline least square algorithm, \sa class TrainLS
  TRAIN_RIDGEREG,  //!< with ridge regression, \sa class TrainRidgeReg
  TRAIN_DS_PI      //!< trains a delay&sum readout with PI \sa class TrainDSPI
};

enum ActivationFunction
{
  ACT_LINEAR,      //!< linear activation function
  ACT_TANH,        //!< tanh activation function
  ACT_TANH2,       //!< tanh activation function with local slope and bias
  ACT_SIGMOID      //!< sigmoid activation function
};
