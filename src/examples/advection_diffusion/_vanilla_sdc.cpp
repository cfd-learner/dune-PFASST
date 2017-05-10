/**
 * Advection-Diffusion with vanilla SDC.
 *
 * @ingroup AdvectionDiffusionFiles
 * @file examples/advection_diffusion/vanilla_sdc.cpp
 * @since v0.1.0
 */

#include <dune/functions/functionspacebases/pqknodalbasis.hh>
#include <dune/istl/matrixmarket.hh>
#include <dune/functions/common/utility.hh>
#include <dune/functions/functionspacebases/interpolate.hh>
#include <dune/grid/io/file/vtk/vtkwriter.hh>

#include <cstdlib>
#include <memory>

#include <pfasst.hpp>
#include <pfasst/controller/sdc.hpp>
#include "../../datatypes/dune_vec.hpp"

#include "../../finite_element_stuff/assemble.hpp"
#include "advection_diffusion_sweeper.hpp"



namespace pfasst
{
  namespace examples
  {
    namespace advection_diffusion
    {
      /**
       * Advection/diffusion example using an encapsulated IMEX sweeper.
       *
       * This example uses a vanilla SDC sweeper.
       *
       * @ingroup AdvectionDiffusion
       */
      error_map run_vanilla_sdc(double abs_residual_tol, double rel_residual_tol=0.0)
      {
        SDC<> sdc;

        auto const nnodes    = config::get_value<size_t>("num_nodes", 3); // call with ./dune-PFASST num_nodes=4
        auto const nelements = config::get_value<size_t>("num_elem", 10); //number of finite elements per dimension
        auto const quad_type = quadrature::QuadratureType::GaussLobatto; //GaussRadau;//GaussLegendre;
	auto const dt        = config::get_value<double>("dt", 0.5);
	auto const tend      = config::get_value<double>("tend", 0.5);

	std::cout << "nodes " << nnodes << std::endl; 
	auto FinEl   = make_shared<fe_manager>(nelements,1); //spatial_dofs
	auto ndofs   = FinEl->get_ndofs(0);
        auto quad    = quadrature::quadrature_factory(nnodes, quad_type);
        auto factory = make_shared<encap::Dune_VectorFactory<double>>(ndofs);
        auto sweeper = make_shared<AdvectionDiffusionSweeper<>>(FinEl);

        sweeper->set_quadrature(quad);
        sweeper->set_factory(factory);
        sweeper->set_residual_tolerances(abs_residual_tol, rel_residual_tol);

        sdc.add_level(sweeper);
        sdc.set_duration(0.0, tend, dt, 20);
        sdc.setup();
	
	auto q0 = sweeper->get_start_state();	

        sweeper->exact(q0, 0.0);

        sdc.run();

	

	
	typedef encap::Dune_VectorEncapsulation<double, double> vector_type;
        vector_type exact(ndofs); //= encap::as_vector<double,double>(q0);
	
	sweeper->exact(exact, tend);	
	//vector_type exact=encap::as_vector<double,double>(neu);
	vector_type u=  encap::as_vector<double,double>(sweeper->get_end_state());
	vector_type u0(ndofs);
	sweeper->exact(u0, 0.0);
	  
	
	//vector_type neu = encap::as_vector<double,double>(q0);	
	
	
	//typedef Dune::YaspGrid<1> GridType; 
        //auto grid = FinEl->get_grid();
        //typedef GridType::LeafGridView GridView;
        //GridType::LeafGridView gridView = grid->leafGridView();
        //Dune::VTKWriter<GridView> vtkWriter(gridView);
        //typedef Dune::BlockVector<Dune::FieldVector<double, 1> > VectorType;

	//std::cout << "num_nodes " << nnodes << " dt " << dt << std::endl; 
	double error = (exact.axpy(-1,u)).infinity_norm();
	std::cout << "error file created " << error << std::endl; 

	ofstream f;
	stringstream ss;
	ss << nelements;
	string s = "solution_sdc/" + ss.str() + ".dat";
	f.open(s, ios::app | std::ios::out );
	f << nelements << " " << dt << " "<< error << endl;


	f.close();

        /*ofstream ff;
        stringstream sss;
        sss << nelements << "_iter";
        string st = "solution_sdc/" + sss.str() + ".dat";
        ff.open(st, ios::app | std::ios::out );
        auto iter = sdc->_it_per_step;
        for (const auto &line : iter) {
          ff << dt <<"  " << line << std::endl;
        }

        ff.close();*/

	//vtkWriter.addVertexData(neu  , "fe_solution");
        //vtkWriter.write("heat_result");
	
        return sweeper->get_errors();	

      }
    }  // ::pfasst::examples::advection_diffusion
  }  // ::pfasst::examples
}  // ::pfasst


#ifndef PFASST_UNIT_TESTING
int main(int argc, char** argv)
{
  pfasst::init(argc, argv, pfasst::examples::advection_diffusion::AdvectionDiffusionSweeper<>::init_logs);
  
  pfasst::examples::advection_diffusion::run_vanilla_sdc(1e-10);
}
#endif