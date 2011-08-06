#ifndef CWQTSERIALIZATION_H
#define CWQTSERIALIZATION_H


//Qt includes
#include <QList>
#include <QVector3D>
#include <QString>
#include <QDate>

//Boost xml includes
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/serialization/split_free.hpp>
#include <boost/serialization/collection_traits.hpp>
#include <boost/serialization/collections_load_imp.hpp>
#include <boost/serialization/collections_save_imp.hpp>

BOOST_SERIALIZATION_COLLECTION_TRAITS(QList)

BOOST_SERIALIZATION_SPLIT_FREE(QString)
BOOST_SERIALIZATION_SPLIT_FREE(QVector3D)
BOOST_SERIALIZATION_SPLIT_FREE(QDate)

namespace boost {
namespace  serialization {


///////////////////////////// QVector3D ///////////////////////////////
template<class Archive>
void save(Archive &archive, const QVector3D &vector, const unsigned int) {

    float x = vector.x();
    float y = vector.y();
    float z = vector.z();

    archive << BOOST_SERIALIZATION_NVP(x);
    archive << BOOST_SERIALIZATION_NVP(y);
    archive << BOOST_SERIALIZATION_NVP(z);
}

template<class Archive>
void load(Archive &archive, QVector3D &vector, const unsigned int) {

    float x;
    float y;
    float z;

    archive >> BOOST_SERIALIZATION_NVP(x);
    archive >> BOOST_SERIALIZATION_NVP(y);
    archive >> BOOST_SERIALIZATION_NVP(z);

    vector = QVector3D(x, y, z);
}

//////////////////////////////// QString /////////////////////////////////////////////
/**
  Save QString
  */
template<class Archive>
void save(Archive &archive, const QString &str, const unsigned int) {
    std::wstring string = str.toStdWString();
    archive << BOOST_SERIALIZATION_NVP( string);
}

/**
 Load a QString
  */
template<class Archive>
void load(Archive &archive, QString &str, const unsigned int) {
    std::wstring string;
    archive >> BOOST_SERIALIZATION_NVP(string);
    str = QString::fromStdWString(string);
}

/////////////////////////////// QList //////////////////////////////////////

//---------------------------------------------------------------------------
/// Saves a QList object to a collection
template<class Archive, class U >
inline void save(Archive &ar, const QList< U > &t, const uint /* file_version */ )
{
    boost::serialization::stl::save_collection< Archive, QList<U> >(ar, t);
}

//---------------------------------------------------------------------------
/// Loads a QList object from a collection
template<class Archive, class U>
inline void load(Archive &ar, QList<U > &t, const uint /* file_version */ )
{
        boost::serialization::stl::load_collection<
            Archive,
            QList<U>,
            boost::serialization::stl::archive_input_seq<Archive, QList<U> >,
            boost::serialization::stl::no_reserve_imp< QList<U> > >(ar, t);
}

//---------------------------------------------------------------------------
/// split non-intrusive serialization function member into separate
/// non intrusive save/load member functions
template<class Archive, class U >
inline void serialize(Archive &ar, QList<U> &t, const uint file_version )
{
    boost::serialization::split_free( ar, t, file_version);
}



////////////////////////////// QDate ////////////////////////////////////
template<class Archive>
void save(Archive &archive, const QDate &date, const unsigned int) {
    int year = date.year();
    int month = date.month();
    int day = date.day();

    archive << BOOST_SERIALIZATION_NVP(year);
    archive << BOOST_SERIALIZATION_NVP(month);
    archive << BOOST_SERIALIZATION_NVP(day);
}

template<class Archive>
void load(Archive &archive, QDate &date, const unsigned int) {
    int year;
    int month;
    int day;

    archive >> BOOST_SERIALIZATION_NVP(year);
    archive >> BOOST_SERIALIZATION_NVP(month);
    archive >> BOOST_SERIALIZATION_NVP(day);

    date = QDate(year, month, day);
}
}
}
#endif
