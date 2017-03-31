#include "NGLM.hpp"
#include <iostream>
#include <iomanip>

/*
inline std::ostream& operator<< (std::ostream& out, const float& x) 
{
    out << " " << std::fixed << std::setprecision(2) << std::setw(7) << x  ;
    return out;
}
*/

inline std::ostream& operator<< (std::ostream& out, const glm::ivec3& v) 
{
    out << "{" 
        << " " << std::setw(4) << v.x 
        << " " << std::setw(4) << v.y 
        << " " << std::setw(4) << v.z
        << "}";
    return out;
}

inline std::ostream& operator<< (std::ostream& out, const glm::vec3& v) 
{
    out << "{" 
        << " " << std::fixed << std::setprecision(2) << std::setw(7) << v.x 
        << " " << std::fixed << std::setprecision(2) << std::setw(7) << v.y
        << " " << std::fixed << std::setprecision(2) << std::setw(7) << v.z 
        << "}";

    return out;
}

inline std::ostream& operator<< (std::ostream& out, const glm::vec4& v) 
{
    out << "{" 
        << " " << std::setprecision(2) << std::setw(7) << v.x 
        << " " << std::setprecision(2) << std::setw(7) << v.y
        << " " << std::setprecision(2) << std::setw(7) << v.z 
        << " " << std::setprecision(2) << std::setw(7) << v.w 
        << "}";
    return out;
}


