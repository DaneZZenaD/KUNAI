#include <iostream>
#include <memory>

#include "KUNAI/mjolnIR/Lifters/lifter_android.hpp"
#include "KUNAI/DEX/dex.hpp"
#include "KUNAI/mjolnIR/Analysis/ir_graph_ssa.hpp"

int main(int argc, char **argv)
{
    if (argc != 5)
    {
        std::cout << "USAGE: " << argv[0] << " <dex_file> <class_name> <method_name> <prototype>" << std::endl;
        return 1;
    }

    std::string class_name = argv[2];
    std::string method_name = argv[3];
    std::string prototype = argv[4];

    std::ifstream dex_file;

    dex_file.open(argv[1], std::ios::binary);

    auto fsize = dex_file.tellg();
    dex_file.seekg(0, std::ios::end);
    fsize = dex_file.tellg() - fsize;
    dex_file.seekg(0);

    // get a unique_ptr from a DEX object.
    // this will parse the DEX file, and nothing more.
    auto dex = KUNAI::DEX::get_unique_dex_object(dex_file, fsize);

    // check if parsing was correct.
    if (!dex->get_parsing_correct())
    {
        std::cerr << "[-] Parsing of DEX file was not correct." << std::endl;
        return 1;
    }

    // Set as disassembler the recursive traversal algorithm
    auto dex_disassember = dex->get_dex_disassembler();

    dex_disassember->set_disassembler_type(KUNAI::DEX::RECURSIVE_TRAVERSAL_DISASSEMBLER);

    auto dex_analysis = dex->get_dex_analysis(true);

    auto main_method = dex_analysis->get_method_analysis_by_name(class_name, method_name, prototype);

    if (main_method == nullptr)
    {
        std::cerr << "[-] Method selected doesn't exist, or it has not been found." << std::endl;
        return 1;
    }

    if (main_method->external())
    {
        std::cerr << "[-] External method, cannot be lifted to IR..." << std::endl;
        return 1;
    }

    auto lifter_android = std::make_shared<KUNAI::LIFTER::LifterAndroid>();

    auto graph = lifter_android->lift_android_method(main_method, dex_analysis);

    auto nodes = graph->get_nodes();

    std::cout << "\n\nPrinting nodes from " << class_name + "->" + method_name + prototype << "in MjolnIR\n";

    for (auto node : nodes)
    {
        std::cout << node->to_string();
        std::cout << "\n";
    }

    std::cout << "\nDumping .dot file into directory\n";

    graph->generate_dot_file(method_name);

    auto complexity = graph->get_cyclomatic_complexity();

    std::cout << "\nCyclomatic complexity = " << complexity << "\n";

    std::cout << "\nDumping the dominance tree\n";

    graph->generate_dominator_tree(method_name+"_dominance_tree");

    auto graph_ssa = std::make_shared<KUNAI::MJOLNIR::IRGraphSSA>(graph);

    std::cout << "\nDumping the Graph in SSA form\n";
    
    graph_ssa->generate_dot_file(method_name+"_ssa");

    auto reachingdefinition = std::make_shared<KUNAI::MJOLNIR::ReachingDefinition>(graph_ssa);

    reachingdefinition->compute();

    auto optimizer = KUNAI::MJOLNIR::NewDefaultOptimizer();

    optimizer->calculate_def_use_and_use_def_analysis(graph, reachingdefinition);

    for (auto& block : graph_ssa->get_nodes())
        for (auto& instr : block->get_statements())
            instr->print_use_def_and_def_use_chain();

    return 0;
}