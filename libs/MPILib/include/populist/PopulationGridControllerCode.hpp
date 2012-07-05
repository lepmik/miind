// Copyright (c) 2005 - 2011 Marc de Kamps
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

#include <sstream>
#include <fstream>
#include <complex>
#include <math.h>
#include "../NumtoolsLib/NumtoolsLib.h"
#include "../UtilLib/UtilLib.h"
#include <MPILib/include/populist/PopulationGridController.hpp>
#include <MPILib/include/populist/IntegralRateComputation.hpp>
#include <MPILib/include/BasicTypes.hpp>
#include <MPILib/include/utilities/Exception.hpp>


#ifdef WIN32
#pragma warning(disable: 4267)
#endif

//using DynamicLib::IterationNumberException;

namespace MPILib {
namespace populist {

template <class Weight>
PopulationGridController<Weight>::PopulationGridController
(
	VALUE_REF_INIT
	const PopulationParameter&			par_pop,
	const PopulistSpecificParameter&	par_spec,
	valarray<double>&					array_state,
	valarray<double>&					array_interpretation,
	Number*								p_grid_size,
	Rate*								p_rate,
	ostringstream*						p_stream
):
VALUE_MEMBER_INIT
_n_initial_bins          (*p_grid_size),
_n_bins_to_add           (par_spec.NrAdd()),
_p_number_of_current_bins(p_grid_size),
_n_bins                  (*p_grid_size),
_time_membrane_constant  
(
	par_pop._tau
),
_time_report		(0),
_time_next_report	(0),
_time_network_step	(0),
_par_pop			(par_pop),
_par_spec			(par_spec),
_f_current_scale	(1.0),
_f_expansion_factor	(par_spec.ExpansionFactor()),
_delta_v			(0),
_p_current_rate     (p_rate),
_array_state_reference
(
	array_state
),
_array_state
(
	array_state
),
_array_interpretation
(
	array_interpretation
),
_p_rebinner(auto_ptr<AbstractRebinner>(par_spec.Rebin().Clone())),
_p_stream(p_stream),
_builder	
(
	VALUE_MEMBER_ARG
	_n_bins,
	_array_state,
	_check_sum,
	_bins,
	_par_pop,
	_par_spec,
	_delta_v
),
_p_zl
(
	_builder.GenerateZeroLeakEquations
	(
		par_spec.ZeroLeakName(),
		par_spec.CirculantName(),
		par_spec.NonCirculantName()
	)
)
{
}

template <class Weight>
PopulationGridController<Weight>::~PopulationGridController()
{
}

template <class Weight>
void PopulationGridController<Weight>::Configure
(
	const SimulationRunParameter&		parameter_simulation
)
{                		
	_number_of_evolution_steps	= 0;
	_maximum_number_of_evolution_steps = parameter_simulation.MaximumNumberIterations();

	_time_report				= parameter_simulation.TReport();
	_time_next_report			= _time_report;
	_time_network_step			= parameter_simulation.TStep();
	_time_current				= parameter_simulation.TBegin();

}

template <class Weight>
bool PopulationGridController<Weight>::CollectExternalInput
(
	predecessor_iterator iter_begin,
	predecessor_iterator iter_end
)
{
	_p_zl->SortConnectionvector(iter_begin, iter_end);

	return true;
}

template <class Weight>
bool PopulationGridController<Weight>::Evolve
(
	Time					time,
	Time*					p_time_current,
	Rate*					p_rate_output,
	predecessor_iterator	iter_begin,
	predecessor_iterator	iter_end
)
{

	// This is the main Evolve step from the PopulistGridController. It is driven by the PopulistAlgorithm Evolve method.


	Time t_next_bin;
	while (*p_time_current < time )
	{
		bool is_finite_membrane_time = _par_pop._tau < TIME_MEMBRANE_INFINITE;

		t_next_bin = ( is_finite_membrane_time ) ? DeltaTimeNextBinAdded(time) : time  - _time_current;

		_p_zl->AdaptParameters();


#ifdef _INVESTIGATE_ALGORITHM
		ReportValue val;
		val._name_quantity = "scale";
		val._value = _f_current_scale;
		val._time  = _time_current;
		_vec_value.push_back(val);
		val._name_quantity = "sum";
		val._value = _array_state.sum();
		_vec_value.push_back(val);
#endif 
		_p_zl->Apply(t_next_bin);

		assert( this->IsFinite() );

		_time_current += t_next_bin;
		*p_time_current = _time_current;


		// Also update current scale factor
		if( is_finite_membrane_time )
			AddNewBins();

		if ( is_finite_membrane_time  && IsTimeToRebin() ){
			Rebin();
			assert( this->IsFinite() );
		}

		if ( ++_number_of_evolution_steps > _maximum_number_of_evolution_steps )
			throw IterationNumberException("Too many iterations in the PopulistAlgorithm");
	

		// must occur last! here the state array is up to date, 
		// and the number of bins is not subject to change anymore
		if ( IsReportPending(*p_time_current) ){
			_array_state_reference = _array_state/_delta_v;
			AdaptInterpretationArray();
			*_p_number_of_current_bins  = _n_bins;
		}

		if (_par_pop._tau < TIME_MEMBRANE_INFINITE )
			*_p_current_rate = _p_zl->CalculateRate();
//		else
//			nothing
	}

	return true;
}


template <class Weight>
void PopulationGridController<Weight>::AddNewBins()
{
	_n_bins += _n_bins_to_add;
	double diff_new = NumberOfGrowingBins();
	double diff_old = static_cast<double>(_n_initial_bins) - static_cast<double>(_bins._index_reversal_bin);

	_f_current_scale= diff_new/diff_old;
	_delta_v = DeltaV();

	while (
		_par_pop._V_reset - BinToCurrentPotential(++_bins._index_current_reset_bin ) >= 0  && 
		_bins._index_current_reset_bin < _n_bins - 1
	)
		;
	--_bins._index_current_reset_bin;
}

template <class Weight>
void PopulationGridController<Weight>::Rebin()
{
	// rebinner may need to access information maintained by the AbstractZeroLeakEquation instance, for example, refractive probability
	_p_rebinner->Rebin(_p_zl.get());
	
	_n_bins = _n_initial_bins;
	_f_current_scale = 1.0;
	_delta_v  = DeltaV();

	// Adapt _H, which is now invalid, to the new bin size. It is still necessary for the output rate calculation !!!! 
	_bins._index_current_reset_bin = _bins._index_original_reset_bin;
	_p_zl->RecalculateSolverParameters();

}

template <class Weight>
void PopulationGridController<Weight>::AdaptInterpretationArray()
{
	_time_next_report += _time_report;

	for( Index i = 0; i < _n_bins;i++ )
		_array_interpretation[i] = BinToCurrentPotential(i);
}

template <class Weight>
bool PopulationGridController<Weight>::IsReportPending(Time time) const
{
	return (time  > _time_next_report);
}

template <class Weight>
bool PopulationGridController<Weight>::IsTimeToRebin() const
{
	// rebin if the next step would take you past the boundary array
	return ( _n_bins + 1 > static_cast<Number>( _array_state.size() ) ) ? true : false;
}

template <class Weight>
Time PopulationGridController<Weight>::DeltaTimeNextBinAdded(Time time) const
{
	return ( _n_bins_to_add > 0 ) ?_time_membrane_constant*log(1 + _n_bins_to_add/NumberOfGrowingBins() ) : _time_network_step;
}

template <class Weight>
Index PopulationGridController<Weight>::IndexReversalBin() const
{
	// no possibility to use find on valarray here
	// note that the reversal potential MUST be in here (guaranteed by initialization)

	Index index = 0;
	while
	( 
		_array_interpretation[index++] != this->_par_pop._V_reversal && 
		index < _array_interpretation.size() 
	)
		;
	--index;

	assert( index < _n_bins - 1);
	return index;
}

template <class Weight>
Potential PopulationGridController<Weight>::DeltaV() const
{
	return (_par_pop._theta - _par_pop._V_reversal)/(NumberOfGrowingBins()-1);
}

template <class Weight>
double PopulationGridController<Weight>::NumberOfGrowingBins() const
{
	return static_cast<double>(_n_bins) - static_cast<double>(_bins._index_reversal_bin);
}

template <class Weight>
Potential PopulationGridController<Weight>::BinToCurrentPotential(Index index) const
{
	assert(index < _n_bins);
	return _par_pop._V_reversal + ( static_cast<int>(index) - static_cast<int>(_bins._index_reversal_bin) )*_delta_v;
}



template <class Weight>
bool PopulationGridController<Weight>::UpdateCheckSum()
{
	_check_sum = _array_state.sum();
	return true;
}

template <class Weight>
bool PopulationGridController<Weight>::IsFinite() const
{
	for (int i = 0; i < static_cast<int>(_array_state.size()); i++)
		if ( ! ::IsFinite(_array_state[i]) )
			return false;

	return true;
}

template <class Weight>
Index PopulationGridController<Weight>::CurrentPotentialToBin(Potential v) const
{
	return static_cast<Index>(_bins._index_reversal_bin + floor( (v -_par_pop._V_reversal)/_delta_v + 0.5));
}

template <class Weight>
Index PopulationGridController<Weight>::IndexOriginalResetBin
(
) const
{
	double difference = numeric_limits<double>::max();
	Index index = 0;

	double test;
	while( (test = fabs( (_array_interpretation)[index++] - this->_par_pop._V_reset )) < difference )
		difference = test;

	index-=2;

	assert( index < _n_bins - 1 );

	return index;
}

template <class Weight>
void PopulationGridController<Weight>::EmbedGrid
( 
	Number					nr_initial_grid,
	const valarray<double>& array_state_initial, 
	const valarray<double>& array_interpretation_initial,
	void*					p_param
)
{
	// Purpose: Embed the initial density grid in a larger one, that can accomodate the increasing
	// number of bins during evolution, until rebinning takes place
	// Author: Marc de Kamps
	// Date: 26-08-2005
	// Adapted from PopulationAlgorithm to PopulationGridController: 09-03-2008

	// first initialize the valarrays to zero, in order to clean the last values
	// then copy the initial density
	
	_array_state			= 0.0;
	_array_state_reference	= 0.0;
	_array_interpretation	= 0.0;

	slice slice_array(0,array_state_initial.size(),1);
	_array_state[slice_array]			= array_state_initial;
	_array_state_reference[slice_array]	= array_state_initial;

	slice slice_interpretation(0,array_interpretation_initial.size(),1);
	_array_interpretation[slice_interpretation] = array_interpretation_initial;

	_check_sum = _array_state.sum();
	*_p_number_of_current_bins	= nr_initial_grid;
	_n_bins						= nr_initial_grid;
	_n_initial_bins				= nr_initial_grid;

	_bins._index_reversal_bin	= IndexReversalBin();
	_delta_v					= DeltaV();

	_array_state				/= _array_state.sum();
	_check_sum					= _array_state.sum();

	_bins._index_original_reset_bin  = IndexOriginalResetBin ();
	_bins._index_current_reset_bin   = _bins._index_original_reset_bin;

	_p_zl->Configure(p_param);


	_p_rebinner->Configure
	(
		_array_state,
		_bins._index_reversal_bin,
		_bins._index_current_reset_bin,			// there are rebinners that need to know about reset bins
		static_cast<Number>(_array_state.size()),
		_n_initial_bins
	);
	
	if 
	( 
		  _n_bins_to_add  == 0 && 
		  _time_membrane_constant < TIME_MEMBRANE_INFINITE 
	)
		throw utilities::Exception(STR_BINS_MUST_BE_ADDED);

	if ( _time_membrane_constant == 0 )
		throw utilities::Exception(STR_MEMBRANE_ZERO);
}
} /* namespace populist */
} /* namespace MPILib */