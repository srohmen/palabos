/* This file is part of the Palabos library.
 *
 * Copyright (C) 2011-2012 FlowKit Sarl
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

#include "dataProcessors/metaStuffFunctional2D.h"
#include "dataProcessors/metaStuffFunctional2D.hh"
#include "latticeBoltzmann/nearestNeighborLattices2D.h"
#include "latticeBoltzmann/nearestNeighborLattices2D.hh"

namespace plb {

template class StoreDynamicsFunctional2D<FLOAT_T,descriptors::DESCRIPTOR_2D>;
template class ExtractDynamicsChainFunctional2D<FLOAT_T,descriptors::DESCRIPTOR_2D>;
template class ExtractTopMostDynamicsFunctional2D<FLOAT_T,descriptors::DESCRIPTOR_2D>;
template class ExtractBottomMostDynamicsFunctional2D<FLOAT_T,descriptors::DESCRIPTOR_2D>;
template class AssignEntireCellFunctional2D<FLOAT_T,descriptors::DESCRIPTOR_2D>;

}

#endif  // COMPILE_2D
