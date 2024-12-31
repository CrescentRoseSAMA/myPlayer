set(FFMPEG_SOURCE /home/ruby/ffmpeg_loc)

set(FFMPEG_INCLUDE_DIR ${FFMPEG_SOURCE}/include)
set(FFMPEG_LIBDIRS_DIR ${FFMPEG_SOURCE}/lib)

find_library(FFMPEG_AVCODEC_LIBRARY avcodec ${FFMPEG_LIBDIRS_DIR})
find_library(FFMPEG_AVFORMAT_LIBRARY avformat ${FFMPEG_LIBDIRS_DIR})
find_library(FFMPEG_AVUTIL_LIBRARY avutil ${FFMPEG_LIBDIRS_DIR})
find_library(FFMPEG_SWSCALE_LIBRARY swscale ${FFMPEG_LIBDIRS_DIR})

set(FFMPEG_LIBS ${FFMPEG_AVCODEC_LIBRARY} ${FFMPEG_AVFORMAT_LIBRARY} ${FFMPEG_AVUTIL_LIBRARY} ${FFMPEG_SWSCALE_LIBRARY})