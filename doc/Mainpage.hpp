/*!
\mainpage Miind Documentation


\section sec_introduction Introduction
MIIND is a simulator for modeling circuits of neural populations, with an emphasis on population density techniques, co-funded by 
the <a href="http://www.humanbrainproject.eu/">Human Brain Project</a>.  

MIIND addresses large neuronal networks at the population level, rather than at that of individual levels. Population density techniques retain a close correspondence to 
spiking neuron models: informally, if you use population density techniques you should get simulation results that are close to that of spiking neuron models, but for 
less overhead. The correspondence between population density techniques and groups of spiking neurons is much more rigorous than for rate based models.
Conceptually, we are close to the <a href="http://alleninstitute.github.io/dipde/">DIPDE</a> simulator developed by the 
<a href="https://www.alleninstitute.org">Allen Brain Institute</a>, but we provide different algorithms. Long term, we expect a common front end for DIPDE and MIIND.

Below, we will  provide an example. We assume that you have
some familiarity with using neural simulators such as <a href="http://www.nest-simulator.org">NEST</a> or <a href="http://briansimulator.org/">BRIAN</a>, 
or else that you've worked with rate based models such as Wilson-Cowan dynamics.


In case of a bug, please raise an <a href="https://github.com/dekamps/miind/issues">issue</a>. (Press on the green button), or send
an email to M.deKamps@leeds.ac.uk


Below, we will  provide an example. We assume that you have
some familiarity with using neural simulators such as <a href="http://www.nest-simulator.org">NEST</a> or <a href="http://briansimulator.org/">BRIAN</a>, 
or else that you've worked with rate based models such as Wilson-Cowan dynamics.

\section sec_intro_example Example  1: a one dimensional density
\image html dense.png
The image shows a density profile: consider a population of simulated neurons, measure the potential of each neuron and represent them in a histogram.
The markers are the result of a NEST simulation of 10000 leaky-integrate-and-fire neurons, the horizontal axis represents the membrane potential. The solid
curve is calculated by MIIND. The entire population is calculated by a single density function. From this, one can calculate population averaged quantities, such as the 
firing rate. Consider the spike raster of the simulation:
\image html spikeraster.png
This corresponds closely to the firing rate calculated from the density function:
\image html rate.png

MIIND provides a simulation framework for neural simulations. It focusses on population level
descriptions of neural dynamics, i.e. it does not provide a simulator for individual neurons,
but models population activity directly. It provides support for simple neural mass models, but
focusses strongly on so-called population density techniques.


To get a feeling for the simulator and its capabilities, go to the \subpage page_examples page.

To run the simulator, go to the \ref page_workflow page, after installation is completed. Make sure
you have had a glance at the \ref  page_examples  page.

\section sec_download Download
The latest tar bal can be found <a href="http://sourceforge.net/projects/miind">here</a>. Install on Unix platforms
is straightforward, using cmake. The procedure is explained
in \ref page_installation. You can checkout a snapshot of the latest
code with:

git clone git://git.code.sf.net/p/miind/git miind-git

In anticipation of a move to GitHub, we push our commits to a mirror repository there:
<a href="https://github.com/dekamps/miind">the MIIND GitHub page</a>

The components are Open Source software under a BSD licence.
\section licence Licence

MIIND is free and Open Source. It is distributed under a  BSD license (see \ref license)

\section sec_install Installation
Use cmake and make. For more details see \ref page_installation.

\section sec_dependencies Dependencies
For more details see \ref dependencies





\section sec_announce the latest developments.
 There is a now <a href="http://miind.sf.net/tutorial.pdf">tutorial</a>.
<<<<<<< HEAD
MIIND version 2.0 
=======
>>>>>>> master

MIIND version 1.03 was released om 9 February 2016. It contains more efficient leaky-integrate-and-fire support, and an extended example section.

MIIND version 1.02 was released on 19 September 2015. It contains a major bug fix.

MIIND version 1.01 was released on 11 March 2015. Appart from bug fixes it contains a visualization tool
to monitor progress of a running simulation (including the evolution of the densities!).



MIIND version 1.0 was released 25 January 2015. It is now solely dedicated to population density techniques 
and neural mass models. Important new features include:
- Full support for 1D neural models. These include leaky-integrate-and-fire and quadratic-integrate-and-fire neurons. Other neuron  models
can easily be provided or implemented.
- Support for arbitrary large synaptic efficacies: Fokker-Planck equations are contained as a special case.
- MPI support

Many new features are in developement. These include:

- Support for multi-dimensional models
- Support for non Poisson statistics
- Support for activity dependent efficacies

<<<<<<< HEAD
The old neural network code, as described , including a HMAX implementation is still available, but will no longer be maintained.
=======
The old neural network code, including a HMAX implementation is still available, but will no longer be maintained.
>>>>>>> master

\section sec_wiki WIKI
The MIIND <a href="http://sourceforge.net/p/miind/wiki/Home">wiki</a>.
The WIKI contains more details about the extra work that needs to be done to install MIIND under windows.

<<<<<<< HEAD

\image html hbp.png
\image latex hbp.png "MIIND development is now co-funded by the Human Brain Project." width=5cm
=======
>>>>>>> master

*/

