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
#ifndef MPILIB_POPULIST_MUSIGMASCALARPRODUCT_HPP_
#define MPILIB_POPULIST_MUSIGMASCALARPRODUCT_HPP_

#include <vector>
#include <utility>
#include <MPILib/include/populist/zeroLeakEquations/MuSigma.hpp>

namespace MPILib {
namespace populist {

// forward declaration
struct OrnsteinUhlenbeckConnection;

//! Evaluates the scalar product of an input which arrives over OU_Connections.
//! The formulae are:
//! \f[
//! \mu = N \tau \sum_i \nu_i J_i
//! \f]
//! and
//! \f[
//! \sigma^2 = N \tau \sum_i \nu_i J^2_i
//! \f]
class MuSigmaScalarProduct {
public:


	//! Evaluate the inner product over connections which are indicated by the iterators
	MuSigma Evaluate(const std::vector<Rate>& nodeVector,
			const std::vector<OrnsteinUhlenbeckConnection>& weightVector, Time //!< membrane time constant
			) const;

private:

	Potential InnerProduct(const std::vector<Rate>& nodeVector,
			const std::vector<OrnsteinUhlenbeckConnection>& weightVector) const;

	Potential InnerSquaredProduct(const std::vector<Rate>& nodeVector,
			const std::vector<OrnsteinUhlenbeckConnection>& weightVector) const;

};

} /* namespace populist */
} /* namespace MPILib */

#endif // include guard MPILIB_POPULIST_MUSIGMASCALARPRODUCT_HPP_