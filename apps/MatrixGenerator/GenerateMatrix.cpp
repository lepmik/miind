
// Copyright (c) 2005 - 2015 Marc de Kamps
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

#include <fstream>
#include <iostream>
#include <TwoDLib.hpp>
#include "Bind.hpp" // still necessary for definitions of split
#include "ConstructResetMapping.hpp"
#include "CorrectStrays.hpp"
#include "GenerateMatrix.hpp"
#include "TranslationObject.hpp"

namespace {
void FixModelFile(const string& basename){
	// get document
	pugi::xml_document doc_model;

	std::string str_model((basename + ".model"));
	pugi::xml_parse_result result = doc_model.load_file(str_model.c_str());

	pugi::xml_node model = doc_model.first_child();
	if ( model.name() != std::string("Model") )
		throw TwoDLib::TwoDLibException("Can't parse model file");

	// see if there is a reset mapping already; if yes, the rest is not necessary
	for (pugi::xml_node mapping = model.child("Mapping"); mapping; mapping = mapping.next_sibling("Mapping"))
	{
	    if (mapping.attribute("type").value() == string("Reset") )
	    	return;
	}

	// get reset mapping
	pugi::xml_document doc_reset;
	string resname = basename + std::string(".res");
	result = doc_reset.load_file(resname.c_str());
	if (!result)
		throw TwoDLib::TwoDLibException("Couldn't parse reset XML file");

	std::string str_reset(doc_reset.first_child().first_child().value());


	// insert a reset mapping node in the document before the threshold node
	pugi::xml_node node_rev = model.append_child("Mapping");
	node_rev.append_attribute("type") = "Reset";
	node_rev.append_child(pugi::node_pcdata).set_value(str_reset.c_str());

	std::ofstream ofst(str_model);
	if (!ofst)
		throw TwoDLib::TwoDLibException("Couldn't reopen model file");

	doc_model.print(ofst);
}

void WriteOutLost(const string& fn, const TwoDLib::TransitionMatrixGenerator& gen){
	vector<TwoDLib::Point> v(gen.LostPoints());
	std::ofstream ofst(fn);
	for(const TwoDLib::Point& p: v)
		ofst << p[0] << "\t" << p[1] << "\n";

}

void Write
(
	const string& fn,
	const std::vector<TwoDLib::TransitionList>& list,
	const TwoDLib::Translation& efficacy,
	MPILib::Index l_min,
	MPILib::Index l_max
)
{
	std::ofstream ofst(fn);
	ofst.precision(10);
	if (l_min == 0)
		ofst << efficacy._v << "\t" << efficacy._w << "\n";

	for (auto it = list.begin(); it != list.end(); it++){
		ofst << it->_number     << ";";
		ofst << it->_origin[0]  << ",";
		ofst << it->_origin[1]  << ";";
		for (auto ithit = it->_destination_list.begin(); ithit !=it-> _destination_list.end(); ithit++ ){
			ofst << ithit->_cell[0] << ",";
			ofst << ithit->_cell[1] << ":";
			ofst << double(ithit->_count)/it->_number << ";";
		}
		ofst << "\n";
	}
}
	void GenerateElements
	(
			const string& base_name,
			const TwoDLib::Mesh& mesh,
			const TwoDLib::Fid& fid,
			double V_th,
			double V_reset,
			std::unique_ptr<TwoDLib::TranslationObject>& p_to,
			double tr_reset,
			MPILib::Number nr_points,
			MPILib::Index l_min,
			MPILib::Index l_max
	){
	    std::vector<TwoDLib::TransitionList> transitions;

	    const std::vector< std::vector<TwoDLib::Translation> >& translation_list = p_to->TranslationList();
	    const TwoDLib::Translation efficacy = p_to->Efficacy();

		vector<TwoDLib::Coordinates> ths   = mesh.findV(V_th,TwoDLib::Mesh::EQUAL);
		vector<TwoDLib::Coordinates> above = mesh.findV(V_th,TwoDLib::Mesh::ABOVE);
		vector<TwoDLib::Coordinates> below = mesh.findV(V_th,TwoDLib::Mesh::BELOW);
		vector<TwoDLib::Coordinates> thres = mesh.findV(V_reset,TwoDLib::Mesh::EQUAL);

		std::cout << "threshold:\t" << V_th  << std::endl;
		std::cout << "reset:\t\t"   << V_reset << std::endl;

		//add stationary points to below, or they will be ignored
		below.push_back(TwoDLib::Coordinates(0,0));
		std::cout << "There are " << below.size() << " cells below threshold" << std::endl;
		std::cout << "There are " << above.size() << " cells above threshold" << std::endl;
		std::cout << "There are " << ths.size()   << " cells on threshold" << std::endl;


		std::cout << "start generation" << std::endl;
		TwoDLib::MeshTree tree(mesh);
		TwoDLib::Uniform uni(123456);

		std::vector<TwoDLib::FiducialElement> list = fid.Generate(mesh);
		TwoDLib::TransitionMatrixGenerator gen(tree,uni,nr_points,list);

		TwoDLib::TransitionList l;

		std::vector<TwoDLib::Coordinates> strays;

		MPILib::Index min_strip, max_strip;
		if (l_min == 0 && l_max == 0){
			min_strip = 0;
			max_strip = mesh.NrQuadrilateralStrips();
		} else {
			min_strip = l_min;
			max_strip = l_max;
		}

		for( MPILib::Index i = min_strip; i <  max_strip; i++){
			std::cout << i << " " << mesh.NrCellsInStrip(i) << std::endl;
			for (MPILib::Index j = 0; j <  mesh.NrCellsInStrip(i); j++){

				// Only generate if the origin is in BELOW, or in EQUAL
				TwoDLib::Coordinates c(i,j);
				if (std::find(above.begin(),above.end(),c) == above.end() ){

					gen.Reset(nr_points);
					TwoDLib::Translation tr = translation_list[i][j];
					gen.GenerateTransition(i,j,tr._v,tr._w);
					l._number = gen.N();
					l._origin = TwoDLib::Coordinates(i,j);
					l._destination_list = gen.HitList();

					TwoDLib::TransitionList lcor = TwoDLib::CorrectStrays(l,ths,above,mesh);

					transitions.push_back(lcor);
				}
			}
		}

		std::cout << "Finished. Writing out" << std::endl;
		std::ostringstream ostfn;
		ostfn.precision(10);

		ostfn << "_" << efficacy._v << "_" << efficacy._w << "_" << l_min << "_" << l_max << "_";
		std::string mat_name = base_name + ostfn.str() + ".mat";
		vector<TwoDLib::Coordinates> resets  = mesh.findV(V_reset,TwoDLib::Mesh::EQUAL);

		std::cout << "There are " << ths.size() << " reset bins." << std::endl;
		Write(mat_name,transitions,efficacy,l_min,l_max);


		// the reset mapping is the same for all ranges
		std::ofstream ofst(base_name + string(".res"));
		TwoDLib::ConstructResetMapping(ofst, mesh, ths, thres, tr_reset, &gen);
		ofst.close(); // needed; FixModelFile will reopen this file

		// the model file now needs to be fixed, to include the reset mapping
		FixModelFile(base_name);

		// Give an account of lost points
		WriteOutLost(base_name + ostfn.str() + string(".lost"), gen);
	}

