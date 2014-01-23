#include "unit_tpl_all.hpp"

#define IT_OFF(it,N) {for (size_t i = 0; i < N; ++i) ++it;}

namespace D = das::tpl;

template<typename T>
void
save_image(shared_ptr<T>& ptr, shared_ptr<D::Database>& db) {
    BOOST_REQUIRE_NO_THROW(
            D::Transaction t = db->begin();
            db->attach(ptr);
            t.commit();
            );
}

template<typename T>
void image_2D_test_(shared_ptr<D::Database> db, const std::string path = "") {

    shared_ptr<test_image2d> ptr;
    BOOST_REQUIRE_NO_THROW(ptr = test_image2d::create("image2d_unit_test", "test_level2"));
    long long id;


    das::Array<T, 2> tiles_0(das::shape(3, 4));
    tiles_0(0, 0) = 0;
    tiles_0(0, 1) = 1;
    tiles_0(0, 2) = 2;
    tiles_0(0, 3) = 3;
    tiles_0(1, 0) = 10;
    tiles_0(1, 1) = 11;
    tiles_0(1, 2) = 12;
    tiles_0(1, 3) = 13;
    tiles_0(2, 0) = 20;
    tiles_0(2, 1) = 21;
    tiles_0(2, 2) = 22;
    tiles_0(2, 3) = 23;

    das::Array<T, 1> tile_0(das::shape(4));
    tile_0(0) = 30;
    tile_0(1) = 31;
    tile_0(2) = 32;
    tile_0(3) = 33;

    das::Array<T, 2> tiles_1(das::shape(2, 4));
    tiles_1(0, 0) = 40;
    tiles_1(0, 1) = 41;
    tiles_1(0, 2) = 42;
    tiles_1(0, 3) = 43;
    tiles_1(1, 0) = 50;
    tiles_1(1, 1) = 51;
    tiles_1(1, 2) = 52;
    tiles_1(1, 3) = 53;

    das::Array<T, 1> tile_1(das::shape(4));
    tile_1(0) = 60;
    tile_1(1) = 61;
    tile_1(2) = 62;
    tile_1(3) = 63;

    das::Array<T, 2> img_s(das::shape(4, 4));
    img_s(0, 0) = 100;
    img_s(0, 1) = 101;
    img_s(0, 2) = 102;
    img_s(0, 3) = 103;
    img_s(1, 0) = 110;
    img_s(1, 1) = 111;
    img_s(1, 2) = 112;
    img_s(1, 3) = 113;
    img_s(2, 0) = 120;
    img_s(2, 1) = 121;
    img_s(2, 2) = 122;
    img_s(2, 3) = 123;
    img_s(3, 0) = 130;
    img_s(3, 1) = 131;
    img_s(3, 2) = 132;
    img_s(3, 3) = 133;


    das::Array<T, 2> tiles_E0(das::shape(3, 5));
    das::Array<T, 1> tile_E0(das::shape(5));
    das::Array<T, 2> tiles_E1(das::shape(3, 3));
    das::Array<T, 1> tile_E1(das::shape(3));

    BOOST_CHECK_THROW((ptr->get_image<T, 2>()), das::empty_image);

    BOOST_REQUIRE_NO_THROW(ptr->append_tiles(tiles_0));

    das::Array<T, 2> i_0(ptr->get_image<T, 2>());
    BOOST_CHECK_EQUAL_COLLECTIONS(i_0.begin(), i_0.end(), tiles_0.begin(), tiles_0.end());

    das::Array<T, 2> p_0(ptr->get_image<T, 2>(das::Range(0, 3, 2), das::Range(1, 4, 2)));
    BOOST_CHECK_EQUAL(p_0(0, 0), tiles_0(0, 1));
    BOOST_CHECK_EQUAL(p_0(0, 1), tiles_0(0, 3));
    BOOST_CHECK_EQUAL(p_0(1, 0), tiles_0(2, 1));
    BOOST_CHECK_EQUAL(p_0(1, 1), tiles_0(2, 3));


    BOOST_REQUIRE_NO_THROW(
            D::Transaction t = db->begin();
            db->persist(ptr, path);
            t.commit();
            );
    id = ptr->das_id();
    BOOST_REQUIRE_NE(id, 0);

    BOOST_CHECK_EQUAL(ptr->get_image_extent(0), 3);
    BOOST_CHECK_EQUAL(ptr->get_image_extent(1), 4);

    save_image(ptr, db);

    BOOST_CHECK_EQUAL(ptr->get_image_extent(0), 3);
    BOOST_CHECK_EQUAL(ptr->get_image_extent(1), 4);

    das::Array<T, 2> i_0b(ptr->get_image<T, 2>());
    BOOST_CHECK_EQUAL_COLLECTIONS(i_0b.begin(), i_0b.end(), tiles_0.begin(), tiles_0.end());

    BOOST_REQUIRE_NO_THROW(ptr->append_tiles(tile_0));
    das::Array<T, 2> i_1(ptr->get_image<T, 2>());

    typename das::Array<T, 2>::iterator i_1_ = i_1.begin();
    IT_OFF(i_1_, 12)
    BOOST_CHECK_EQUAL_COLLECTIONS(i_1.begin(), i_1_, tiles_0.begin(), tiles_0.end());
    BOOST_CHECK_EQUAL_COLLECTIONS(i_1_, i_1.end(), tile_0.begin(), tile_0.end());

    BOOST_CHECK_EQUAL(ptr->get_image_extent(0), 4);
    BOOST_CHECK_EQUAL(ptr->get_image_extent(1), 4);

    save_image(ptr, db);

    BOOST_CHECK_EQUAL(ptr->get_image_extent(0), 4);
    BOOST_CHECK_EQUAL(ptr->get_image_extent(1), 4);

    BOOST_REQUIRE_NO_THROW(ptr->append_tiles(tiles_1));

    BOOST_REQUIRE_NO_THROW(ptr->append_tiles(tile_1));

    BOOST_REQUIRE_NO_THROW(ptr->append_tiles(tile_1));

    das::Array<T, 2> p_1(ptr->get_image<T, 2>(das::Range(1, 8, 2), das::Range(1, 4, 2)));
    BOOST_CHECK_EQUAL(p_1(0, 0), tiles_0(1, 1));
    BOOST_CHECK_EQUAL(p_1(0, 1), tiles_0(1, 3));
    BOOST_CHECK_EQUAL(p_1(1, 0), tile_0(1));
    BOOST_CHECK_EQUAL(p_1(1, 1), tile_0(3));
    BOOST_CHECK_EQUAL(p_1(2, 0), tiles_1(1, 1));
    BOOST_CHECK_EQUAL(p_1(2, 1), tiles_1(1, 3));
    BOOST_CHECK_EQUAL(p_1(3, 0), tile_1(1));
    BOOST_CHECK_EQUAL(p_1(3, 1), tile_1(3));

    BOOST_CHECK_EQUAL(ptr->get_image_extent(0), 8);
    BOOST_CHECK_EQUAL(ptr->get_image_extent(1), 4);

    save_image(ptr, db);

    BOOST_CHECK_EQUAL(ptr->get_image_extent(0), 8);
    BOOST_CHECK_EQUAL(ptr->get_image_extent(1), 4);

    BOOST_CHECK_THROW(ptr->append_tiles(tiles_E0), das::incompatible_array_shape);
    BOOST_CHECK_THROW(ptr->append_tiles(tile_E0), das::incompatible_array_shape);
    BOOST_CHECK_THROW(ptr->append_tiles(tiles_E1), das::incompatible_array_shape);
    BOOST_CHECK_THROW(ptr->append_tiles(tile_E1), das::incompatible_array_shape);

    das::Array<T, 2> p_2(ptr->get_image<T, 2>(das::Range(1, 8, 2), das::Range(1, 4, 2)));
    BOOST_CHECK_EQUAL(p_2(0, 0), tiles_0(1, 1));
    BOOST_CHECK_EQUAL(p_2(0, 1), tiles_0(1, 3));
    BOOST_CHECK_EQUAL(p_2(1, 0), tile_0(1));
    BOOST_CHECK_EQUAL(p_2(1, 1), tile_0(3));
    BOOST_CHECK_EQUAL(p_2(2, 0), tiles_1(1, 1));
    BOOST_CHECK_EQUAL(p_2(2, 1), tiles_1(1, 3));
    BOOST_CHECK_EQUAL(p_2(3, 0), tile_1(1));
    BOOST_CHECK_EQUAL(p_2(3, 1), tile_1(3));

    ptr.reset();

    BOOST_REQUIRE_NO_THROW(
            D::Transaction t = db->begin();
            ptr = db->load<test_image2d>(id);
            t.commit();
            );

    das::Array<T, 2> p_3(ptr->get_image<T, 2>(das::Range(1, 8, 2), das::Range(1, 4, 2)));
    BOOST_CHECK_EQUAL(p_3(0, 0), tiles_0(1, 1));
    BOOST_CHECK_EQUAL(p_3(0, 1), tiles_0(1, 3));
    BOOST_CHECK_EQUAL(p_3(1, 0), tile_0(1));
    BOOST_CHECK_EQUAL(p_3(1, 1), tile_0(3));
    BOOST_CHECK_EQUAL(p_3(2, 0), tiles_1(1, 1));
    BOOST_CHECK_EQUAL(p_3(2, 1), tiles_1(1, 3));
    BOOST_CHECK_EQUAL(p_3(3, 0), tile_1(1));
    BOOST_CHECK_EQUAL(p_3(3, 1), tile_1(3));

    das::Array<T, 2> i_2(ptr->get_image<T, 2>());

    typename das::Array<T, 2>::iterator i_2_1 = i_2.begin();
    IT_OFF(i_2_1, 12);

    typename das::Array<T, 2>::iterator i_2_2 = i_2_1;
    IT_OFF(i_2_2, 4);

    typename das::Array<T, 2>::iterator i_2_3 = i_2_2;
    IT_OFF(i_2_3, 8);

    typename das::Array<T, 2>::iterator i_2_4 = i_2_3;
    IT_OFF(i_2_4, 4);

    typename das::Array<T, 2>::iterator i_2_5 = i_2_4;
    IT_OFF(i_2_5, 4);

    BOOST_CHECK_EQUAL_COLLECTIONS(i_2.begin(), i_2_1, tiles_0.begin(), tiles_0.end());
    BOOST_CHECK_EQUAL_COLLECTIONS(i_2_1, i_2_2, tile_0.begin(), tile_0.end());
    BOOST_CHECK_EQUAL_COLLECTIONS(i_2_2, i_2_3, tiles_1.begin(), tiles_1.end());
    BOOST_CHECK_EQUAL_COLLECTIONS(i_2_3, i_2_4, tile_1.begin(), tile_1.end());
    BOOST_CHECK_EQUAL_COLLECTIONS(i_2_4, i_2.end(), tile_1.begin(), tile_1.end());

    BOOST_CHECK_EQUAL(ptr->get_image_extent(0), 8);
    BOOST_CHECK_EQUAL(ptr->get_image_extent(1), 4);

    BOOST_REQUIRE_NO_THROW(ptr->set_image(img_s));

    das::Array<T, 2> i_3(ptr->get_image<T, 2>());
    BOOST_CHECK_EQUAL_COLLECTIONS(i_3.begin(), i_3.end(), img_s.begin(), img_s.end());

    BOOST_CHECK_EQUAL(ptr->get_image_extent(0), 4);
    BOOST_CHECK_EQUAL(ptr->get_image_extent(1), 4);

    save_image(ptr, db);

    BOOST_CHECK_EQUAL(ptr->get_image_extent(0), 4);
    BOOST_CHECK_EQUAL(ptr->get_image_extent(1), 4);

    das::Array<T, 2> i_4(ptr->get_image<T, 2>());
    BOOST_CHECK_EQUAL_COLLECTIONS(i_4.begin(), i_4.end(), img_s.begin(), img_s.end());
}



