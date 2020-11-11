project(ElastixTest)

cmake_minimum_required(VERSION 3.10)

SET(ElastixTest_SRCS
  main.cpp
)

# Find ITK
FIND_PACKAGE( ITK REQUIRED )
IF( ITK_FOUND )
  INCLUDE( ${ITK_USE_FILE} )
ENDIF( ITK_FOUND )

# Find Elastix
FIND_PACKAGE( Elastix REQUIRED )
IF( Elastix_FOUND )
  include( ${ELASTIX_USE_FILE} )
ENDIF( Elastix_FOUND )

ADD_EXECUTABLE(ElastixProxyApp ${ElastixProxyApp_SRCS})

TARGET_LINK_LIBRARIES(ElastixProxyApp
   elastix elxCommon elxCore param ${ITK_LIBRARIES} 
)
