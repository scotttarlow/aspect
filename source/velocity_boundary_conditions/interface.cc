/*
  Copyright (C) 2011 - 2016 by the authors of the ASPECT code.

  This file is part of ASPECT.

  ASPECT is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2, or (at your option)
  any later version.

  ASPECT is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with ASPECT; see the file doc/COPYING.  If not see
  <http://www.gnu.org/licenses/>.
*/


#include <aspect/global.h>
#include <aspect/velocity_boundary_conditions/interface.h>

#include <deal.II/base/exceptions.h>
#include <deal.II/base/std_cxx11/tuple.h>

#include <list>


namespace aspect
{
  namespace VelocityBoundaryConditions
  {
    template <int dim>
    Interface<dim>::~Interface ()
    {}


    template <int dim>
    void
    Interface<dim>::initialize ()
    {}



    template <int dim>
    void
    Interface<dim>::update ()
    {}



    template <int dim>
    Tensor<1,dim>
    Interface<dim>::boundary_velocity (const Point<dim> &) const
    {
      /**
       * We can only get here if the new-style boundary_velocity function (with
       * two arguments) calls it. This means that the derived class did not override
       * the new-style boundary_velocity function, and because we are here, it also
       * did not override this old-style boundary_velocity function (with one argument).
       */
      Assert (false, ExcMessage ("A derived class needs to override either the "
                                 "boundary_velocity(position) (deprecated) or "
                                 "boundary_velocity(types::boundary_id,position) "
                                 "function."));

      return Tensor<1,dim>();
    }


    DEAL_II_DISABLE_EXTRA_DIAGNOSTICS
    template <int dim>
    Tensor<1,dim>
    Interface<dim>::boundary_velocity (const types::boundary_id ,
                                       const Point<dim> &position) const
    {
      // Call the old-style function without the boundary id to maintain backwards
      // compatibility. Normally the derived class should override this function.
      return this->boundary_velocity(position);
    }
    DEAL_II_ENABLE_EXTRA_DIAGNOSTICS


    template <int dim>
    void
    Interface<dim>::
    declare_parameters (dealii::ParameterHandler &)
    {}


    template <int dim>
    void
    Interface<dim>::parse_parameters (dealii::ParameterHandler &)
    {}


// -------------------------------- Deal with registering velocity_boundary_conditions models and automating
// -------------------------------- their setup and selection at run time

    namespace
    {
      std_cxx11::tuple
      <void *,
      void *,
      internal::Plugins::PluginList<Interface<2> >,
      internal::Plugins::PluginList<Interface<3> > > registered_plugins;
    }



    template <int dim>
    void
    register_velocity_boundary_conditions_model (const std::string &name,
                                                 const std::string &description,
                                                 void (*declare_parameters_function) (ParameterHandler &),
                                                 Interface<dim> *(*factory_function) ())
    {
      std_cxx11::get<dim>(registered_plugins).register_plugin (name,
                                                               description,
                                                               declare_parameters_function,
                                                               factory_function);
    }


    template <int dim>
    Interface<dim> *
    create_velocity_boundary_conditions (const std::string &name)
    {
      Interface<dim> *plugin = std_cxx11::get<dim>(registered_plugins).create_plugin (name,
                                                                                      "Velocity boundary conditions");
      return plugin;
    }



    template <int dim>
    std::string
    get_names ()
    {
      return std_cxx11::get<dim>(registered_plugins).get_pattern_of_names ();
    }


    template <int dim>
    void
    declare_parameters (ParameterHandler &prm)
    {
      std_cxx11::get<dim>(registered_plugins).declare_parameters (prm);
    }
  }
}

// explicit instantiations
namespace aspect
{
  namespace internal
  {
    namespace Plugins
    {
      template <>
      std::list<internal::Plugins::PluginList<VelocityBoundaryConditions::Interface<2> >::PluginInfo> *
      internal::Plugins::PluginList<VelocityBoundaryConditions::Interface<2> >::plugins = 0;
      template <>
      std::list<internal::Plugins::PluginList<VelocityBoundaryConditions::Interface<3> >::PluginInfo> *
      internal::Plugins::PluginList<VelocityBoundaryConditions::Interface<3> >::plugins = 0;
    }
  }

  namespace VelocityBoundaryConditions
  {
#define INSTANTIATE(dim) \
  template class Interface<dim>; \
  \
  template \
  void \
  register_velocity_boundary_conditions_model<dim> (const std::string &, \
                                                    const std::string &, \
                                                    void ( *) (ParameterHandler &), \
                                                    Interface<dim> *( *) ()); \
  \
  template  \
  void \
  declare_parameters<dim> (ParameterHandler &); \
  \
  template  \
  std::string \
  get_names<dim> (); \
  \
  template \
  Interface<dim> * \
  create_velocity_boundary_conditions<dim> (const std::string &);

    ASPECT_INSTANTIATE(INSTANTIATE)
  }
}
