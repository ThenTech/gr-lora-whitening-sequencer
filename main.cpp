#include <iostream>
#include <iomanip>
#include <stdint.h>
#include <vector>
#include <bitset>

#include "utils.hpp"

/** \brief  Global settings fetched from command line arguments.
 */
static struct SETTINGS {
    size_t      sf         = 7u;      ///< The Spreading Factor.
    std::string version    = "0.0.1";
    bool        includeHDR = false;   ///< Whether the given input bits contain a header
    bool        toOut      = false;   ///< Whether to write to outFile or defaultOut
    std::string inFile     = "";      ///< The path to te input file.
    std::string outFile    = "";      ///< The path to the output file.
    std::string defaultOut = "./";    ///< The path to the default output file.
} LoRaSettings;

/**
 *  \brief  Print the usage (command line arguments) of this program and exit.
 *
 *  \param  _name
 *          The name of the exe this code is called from.
 */
void showUsage(char* _name) {
    std::string name(_name),
                help_1 = " [-h|-H] [-sf|-SF <uint>] [-hdr]",
                help_2 = " [-i|-in <file>] [-o|-out <file>]";
    std::stringstream usage;

    std::strEraseToLast(name, "/");

    name = "Usage: " + (name[0] == '/' ? name.substr(1) : name);

    usage << name << help_1 << std::endl
          << std::setw(name.size() + help_2.size()) << help_2                                 << std::endl << std::endl
          << "  LoRa whitening sequencer expects a csv file with bytes in 0x.. format"        << std::endl
          << "  and will compare the bits of each byte on each row and write the highest"     << std::endl
          << "  occuring '1's to the specified file."                                         << std::endl << std::endl
          << "  -h,   -H,   --help             Show this help message"                        << std::endl
          << "  -sf,  -SF   <uint>             Set the spreading factor"                      << std::endl
          << "  -hdr,       --header           Input has header bits"                         << std::endl
          << "  -i,   -in,  --input  <file>    Specify input csv file with data"              << std::endl
          << "  -o,   -out, --output <file>    Specify output file for appending sequence"    << std::endl
          << std::endl;

    //fprintf(stderr, usage.str().c_str());
    std::cerr << usage.str();
}

/**
 *  \brief  Run main program.
 *
 *  Command line arguments (showUsage()):
 *
 *      Usage: lora-whitening-sequencer [-h|-H] [-sf|-SF <uint>] [-hdr]
 *                                      [-i|-in <file>] [-o|-out <file>]
 *
 *        LoRa whitening sequencer expects a csv file with bytes in 0x.. format
 *        and will compare the bits of each byte on each row and write the highest
 *        occuring '1's to the specified file.
 *
 *        -h,   -H,   --help             Show this help message
 *        -sf,  -SF   <uint>             Set the spreading factor
 *        -hdr,       --header           Input has header bits
 *        -i,   -in,  --input  <file>    Specify input csv file with data
 *        -o,   -out, --output <file>    Specify output file for appending sequence
 *
 *
 */
int main(int argc, char *argv[]) {
    int i;
    std::string arg;

    if (argc < 2) {
        showUsage(argv[0]);
        exit(1);
    }

    try {
        for (i = 1; i < argc; ++i) {
            arg = argv[i];

            if (arg == "-h" || arg == "-H" || arg == "--help") {
                // Show usage
                showUsage(argv[0]);
                exit(1);
            } else if (arg == "-sf" || arg == "-SF" || arg == "--spreading-factor") {
                // Set Spreading Factor
                LoRaSettings.sf = SysUtils::lexical_cast<size_t>(argv[++i]);
            } else if (arg == "-i" || arg == "-in" || arg == "--input") {
                // Set input file
                LoRaSettings.inFile = std::string(argv[++i]);
            } else if (arg == "-o" || arg == "-out" || arg == "--output") {
                // Set output file
                LoRaSettings.toOut   = true;
                LoRaSettings.outFile = std::string(argv[++i]);
            } else if (arg == "-hdr" || arg == "--header") {
                // Set if input contains header
                LoRaSettings.includeHDR = true;
            } else {
                // Show usage
                showUsage(argv[0]);
                exit(1);
            }
        }

        // 1 Read File
        const std::string *input_bits = SysUtils::readStringFromFile(LoRaSettings.inFile);

        // 2 Parse to vector of vectors
        char buff[20];
        std::string line, item;
        std::stringstream ss(*input_bits);
        std::vector< std::vector<uint8_t>* > whitening_bits;

        std::stringstream outstring;    // Has averaged whitening bits
        outstring << "const uint8_t prng_payload_sf" << LoRaSettings.sf << "[] = {\n    ";

        size_t max_bit_len = 1e7;

        while (std::getline(ss, line)) {
            std::trim(line);
            std::stringstream iss(line);
            std::vector<uint8_t> *bits = new std::vector<uint8_t>();

            while(std::getline(iss, item, ',')) {
                std::trim(item);
                bits->push_back(SysUtils::lexical_cast<size_t>(item.c_str()) & 0xFF);
            }

            if (bits->size() < max_bit_len) max_bit_len = bits->size();
            whitening_bits.push_back(bits);
        }

        // 3 Match bits
        const size_t numberOfSeqs = whitening_bits.size();
        uint8_t avg_sequence;

        for (size_t byte = 0u; byte < max_bit_len; ++byte) {
            size_t count1[8] = { 0u };
            avg_sequence     = 0u;

//            printf("byte %d count 1s:\n", byte);

            for (std::vector<uint8_t>* white : whitening_bits) {
//                printf("0x%02X : %s\n", (*white)[byte], std::bitset<8>((*white)[byte]).to_string().c_str());
                for (size_t i = 0; i < 8; ++i)
                    count1[i] += ((*white)[byte] & (1 << i)) > 0;
            }

//            printf("       ");
//            for (int i = 7; i >= 0; --i)
//                printf("%d, ", count1[i]);
//            putchar('\n');

            for (size_t i = 0; i < 8; ++i)
                avg_sequence |= (count1[i] >= (numberOfSeqs - count1[i])) << i;

            outstring << (byte ? ", " : "") << SysUtils::stringFormat(buff, 20, "0x%02X", avg_sequence);
        }

        // 4 Return most frequent and write to file
        outstring << "\n};\n";

        SysUtils::writeStringToFile(LoRaSettings.toOut ? LoRaSettings.outFile
                                                       : LoRaSettings.outFile + "WhiteningSeq_SF" +
                                                         std::to_string(LoRaSettings.sf) + ".h",
                                    outstring.str());

        // 5 Clean
        delete input_bits;
        for (auto *v : whitening_bits) {
            v->clear();
            delete v;
        }
        whitening_bits.clear();

    } catch (Exceptions::Exception const& e) {
        std::cerr << e.getMessage() << std::endl;
        exit(-1);
    } catch(std::exception const& e) {
        std::cerr << "std::exception: " << e.what() << std::endl;
        exit(-2);
    } catch(...) {
        std::cerr << "Unknown exception." << std::endl;
        exit(-3);
    }

    return 0;
}
