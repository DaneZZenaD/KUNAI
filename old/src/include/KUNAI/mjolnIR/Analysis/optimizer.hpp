/**
 * @file optimizer.hpp
 * @author Farenain
 * @brief Class for applying optimization passes to IR, working in IR
 *        we will be able to create more generic optimizations, and having
 *        an optimizer will allow us to have different optimizatio passes.
 *
 */

#ifndef OPTIMIZER_HPP
#define OPTIMIZER_HPP

#include <memory>

#include "KUNAI/Utils/utils.hpp"
#include "KUNAI/mjolnIR/ir_graph.hpp"
#include "KUNAI/mjolnIR/ir_grammar.hpp"
#include "KUNAI/mjolnIR/Analysis/reachingDefinition.hpp"

#include "KUNAI/mjolnIR/Analysis/single_instruction_optimizations.hpp"
#include "KUNAI/mjolnIR/Analysis/single_block_optimizations.hpp"

namespace KUNAI
{
    namespace MJOLNIR
    {
        class Optimizer;

        using optimizer_t = std::shared_ptr<Optimizer>;

        using one_stmnt_opt_t = std::optional<irstmnt_t> (*)(irstmnt_t &);
        using one_block_opt_t = std::optional<irblock_t> (*)(irblock_t &, irgraph_t &);

        class Optimizer
        {
        public:
            /**
             * @brief Add a single line optimization to the vector of optimizations
             *
             * @param opt
             */
            void add_single_stmnt_pass(one_stmnt_opt_t opt);

            /**
             * @brief Add a single block optimization to the vector of optimizations
             * 
             * @param opt 
             */
            void add_single_block_pass(one_block_opt_t opt);

            /**
             * @brief Run all the selected optimizations.
             *
             * @param func
             */
            void run_analysis(irgraph_t func);

            /**
             * @brief Analyze the basic blocks of the graph in order to create the
             *        fallthrough edges between blocks which are from conditional
             *        jumps, these has no edges so for the moment one block looks
             *        like goes nowehere:
             *
             *                          +----------------+
             *                          |                |
             *                          |  jcc           |
             *                          +----------------+
             *           fallthrough   /                  \  target
             *                        /                    \
             *               +----------------+        +----------------+
             *               | points         |        |                |
             *               | nowhere        |        |  jmp           |
             *               +----------------+        +----------------+
             *                                                  |
             *                                                  |
             *                                         +----------------+
             *                                         |                |
             *                                         |                |
             *                                         +----------------+
             *
             *         The one on the left points nowhere but this is the real previous
             *         block before the last block, but last block is divided because
             *         there's a jump to it, so we will create an edge between, the one
             *         on the left, and the last one, as it's a continuation.
             *
             *                          +----------------+
             *                          |                |
             *                          |  jcc           |
             *                          +----------------+
             *           fallthrough   /                  \  target
             *                        /                    \
             *               +----------------+        +----------------+
             *               | points         |        |                |
             *               | nowhere        |        |  jmp           |
             *               +----------------+        +----------------+
             *                      \                          /
             *                       \                        /
             *                        \                      /
             *                         \                    /
             *                          \                  /
             *                           +----------------+
             *                           |                |
             *                           |                |
             *                           +----------------+
             * @param ir_graph
             */
            void fallthrough_target_analysis(MJOLNIR::irgraph_t ir_graph);

            /**
             * @brief Calculate the def-use and use-def chains in an IRGraph
             *        for doing that we need to accept a reaching definition
             *        with the analysis already run. All the changes will be
             *        applied directly to the instructions of the IRGraph.
             *
             * @param ir_graph graph of a function in MjolnIR to calculate its def-use, use-def chains
             * @param reachingdefinition the object with the reaching definition.
             */
            void calculate_def_use_and_use_def_analysis(MJOLNIR::irgraph_t ir_graph,
                                                        reachingdefinition_t &reachingdefinition);

        private:
            /**
             * @brief Solve a def_use and use_def chain given an operand and a instruction
             *        here we will solve the reaching definition value and then we will cross-reference
             *        the instructions.
             *
             * @param operand
             * @param expr
             * @param reach_def_set
             * @param ir_graph
             */
            void solve_def_use_use_def(irexpr_t &operand, irstmnt_t expr, regdefinitionset_t &reach_def_set, MJOLNIR::irgraph_t ir_graph);

            std::vector<one_stmnt_opt_t> single_statement_optimization;
            std::vector<one_block_opt_t> single_block_optimization;
            reachingdefinition_t reachingdefinition;
        };

        /**
         * @brief Return a default optimizer object with all the configured passes.
         *
         * @return optimizer_t
         */
        optimizer_t NewDefaultOptimizer();
    }
}

#endif