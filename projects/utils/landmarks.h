#pragma once
#include "../common.h"
#include <map>

NAMESPACE_BEGIN
#define VECSTR std::vector<std::string>
#define VECPTS std::vector<snow::float2>

class Landmarks {
protected:
    static int                    gNumPoints;
    static VECSTR                 gGenreList;
    std::map<std::string, VECPTS> mGenreMap;
public:
    static void   SetNumPoints(int num)                { gNumPoints = num;    }
    static void   SetGenreList(const VECSTR &genres)   { gGenreList = genres; }
    static int    NumPoints()                          { return gNumPoints;   }
    static VECSTR GenreList()                          { return gGenreList;   }
    static bool   ValidGenre(const std::string &genre) { return std::find(gGenreList.begin(), gGenreList.end(), genre) != gGenreList.end(); }

    Landmarks();
    Landmarks(const Landmarks &other);
    virtual ~Landmarks();

    bool            isNull() const { return (mGenreMap.size() == 0); }
    bool            hasGenre(const std::string &genre) const { return (mGenreMap.find(genre) != mGenreMap.end()); }
    void            setLandmarks(const std::string &genre);
    void            setLandmarks(const std::string &genre, const VECPTS &ptsList);
    VECPTS &        landmarks(const std::string &genre);
    const VECPTS &  landmarks(const std::string &genre) const;

    friend std::ostream& operator<<(std::ostream &out, const Landmarks &lms);
    friend std::istream& operator>>(std::istream &inp, Landmarks &lms);
};

std::ostream& operator<<(std::ostream &out, const Landmarks &lms);
std::istream& operator>>(std::istream &inp, Landmarks &lms);


#undef VECPTS
#undef VECSTR
NAMESPACE_END
