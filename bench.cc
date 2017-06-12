#include <deal.II/base/geometry_info.h>

#include <deal.II/grid/grid_in.h>
#include <deal.II/grid/manifold_lib.h>
#include <deal.II/grid/tria.h>

#include <chrono>
#include <fstream>
#include <iostream>

using namespace dealii;

int main()
{
  constexpr int dim = 2;
  PolarManifold<dim> polar_manifold;
  TransfiniteInterpolationManifold<dim> inner_manifold;
  constexpr types::manifold_id polar_manifold_id = 1;
  constexpr types::manifold_id inner_manifold_id = 2;

  Triangulation<dim> triangulation;
  GridIn<dim> grid_in;
  std::ifstream input_grid("triangulation.msh");
  grid_in.attach_triangulation(triangulation);
  grid_in.read_msh(input_grid);
  triangulation.set_all_manifold_ids(inner_manifold_id);
  for (auto cell : triangulation.active_cell_iterators())
    {
      if (cell->at_boundary())
        {
          cell->set_all_manifold_ids(polar_manifold_id);
        }
    }
  triangulation.set_manifold(polar_manifold_id, polar_manifold);
  inner_manifold.initialize(triangulation);
  triangulation.set_manifold(inner_manifold_id, inner_manifold);

#ifdef DEBUG
  constexpr unsigned int n_refinements = 6;
#else
  constexpr unsigned int n_refinements = 8;
#endif

  for (unsigned int refinement_n = 0; refinement_n < n_refinements; ++refinement_n)
    {
      const auto t0 = std::chrono::high_resolution_clock::now();
      triangulation.refine_global(1);
      const auto t1 = std::chrono::high_resolution_clock::now();
      std::cout << "Number of active cells: "
                << triangulation.n_active_cells()
                << '\n';
      std::cout << "Refinement time: "
                << std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count()
                << '\n';
    }
}
