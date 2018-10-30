#include "landmarks.h"
#include <limits>
#include <iomanip>

NAMESPACE_BEGIN
#define VECSTR std::vector<std::string>
#define VECPTS std::vector<snow::float2>

/* static members */
int    Landmarks::gNumPoints = 0;
VECSTR Landmarks::gGenreList = {};

/* constructor */
Landmarks::Landmarks() : mGenreMap() {}
Landmarks::Landmarks(const Landmarks &other) : mGenreMap(other.mGenreMap) {}
Landmarks::~Landmarks() { mGenreMap.clear(); }

/* methods */
void Landmarks::setLandmarks(const std::string &genre) {
    this->setLandmarks(genre, VECPTS(gNumPoints, {0, 0}));
}
void Landmarks::setLandmarks(const std::string &genre, const VECPTS &ptsList) {
    snow::assertion(ValidGenre(genre), "[Landmarks]: no such genre {}", genre);
    auto iter = mGenreMap.find(genre);
    if (iter == mGenreMap.end())
        mGenreMap.insert({genre, ptsList});
    else
        iter->second = ptsList;
}
VECPTS &Landmarks::landmarks(const std::string &genre) {
    auto iter = mGenreMap.find(genre);
    snow::assertion(iter != mGenreMap.end(), "[Landmarks]: not set genre {}", genre);
    return iter->second;
}
const VECPTS &Landmarks::landmarks(const std::string &genre) const {
    auto iter = mGenreMap.find(genre);
    snow::assertion(iter != mGenreMap.end(), "[Landmarks]: not set genre {}", genre);
    return iter->second;
}
void Landmarks::removeLandmarks(const std::string &genre) {
    auto iter = mGenreMap.find(genre);
    if (iter != mGenreMap.end()) mGenreMap.erase(iter);
}

/* stream */
std::ostream& operator<<(std::ostream &out, const Landmarks &lms) {
    out.precision(std::numeric_limits<float>::max_digits10);
    out << Landmarks::gNumPoints << std::endl;
    for (auto iter = lms.mGenreMap.begin(); iter != lms.mGenreMap.end(); ++iter) {
        out << "genre: " << iter->first;
        const auto &pts = iter->second;
        snow::assertion(pts.size() == Landmarks::gNumPoints,
                        "[Landamrks]: ostream << failed, due to size mismatch {} != {} (gNumPoints)",
                        pts.size(), Landmarks::gNumPoints);
        if (pts.size() == 0) out << std::endl;
        else 
            for (size_t i = 0; i < pts.size(); ++i) {
                if (i % 5 == 0) out << std::endl;
                out << std::setw(16) << pts[i].x << " "
                    << std::setw(16) << pts[i].y << " ";
            }
        out << std::endl;
    }
    // mark of end
    out << std::endl;
    return out;
}
std::istream& operator>>(std::istream &inp, Landmarks &lms) {
    int numPoints;
    std::string str, genre;
    inp >> numPoints; std::getline(inp, str);
    snow::assertion(numPoints == Landmarks::gNumPoints,
                    "[Landmarks]: istream >> failed, due to size mismatch {} (read) != {} (gNumPoints)",
                    numPoints, Landmarks::gNumPoints);
    while (true) {
        std::getline(inp, genre);
        genre = snow::Trim(genre);
        if (genre.length() == 0) break;
        genre = genre.substr(7);
        VECPTS pts(numPoints);
        for (size_t i = 0; i < pts.size(); ++i)
            inp >> pts[i].x >> pts[i].y;
        std::getline(inp, str);
        lms.setLandmarks(genre, pts);
    }
    return inp;
}

#undef VECPTS
#undef VECSTR
NAMESPACE_END