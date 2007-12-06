/***************************************************************************/
/*!
 *  \file   simulate.hpp
 *
 *  \brief  simulation algorithms for Echo State Networks
 *
 *  \author Georg Holzmann, grh _at_ mur _dot_ at
 *  \date   Sept 2007
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

#include <assert.h>

namespace aureservoir
{

//! @name class SimBase Implementation
//@{

template <typename T>
SimBase<T>::SimBase(ESN<T> *esn)
{
  esn_=esn;
  reallocate();
}

template <typename T>
void SimBase<T>::reallocate()
{
  last_out_.resize(esn_->outputs_, 1);
  t_.resize(esn_->neurons_);
}

template <typename T>
void SimBase<T>::setBPCutoffConst(T f1, T f2) throw(AUExcept)
{
  std::string str = "SimBase::setBPCutoffConst: ";
  str += "this is not implemented in standard ESNs, ";
  str += "use e.g. SIM_BP !";

  throw AUExcept( str );
}

template <typename T>
void SimBase<T>::setBPCutoff(const typename ESN<T>::DEVector &f1,
                             const typename ESN<T>::DEVector &f2)
  throw(AUExcept)
{
  std::string str = "SimBase::setBPCutoff: ";
  str += "this is not implemented in standard ESNs, ";
  str += "use e.g. SIM_BP !";

  throw AUExcept( str );
}

template <typename T>
void SimBase<T>::setIIRCoeff(const typename DEMatrix<T>::Type &B,
                             const typename DEMatrix<T>::Type &A)
  throw(AUExcept)
{
  std::string str = "SimBase::setIIRCoeff: ";
  str += "this is not implemented in standard ESNs, ";
  str += "use e.g. SIM_FILTER !";

  throw AUExcept( str );
}

//@}
//! @name class SimStd Implementation
//@{

template <typename T>
void SimStd<T>::simulate(const typename ESN<T>::DEMatrix &in,
                         typename ESN<T>::DEMatrix &out)
{
  assert( in.numRows() == esn_->inputs_ );
  assert( out.numRows() == esn_->outputs_ );
  assert( in.numCols() == out.numCols() );
  assert( last_out_.numRows() == esn_->outputs_ );

  int steps = in.numCols();
  typename ESN<T>::DEMatrix::View
    Wout1 = esn_->Wout_(_,_(1, esn_->neurons_)),
    Wout2 = esn_->Wout_(_,_(esn_->neurons_+1, esn_->neurons_+esn_->inputs_));

  /// \todo direkte blas implementation hier machen ?
  ///       -> ist nicht immer dieses ganze error checking
  ///       -> vielleicht auch direkt floats übergeben ?

  /// \todo optimierte version für vektor/einzelwerte auch machen ?
  ///       -> das wirklich sinnvoll ?
  /// \todo optimierte version ohne Wback_ !
  /// \todo optimierte version ohne noise


  // First run with output from last simulation

  t_ = esn_->x_; // temp object needed for BLAS
  esn_->x_ = esn_->Win_*in(_,1) + esn_->W_*t_ + esn_->Wback_*last_out_(_,1);
  // add noise
  Rand<T>::uniform(t_, -1.*esn_->noise_, esn_->noise_);
  esn_->x_ += t_;
  esn_->reservoirAct_( esn_->x_.data(), esn_->x_.length() );

  // output = Wout * [x; in]
  last_out_(_,1) = Wout1*esn_->x_ + Wout2*in(_,1);

  // output activation
  esn_->outputAct_( last_out_.data(),
                    last_out_.numRows()*last_out_.numCols() );
  out(_,1) = last_out_(_,1);


  // the rest

  for(int n=2; n<=steps; ++n)
  {
    t_ = esn_->x_; // temp object needed for BLAS
    esn_->x_ = esn_->Win_*in(_,n) + esn_->W_*t_ + esn_->Wback_*out(_,n-1);
    // add noise
    Rand<T>::uniform(t_, -1.*esn_->noise_, esn_->noise_);
    esn_->x_ += t_;
    esn_->reservoirAct_( esn_->x_.data(), esn_->x_.length() );

    // output = Wout * [x; in]
    last_out_(_,1) = Wout1*esn_->x_ + Wout2*in(_,n);

    // output activation
    esn_->outputAct_( last_out_.data(),
                      last_out_.numRows()*last_out_.numCols() );
    out(_,n) = last_out_(_,1);
  }
}

//@}
//! @name class SimSquare Implementation
//@{

template <typename T>
void SimSquare<T>::simulate(const typename ESN<T>::DEMatrix &in,
                            typename ESN<T>::DEMatrix &out)
{
  assert( in.numRows() == esn_->inputs_ );
  assert( out.numRows() == esn_->outputs_ );
  assert( in.numCols() == out.numCols() );
  assert( last_out_.numRows() == esn_->outputs_ );

  // we have to resize Wout_ to also have connections for
  // the squared states
  esn_->Wout_.resize(esn_->outputs_, 2*(esn_->neurons_+esn_->inputs_));

  int steps = in.numCols();
  /// \todo speicher nicht hier allozieren
  typename ESN<T>::DEVector insq(esn_->inputs_);
  typename ESN<T>::DEMatrix::View
    Wout1 = esn_->Wout_(_,_(1, esn_->neurons_)),
    Wout2 = esn_->Wout_(_,_(esn_->neurons_+1,esn_->neurons_+esn_->inputs_)),
    Wout3 = esn_->Wout_(_,_(esn_->neurons_+esn_->inputs_+1,
                            2*esn_->neurons_+esn_->inputs_)),
    Wout4 = esn_->Wout_(_,_(2*esn_->neurons_+esn_->inputs_+1,
                            2*(esn_->neurons_+esn_->inputs_)));

  /// \todo siehe class SimStd

  // First run with output from last simulation

  t_ = esn_->x_; // temp object needed for BLAS
  esn_->x_ = esn_->Win_*in(_,1) + esn_->W_*t_ + esn_->Wback_*last_out_(_,1);
  // add noise
  Rand<T>::uniform(t_, -1.*esn_->noise_, esn_->noise_);
  esn_->x_ += t_;
  esn_->reservoirAct_( esn_->x_.data(), esn_->x_.length() );

  // calculate squared state version
  /// \todo vectorize these functions
  for(int i=1; i<=t_.length(); ++i)
    t_(i) = pow( esn_->x_(i), 2 );
  // calculate squared input version
  for(int i=1; i<=insq.length(); ++i)
    insq(i) = pow( in(i,1), 2 );

  // output = Wout * [x; in; x^2; in^2]
  last_out_(_,1) = Wout1*esn_->x_ + Wout2*in(_,1) + Wout3*t_ + Wout4*insq;

  // output activation
  esn_->outputAct_( last_out_.data(),
                    last_out_.numRows()*last_out_.numCols() );
  out(_,1) = last_out_(_,1);


  // the rest

  for(int n=2; n<=steps; ++n)
  {
    t_ = esn_->x_; // temp object needed for BLAS
    esn_->x_ = esn_->Win_*in(_,n) + esn_->W_*t_ + esn_->Wback_*out(_,n-1);
    // add noise
    Rand<T>::uniform(t_, -1.*esn_->noise_, esn_->noise_);
    esn_->x_ += t_;
    esn_->reservoirAct_( esn_->x_.data(), esn_->x_.length() );

    // calculate squared state version
    for(int i=1; i<=t_.length(); ++i)
      t_(i) = pow( esn_->x_(i), 2 );
    // calculate squared input version
    for(int i=1; i<=insq.length(); ++i)
      insq(i) = pow( in(i,n), 2 );

    // output = Wout * [x; in; x^2; in^2]
    last_out_(_,1) = Wout1*esn_->x_ + Wout2*in(_,n) + Wout3*t_ + Wout4*insq;

    // output activation
    esn_->outputAct_( last_out_.data(),
                      last_out_.numRows()*last_out_.numCols() );
    out(_,n) = last_out_(_,1);
  }
}

//@}
//! @name class SimLI Implementation
//@{

template <typename T>
void SimLI<T>::simulate(const typename ESN<T>::DEMatrix &in,
                        typename ESN<T>::DEMatrix &out)
{
  assert( in.numRows() == esn_->inputs_ );
  assert( out.numRows() == esn_->outputs_ );
  assert( in.numCols() == out.numCols() );
  assert( last_out_.numRows() == esn_->outputs_ );

  int steps = in.numCols();
  typename ESN<T>::DEMatrix::View
    Wout1 = esn_->Wout_(_,_(1, esn_->neurons_)),
    Wout2 = esn_->Wout_(_,_(esn_->neurons_+1, esn_->neurons_+esn_->inputs_));

  /// \todo see SimStd


  // First run with output from last simulation

  t_ = esn_->x_; // temp object needed for BLAS
  esn_->x_ = (1. - esn_->init_params_[LEAKING_RATE])*t_ +
             esn_->Win_*in(_,1) + esn_->W_*t_ + esn_->Wback_*last_out_(_,1);
  // add noise
  Rand<T>::uniform(t_, -1.*esn_->noise_, esn_->noise_);
  esn_->x_ += t_;
  esn_->reservoirAct_( esn_->x_.data(), esn_->x_.length() );

  // output = Wout * [x; in]
  last_out_(_,1) = Wout1*esn_->x_ + Wout2*in(_,1);

  // output activation
  esn_->outputAct_( last_out_.data(),
                    last_out_.numRows()*last_out_.numCols() );
  out(_,1) = last_out_(_,1);


  // the rest

  for(int n=2; n<=steps; ++n)
  {
    t_ = esn_->x_; // temp object needed for BLAS
    esn_->x_ = (1. - esn_->init_params_[LEAKING_RATE])*t_ +
               esn_->Win_*in(_,n) + esn_->W_*t_ + esn_->Wback_*out(_,n-1);
    // add noise
    Rand<T>::uniform(t_, -1.*esn_->noise_, esn_->noise_);
    esn_->x_ += t_;
    esn_->reservoirAct_( esn_->x_.data(), esn_->x_.length() );

    // output = Wout * [x; in]
    last_out_(_,1) = Wout1*esn_->x_ + Wout2*in(_,n);

    // output activation
    esn_->outputAct_( last_out_.data(),
                      last_out_.numRows()*last_out_.numCols() );
    out(_,n) = last_out_(_,1);
  }
}

//@}
//! @name class SimBP Implementation
//@{

template <typename T>
void SimBP<T>::setBPCutoffConst(T f1, T f2) throw(AUExcept)
{
  typename ESN<T>::DEVector f1vec(esn_->neurons_);
  typename ESN<T>::DEVector f2vec(esn_->neurons_);

  std::fill_n( f1vec.data(), f1vec.length(), f1 );
  std::fill_n( f2vec.data(), f2vec.length(), f2 );
  filter_.setBPCutoff(f1vec,f2vec);
}

template <typename T>
void SimBP<T>::setBPCutoff(const typename ESN<T>::DEVector &f1,
                           const typename ESN<T>::DEVector &f2)
  throw(AUExcept)
{
  if( f1.length() != esn_->neurons_ )
    throw AUExcept("SimBP: f1 must have same length as reservoir neurons!");
  if( f2.length() != esn_->neurons_ )
    throw AUExcept("SimBP: f2 must have same length as reservoir neurons!");

  filter_.setBPCutoff(f1,f2);
}

template <typename T>
void SimBP<T>::simulate(const typename ESN<T>::DEMatrix &in,
                        typename ESN<T>::DEMatrix &out)
{
  assert( in.numRows() == esn_->inputs_ );
  assert( out.numRows() == esn_->outputs_ );
  assert( in.numCols() == out.numCols() );
  assert( last_out_.numRows() == esn_->outputs_ );

  int steps = in.numCols();
  typename ESN<T>::DEMatrix::View
    Wout1 = esn_->Wout_(_,_(1, esn_->neurons_)),
    Wout2 = esn_->Wout_(_,_(esn_->neurons_+1, esn_->neurons_+esn_->inputs_));

  /// \todo see SimStd

  // First run with output from last simulation

  // calc neuron activation
  t_ = esn_->x_;
  esn_->x_ = esn_->Win_*in(_,1) + esn_->W_*t_ + esn_->Wback_*last_out_(_,1);
  // add noise
  Rand<T>::uniform(t_, -1.*esn_->noise_, esn_->noise_);
  esn_->x_ += t_;
  esn_->reservoirAct_( esn_->x_.data(), esn_->x_.length() );

  // Bandpass Filtering
  filter_.calc(esn_->x_);

  // output = Wout * [x; in]
  last_out_(_,1) = Wout1*esn_->x_ + Wout2*in(_,1);

  // output activation
  esn_->outputAct_( last_out_.data(),
                    last_out_.numRows()*last_out_.numCols() );
  out(_,1) = last_out_(_,1);


  // the rest

  for(int n=2; n<=steps; ++n)
  {
    t_ = esn_->x_; // temp object needed for BLAS
    esn_->x_ = esn_->Win_*in(_,n) + esn_->W_*t_ + esn_->Wback_*out(_,n-1);
    // add noise
    Rand<T>::uniform(t_, -1.*esn_->noise_, esn_->noise_);
    esn_->x_ += t_;
    esn_->reservoirAct_( esn_->x_.data(), esn_->x_.length() );

    // Bandpass Filtering
    filter_.calc(esn_->x_);

    // output = Wout * [x; in]
    last_out_(_,1) = Wout1*esn_->x_ + Wout2*in(_,n);

    // output activation
    esn_->outputAct_( last_out_.data(),
                      last_out_.numRows()*last_out_.numCols() );
    out(_,n) = last_out_(_,1);
  }
}

//@}
//! @name class SimFilter Implementation
//@{

template <typename T>
void SimFilter<T>::setIIRCoeff(const typename DEMatrix<T>::Type &B,
                           const typename DEMatrix<T>::Type &A)
  throw(AUExcept)
{
  if( B.numRows() != esn_->neurons_ )
    throw AUExcept("SimFilter: B must have same rows as reservoir neurons!");
  if( A.numRows() != esn_->neurons_ )
    throw AUExcept("SimBP: A must have same rows as reservoir neurons!");

  filter_.setIIRCoeff(B,A);
}

template <typename T>
void SimFilter<T>::simulate(const typename ESN<T>::DEMatrix &in,
                            typename ESN<T>::DEMatrix &out)
{
  assert( in.numRows() == esn_->inputs_ );
  assert( out.numRows() == esn_->outputs_ );
  assert( in.numCols() == out.numCols() );
  assert( last_out_.numRows() == esn_->outputs_ );

  int steps = in.numCols();
  typename ESN<T>::DEMatrix::View
    Wout1 = esn_->Wout_(_,_(1, esn_->neurons_)),
    Wout2 = esn_->Wout_(_,_(esn_->neurons_+1, esn_->neurons_+esn_->inputs_));

  /// \todo see SimStd

  // First run with output from last simulation

  // calc neuron activation
  t_ = esn_->x_;
  esn_->x_ = esn_->Win_*in(_,1) + esn_->W_*t_ + esn_->Wback_*last_out_(_,1);
  // add noise
  Rand<T>::uniform(t_, -1.*esn_->noise_, esn_->noise_);
  esn_->x_ += t_;
  esn_->reservoirAct_( esn_->x_.data(), esn_->x_.length() );

  // IIR Filtering
  filter_.calc(esn_->x_);

  // output = Wout * [x; in]
  last_out_(_,1) = Wout1*esn_->x_ + Wout2*in(_,1);

  // output activation
  esn_->outputAct_( last_out_.data(),
                    last_out_.numRows()*last_out_.numCols() );
  out(_,1) = last_out_(_,1);


  // the rest

  for(int n=2; n<=steps; ++n)
  {
    t_ = esn_->x_; // temp object needed for BLAS
    esn_->x_ = esn_->Win_*in(_,n) + esn_->W_*t_ + esn_->Wback_*out(_,n-1);
    // add noise
    Rand<T>::uniform(t_, -1.*esn_->noise_, esn_->noise_);
    esn_->x_ += t_;
    esn_->reservoirAct_( esn_->x_.data(), esn_->x_.length() );

    // IIR Filtering
    filter_.calc(esn_->x_);

    // output = Wout * [x; in]
    last_out_(_,1) = Wout1*esn_->x_ + Wout2*in(_,n);

    // output activation
    esn_->outputAct_( last_out_.data(),
                      last_out_.numRows()*last_out_.numCols() );
    out(_,n) = last_out_(_,1);
  }
}

//@}

} // end of namespace aureservoir