template<typename T>
void image_3D_test_(shared_ptr<D::Database> db, const std::string path = "") {

    shared_ptr<test_image3d> ptr;
    BOOST_REQUIRE_NO_THROW(ptr = test_image3d::create("image3d_unit_test", "test_level2"));
    long long id;

    das::Array<T, 3> tiles_0(das::shape(3,4,2));
    tiles_0(0, 0, 0) =   0;
    tiles_0(0, 0, 1) =   1;
    tiles_0(0, 1, 0) =  10;
    tiles_0(0, 1, 1) =  12;
    tiles_0(0, 2, 0) =  20;
    tiles_0(0, 2, 1) =  21;
    tiles_0(0, 3, 0) =  30;
    tiles_0(0, 3, 1) =  31;
    tiles_0(1, 0, 0) = 100;
    tiles_0(1, 0, 1) = 101;
    tiles_0(1, 1, 0) = 110;
    tiles_0(1, 1, 1) = 111;
    tiles_0(1, 2, 0) = 120;
    tiles_0(1, 2, 1) = 121;
    tiles_0(1, 3, 0) = 130;
    tiles_0(1, 3, 1) = 131;
    tiles_0(2, 0, 0) = 200;
    tiles_0(2, 0, 1) = 201;
    tiles_0(2, 1, 0) = 210;
    tiles_0(2, 1, 1) = 211;
    tiles_0(2, 2, 0) = 220;
    tiles_0(2, 2, 1) = 221;
    tiles_0(2, 3, 0) = 230;
    tiles_0(2, 3, 1) = 231;   
    
    das::Array<T, 2> tile_0(das::shape(4,2));
    tile_0(0,0)      = 300;
    tile_0(0,1)      = 301;
    tile_0(1,0)      = 310;
    tile_0(1,1)      = 311;
    tile_0(2,0)      = 320;
    tile_0(2,1)      = 321;
    tile_0(3,0)      = 330;
    tile_0(3,1)      = 331;
    
    das::Array<T, 3> tiles_1(das::shape(2, 4 ,2));
    tiles_1(0, 0, 0) = 400;
    tiles_1(0, 0, 1) = 401;
    tiles_1(0, 1, 0) = 410;
    tiles_1(0, 1, 1) = 412;
    tiles_1(0, 2, 0) = 420;
    tiles_1(0, 2, 1) = 421;
    tiles_1(0, 3, 0) = 430;
    tiles_1(0, 3, 1) = 431;
    tiles_1(1, 0, 0) = 500;
    tiles_1(1, 0, 1) = 501;
    tiles_1(1, 1, 0) = 510;
    tiles_1(1, 1, 1) = 511;
    tiles_1(1, 2, 0) = 520;
    tiles_1(1, 2, 1) = 521;
    tiles_1(1, 3, 0) = 530;
    tiles_1(1, 3, 1) = 531;

    das::Array<T, 2> tile_1(das::shape(4,2));
    tile_1(0,0)      = 600;
    tile_1(0,1)      = 601;
    tile_1(1,0)      = 610;
    tile_1(1,1)      = 611;
    tile_1(2,0)      = 620;
    tile_1(2,1)      = 621;
    tile_1(3,0)      = 630;
    tile_1(3,1)      = 631;

    das::Array<T, 3> img_s(das::shape(2,2,2));
    img_s(0, 0, 0) = 1100;
    img_s(0, 0, 1) = 1101;
    img_s(0, 1, 0) = 1102;
    img_s(0, 1, 1) = 1103;
    img_s(1, 0, 0) = 1110;
    img_s(1, 0, 1) = 1111;
    img_s(1, 1, 0) = 1112;
    img_s(1, 1, 1) = 1113;



    das::Array<T, 3> tiles_E0(das::shape(3, 5, 2));
    das::Array<T, 2> tile_E0(das::shape(5,2));
    das::Array<T, 3> tiles_E1(das::shape(3, 3,2));
    das::Array<T, 2> tile_E1(das::shape(4,1));

    BOOST_CHECK_THROW((ptr->get_image<T, 3>()), das::empty_image);

    BOOST_REQUIRE_NO_THROW(ptr->append_tiles(tiles_0));

    das::Array<T, 3> i_0(ptr->get_image<T, 3>());
    BOOST_CHECK_EQUAL_COLLECTIONS(i_0.begin(), i_0.end(), tiles_0.begin(), tiles_0.end());

/*
    das::Array<T, 3> p_0(ptr->get_image<T, 3>(das::Range(0, 3, 2), das::Range(1, 4, 2)));
    BOOST_CHECK_EQUAL(p_0(0, 0), tiles_0(0, 1));
    BOOST_CHECK_EQUAL(p_0(0, 1), tiles_0(0, 3));
    BOOST_CHECK_EQUAL(p_0(1, 0), tiles_0(2, 1));
    BOOST_CHECK_EQUAL(p_0(1, 1), tiles_0(2, 3));
*/

    BOOST_REQUIRE_NO_THROW(
            D::Transaction t = db->begin();
            db->persist(ptr, path);
            t.commit();
            );
    id = ptr->das_id();
    BOOST_REQUIRE_NE(id, 0);

    BOOST_CHECK_EQUAL(ptr->get_image_extent(0), 3);
    BOOST_CHECK_EQUAL(ptr->get_image_extent(1), 4);
    BOOST_CHECK_EQUAL(ptr->get_image_extent(2), 2);

    save_image(ptr, db);

    BOOST_CHECK_EQUAL(ptr->get_image_extent(0), 3);
    BOOST_CHECK_EQUAL(ptr->get_image_extent(1), 4);
    BOOST_CHECK_EQUAL(ptr->get_image_extent(2), 2);

    das::Array<T, 3> i_0b(ptr->get_image<T, 3>());
    BOOST_CHECK_EQUAL_COLLECTIONS(i_0b.begin(), i_0b.end(), tiles_0.begin(), tiles_0.end());

    BOOST_REQUIRE_NO_THROW(ptr->append_tiles(tile_0));
    das::Array<T, 3> i_1(ptr->get_image<T, 3>());

    typename das::Array<T, 3>::iterator i_1_ = i_1.begin();
    IT_OFF(i_1_, 24)
    BOOST_CHECK_EQUAL_COLLECTIONS(i_1.begin(), i_1_, tiles_0.begin(), tiles_0.end());
    BOOST_CHECK_EQUAL_COLLECTIONS(i_1_, i_1.end(), tile_0.begin(), tile_0.end());

    BOOST_CHECK_EQUAL(ptr->get_image_extent(0), 4);
    BOOST_CHECK_EQUAL(ptr->get_image_extent(1), 4);
    BOOST_CHECK_EQUAL(ptr->get_image_extent(2), 2);

    save_image(ptr, db);

    BOOST_CHECK_EQUAL(ptr->get_image_extent(0), 4);
    BOOST_CHECK_EQUAL(ptr->get_image_extent(1), 4);
    BOOST_CHECK_EQUAL(ptr->get_image_extent(2), 2);

    BOOST_REQUIRE_NO_THROW(ptr->append_tiles(tiles_1));

    BOOST_REQUIRE_NO_THROW(ptr->append_tiles(tile_1));

    BOOST_REQUIRE_NO_THROW(ptr->append_tiles(tile_1));

/*
    das::Array<T, 3> p_1(ptr->get_image<T, 3>(das::Range(1, 8, 2), das::Range(1, 4, 2)));
    BOOST_CHECK_EQUAL(p_1(0, 0), tiles_0(1, 1));
    BOOST_CHECK_EQUAL(p_1(0, 1), tiles_0(1, 3));
    BOOST_CHECK_EQUAL(p_1(1, 0), tile_0(1));
    BOOST_CHECK_EQUAL(p_1(1, 1), tile_0(3));
    BOOST_CHECK_EQUAL(p_1(2, 0), tiles_1(1, 1));
    BOOST_CHECK_EQUAL(p_1(2, 1), tiles_1(1, 3));
    BOOST_CHECK_EQUAL(p_1(3, 0), tile_1(1));
    BOOST_CHECK_EQUAL(p_1(3, 1), tile_1(3));
*/
    BOOST_CHECK_EQUAL(ptr->get_image_extent(0), 8);
    BOOST_CHECK_EQUAL(ptr->get_image_extent(1), 4);
    BOOST_CHECK_EQUAL(ptr->get_image_extent(2), 2);

    save_image(ptr, db);

    BOOST_CHECK_EQUAL(ptr->get_image_extent(0), 8);
    BOOST_CHECK_EQUAL(ptr->get_image_extent(1), 4);
    BOOST_CHECK_EQUAL(ptr->get_image_extent(2), 2);

    BOOST_CHECK_THROW(ptr->append_tiles(tiles_E0), das::incompatible_array_shape);
    BOOST_CHECK_THROW(ptr->append_tiles(tile_E0), das::incompatible_array_shape);
    BOOST_CHECK_THROW(ptr->append_tiles(tiles_E1), das::incompatible_array_shape);
    BOOST_CHECK_THROW(ptr->append_tiles(tile_E1), das::incompatible_array_shape);
/*
    das::Array<T, 3> p_2(ptr->get_image<T, 3>(das::Range(1, 8, 2), das::Range(1, 4, 2)));
    BOOST_CHECK_EQUAL(p_2(0, 0), tiles_0(1, 1));
    BOOST_CHECK_EQUAL(p_2(0, 1), tiles_0(1, 3));
    BOOST_CHECK_EQUAL(p_2(1, 0), tile_0(1));
    BOOST_CHECK_EQUAL(p_2(1, 1), tile_0(3));
    BOOST_CHECK_EQUAL(p_2(2, 0), tiles_1(1, 1));
    BOOST_CHECK_EQUAL(p_2(2, 1), tiles_1(1, 3));
    BOOST_CHECK_EQUAL(p_2(3, 0), tile_1(1));
    BOOST_CHECK_EQUAL(p_2(3, 1), tile_1(3));
*/
    ptr.reset();

    BOOST_REQUIRE_NO_THROW(
            D::Transaction t = db->begin();
            ptr = db->load<test_image3d>(id);
            t.commit();
            );
/*
    das::Array<T, 3> p_3(ptr->get_image<T, 3>(das::Range(1, 8, 2), das::Range(1, 4, 2)));
    BOOST_CHECK_EQUAL(p_3(0, 0), tiles_0(1, 1));
    BOOST_CHECK_EQUAL(p_3(0, 1), tiles_0(1, 3));
    BOOST_CHECK_EQUAL(p_3(1, 0), tile_0(1));
    BOOST_CHECK_EQUAL(p_3(1, 1), tile_0(3));
    BOOST_CHECK_EQUAL(p_3(2, 0), tiles_1(1, 1));
    BOOST_CHECK_EQUAL(p_3(2, 1), tiles_1(1, 3));
    BOOST_CHECK_EQUAL(p_3(3, 0), tile_1(1));
    BOOST_CHECK_EQUAL(p_3(3, 1), tile_1(3));
*/
    das::Array<T, 3> i_2(ptr->get_image<T, 3>());

    typename das::Array<T, 3>::iterator i_2_1 = i_2.begin();
    IT_OFF(i_2_1, 24);

    typename das::Array<T, 3>::iterator i_2_2 = i_2_1;
    IT_OFF(i_2_2, 8);

    typename das::Array<T, 3>::iterator i_2_3 = i_2_2;
    IT_OFF(i_2_3, 16);

    typename das::Array<T, 3>::iterator i_2_4 = i_2_3;
    IT_OFF(i_2_4, 8);

    typename das::Array<T, 3>::iterator i_2_5 = i_2_4;
    IT_OFF(i_2_5, 8);

    BOOST_CHECK_EQUAL_COLLECTIONS(i_2.begin(), i_2_1, tiles_0.begin(), tiles_0.end());
    BOOST_CHECK_EQUAL_COLLECTIONS(i_2_1, i_2_2, tile_0.begin(), tile_0.end());
    BOOST_CHECK_EQUAL_COLLECTIONS(i_2_2, i_2_3, tiles_1.begin(), tiles_1.end());
    BOOST_CHECK_EQUAL_COLLECTIONS(i_2_3, i_2_4, tile_1.begin(), tile_1.end());
    BOOST_CHECK_EQUAL_COLLECTIONS(i_2_4, i_2.end(), tile_1.begin(), tile_1.end());

    BOOST_CHECK_EQUAL(ptr->get_image_extent(0), 8);
    BOOST_CHECK_EQUAL(ptr->get_image_extent(1), 4);
    BOOST_CHECK_EQUAL(ptr->get_image_extent(2), 2);
    
    BOOST_REQUIRE_NO_THROW(ptr->set_image(img_s));

    das::Array<T, 3> i_3(ptr->get_image<T, 3>());
    BOOST_CHECK_EQUAL_COLLECTIONS(i_3.begin(), i_3.end(), img_s.begin(), img_s.end());

    BOOST_CHECK_EQUAL(ptr->get_image_extent(0), 2);
    BOOST_CHECK_EQUAL(ptr->get_image_extent(1), 2);
    BOOST_CHECK_EQUAL(ptr->get_image_extent(2), 2);

    save_image(ptr, db);

    BOOST_CHECK_EQUAL(ptr->get_image_extent(0), 2);
    BOOST_CHECK_EQUAL(ptr->get_image_extent(1), 2);
    BOOST_CHECK_EQUAL(ptr->get_image_extent(2), 2);

    das::Array<T, 3> i_4(ptr->get_image<T, 3>());
    BOOST_CHECK_EQUAL_COLLECTIONS(i_4.begin(), i_4.end(), img_s.begin(), img_s.end());
}


