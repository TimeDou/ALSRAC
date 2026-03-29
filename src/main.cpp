#include <filesystem>
#include "headers.h"
#include "cmdline.h"
#include "dcals.h"
#include "simulatorPro.h"


using namespace std;
using namespace std::filesystem;
using namespace cmdline;


parser Cmdline_Parser(int argc, char * argv[])
{
    parser option;
    option.add <string> ("input", 'i', "Original Circuit file", true);
    option.add <string> ("approx", 'x', "Approximate Circuit file", false, "");
    option.add <string> ("library", 'l', "Standard Cell Library", false, "data/library/mcnc.genlib");
    option.add <string> ("metricType", 'm', "Error metric type, ER, NMED, MRED", false, "er");
    option.add <string> ("output", 'o', "Output path of circuits", false, "appntk/");
    option.add <int> ("mapType", 't', "Mapping Type, 0 = mcnc, 1 = lut", false, 0, range(0, 1));
    option.add <int> ("nOutput", 'n', "Number of outputs", false, 1);
    option.add <bool> ("isSigned", 'g', "Whether the circuit is signed", false, false);
    option.add <int> ("nFrame", 'f', "Initial Simulation Round", false, 64, range(1, INT_MAX));
    option.add <double> ("errorBound", 'b', "Error constraint upper bound", false, 0.002, range(0.0, 1.0));
    option.parse_check(argc, argv);
    return option;
}


int main(int argc, char * argv[])
{
    parser option = Cmdline_Parser(argc, argv);
    string input = option.get <string> ("input");
    string approx = option.get <string> ("approx");
    string library = option.get <string> ("library");
    string metricType = option.get <string> ("metricType");
    string output = option.get <string> ("output");
    int mapType = option.get <int> ("mapType");
    int nFrame = option.get <int> ("nFrame");
    double errorBound = option.get <double> ("errorBound");
    int nOutput = option.get <int> ("nOutput");
    bool isSigned = option.get <bool> ("isSigned");
    setisSigned(isSigned);
    setOutputNum(nOutput);
    setisSigned_dcals(isSigned);
    setOutputNum_dcals(nOutput);

    // create output path
    path outPath(output);
    if (!exists(outPath))
        create_directories(outPath);
    if (output[output.length() - 1] != '/')
        output += "/";

    Abc_Start();
    Abc_Frame_t * pAbc = Abc_FrameGetGlobalFrame();
    ostringstream command("");
    command << "read " << library;
    DASSERT(!Cmd_CommandExecute(pAbc, command.str().c_str()));

    command.str("");
    command << "read_blif " << input;
    DASSERT(!Cmd_CommandExecute(pAbc, command.str().c_str()));
    Abc_Ntk_t * pNtk = Abc_NtkDup(Abc_FrameReadNtk(pAbc));
    uint32_t pos0 = input.find(".blif");
    DASSERT(pos0 != input.npos);
    uint32_t pos1 = input.rfind("/");
    if (pos1 == input.npos)
        pos1 = -1;
    Ckt_NtkRename(pNtk, input.substr(pos1 + 1, pos0 - pos1 - 1).c_str());

    if (metricType == "ER" || metricType == "er") {
        Dcals_Man_t alsEng(pNtk, nFrame, errorBound, Metric_t::ER, mapType, output);
        alsEng.DCALS();
    }
    else if (metricType == "NMED" || metricType == "nmed") {
        Dcals_Man_t alsEng(pNtk, nFrame, errorBound, Metric_t::NMED, mapType, output);
        alsEng.DCALS();
    }
    else if (metricType == "MRED" || metricType == "mred") {
        Dcals_Man_t alsEng(pNtk, nFrame, errorBound, Metric_t::MRED, mapType, output);
        alsEng.DCALS();
    }

    Abc_NtkDelete(pNtk);

    // recycle memory
    Abc_Stop();

    return 0;
}
