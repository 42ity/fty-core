#include <cstdlib>
#include <cstring>
#include <iostream>
#include <fstream>
#include <algorithm>

#include "db/inout.h"
#include "log.h"
#include "csv.h"

static void
s_usage()
{
    std::cerr << "Usage: bios-cli [export|is-subset-of]" << std::endl;
    std::cerr << "       export     export csv file from current DB" << std::endl;
    std::cerr << "       is-subset-of   file1 file2  return if file1 is subset of file2" << std::endl;
}

static void
s_die_usage()
{
    s_usage();
    exit(EXIT_FAILURE);
}

static bool
s_is_subset_of(
        const char* file1,
        const char* file2)
{
    std::ifstream   sfile1{file1};
    std::ifstream   sfile2{file2};

    shared::CsvMap c1 = shared::CsvMap_from_istream(sfile1);
    shared::CsvMap c2 = shared::CsvMap_from_istream(sfile2);

    // 0. subset can't have more columns
    if (c1.cols() > c2.cols()) {
        log_error("different number of columns, %s: %zu != %s: %zu", file1, c1.cols(), file2, c2.cols());
        return false;
    }

    // 1. number of rows must be the same
    if (c1.rows() != c2.rows()) {
        log_error("different number of rows, %s: %zu != %s: %zu", file1, c1.rows(), file2, c2.rows());
        return false;
    }

    auto t1 = c1.getTitles();
    auto t2 = c2.getTitles();

    // 2. if headers are not subset, fail
    // here we need to change the order to match libstdc++
    if (!std::includes(
        t2.cbegin(), t2.cend(),
        t1.cbegin(), t1.cend())) {
        log_error("name of columns of %s is not subset of %s", file1, file2);
        return false;
    }

    // 3. for each line and each title, check the fields
    for (size_t line = 1; line != t1.size(); line++) {
        for (const std::string& title: t1) {
            if (c1.get(line, title) != c2.get(line, title)) {
                log_error("%s[%zu][%s] = %s != %s[%zu][%s] = %s",
                        file1, line, title.c_str(), c1.get(line, title).c_str(),
                        file2, line, title.c_str(), c2.get(line, title).c_str()
                        );
                return false;
            }
        }
    }

    log_info("'%s' is-subset-of '%s'", file1, file2);
    return true;
}

int main(int argc, char** argv)
{
    if (argc <= 1)
        s_die_usage();

    if (!strcmp(argv[1], "export"))
    {
        log_set_level(LOG_WARNING); //to suppress messages from src/db
        persist::export_asset_csv(std::cout);
    }
    else
    if (!strcmp(argv[1], "is-subset-of"))
    {
        log_set_level(LOG_INFO);
        if (argc < 4)
            s_die_usage();

        const char* file1 = argv[2];
        const char* file2 = argv[3];
        if (!s_is_subset_of(file1, file2))
            exit(EXIT_FAILURE);
    }
    else
    {
        log_error("Unknown command '%s'", argv[1]);
        exit(EXIT_FAILURE);
    }
    exit(EXIT_SUCCESS);

}