	void SetupObjects
	(
		const std::string& base_name,
		unsigned int nr_points,
		std::unique_ptr<TwoDLib::TranslationObject>& p_to,
		double tr_reset,
		unsigned int l_min,
		unsigned int l_max
		)
	{
		std::cout << "Setup" << std::endl;

		pugi::xml_document doc;

		pugi::xml_parse_result result = doc.load_file((base_name + std::string(".model")).c_str());
		if (!result)
			throw TwoDLib::TwoDLibException("Couldn't use stream for Model.");

		pugi::xml_node mesh_node = doc.first_child().child("Mesh");
		std::ostringstream o_mesh;
		mesh_node.print(o_mesh," ");
		std::istringstream i_mesh(o_mesh.str());
		TwoDLib::Mesh mesh(i_mesh);

		if (l_max > mesh.NrQuadrilateralStrips()){
			std::cout << "Upper range too large: should not exceed number of strips" << std::endl;
			exit(0);
		}

		std::cout << "There are: " << mesh.NrQuadrilateralStrips() << " strips" << std::endl;

		TwoDLib::Fid fid((base_name + std::string(".fid")).c_str());

		pugi::xml_node rev_node = doc.first_child().child("Mapping");
		std::ostringstream o_rev;
		rev_node.print(o_rev," ");
		std::istringstream i_rev(o_rev.str());
		std::vector<TwoDLib::Redistribution> vec_rev = TwoDLib::ReMapping(i_rev);

		pugi::xml_node th_node = doc.first_child().child("threshold");
		std::istringstream ith(th_node.first_child().value());
		double theta;
		ith >> theta;

		pugi::xml_node vr_node = doc.first_child().child("V_reset");
		std::istringstream ivr(vr_node.first_child().value());
		double V_reset;
		ivr >> V_reset;

		// make sure the TranslationObject is filled, consistent with mesh

		p_to->GenerateTranslationList(mesh);

		GenerateElements(base_name, mesh,fid,theta,V_reset, p_to, tr_reset, nr_points, l_min,l_max);
	}

