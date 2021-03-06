// Copyright (c) 2005 - 2012 Marc de Kamps
//						2012 David-Matthias Sichau
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

//Hack to test privat members
#define private public
#define protected public
#include <MPILib/algorithm/WilsonCowanAlgorithm.hpp>
#undef protected
#undef private

#include <MPILib/include/report/handler/RootReportHandler.hpp>
#include <MPILib/include/SimulationRunParameter.hpp>

#include <boost/test/minimal.hpp>
using namespace boost::unit_test;
using namespace MPILib::algorithm;


void test_Constructor() {

	MPILib::Time tau = 10e-3; //10 ms
	MPILib::Rate rate_max = 100.0;
	double noise = 1.0;



	// Define the receiving node
	WilsonCowanParameter par_sigmoid(tau, rate_max, noise);

	WilsonCowanAlgorithm algorithm_exc(par_sigmoid);

	BOOST_CHECK(algorithm_exc._parameter._f_noise == par_sigmoid._f_noise);
	BOOST_CHECK(
			algorithm_exc._integrator.Parameter()._f_noise == par_sigmoid._f_noise);

}

void test_clone() {
	MPILib::Time tau = 10e-3; //10 ms
	MPILib::Rate rate_max = 100.0;
	double noise = 1.0;


	// Define the receiving node
	WilsonCowanParameter par_sigmoid(tau, rate_max, noise);

	WilsonCowanAlgorithm algorithm_exc(par_sigmoid);

	WilsonCowanAlgorithm* alg = algorithm_exc.clone();
	BOOST_CHECK(alg->_parameter._f_noise == 1.0);
	delete alg;

	AlgorithmInterface<double>* algI;
	algI = algorithm_exc.clone();

	if (dynamic_cast<WilsonCowanAlgorithm *>(algI)) {
	} else {
		BOOST_ERROR("should be of dynamic type WilsonCowanAlgorithm");
	}
	delete algI;
}

void test_configure() {
	const MPILib::report::handler::RootReportHandler WILSONCOWAN_HANDLER(
			"test/wilsonresponse.root", // file where the simulation results are written
			false // only rate diagrams
			);

	const MPILib::SimulationRunParameter PAR_WILSONCOWAN(WILSONCOWAN_HANDLER, // the handler object
			1000000, // maximum number of iterations
			0, // start time of simulation
			0.5, // end time of simulation
			1e-4, // report time
			1e-5, // network step time
			"test/wilsonresponse.log" // log file name
			);

	MPILib::Time tau = 10e-3; //10 ms
	MPILib::Rate rate_max = 100.0;
	double noise = 1.0;


	// Define the receiving node
	WilsonCowanParameter par_sigmoid(tau, rate_max, noise);

	WilsonCowanAlgorithm algorithm_exc(par_sigmoid);

	algorithm_exc.configure(PAR_WILSONCOWAN);
	BOOST_CHECK(algorithm_exc._integrator._time_begin==0);
	BOOST_CHECK(algorithm_exc._integrator._step==1e-5);

}

void test_evolveNodeState() {
	/// @todo DS implement this test
}

void test_getCurrentTime() {
	MPILib::Time tau = 10e-3; //10 ms
	MPILib::Rate rate_max = 100.0;
	double noise = 1.0;

	// Define the receiving node
	WilsonCowanParameter par_sigmoid(tau, rate_max, noise);

	WilsonCowanAlgorithm algorithm_exc(par_sigmoid);
	BOOST_CHECK(algorithm_exc.getCurrentTime() == 0.0);
}

void test_getCurrentRate() {
	MPILib::Time tau = 10e-3; //10 ms
	MPILib::Rate rate_max = 100.0;
	double noise = 1.0;


	// Define the receiving node
	WilsonCowanParameter par_sigmoid(tau, rate_max, noise);

	WilsonCowanAlgorithm algorithm_exc(par_sigmoid);
	BOOST_CHECK(algorithm_exc.getCurrentTime() == 0.0);
}

void test_innerProduct() {
	MPILib::Time tau = 10e-3; //10 ms
	MPILib::Rate rate_max = 100.0;
	double noise = 1.0;


	// Define the receiving node
	WilsonCowanParameter par_sigmoid(tau, rate_max, noise);

	WilsonCowanAlgorithm algorithm_exc(par_sigmoid);

	std::vector<double> weightVector {4.5,5.2,7.2};


	std::vector<double> nodeVector {2.3,5.3,9.3};


	//need to calculate this
	double res = algorithm_exc.innerProduct(weightVector, nodeVector);
	// only works if in the correct range
	BOOST_CHECK(int(res) == 104);

}

void test_getInitialState() {
	MPILib::Time tau = 10e-3; //10 ms
	MPILib::Rate rate_max = 100.0;
	double noise = 1.0;


	// Define the receiving node
	WilsonCowanParameter par_sigmoid(tau, rate_max, noise);

	WilsonCowanAlgorithm alg(par_sigmoid);

	std::vector<double> v =alg.getInitialState();
	BOOST_CHECK(v[0]==0);
}

void test_getGrid() {
	MPILib::Time tau = 10e-3; //10 ms
	MPILib::Rate rate_max = 100.0;
	double noise = 1.0;

	// Define the receiving node
	WilsonCowanParameter par_sigmoid(tau, rate_max, noise);

	WilsonCowanAlgorithm alg(par_sigmoid);
	AlgorithmGrid grid = alg.getGrid(0);
	BOOST_CHECK(grid._arrayState[0]==0.0);
	BOOST_CHECK(grid._arrayInterpretation[0]==0.0);
}

int test_main(int argc, char* argv[]) // note the name!
		{

	test_Constructor();
	test_clone();
	test_configure();
	test_evolveNodeState();
	test_getCurrentTime();
	test_getCurrentRate();
	test_innerProduct();
	test_getInitialState();
	test_getGrid();

	return 0;
//    // six ways to detect and report the same error:
//    BOOST_CHECK( add( 2,2 ) == 4 );        // #1 continues on error
//    BOOST_CHECK( add( 2,2 ) == 4 );      // #2 throws on error
//    if( add( 2,2 ) != 4 )
//        BOOST_ERROR( "Ouch..." );          // #3 continues on error
//    if( add( 2,2 ) != 4 )
//        BOOST_FAIL( "Ouch..." );           // #4 throws on error
//    if( add( 2,2 ) != 4 ) throw "Oops..."; // #5 throws on error
//
//    return add( 2, 2 ) == 4 ? 0 : 1;       // #6 returns error code
}
