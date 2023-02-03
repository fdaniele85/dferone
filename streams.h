//
// Created by daniele on 27/11/22.
//

#pragma once

#include <fstream>
#include <limits>

namespace dferone::streams {

    /** @brief  Skips a certain number of lines from an input file stream.
        *
        *  @param stream    The file stream.
        *  @param how_many  Number of lines to skip.
        */
    inline void skip_lines(std::ifstream& stream, std::size_t how_many = 1u) {
        for(auto i = 0u; i < how_many; ++i) {
            stream.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
    };

}