BOOST_AUTO_TEST_SUITE(image_data_unit_tests)

BOOST_AUTO_TEST_CASE(image_2D) {
    shared_ptr<D::Database> db;
    BOOST_REQUIRE_NO_THROW(db = D::Database::create("test_level2"));

    das::DatabaseConfig::database("test_level2").buffered_data(true);

    image_2D_test_<double>(db);
    image_2D_test_<float>(db);

    image_2D_test_<double>(db, "dir_unit_test/2D/image_D");
    image_2D_test_<float> (db, "dir_unit_test/2D/image_F");


    das::DatabaseConfig::database("test_level2").buffered_data(false);

    image_2D_test_<double>(db);
    image_2D_test_<float>(db);

    image_2D_test_<double>(db, "dir_unit_test/2D/image_D");
    image_2D_test_<float> (db, "dir_unit_test/2D/image_F");
}

BOOST_AUTO_TEST_CASE(image_3D) {
    shared_ptr<D::Database> db;
    BOOST_REQUIRE_NO_THROW(db = D::Database::create("test_level2"));

    das::DatabaseConfig::database("test_level2").buffered_data(true);

    image_3D_test_<double>(db);
    image_3D_test_<float>(db);

    image_3D_test_<double>(db, "dir_unit_test/3D/image_D");
    image_3D_test_<float> (db, "dir_unit_test/3D/image_F");


    das::DatabaseConfig::database("test_level2").buffered_data(false);

    image_3D_test_<double>(db);
    image_3D_test_<float>(db);

    image_3D_test_<double>(db, "dir_unit_test/3D/image_D");
    image_3D_test_<float> (db, "dir_unit_test/3D/image_F");
}

BOOST_AUTO_TEST_SUITE_END()