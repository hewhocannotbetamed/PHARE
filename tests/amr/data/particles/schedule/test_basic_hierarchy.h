#ifndef PHARE_TEST_BASIC_HIERARCHY_H
#define PHARE_TEST_BASIC_HIERARCHY_H


#include "amr/data/particles/particles_variable.h"
#include "amr/data/particles/refine/particles_data_split.h"
#include "amr/data/particles/refine/split.h"
#include "test_tag_strategy.h"


#include <SAMRAI/geom/CartesianGridGeometry.h>
#include <SAMRAI/hier/PatchHierarchy.h>
#include <SAMRAI/hier/RefineOperator.h>
#include <SAMRAI/hier/VariableContext.h>
#include <SAMRAI/mesh/ChopAndPackLoadBalancer.h>
#include <SAMRAI/mesh/GriddingAlgorithm.h>
#include <SAMRAI/mesh/StandardTagAndInitialize.h>
#include <SAMRAI/mesh/TileClustering.h>
#include <SAMRAI/tbox/InputManager.h>
#include <SAMRAI/xfer/RefineAlgorithm.h>
#include <SAMRAI/xfer/RefineSchedule.h>

#include <memory>
#include <string>


#include "input_config.h"

using namespace PHARE::core;
using namespace PHARE::amr;

/**
 * @brief  In this class we will create a hierarchy of two levels
 * the raffined level will be determined by the refined box defined
 * in the input files. The creation of the patch is managed directly
 * by SAMRAI ( thus the use of a GriddingAlgorithm ). To be able to
 * use a GriddingAlgorithm, we have to implement a TagStrategy.
 * The one implemented do the allocation of the data, the initialization of
 * root level, and finally the refine interpolation between root level
 * and the level 1
 */
template<std::size_t dimension, std::size_t interpOrder>
class BasicHierarchy
{
public:
    /**
     * @brief Construct the hierarchy consisting of two levels
     *
     * Only the ratio argument is required, we will use one input file per ratio.
     * Here we do multiple things, first we parse the inputFile to create the inputDatabase
     * that will be used by others SAMRAI's objects (GridGeometry, PatchHierarchy, LoadBalancer,
     * BoxGenerator, GriddingAlgorithm, StandardTagAndInitialize), they contain information for each
     * of them.
     *
     * We then store a view to the VariableDatabase singleton , that we will use to register all our
     * Variables. For each hybridQuantities we register a variable.
     *
     * We instantiate each objects needed by the GriddingAlgorithm, and then instantiate
     * a GriddingAlgorithm with the previous objects
     *
     */
    explicit BasicHierarchy()
        : inputDatabase_{SAMRAI::tbox::InputManager::getManager()->parseInputFile(
            inputBase + "input/input_" + std::to_string(dimension) + "d.txt")}
        , patchHierarchyDatabase_{inputDatabase_->getDatabase("PatchHierarchy")}
        , variableDatabase_{SAMRAI::hier::VariableDatabase::getDatabase()}
        , protons_{std::make_shared<ParticlesVariable<dimension, interpOrder>>(
              "protons", true,
              SAMRAI::hier::IntVector{SAMRAI::tbox::Dimension{dimension},
                                      ghostWidthForParticles<interpOrder>()})}

        , context_{variableDatabase_->getContext("context")}
        , variablesIds_{getVariablesIds_()}

        , gridGeometry_{std::make_shared<SAMRAI::geom::CartesianGridGeometry>(
              dimension_, "cartesian", inputDatabase_->getDatabase("CartesianGridGeometry"))}
        , hierarchy_{std::make_shared<SAMRAI::hier::PatchHierarchy>("PatchHierarchy", gridGeometry_,
                                                                    patchHierarchyDatabase_)}
        , loadBalancer_{std::make_shared<SAMRAI::mesh::ChopAndPackLoadBalancer>(
              dimension_, "ChopAndPackLoadBalancer",
              inputDatabase_->getDatabase("ChopAndPackLoadBalancer"))}

        , tagStrategy_{std::make_shared<TagStrategy<dimension>>(variablesIds_)}

        , standardTag_{std::make_shared<SAMRAI::mesh::StandardTagAndInitialize>(
              "StandardTagAndInitialize", tagStrategy_.get(),
              inputDatabase_->getDatabase("StandardTagAndInitialize"))}

        , clustering_{std::make_shared<SAMRAI::mesh::TileClustering>(
              dimension_, inputDatabase_->getDatabase("TileClustering"))}

        , gridding_{std::make_shared<SAMRAI::mesh::GriddingAlgorithm>(
              hierarchy_, "GriddingAlgorithm", inputDatabase_->getDatabase("GriddingAlgorithm"),
              standardTag_, clustering_, loadBalancer_)}
    {
        // First step is to make the CoarsestLevel, and then to refine
        // as much as needed
        gridding_->makeCoarsestLevel(0.0);

        // We have our hierarchy setup, now is time to register the refineOperator
        // that we will use

        auto particlesVariableTypeName = typeid(ParticlesVariable<dimension, interpOrder>).name();
    }



    SAMRAI::hier::PatchHierarchy& getHierarchy() { return *hierarchy_; }


    std::map<std::string, int> const& getVariables() const { return variablesIds_; }


    void TearDown()
    {
        for (auto const& namesToId : variablesIds_)
        {
            variableDatabase_->removeVariable(namesToId.first);
        }
    }


private:
    std::map<std::string, int> getVariablesIds_()
    {
        std::map<std::string, int> variablesIds;


        SAMRAI::hier::IntVector ghostWidth{dimension_, ghostWidthForParticles<interpOrder>()};

        variablesIds.try_emplace("protons", variableDatabase_->registerVariableAndContext(
                                                protons_, context_, ghostWidth));

        return variablesIds;
    }

    std::shared_ptr<SAMRAI::tbox::Database> inputDatabase_;
    std::shared_ptr<SAMRAI::tbox::Database> patchHierarchyDatabase_;

    SAMRAI::tbox::Dimension dimension_{dimension};

    SAMRAI::hier::VariableDatabase* variableDatabase_;

    std::shared_ptr<ParticlesVariable<dimension, interpOrder>> protons_;



    std::shared_ptr<SAMRAI::hier::VariableContext> context_;

    std::map<std::string, int> variablesIds_;

    std::shared_ptr<SAMRAI::geom::CartesianGridGeometry> gridGeometry_;

    std::shared_ptr<SAMRAI::hier::PatchHierarchy> hierarchy_;
    std::shared_ptr<SAMRAI::mesh::ChopAndPackLoadBalancer> loadBalancer_;

    std::shared_ptr<SAMRAI::hier::RefineOperator> refineOperator_;

    std::shared_ptr<TagStrategy<dimension>> tagStrategy_;
    std::shared_ptr<SAMRAI::mesh::StandardTagAndInitialize> standardTag_;
    std::shared_ptr<SAMRAI::mesh::TileClustering> clustering_;

    std::shared_ptr<SAMRAI::mesh::GriddingAlgorithm> gridding_;
};

#endif
