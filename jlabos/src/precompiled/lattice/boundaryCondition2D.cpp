/* This file is part of the Palabos library.
 *
 * Copyright (C) 2011-2017 FlowKit Sarl
 * Route d'Oron 2
 * 1010 Lausanne, Switzerland
 * E-mail contact: contact@flowkit.com
 *
 * The most recent release of Palabos can be downloaded at 
 * <http://www.palabos.org/>
 *
 * The library Palabos is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * The library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifdef COMPILE_2D


#include "boundaryCondition/boundaryCondition2D.h"
#include "boundaryCondition/boundaryCondition2D.hh"
#include "latticeBoltzmann/nearestNeighborLattices2D.h"
#include "latticeBoltzmann/nearestNeighborLattices2D.hh"

namespace plb {

    template class OnLatticeBoundaryCondition2D<FLOAT_T, descriptors::DESCRIPTOR_2D>;

    template class BoundaryConditionInstantiator2D
        <
            FLOAT_T, descriptors::DESCRIPTOR_2D,
            RegularizedBoundaryManager2D < FLOAT_T, descriptors::DESCRIPTOR_2D >
        >;

    template OnLatticeBoundaryCondition2D<FLOAT_T,descriptors::DESCRIPTOR_2D>*
        createLocalBoundaryCondition2D < FLOAT_T,descriptors::DESCRIPTOR_2D >();


    template class BoundaryConditionInstantiator2D
        <
            FLOAT_T, descriptors::DESCRIPTOR_2D,
            EquilibriumBoundaryManager2D < FLOAT_T, descriptors::DESCRIPTOR_2D >
        >;

    template OnLatticeBoundaryCondition2D<FLOAT_T,descriptors::DESCRIPTOR_2D>*
        createEquilibriumBoundaryCondition2D < FLOAT_T,descriptors::DESCRIPTOR_2D >();


    template class BoundaryConditionInstantiator2D
        <
            FLOAT_T, descriptors::DESCRIPTOR_2D,
            InterpolationBoundaryManager2D < FLOAT_T, descriptors::DESCRIPTOR_2D >
        >;

    template OnLatticeBoundaryCondition2D<FLOAT_T,descriptors::DESCRIPTOR_2D>*
        createInterpBoundaryCondition2D < FLOAT_T,descriptors::DESCRIPTOR_2D >();
}

#endif  // COMPILE_2D
