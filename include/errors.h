/*!
 * miledger.
 * errors.h
 *
 * \date 2021
 * \author Eduard Maximovich (edward.vstock@gmail.com)
 * \link   https://github.com/edwardstock
 */

#ifndef MILEDGER_ERRORS_H
#define MILEDGER_ERRORS_H

#include <stdexcept>

struct image_already_in_work_error : public std::exception {
    const char* what() const _NOEXCEPT override {
        return "Image already in work";
    }
};

#endif // MILEDGER_ERRORS_H
