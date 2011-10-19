//-------------------------------------------------------------
//    $Id: simulator.h 232 2011-10-19 13:30:15Z bangerth $
//
//    Copyright (C) 2011 by the authors of the ASPECT code
//
//-------------------------------------------------------------
#ifndef __aspect__simulator_h
#define __aspect__simulator_h

#include <deal.II/base/timer.h>
#include <deal.II/base/tensor.h>
#include <deal.II/base/symmetric_tensor.h>
#include <deal.II/base/parameter_handler.h>
#include <deal.II/base/conditional_ostream.h>

#include <deal.II/lac/trilinos_block_vector.h>
#include <deal.II/lac/trilinos_sparse_matrix.h>
#include <deal.II/lac/trilinos_block_sparse_matrix.h>
#include <deal.II/lac/trilinos_precondition.h>
#include <deal.II/lac/trilinos_solver.h>

#include <deal.II/distributed/tria.h>

#include <deal.II/dofs/dof_handler.h>

#include <deal.II/fe/fe_system.h>
#include <deal.II/fe/mapping_q.h>

class P;
class P;

namespace aspect
{
  using namespace dealii;

  namespace internal
  {
    namespace Assembly
    {
      namespace Scratch
      {
        template <int dim>      struct StokesPreconditioner;
        template <int dim>      struct StokesSystem;
        template <int dim>      struct TemperatureMatrix;
        template <int dim>      struct TemperatureRHS;
      }

      namespace CopyData
      {
        template <int dim>      struct StokesPreconditioner;
        template <int dim>      struct StokesSystem;
        template <int dim>      struct TemperatureMatrix;
        template <int dim>      struct TemperatureRHS;
      }
    }
  }

  namespace Postprocess
  {
    template <int dim> class SimulatorAccess;
  }

  template <int dim>
  class Simulator : public Subscriptor
  {
    public:
      struct Parameters;
      Simulator (Parameters &parameters);
      void run ();

    private:
      void setup_dofs ();
      void assemble_stokes_preconditioner ();
      void build_stokes_preconditioner ();
      void assemble_stokes_system ();
      void assemble_temperature_matrix ();
      void assemble_temperature_system ();
      void set_initial_temperature_field ();
      void compute_initial_pressure_field ();
      double get_maximal_velocity () const;
      double get_cfl_number () const;
      double get_entropy_variation (const double average_temperature) const;
      void compute_temperature_stats ();
      void compute_velocity_stats ();
      void compute_heat_flux_stats ();
      std::pair<double,double> get_extrapolated_temperature_range () const;
      void solve ();
      void postprocess ();
      void output_results ();
      void refine_mesh (const unsigned int max_grid_level);

      double
      compute_viscosity(const std::vector<double>          &old_temperature,
                        const std::vector<double>          &old_old_temperature,
                        const std::vector<Tensor<1,dim> >  &old_temperature_grads,
                        const std::vector<Tensor<1,dim> >  &old_old_temperature_grads,
                        const std::vector<double>          &old_temperature_laplacians,
                        const std::vector<double>          &old_old_temperature_laplacians,
                        const std::vector<Tensor<1,dim> >  &old_velocity_values,
                        const std::vector<Tensor<1,dim> >  &old_old_velocity_values,
                        const std::vector<SymmetricTensor<2,dim> >  &old_strain_rates,
                        const std::vector<SymmetricTensor<2,dim> >  &old_old_strain_rates,
                        const std::vector<double>          &old_pressure,
                        const std::vector<double>          &old_old_pressure,
                        const double                        global_u_infty,
                        const double                        global_T_variation,
                        const double                        average_temperature,
                        const double                        global_entropy_variation,
                        const std::vector<Point<dim> >     &evaluation_points,
                        const double                        cell_diameter) const;

    public:
      struct Parameters
      {
        Parameters (const std::string &parameter_filename);

        static void declare_parameters (ParameterHandler &prm);
        void parse_parameters (ParameterHandler &prm);

        double end_time;

        bool resume_computation;

        unsigned int initial_global_refinement;
        unsigned int initial_adaptive_refinement;
        double       refinement_fraction;
        double       coarsening_fraction;

        std::vector<double> additional_refinement_times;

        bool         generate_graphical_output;
        double       graphical_output_interval;

        unsigned int adaptive_refinement_interval;

        double       stabilization_alpha;
        double       stabilization_c_R;
        double       stabilization_beta;

        unsigned int stokes_velocity_degree;
        bool         use_locally_conservative_discretization;

        unsigned int temperature_degree;
        double perturbation_Angle;
        double perturbation_depth;
        double perturbation_Amplitude;
        double perturbation_Sigma;
        double perturbation_Sign;
        bool perturbation_GaussianPerturbation;

        int IsCompressible;
        int ShearHeating;
        int AdiabaticCompression;
        double kappa;
        double reference_density;
        double reference_temperature;
        double radiogenic_heating;
        double thermal_expansivity;
        double thermal_conductivity;
        double R1;
        double R0;
        double apperture_angle;
        double T1;
        double T0;
        double reference_eta;
        double reference_gravity;


      };

