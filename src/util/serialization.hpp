//
// Created by fotyev on 2017-07-26.
//

#ifndef ZENGINE_SERIALIZATION_HPP
#define ZENGINE_SERIALIZATION_HPP

#include <experimental/string_view>

using std::experimental::string_view;

#include <boost/preprocessor/seq/for_each.hpp>


// SERIALIZABLE((field1) (field2))
// template<class Archive> void serialize(Archive& ar) { ar&field1&field2; }

#define SERIALIZABLE(seq) \
template<class Archive> void serialize(Archive& ar) { (void)(ar \
BOOST_PP_SEQ_FOR_EACH(SERIALIZABLE_HELPER, ar, seq) \
); }

#ifdef DEBUG_SERIALIZATION
#define SERIALIZABLE_HELPER(r, ar, elem) & CEREAL_NVP(elem)
#else
#define SERIALIZABLE_HELPER(r, ar, elem) & elem
#endif

#define SERIALIZABLE2(...) \
template<class Archive> void serialize(Archive& ar) { (void)ar(__VA_ARGS__); }


//#define SERIALIZABLE(...) template<class Archive> void serialize(Archive& ar) { ar(__VA_ARGS__); }

#ifdef DEBUG_SERIALIZATION
#include <cereal/archives/xml.hpp>
#define ARCHIVE Xml
#else
#include <cereal/archives/portable_binary.hpp>
#define ARCHIVE PortableBinary
#endif

#include <boost/preprocessor/cat.hpp>

#include <sstream>

template <typename... Args>
inline std::string cserialize(Args&&... args)
{
    std::ostringstream ss;
    using namespace cereal;
    BOOST_PP_CAT(ARCHIVE, OutputArchive) ar(ss);
    ar(std::forward<Args>(args)...);
    return ss.str();
}



class stream_view : public std::streambuf
{
public:
    stream_view(char *data, size_t size)
    {
        this->setg(data, data, data + size);
    }
    stream_view(string_view data)
    {
        this->setg(const_cast<char *>(data.data()),
                   const_cast<char *>(data.data()),
                   const_cast<char *>(data.data() + data.size()));
    }
};

template <typename... Args>
void deserialize(string_view data, Args&... args)
{
    stream_view sv(data);
    std::istream s(&sv);
    using namespace cereal;

    BOOST_PP_CAT(ARCHIVE, InputArchive) ar(s);
    ar(args...);
}


#endif //ZENGINE_SERIALIZATION_HPP
