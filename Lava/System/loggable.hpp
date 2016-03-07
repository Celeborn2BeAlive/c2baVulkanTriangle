#ifndef LOGGABLE_H
#define LOGGABLE_H

#include <iostream>

class Loggable
{
public:
    Loggable(std::ostream &stream = std::cout);

protected:
    std::ostream &mStream;
};

#endif // LOGGABLE_H
