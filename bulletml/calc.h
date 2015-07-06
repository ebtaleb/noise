#ifndef CALC_H_
#define CALC_H_

#include "formula.h"

#include <string>
#include <memory>

 std::auto_ptr<Formula<double> > calc(const std::string& str);

#endif // CALC_H_

