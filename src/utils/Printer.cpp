/*
 * MRCPP, a numerical library based on multiresolution analysis and
 * the multiwavelet basis which provide low-scaling algorithms as well as
 * rigorous error control in numerical computations.
 * Copyright (C) 2019 Stig Rune Jensen, Jonas Juselius, Luca Frediani and contributors.
 *
 * This file is part of MRCPP.
 *
 * MRCPP is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * MRCPP is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with MRCPP.  If not, see <https://www.gnu.org/licenses/>.
 *
 * For information on the complete list of contributors to MRCPP, see:
 * <https://mrcpp.readthedocs.io/>
 */

#include <fstream>

#include "MRCPP/config.h"
#include "MRCPP/version.h"

#include "Printer.h"
#include "Timer.h"

#include "trees/MWTree.h"

#include "utils/details.h"

namespace mrcpp {

static std::ofstream tp_outfile;
int Printer::printWidth = 60;
int Printer::printLevel = -1;
int Printer::printPrec = 12;
int Printer::printRank = 0;
int Printer::printSize = 1;
std::ostream *Printer::out = &std::cout;

void Printer::init(int level, int rank, int size, const char *file) {
    printLevel = level;
    printRank = rank;
    printSize = size;
    if (file != nullptr) {
        std::stringstream fname;
        if (printSize > 1) {
            fname << file << "-" << printRank << ".out";
        } else {
            fname << file << ".out";
        }
        if (not fname.str().empty()) {
            tp_outfile.open(fname.str().c_str());
            setOutputStream(tp_outfile);
        }
    } else {
        if (printRank > 0) {
            setPrintLevel(-1); // Higher ranks be quiet
        }
    }
    setScientific();
}

int Printer::setWidth(int i) {
    int oldWidth = printWidth;
    printWidth = i;
    return oldWidth;
}

int Printer::setPrecision(int i) {
    int oldPrec = printPrec;
    printPrec = i;
    *out << std::setprecision(i);
    return oldPrec;
}

int Printer::setPrintLevel(int i) {
    int oldLevel = printLevel;
    printLevel = i;
    return oldLevel;
}

// clang-format off
void print::environment(int level) {
    if (level > Printer::getPrintLevel()) return;

    printout(level, std::endl);
    print::separator(level, '-', 1);
    println(level, " MRCPP version         : " << program_version());
    println(level, " Git branch            : " << git_branch());
    println(level, " Git commit hash       : " << git_describe());
    println(level, " Git commit author     : " << git_commit_author());
    println(level, " Git commit date       : " << git_commit_date() << std::endl);

#ifdef HAVE_BLAS
    println(level, " Linear algebra        : EIGEN v" << EIGEN_WORLD_VERSION << "."
                                                      << EIGEN_MAJOR_VERSION << "."
                                                      << EIGEN_MINOR_VERSION << " w/BLAS");
#else
    println(level, " Linear algebra        : EIGEN v" << EIGEN_WORLD_VERSION << "."
                                                      << EIGEN_MAJOR_VERSION << "."
                                                      << EIGEN_MINOR_VERSION);
#endif

#ifdef HAVE_MPI
#ifdef _OPENMP
    println(level, " Parallelization       : MPI/OpenMP");
#else
    println(level, " Parallelization       : MPI");
#endif
#else
#ifdef _OPENMP
    println(level, " Parallelization       : OpenMP");
#else
    println(level, " Parallelization       : NONE");
#endif
#endif

    printout(level, std::endl);
    print::separator(level, '-', 2);
}
// clang-format on

void print::separator(int level, const char &c, int newlines) {
    if (level > Printer::getPrintLevel()) return;
    printout(level, std::string(Printer::getWidth(), c));
    for (int i = 0; i <= newlines; i++) printout(level, std::endl);
}

void mrcpp::print::header(int level, const std::string &txt, int newlines, const char &c) {
    if (level > Printer::getPrintLevel()) return;

    auto n_spaces = (Printer::getWidth() - txt.size()) / 2;

    print::separator(level, c);
    println(level, std::string(n_spaces, ' ') << txt);
    print::separator(level, '-', newlines);
}

void print::footer(int level, const Timer &t, int newlines, const char &c) {
    if (level > Printer::getPrintLevel()) return;

    auto line_width = Printer::getWidth() - 2;
    auto txt_width = line_width / 2;
    auto val_width = 11;
    auto val_prec = 5;

    std::stringstream o;
    o << std::setw(val_width) << std::setprecision(val_prec) << std::scientific << t.elapsed() << " sec";

    print::separator(level, '-');
    printout(level, std::setw(txt_width) << "Wall time: ");
    printout(level, o.str() << std::endl);
    print::separator(level, c, newlines);
}

void print::value(int level, const std::string &txt, double v, const std::string &unit, int p, bool sci) {
    if (level > Printer::getPrintLevel()) return;

    if (p < 0) p = Printer::getPrecision();
    auto line_width = Printer::getWidth() - 2;
    auto txt_width = line_width / 2;
    auto unit_width = txt_width / 3;
    auto val_width = line_width - (txt_width + unit_width);
    auto n_spaces = std::max(0, txt_width - static_cast<int>(txt.size()));

    std::stringstream o;
    o << " " << txt;
    o << std::string(n_spaces, ' ');
    o << std::setw(unit_width) << unit;
    if (sci) {
        o << std::setw(val_width) << std::setprecision(p) << std::scientific << v;
    } else {
        o << std::setw(val_width) << std::setprecision(p) << std::fixed << v;
    }
    println(level, o.str());
}

void print::tree(int level, const std::string &txt, int n, int m, double t) {
    if (level > Printer::getPrintLevel()) return;

    auto line_width = Printer::getWidth() - 2;
    auto val_width = 2 * line_width / 9;
    auto txt_width = line_width - 3 * val_width;
    auto n_spaces = std::max(0, txt_width - static_cast<int>(txt.size()));

    std::string node_unit = " nds";

    double mem_val = 1.0 * m;
    std::string mem_unit = " kB";
    if (mem_val > 512.0) {
        mem_val = mem_val / 1024.0;
        mem_unit = " MB";
    }
    if (mem_val > 512.0) {
        mem_val = mem_val / 1024.0;
        mem_unit = " GB";
    }

    double time_val = 1.0 * t;
    std::string time_unit = " sec";
    if (time_val < 0.01) {
        time_val = time_val * 1000.0;
        time_unit = "  ms";
    } else if (time_val > 60.0) {
        time_val = time_val / 60.0;
        time_unit = " min";
    }

    std::stringstream o;
    o << " " << txt;
    o << std::string(n_spaces, ' ');
    o << std::setw(val_width - 4) << n << node_unit;
    o << std::setw(val_width - 3) << std::setprecision(2) << std::fixed << mem_val << mem_unit;
    o << std::setw(val_width - 4) << std::setprecision(2) << std::fixed << time_val << time_unit;
    println(level, o.str());
}

template <int D> void print::tree(int level, const std::string &txt, const MWTree<D> &tree, const Timer &timer) {
    if (level > Printer::getPrintLevel()) return;

    auto n = tree.getNNodes();
    auto m = tree.getSizeNodes();
    auto t = timer.elapsed();
    print::tree(level, txt, n, m, t);
}

void print::time(int level, const std::string &txt, const Timer &timer) {
    if (level > Printer::getPrintLevel()) return;

    auto line_width = Printer::getWidth() - 2;
    auto txt_width = line_width / 2;
    auto unit_width = txt_width / 3;
    auto val_width = line_width - (txt_width + unit_width);
    auto n_spaces = std::max(0, txt_width - static_cast<int>(txt.size()));

    std::stringstream o;
    o << " " << txt;
    o << std::string(n_spaces, ' ');
    o << std::setw(unit_width) << "(sec)";
    o << std::setw(val_width) << std::setprecision(5) << std::scientific << timer.elapsed();
    println(level, o.str());
}

/** Prints the current memory usage of this process
 */
void print::memory(int level, const std::string &txt) {
    if (level > Printer::getPrintLevel()) return;

    auto mem_val = static_cast<double>(details::get_memory_usage());
    std::string mem_unit = "(kB)";
    if (mem_val > 512.0) {
        mem_val /= 1024.0;
        mem_unit = "(MB)";
    }
    if (mem_val > 512.0) {
        mem_val /= 1024.0;
        mem_unit = "(GB)";
    }

    auto line_width = Printer::getWidth() - 2;
    auto txt_width = line_width / 2;
    auto unit_width = txt_width / 3;
    auto val_width = line_width - (txt_width + unit_width);
    auto n_spaces = std::max(0, txt_width - static_cast<int>(txt.size()));

    std::stringstream o;
    o << " " << txt;
    o << std::string(n_spaces, ' ');
    o << std::setw(unit_width) << mem_unit;
    o << std::setw(val_width) << std::setprecision(2) << std::fixed << mem_val;
    println(level, o.str());
}

template void print::tree<1>(int level, const std::string &txt, const MWTree<1> &tree, const Timer &timer);
template void print::tree<2>(int level, const std::string &txt, const MWTree<2> &tree, const Timer &timer);
template void print::tree<3>(int level, const std::string &txt, const MWTree<3> &tree, const Timer &timer);

} // namespace mrcpp
