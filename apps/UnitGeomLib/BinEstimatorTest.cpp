// Copyright (c) 2005 - 2014 Marc de Kamps
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
//
//    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
//    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation
//      and/or other materials provided with the distribution.
//    * Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software
//      without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
// USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//      If you use this software in work leading to a scientific publication, you should include a reference there to
//      the 'currently valid reference', which can be found at http://miind.sourceforge.net
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include <GeomLib.hpp>
//#include "AlgorithmInputSimulationFixture.h"

using namespace GeomLib;
using namespace MPILib;

BOOST_AUTO_TEST_CASE(BinEstimatorTest)
{
    Number n_bins     = 5;

    Time t_mem        =  10e-3;
    Time t_ref        =  0.0;
    Potential V_min   = -10.0;
    Potential V_peak  =  10.0;
    Potential V_reset =   0.0;
    Potential gamma   =  0.5;

    NeuronParameter par_neuron(V_peak, V_reset, V_reset, t_ref, t_mem);
    QifParameter
    par_qif
    (
        gamma
    );

    OdeParameter
    par_ode
    (
    	n_bins,
		V_min,
		par_neuron,
        InitialDensityParameter(V_reset, 0.0)
    );

    SpikingQifNeuralDynamics dyn(par_ode,par_qif);
    QifOdeSystem sys(dyn);

    vector<double> array_interpretation(n_bins, 0);
    vector<double> array_density(n_bins);

    sys.PrepareReport(&array_interpretation[0], &array_density[0]);

    BinEstimator
		estimator
		(
			array_interpretation,
			par_ode
		);

   Index i_new = estimator.SearchBin(0.0);
    BOOST_CHECK(i_new == 2);
    i_new = estimator.SearchBin(9.64);
    BOOST_CHECK(i_new == 4);
    i_new = estimator.SearchBin(par_ode._V_min + 1e-10);
    BOOST_CHECK(i_new == 0);
    i_new = estimator.SearchBin(par_ode._V_min);
    BOOST_CHECK(i_new == 0);
    bool b_catch = false;

    try {
        i_new = estimator.SearchBin(par_ode._V_min - 1e-10);
    } catch (GeomLibException&) {
        b_catch = true;
    }

    BOOST_REQUIRE(b_catch);
}

BOOST_AUTO_TEST_CASE(ExcitatoryBinEstimatorCoverTest)
{

    // test values in this routine are generated by master.py
    Number n_bins = 5;

    Time t_mem        =  10e-3;
    Time t_ref        =  0.0;
    Potential V_min   = -10.0;
    Potential V_peak  =  10.0;
    Potential V_reset = -10.0;
    Potential gamma   =  0.5;

    QifParameter
    par_qif
    (
        gamma
    );

    NeuronParameter par_neuron(V_peak, V_reset, V_reset, t_ref, t_mem);
    OdeParameter par_ode
    (
    	n_bins,
		V_min,
		par_neuron,
        InitialDensityParameter(V_reset, 0.0)
    );

    Potential h = 0.1;

    SpikingQifNeuralDynamics dyn(par_ode, par_qif);
    QifOdeSystem sys(dyn);

    vector<double> array_interpretation(n_bins, 0);
    vector<double> array_density(n_bins);

    sys.PrepareReport(&array_interpretation[0], &array_density[0]);

    BinEstimator estimator(array_interpretation, par_ode);
    Index i = estimator.SearchBin(0.0);
    BinEstimator::CoverPair pair = estimator.CalculateBinCover(i, -h);

    BOOST_CHECK(pair.first._index  == 1);
    BOOST_CHECK(pair.second._index == 2);

    BOOST_CHECK_CLOSE(pair.first._alpha,  0.14869352, 0.01);
    BOOST_CHECK_CLOSE(pair.second._alpha, 1 - 0.22855551, 0.01); // master.py subtracts -1 from diagonal

    i = estimator.SearchBin(V_min);
    pair = estimator.CalculateBinCover(i, -h);
    BOOST_CHECK(pair.first._index == -1);
    BOOST_CHECK(pair.second._index == 0);

    BOOST_CHECK(pair.first._alpha == 0.0);
    BOOST_CHECK_CLOSE(pair.second._alpha, 1 - 0.0109785 , 1e-4); // master.py subtracts 1 from diagonal

    i = estimator.SearchBin(V_peak);
    pair = estimator.CalculateBinCover(i, -h);

    BOOST_CHECK(pair.first._index  == 3);
    BOOST_CHECK(pair.second._index == 4);
    pair = estimator.CalculateBinCover(i, -h);

    BOOST_CHECK_CLOSE(pair.first._alpha,  0.14869352, 1e-4);
    BOOST_CHECK_CLOSE(pair.second._alpha, 1 - 0.0109785 , 1e-4);

    h = 9.5;
    i = estimator.SearchBin(V_min);
    pair = estimator.CalculateBinCover(i, -h);

    BOOST_CHECK(pair.first._index == -1);
    BOOST_CHECK(pair.second._index == -1);

    i = 1;
    pair = estimator.CalculateBinCover(i, -h);

    BOOST_CHECK(pair.first._index == -1);
    BOOST_CHECK(pair.second._index == 0);

    BOOST_CHECK_CLOSE(pair.second._alpha, 0.03087537, 1e-4);

    i = estimator.SearchBin(V_peak);
    pair = estimator.CalculateBinCover(i, -h);

    BOOST_CHECK(pair.first._index  == 0);
    BOOST_CHECK(pair.second._index == 3);

    BOOST_CHECK_CLOSE(pair.first._alpha,  0.84725725, 1e-4);
    BOOST_CHECK_CLOSE(pair.second._alpha, 0.41817786 , 1e-4);

    h = 19.5;
    i = 3;
    pair = estimator.CalculateBinCover(i, -h);
    BOOST_CHECK(pair.first._index == -1);
    BOOST_CHECK(pair.second._index == -1);

    i = estimator.SearchBin(V_peak);
    pair = estimator.CalculateBinCover(i, -h);
    BOOST_CHECK(pair.second._index == 0);
    BOOST_CHECK_CLOSE(pair.second._alpha, 0.05489251 , 1e-4);
}

