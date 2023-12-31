cmake_minimum_required(VERSION 3.5 FATAL_ERROR)
set(name GGeoTest)
project(${name} VERSION 0.1.0)
include(OpticksBuildOptions)   

set(TEST_SOURCES

    GConstantTest.cc
    GDomainTest.cc
    GAryTest.cc

    GBufferTest.cc
    GVectorTest.cc

    GPropertyMapEditTest.cc
    GPropertyMapBaseTest.cc
    GPropertyBaseTest.cc

    GItemListTest.cc
    GMaterialLibTest.cc

    GSkinSurfaceTest.cc
    GBorderSurfaceTest.cc
    GScintillatorLibTest.cc
    GSourceTest.cc
    GSourceLibTest.cc
    GBndLibTest.cc
    GBndLibInitTest.cc

    GOpticalSurfaceTest.cc 

    GMeshTest.cc
    GNodeTest.cc
    GVolumeTest.cc
    GCIETest.cc
    GArrayTest.cc
    GTransformsTest.cc
    GIdsTest.cc
    GItemIndexTest.cc
    GItemIndex2Test.cc
    GTreePresentTest.cc
    GCSGTest.cc
    GPartsTest.cc
    GPartsCreateTest.cc
    GPtsTest.cc
    GPtTest.cc
    GGeoNCSGTest.cc

    GGeoLoadFromDirTest.cc 


    deferredCreateGPartsTest.cc

    BoundariesNPYTest.cc
    GAttrSeqTest.cc
    GBBoxMeshTest.cc

    GColorsTest.cc
    GFlagsTest.cc
    GGeoLibTest.cc

    GGeoTest.cc
    GGeoIdentityTest.cc
    GGeoConvertTest.cc
    GGeoTestTest.cc

    GMakerTest.cc

    GMergedMeshTest.cc
    GMatrixTest.cc
    GMaterialTest.cc

    GPropertyDebugTest.cc
    GPropertyMapTest.cc
    GPropertyMap_make_table_Test.cc 

    GPropertyTest.cc
    GSurfaceLibTest.cc
    GTestBoxTest.cc

    RecordsNPYTest.cc

    GMeshLibTest.cc
    GNodeLibTest.cc
    GPhoTest.cc

    GGeoDumpTest.cc
    GLogTest.cc
)


set(FORMER_TEST_SOURCES

    GSceneTest.cc
    NLookupTest.cc   // TODO: reinstate for new resource handling 
    GPmtTest.cc

)



set(_TEST_SOURCES

    GSurfaceLibTest.cc
    GScintillatorLibTest.cc
    GMaterialLibTest.cc
    GPropertyMapTest.cc
    GPropertyTest.cc

)





foreach(TEST_CC_SRC ${TEST_SOURCES})
    get_filename_component(TGT ${TEST_CC_SRC} NAME_WE)
    add_executable(${TGT} ${TEST_CC_SRC})
    add_test(${name}.${TGT} ${TGT})
    #add_dependencies(check ${TGT})
    target_link_libraries(${TGT} GGeo)
    install(TARGETS ${TGT} DESTINATION lib)
endforeach()


# promote some python scripts to the standard binary location
set(PY_TEST_SOURCES

   GBndLibTest.py

)
install(FILES ${PY_TEST_SOURCES} DESTINATION lib 
        PERMISSIONS 
            WORLD_READ GROUP_READ OWNER_READ 
            WORLD_WRITE GROUP_WRITE OWNER_WRITE 
            WORLD_EXECUTE GROUP_EXECUTE OWNER_EXECUTE 
 )



