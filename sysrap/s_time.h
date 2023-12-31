#pragma once
/**
s_time.h 
==========================================================

This follows the approach of plog. 
Good for string time stamps, but not convenient for
measuring durations. See schrono.h or stimer.h for that. 

**/

#include <sys/time.h>
#include <string>
#include <iomanip>
#include <sstream>

namespace s_time
{
    inline int EpochSeconds()
    {
        time_t now = time(0);
        return now ; 
    }

    constexpr const char* FMT = "%Y%m%d_%H%M%S" ; 
 
    inline std::string Format(int epochseconds=0, const char* fmt=nullptr)
    {
        const char* ufmt = fmt == nullptr ? FMT : fmt ;  

        int t = epochseconds == 0 ? EpochSeconds() : epochseconds ; 
        time_t now(t) ;  
        struct tm  tstruct;
        char       buf[80];
        tstruct = *localtime(&now);
        strftime(buf, sizeof(buf), ufmt, &tstruct);
        return buf ;
    }

    inline std::string Stamp()
    {
        return Format(0, nullptr); 
    }
    inline std::string Now()
    {
        return Format(0, "%c"); 
    }


    inline void localtime_s(struct tm* t, const time_t* time)
    {
        ::localtime_r(time, t); 
    }

    struct Time
    {   
        time_t time;
        unsigned short millitm;
    };  

    inline void init(Time* stamp)  // ftime in plog
    {   
        timeval tv; 
        ::gettimeofday(&tv, NULL); // 2nd arg is obsolete timezone

        stamp->time = tv.tv_sec;
        stamp->millitm = static_cast<unsigned short>(tv.tv_usec / 1000);
    }   

    inline std::string Desc(Time* stamp)
    {
         tm t ; 
         localtime_s(&t, &stamp->time) ; 

         std::stringstream ss ; 
         ss 
            << t.tm_year + 1900  
            << "-"
            << std::setfill('0')
            << std::setw(2) << t.tm_mon + 1
            << "-"
            << std::setfill('0')
            << std::setw(2) << t.tm_mday 
            << " "
            << std::setfill('0')
            << std::setw(2) << t.tm_hour
            << ":"
            << std::setfill('0')
            << std::setw(2) << t.tm_min 
            << ":"
            << std::setfill('0')
            << std::setw(2) << t.tm_sec
            << "."
            << std::setfill('0')
            << std::setw(3) << static_cast<int> (stamp->millitm)
            << " "
            << std::endl 
            ; 
         std::string s = ss.str(); 
         return s ; 
     }
}; 