BOOST_AUTO_TEST_CASE(InhibitoryBinEstimatorCoverTest)
{
    // test values in this routine are generated by master.py
    Number n_bins = 5;

    Time t_mem        =  10e-3;
    Time t_ref        =  0.0;
    Potential V_min   = -10.0;
    Potential V_peak  =  10.0;
    Potential V_reset = -10.0;
    Potential gamma   =  0.5;

    QifParameter
    par_qif
    (
        gamma
    );

    NeuronParameter par_neuron(V_peak, V_reset, V_reset, t_ref, t_mem);
    OdeParameter
    par_ode
    (
    	n_bins,
		V_min,
		par_neuron,
        InitialDensityParameter(V_reset, 0.0)
    );

    Potential h = 0.1;

    SpikingQifNeuralDynamics dyn(par_ode, par_qif);
    QifOdeSystem sys(dyn);

    vector<double> array_interpretation(n_bins, 0);
    vector<double> array_density(n_bins);

    sys.PrepareReport(&array_interpretation[0], &array_density[0]);

    BinEstimator estimator(array_interpretation, par_ode);
    Index i = estimator.SearchBin(V_min);
    BinEstimator::CoverPair pair = estimator.CalculateBinCover(i, h);

    BOOST_CHECK(pair.first._index  == 0);
    BOOST_CHECK(pair.second._index == 1);

    BOOST_CHECK_CLOSE(pair.first._alpha, 0.9890214 , 1e-4);
    BOOST_CHECK_CLOSE(pair.second._alpha, 0.14869352, 1e-4);

    i = estimator.SearchBin(V_peak);
    pair = estimator.CalculateBinCover(i, h);

    BOOST_CHECK(pair.first._index == 4);
    BOOST_CHECK(pair.second._index == 5);

    BOOST_CHECK_CLOSE(pair.first._alpha, 1 - 0.0109785, 1e-4);
    BOOST_CHECK_CLOSE(pair.second._alpha, 0.0, 1e-4);

    h = 19.5;

    i = estimator.SearchBin(V_min);
    pair = estimator.CalculateBinCover(i, h);

    BOOST_CHECK(pair.first._index  == 4);
    BOOST_CHECK(pair.second._index == 5);

    BOOST_CHECK_CLOSE(pair.first._alpha, 0.05489251, 1e-4);
    BOOST_CHECK(pair.second._alpha == 0.0);
}

