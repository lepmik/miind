/*
 * ProgressBar.cpp
 *
 *  Created on: 14.06.2012
 *      Author: david
 */

#include <MPILib/include/utilities/ProgressBar.hpp>
#include <boost/mpi/communicator.hpp>

namespace MPILib {
namespace utilities {

ProgressBar::ProgressBar(unsigned long expected_count,
		const std::string & description, std::ostream& os) :
		_description(description), _outputStream(os) {
	boost::mpi::communicator world;

	if (world.rank() == 0) {
		restart(expected_count);
	}
}

void ProgressBar::restart(unsigned long expected_count) {
	_count = _next_tic_count = _tic = 0;
	_expected_count = expected_count;
	_outputStream << _description << "\n"
			<< "0%   10   20   30   40   50   60   70   80   90   100%\n"
			<< "|----|----|----|----|----|----|----|----|----|----|"
			<< std::endl;

}

unsigned long ProgressBar::operator+=(unsigned long increment) {
	boost::mpi::communicator world;

	if (world.rank() == 0) {
		if ((_count += increment) >= _next_tic_count) {
			display_tic();
		}
	}
	return _count;
}

unsigned long ProgressBar::operator++() {
	return operator+=(1);
}

unsigned long ProgressBar::operator++(int) {
	return operator+=(1);
}

void ProgressBar::display_tic() {
	unsigned int tics_needed =
			static_cast<unsigned int>((static_cast<double>(_count)
					/ _expected_count) * 50.0);
	do {
		_outputStream << '*' << std::flush;
	} while (++_tic < tics_needed);
	_next_tic_count =
			static_cast<unsigned long>((_tic / 50.0) * _expected_count);
	if (_count == _expected_count) {
		if (_tic < 51)
			_outputStream << '*';
		_outputStream << std::endl;
	}
}

} /* namespace utilities */
} /* namespace MPILib */
