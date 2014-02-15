/* This file is part of the Palabos library.
 *
 * Copyright (C) 2011-2013 FlowKit Sarl
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

/** \file
 * Operations on the 2D multiblock -- implementation.
 */

#include "multiBlock/multiBlockOperations2D.h"
#include "multiBlock/multiBlock2D.h"
#include "multiBlock/domainManipulation2D.h"
#include "atomicBlock/atomicBlock2D.h"
#include "atomicBlock/dataProcessor2D.h"
#include "atomicBlock/atomicBlockOperations2D.h"
#include "multiGrid/multiScale.h"
#include "core/plbDebug.h"


namespace plb {

template<class OriginalGenerator, class MutableGenerator>
class MultiProcessing2D {
public:
    MultiProcessing2D( OriginalGenerator& generator_,
                       std::vector<MultiBlock2D*> multiBlocks_ );
    ~MultiProcessing2D();
    void extractProcessorsOnFirstBlock(BlockDomain::DomainT appliesTo);
    void intersectWithRemainingBlocks(BlockDomain::DomainT appliesTo);
    void subdivideGenerator();
    void adjustCoordinates();
    std::vector<MutableGenerator*> const& getRetainedGenerators() const;
    std::vector<std::vector<plint> > const& getAtomicBlockNumbers() const;
    void multiBlocksWhichRequireUpdate (
            std::vector<MultiBlock2D*>& multiBlocksModifiedByProcessor,
            std::vector<modif::ModifT>& typesOfModification ) const;
    void updateEnvelopesWhereRequired();
private:
    void extractGeneratorOnBlocks(std::vector<Box2D> const& finalDomains,
                                  std::vector<std::vector<plint> > const& finalIds,
                                  plint shiftX=0, plint shiftY=0);
private:
    OriginalGenerator&               generator;
    std::vector<MultiBlock2D*>       multiBlocks;
    MultiBlock2D*                    firstMultiBlock;
    std::vector<MutableGenerator*>   retainedGenerators;
    std::vector<std::vector<plint> > atomicBlockNumbers;
};

/* *************** Class MultiProcessing2D *************************** */

template<class OriginalGenerator, class MutableGenerator>
MultiProcessing2D<OriginalGenerator,MutableGenerator>::MultiProcessing2D (
        OriginalGenerator& generator_,
        std::vector<MultiBlock2D*> multiBlocks_ )
    : generator(generator_),
      multiBlocks(multiBlocks_)
{
    PLB_PRECONDITION( multiBlocks.size()>=1 );
    firstMultiBlock = multiBlocks[0];

    // Subdivide the original generator into smaller generators which act
    //   on the intersection of all implied blocks. At this stage, all coordinates
    //   are global, thus relative to the multi-block, not to the individual
    //   atomic-blocks.
    subdivideGenerator();

    // Then, convert coordinates from a global representation to a local one,
    //   relative to the atomoic-blocks of the first multi-block.
    adjustCoordinates();
}

template<class OriginalGenerator, class MutableGenerator>
void MultiProcessing2D<OriginalGenerator,MutableGenerator>::subdivideGenerator()
{
    // To start with, determine which multi-blocks are read and which are written
    std::vector<bool> isWritten(multiBlocks.size());
    generator.getModificationPattern(isWritten);
    PLB_ASSERT( isWritten.size() == multiBlocks.size() );

    // The reference block (the one for which the envelope is included if
    //   the domain generator.appliesTo() include the envelope) is either the
    //   multi-block which is written, or the first multi-block if all are read-only.
    pluint referenceBlock = 0;
    for (pluint iBlock=0; iBlock<isWritten.size(); ++iBlock) {
        if (isWritten[iBlock]) {
            referenceBlock = iBlock;
            break;
        }
    }

    // In debug mode, make sure that a most one multi-block is written when envelope is included.
#ifdef PLB_DEBUG
    if ( BlockDomain::usesEnvelope(generator.appliesTo()) ) {
        plint numWritten = 0;
        for (pluint iBlock=0; iBlock<isWritten.size(); ++iBlock) {
            if (isWritten[iBlock]) {
                ++numWritten;
            }
        }
        PLB_ASSERT( numWritten <= 1 );
    }
#endif
    
    // The first step is to access the domains of the the atomic blocks, as well
    //   as their IDs in each of the coupled multi blocks. The domain corresponds
    //   to the bulk and/or to the envelope, depending on the value of generator.appliesTo().
    std::vector<std::vector<DomainAndId2D> > domainsWithId(multiBlocks.size());
    for (pluint iMulti=0; iMulti<multiBlocks.size(); ++iMulti) {
        std::vector<plint> const& blocks
            = multiBlocks[iMulti]->getMultiBlockManagement().getLocalInfo().getBlocks();
        for (pluint iBlock=0; iBlock<blocks.size(); ++iBlock) {
            plint blockId = blocks[iBlock];
            SmartBulk2D bulk(multiBlocks[iMulti]->getMultiBlockManagement(), blockId);
            switch (generator.appliesTo()) {
                case BlockDomain::bulk:
                    domainsWithId[iMulti].push_back(DomainAndId2D(bulk.getBulk(),blockId));
                    break;
                case BlockDomain::bulkAndEnvelope:
                    // It's only the reference block that should have the envelope. However, we start
                    //   by assigning bulk and envelope to all of them, and eliminate overlapping
                    //   envelope components further down.
                    domainsWithId[iMulti].push_back(DomainAndId2D(bulk.computeEnvelope(),blockId));
                    break;
                case BlockDomain::envelope:
                    // For the reference block, we restrict ourselves to the envelope, because
                    //   that's the desired domain of application.
                    if (iMulti==referenceBlock) {
                        std::vector<Box2D> envelopeOnly;
                        except(bulk.computeEnvelope(), bulk.getBulk(), envelopeOnly);
                        for (pluint iEnvelope=0; iEnvelope<envelopeOnly.size(); ++iEnvelope) {
                            domainsWithId[iMulti].push_back(DomainAndId2D(envelopeOnly[iEnvelope], blockId));
                        }
                    }
                    // For the other blocks, we need to take bulk and envelope, because all these domains
                    //   potentially intersect with the envelope of the reference block.
                    else {
                        domainsWithId[iMulti].push_back(DomainAndId2D(bulk.computeEnvelope(),blockId));
                    }
                    break;
            }
        }
    }

    // If the multi-blocks are not at the same level of grid refinement, the level
    //   of the first block is taken as reference, and the coordinates of the other
    //   blocks are rescaled accordingly.
    plint firstLevel = multiBlocks[0]->getMultiBlockManagement().getRefinementLevel();
    for (pluint iMulti=1; iMulti<multiBlocks.size(); ++iMulti) {
        plint relativeLevel = firstLevel -
                             multiBlocks[iMulti]->getMultiBlockManagement().getRefinementLevel();
        if (relativeLevel != 0) {
            for (pluint iBlock=0; iBlock<domainsWithId[iMulti].size(); ++iBlock) {
                domainsWithId[iMulti][iBlock].domain =
                    global::getDefaultMultiScaleManager().scaleBox (
                            domainsWithId[iMulti][iBlock].domain, relativeLevel );
            }
        }
    }

    // If the envelopes are included as well, it is assumed that at most one of
    //   the multi blocks has write-access. All others (those that have read-only
    //   access) need to be non-overlaping, to avoid multiple writes on the cells
    //   of the write-access-multi-block. Thus, overlaps are now eliminitated in
    //   the read-access-multi-blocks.
    if ( BlockDomain::usesEnvelope(generator.appliesTo()) ) {
        for (pluint iMulti=0; iMulti<multiBlocks.size(); ++iMulti) {
            if (!isWritten[iMulti]) {
                std::vector<DomainAndId2D> nonOverlapBlocks(getNonOverlapingBlocks(domainsWithId[iMulti]));
                domainsWithId[iMulti].swap(nonOverlapBlocks);
            }
        }
    }

    // This is the heart of the whole procedure: intersecting atomic blocks
    //   between all coupled multi blocks are identified.
    std::vector<Box2D> finalDomains;
    std::vector<std::vector<plint> > finalIds;
    intersectDomainsAndIds(domainsWithId, finalDomains, finalIds);

    // And, to end with, re-create processor generators adapted to the
    //   computed domains of intersection.
    if ( BlockDomain::usesEnvelope(generator.appliesTo()) ) {
        // In case the envelope is included, periodicity must be explicitly treated.
        //   Indeed, the user indicates the domain of applicability with respect to
        //   bulk nodes only. The generator is therefore shifted in all space directions
        //   to englobe periodic boundary nodes as well.
        plint shiftX = firstMultiBlock->getNx();
        plint shiftY = firstMultiBlock->getNy();
        PeriodicitySwitch2D const& periodicity = firstMultiBlock->periodicity();
        for (plint orientX=-1; orientX<=+1; ++orientX) {
        for (plint orientY=-1; orientY<=+1; ++orientY) {
                if (periodicity.get(orientX,orientY)) {
                    extractGeneratorOnBlocks( finalDomains, finalIds,
                                              orientX*shiftX, orientY*shiftY );
                }
            }
        }
    }
    else {
        extractGeneratorOnBlocks(finalDomains, finalIds);
    }
}

template<class OriginalGenerator, class MutableGenerator>
void MultiProcessing2D<OriginalGenerator,MutableGenerator>::extractGeneratorOnBlocks (
        std::vector<Box2D> const& finalDomains,
        std::vector<std::vector<plint> > const& finalIds,
        plint shiftX, plint shiftY)
{
    MutableGenerator* originalGenerator=generator.clone();
    originalGenerator->shift(shiftX,shiftY);
    for (pluint iDomain=0; iDomain<finalDomains.size(); ++iDomain) {
        MutableGenerator* extractedGenerator = originalGenerator->clone();
        if (extractedGenerator->extract(finalDomains[iDomain]) ) {
            retainedGenerators.push_back(extractedGenerator);
            atomicBlockNumbers.push_back(finalIds[iDomain]);
        }
        else {
            delete extractedGenerator;
        }
    }
    delete originalGenerator;
}


template<class OriginalGenerator, class MutableGenerator>
MultiProcessing2D<OriginalGenerator,MutableGenerator>::~MultiProcessing2D() {
    for (pluint iGenerator=0; iGenerator<retainedGenerators.size(); ++iGenerator) {
        delete retainedGenerators[iGenerator];
    }
}

template<class OriginalGenerator, class MutableGenerator>
void MultiProcessing2D<OriginalGenerator,MutableGenerator>::adjustCoordinates() {
    for (pluint iGenerator=0; iGenerator<retainedGenerators.size(); ++iGenerator) {
        // The generator is adjusted to local coordinates with respect to the first block.
        //   If the other blocks have a relative displacement wrt. the first block, this
        //   must be explicitly coded in the data processor.
        plint firstNumber = atomicBlockNumbers[iGenerator][0];
        Box2D envelope = SmartBulk2D( firstMultiBlock->getMultiBlockManagement(),
                                      firstNumber ).computeEnvelope();
        retainedGenerators[iGenerator]->shift(-envelope.x0, -envelope.y0);
    }
}


template<class OriginalGenerator, class MutableGenerator>
std::vector<MutableGenerator*> const&
    MultiProcessing2D<OriginalGenerator,MutableGenerator>::getRetainedGenerators() const
{
    return retainedGenerators;
}

template<class OriginalGenerator, class MutableGenerator>
std::vector<std::vector<plint> > const&
    MultiProcessing2D<OriginalGenerator,MutableGenerator>::getAtomicBlockNumbers() const
{
    return atomicBlockNumbers;
}

template<class OriginalGenerator, class MutableGenerator>
void MultiProcessing2D<OriginalGenerator,MutableGenerator>::multiBlocksWhichRequireUpdate (
        std::vector<MultiBlock2D*>& multiBlocksModifiedByProcessor,
        std::vector<modif::ModifT>& typesOfModification ) const
{
    multiBlocksModifiedByProcessor.clear();
    typesOfModification.clear();
    // If the generator includes envelopes, the envelopes need no update in any case.
    if ( ! BlockDomain::usesEnvelope(generator.appliesTo()) ) {
        // Otherwise, all blocks that have been modified by the processor must
        //   be updated.
        std::vector<modif::ModifT> allModifications(multiBlocks.size(), modif::undefined);
        // Default initialize to no-change.
        generator.getTypeOfModification(allModifications);
        for (pluint iBlock=0; iBlock<allModifications.size(); ++iBlock) {
            PLB_ASSERT( allModifications[iBlock] != modif::undefined );
            if (allModifications[iBlock] != modif::nothing) {
                multiBlocksModifiedByProcessor.push_back(multiBlocks[iBlock]);
                typesOfModification.push_back(allModifications[iBlock]);
            }
        }
    }
}

template<class OriginalGenerator, class MutableGenerator>
void MultiProcessing2D<OriginalGenerator,MutableGenerator>::updateEnvelopesWhereRequired()
{
    std::vector<MultiBlock2D*> updatedMultiBlocks;
    std::vector<modif::ModifT> typeOfModification;
    multiBlocksWhichRequireUpdate(updatedMultiBlocks, typeOfModification);
    for (pluint iBlock=0; iBlock<updatedMultiBlocks.size(); ++iBlock) {
        updatedMultiBlocks[iBlock]->duplicateOverlaps(typeOfModification[iBlock]);
    }
}


void executeDataProcessor( DataProcessorGenerator2D const& generator,
                           std::vector<MultiBlock2D*> multiBlocks )
{
    MultiProcessing2D<DataProcessorGenerator2D const, DataProcessorGenerator2D >
        multiProcessing(generator, multiBlocks);
    std::vector<DataProcessorGenerator2D*> const& retainedGenerators = multiProcessing.getRetainedGenerators();
    std::vector<std::vector<plint> > const& atomicBlockNumbers = multiProcessing.getAtomicBlockNumbers();

    for (pluint iGenerator=0; iGenerator<retainedGenerators.size(); ++iGenerator) {
        std::vector<AtomicBlock2D*> extractedAtomicBlocks(multiBlocks.size());
        for (pluint iBlock=0; iBlock<extractedAtomicBlocks.size(); ++iBlock) {
            extractedAtomicBlocks[iBlock]
                = &multiBlocks[iBlock]->getComponent(atomicBlockNumbers[iGenerator][iBlock]);
        }
        // Delegate to the "AtomicBlock version" of executeDataProcessor.
        plb::executeDataProcessor(*retainedGenerators[iGenerator], extractedAtomicBlocks);
    }
    // In the "executeProcessor" version, envelopes are updated right here, because the processor
    //   has already been executed. This behavior is unlike the behavior of the "addInternalProcessor" version,
    //   where envelopes are updated from within the multi-block, after the execution of internal processors.
    multiProcessing.updateEnvelopesWhereRequired();
}

void executeDataProcessor( DataProcessorGenerator2D const& generator,
                           MultiBlock2D& object )
{
    std::vector<MultiBlock2D*> objects(1);
    objects[0] = &object;
    executeDataProcessor(generator, objects);
}

void executeDataProcessor( DataProcessorGenerator2D const& generator,
                           MultiBlock2D& object1, MultiBlock2D& object2 )
{
    std::vector<MultiBlock2D*> objects(2);
    objects[0] = &object1;
    objects[1] = &object2;
    executeDataProcessor(generator, objects);
}

void executeDataProcessor( ReductiveDataProcessorGenerator2D& generator,
                           std::vector<MultiBlock2D*> multiBlocks )
{
    MultiProcessing2D<ReductiveDataProcessorGenerator2D, ReductiveDataProcessorGenerator2D >
        multiProcessing(generator, multiBlocks);
    std::vector<ReductiveDataProcessorGenerator2D*> const& retainedGenerators = multiProcessing.getRetainedGenerators();
    std::vector<std::vector<plint> > const& atomicBlockNumbers = multiProcessing.getAtomicBlockNumbers();

    std::vector<BlockStatistics const*> individualStatistics(retainedGenerators.size());
    for (pluint iGenerator=0; iGenerator<retainedGenerators.size(); ++iGenerator) {
        std::vector<AtomicBlock2D*> extractedAtomicBlocks(multiBlocks.size());
        for (pluint iBlock=0; iBlock<extractedAtomicBlocks.size(); ++iBlock) {
            extractedAtomicBlocks[iBlock] = &multiBlocks[iBlock]->getComponent(atomicBlockNumbers[iGenerator][iBlock]);
        }
        // Delegate to the "AtomicBlock Reductive version" of executeDataProcessor.
        plb::executeDataProcessor(*retainedGenerators[iGenerator], extractedAtomicBlocks);
        individualStatistics[iGenerator] = &(retainedGenerators[iGenerator]->getStatistics());
    }
    multiBlocks[0]->getCombinedStatistics().combine(individualStatistics, generator.getStatistics());
    // In the "executeProcessor" version, envelopes are updated right here, because the processor
    //   has already been executed. This behavior is unlike the behavior of the "addInternalProcessor" version,
    //   where envelopes are updated from within the multi-block, after the execution of internal processors.
    multiProcessing.updateEnvelopesWhereRequired();
}

void executeDataProcessor( ReductiveDataProcessorGenerator2D& generator,
                           MultiBlock2D& object )
{
    std::vector<MultiBlock2D*> objects(1);
    objects[0] = &object;
    executeDataProcessor(generator, objects);
}

void executeDataProcessor( ReductiveDataProcessorGenerator2D& generator,
                           MultiBlock2D& object1, MultiBlock2D& object2 )
{
    std::vector<MultiBlock2D*> objects(2);
    objects[0] = &object1;
    objects[1] = &object2;
    executeDataProcessor(generator, objects);
}


void addInternalProcessor( DataProcessorGenerator2D const& generator,
                           std::vector<MultiBlock2D*> multiBlocks, plint level )
{
    MultiProcessing2D<DataProcessorGenerator2D const, DataProcessorGenerator2D >
        multiProcessing(generator, multiBlocks);
    std::vector<DataProcessorGenerator2D*> const& retainedGenerators = multiProcessing.getRetainedGenerators();
    std::vector<std::vector<plint> > const& atomicBlockNumbers = multiProcessing.getAtomicBlockNumbers();

    for (pluint iGenerator=0; iGenerator<retainedGenerators.size(); ++iGenerator) {
        std::vector<AtomicBlock2D*> extractedAtomicBlocks(multiBlocks.size());
        for (pluint iBlock=0; iBlock<extractedAtomicBlocks.size(); ++iBlock) {
            extractedAtomicBlocks[iBlock] =
                &multiBlocks[iBlock]->getComponent(atomicBlockNumbers[iGenerator][iBlock]);
        }
        // Delegate to the "AtomicBlock version" of addInternal.
        plb::addInternalProcessor(*retainedGenerators[iGenerator], extractedAtomicBlocks, level);
    }
    // Subscribe the processor in the multi-block. This guarantees that the multi-block is aware
    //   of the maximal current processor level, and it instantiates the communication pattern
    //   for an update of envelopes after processor execution.
    std::vector<MultiBlock2D*> updatedMultiBlocks;
    std::vector<modif::ModifT> typeOfModification;
    multiProcessing.multiBlocksWhichRequireUpdate(updatedMultiBlocks, typeOfModification);
    multiBlocks[0]->subscribeProcessor (
            level,
            updatedMultiBlocks, typeOfModification,
            BlockDomain::usesEnvelope(generator.appliesTo()) );
    multiBlocks[0]->storeProcessor(generator, multiBlocks, level);
}

void addInternalProcessor( DataProcessorGenerator2D const& generator,
                           MultiBlock2D& object, plint level )
{
    std::vector<MultiBlock2D*> objects(1);
    objects[0] = &object;
    addInternalProcessor(generator, objects, level);
}

void addInternalProcessor( DataProcessorGenerator2D const& generator,
                           MultiBlock2D& object1, MultiBlock2D& object2,
                           plint level )
{
    std::vector<MultiBlock2D*> objects(2);
    objects[0] = &object1;
    objects[1] = &object2;
    addInternalProcessor(generator, objects, level);
}

}  // namespace plb