	void PrepareMatrixGeneration(int argc, char** argv, TwoDLib::UserTranslationMode mode){
		std::cout << "Generate transition matrix" << std::endl;
		// this bit is common: model file, fid file, nr of translation points

		std::string model_name(argv[1]);
		std::vector<string> elem;
		TwoDLib::split(model_name,'.',elem);
		std::string base_name(elem[0]);
		if (elem.size() < 2 || elem[1] != string("model"))
			throw TwoDLib::TwoDLibException("Model extension not .model");


		std::string fid_name(argv[2]);
		elem.clear();
		TwoDLib::split(fid_name,'.',elem);
		if (elem.size() < 2 || elem[1] != string("fid"))
			throw TwoDLib::TwoDLibException("Fiducial extension not .fid");

		std::istringstream istp(argv[3]);
		unsigned int nr_points;
		istp >> nr_points;

		// this is also common if the argument parse does not provide values, they are 0
		unsigned int l_min = 0;
		unsigned int l_max = 0;

		// here, either the translation arguments are provided, or the jump file,
		// both must provide a translation object
		std::unique_ptr<TwoDLib::TranslationObject> p_to;

		// finally, at the moment we don't envisage reset values for jump files, but this may change.
		// in that case we probably should make it integral part of the TranslationObject. For now
		// we use it separately, as it is used differently by GenerateElements.
		double tr_reset = 0.;
		if (mode == TwoDLib::TranslationArguments){
			std::istringstream istrv(argv[4]);
			double tr_v;
			istrv >> tr_v;

			std::istringstream istrw(argv[5]);
			double tr_w;
			istrw >> tr_w;

			std::istringstream istre(argv[6]);
			// already declared and initialized above
			istre >> tr_reset;

			p_to = std::unique_ptr<TwoDLib::TranslationObject>{new TwoDLib::TranslationObject(tr_v, tr_w)};

			if (argc == 9){
				std::istringstream ist_min(argv[7]);
				std::istringstream ist_max(argv[8]);
				ist_min >> l_min;
				ist_max >> l_max;
			}

			if (l_min > l_max){
				std::cout << "Upper bound range smaller than lower bound" << std::endl;
				exit(0);
			}

			std::cout << "Range: " << l_min << " " << l_max << std::endl;
		} else {
			std::ifstream ifst(argv[4]);
			if (!ifst)
				throw TwoDLib::TwoDLibException("Could not open the jump file.");
			else
				p_to = std::unique_ptr<TwoDLib::TranslationObject>{ new TwoDLib::TranslationObject(ifst)};
			if (argc == 7){
				std::istringstream ist_min(argv[5]);
				std::istringstream ist_max(argv[6]);
				ist_min >> l_min;
				ist_max >> l_max;
			}

			if (l_min > l_max){
				std::cout << "Upper bound range smaller than lower bound" << std::endl;
				exit(0);
			}
		}

		SetupObjects(base_name,nr_points,p_to,tr_reset,l_min,l_max);
	}
}


void TwoDLib::GenerateMatrix(int argc, char** argv, TwoDLib::UserTranslationMode mode){

	PrepareMatrixGeneration(argc,argv, mode);
}
