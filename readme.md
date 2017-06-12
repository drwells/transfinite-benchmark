# Transfinite Benchmark

## Overview
deal.II recently added the `TransfiniteInterpolationManifold` class, which
implements blending between manifolds in a smooth way. These calculations are
inherently expensive, but we can improve performance by being more careful with
which functions we mark as `inline` and how we cache values that may be
(relatively) expensive to compute.

This benchmark implements a standard use case for
`TransfiniteInterpolationManifold`: we discretize a circle with the standard
approach on the inside (a square initially split into five cells) surrounded by
an actual annular grid. The `TransfiniteInterpolationManifold`'s job is to
correctly blend between the polar coordinates description given on the annulus
and the Cartesian description at the center of the circle. To do this, we have a
coarse grid (about twenty cells) where the outermost ring corresponds to the
annulus and the rest of the cells correspond to the circle (including its
Cartesian center).

## Resulting patches
These are in progress.
1. Remove all dynamic memory allocation in the deal.II Manifold code. This is
   completely backwards incompatible but improves performance by about
   30%: Callgrind calculated that the original version of this benchmark spent
   about 30% of its time calling `new`/`malloc` and `delete`/`free`: using
   `ArrayView` instead of `std::vector` completely eliminates this overhead.
2. Better inlining of `GeometryInfo` functions: the benchmark took about 10% of
   its time running `GeometryInfo<2>::line_to_cell_vertices`, which is
   essentially just a lookup table call. Marking that function as `inline`
   completely eliminated its call: since the loop bounds were fixed GCC was
   smart enough to just plug in the returned offsets directly.
3. Vertex caching: looking up vertices in the global array requires several
   function calls. Improving the caching (so that vertices are only copied once
   per function call) improved performance by a further 10%.
4. Inlining vertex lookup: marking all of the vertex lookup functions as
   `inline` (they are already in headers anyway) lowered the number of vertex
   lookups from 229 million to 46 million: somehow GCC managed to cache things
   in a more intelligent way and avoid most of the lookups. This improved
   performance by about 6%.


Overall, these roughly halve the total computational time of creating these
refined grids. The Manifold changes are very invasive, but with better inlining
we can still get about 20% of the total speedup: not bad.