    private:
      Parameters                           &parameters;
      ConditionalOStream                  pcout;

      parallel::distributed::Triangulation<dim> triangulation;
      double                              global_Omega_diameter;
      double                              global_volume;

      const MappingQ<dim>                 mapping;

      const FESystem<dim>                 stokes_fe;

      DoFHandler<dim>                     stokes_dof_handler;
      ConstraintMatrix                    stokes_constraints;

      TrilinosWrappers::BlockSparseMatrix stokes_matrix;
      TrilinosWrappers::BlockSparseMatrix stokes_preconditioner_matrix;

      TrilinosWrappers::MPI::BlockVector  stokes_solution;
      TrilinosWrappers::MPI::BlockVector  old_stokes_solution;
      TrilinosWrappers::MPI::BlockVector  stokes_rhs;
      TrilinosWrappers::MPI::BlockVector  stokes_rhs_helper;


      FE_Q<dim>                           temperature_fe;
      DoFHandler<dim>                     temperature_dof_handler;
      ConstraintMatrix                    temperature_constraints;

      TrilinosWrappers::SparseMatrix      temperature_mass_matrix;
      TrilinosWrappers::SparseMatrix      temperature_stiffness_matrix;
      TrilinosWrappers::SparseMatrix      temperature_matrix;

      TrilinosWrappers::MPI::Vector       temperature_solution;
      TrilinosWrappers::MPI::Vector       old_temperature_solution;
      TrilinosWrappers::MPI::Vector       old_old_temperature_solution;
      TrilinosWrappers::MPI::Vector       temperature_rhs;


      double time;
      double time_step;
      double old_time_step;
      unsigned int timestep_number;
      unsigned int out_index;

      std_cxx1x::shared_ptr<TrilinosWrappers::PreconditionAMG> Amg_preconditioner;
      std_cxx1x::shared_ptr<TrilinosWrappers::PreconditionILU> Mp_preconditioner;
      std_cxx1x::shared_ptr<TrilinosWrappers::PreconditionIC>  T_preconditioner;

      bool rebuild_stokes_matrix;
      bool rebuild_stokes_preconditioner;
      bool rebuild_temperature_matrices;
      bool rebuild_temperature_preconditioner;

      TimerOutput computing_timer;

      void setup_stokes_matrix (const std::vector<IndexSet> &stokes_partitioning);
      void setup_stokes_preconditioner (const std::vector<IndexSet> &stokes_partitioning);
      void setup_temperature_matrices (const IndexSet &temperature_partitioning);

      void
      local_assemble_stokes_preconditioner (const typename DoFHandler<dim>::active_cell_iterator &cell,
                                            internal::Assembly::Scratch::StokesPreconditioner<dim> &scratch,
                                            internal::Assembly::CopyData::StokesPreconditioner<dim> &data);

      void
      copy_local_to_global_stokes_preconditioner (const internal::Assembly::CopyData::StokesPreconditioner<dim> &data);


      void
      local_assemble_stokes_system (const typename DoFHandler<dim>::active_cell_iterator &cell,
                                    internal::Assembly::Scratch::StokesSystem<dim>  &scratch,
                                    internal::Assembly::CopyData::StokesSystem<dim> &data);

      void
      copy_local_to_global_stokes_system (const internal::Assembly::CopyData::StokesSystem<dim> &data);


      void
      local_assemble_temperature_matrix (const typename DoFHandler<dim>::active_cell_iterator &cell,
                                         internal::Assembly::Scratch::TemperatureMatrix<dim>  &scratch,
                                         internal::Assembly::CopyData::TemperatureMatrix<dim> &data);

      void
      copy_local_to_global_temperature_matrix (const internal::Assembly::CopyData::TemperatureMatrix<dim> &data);



      void
      local_assemble_temperature_rhs (const std::pair<double,double> global_T_range,
                                      const double                   global_max_velocity,
                                      const double                   global_entropy_variation,
                                      const typename DoFHandler<dim>::active_cell_iterator &cell,
                                      internal::Assembly::Scratch::TemperatureRHS<dim> &scratch,
                                      internal::Assembly::CopyData::TemperatureRHS<dim> &data);

      void
      copy_local_to_global_temperature_rhs (const internal::Assembly::CopyData::TemperatureRHS<dim> &data);

      void normalize_pressure(TrilinosWrappers::MPI::BlockVector &vector);


      void create_snapshot();
      void resume_from_snapshot();

      template<class Archive>
      void serialize (Archive &ar, const unsigned int version);

      void make_pressure_rhs_compatible(TrilinosWrappers::MPI::BlockVector &vector,
                                        const TrilinosWrappers::MPI::BlockVector &helper);

      class Postprocessor;

      friend class boost::serialization::access;
      friend class Postprocess::SimulatorAccess<dim>;
  };
}


#endif

class C;
