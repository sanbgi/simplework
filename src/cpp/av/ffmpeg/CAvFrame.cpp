
#include "av_ffmpeg.h"
#include "CAvFrame.h"
#include "CAvOutStreaming.h"
#include "CAvSampleType.h"

#include <string>
#include <algorithm>

using namespace std;

FFMPEG_NAMESPACE_ENTER

static SCtx sCtx("CAvFrame");
int CAvFrame::createFrame(AVStream* pStreaming, int iStreamingId, AVFrame* pFrame, SAvFrame& spNewFrame){
    CPointer<CAvFrame> spFrame;
    CObject::createObject(spFrame);
    AVCodecParameters * pCodecCtx = pStreaming->codecpar;
    int ret = 0;
    switch (pCodecCtx->codec_type)
    {
    case AVMEDIA_TYPE_VIDEO:
        ret = spFrame->writeVideoFrame(pStreaming, iStreamingId, pFrame);
        break;
    
    case AVMEDIA_TYPE_AUDIO:
        ret = spFrame->writeAudioSampleFrame(pStreaming, iStreamingId, pFrame);
        break;

    default:
        return sCtx.error("未知的帧类型");
    }
    if( ret == sCtx.success()) {
        spNewFrame.setPtr(spFrame.getPtr());
    }
    return ret;
}

int CAvFrame::writeVideoFrame(AVStream* pStreaming, int iStreamingId, AVFrame* pFrame) {
    if( allocImageDataBuffer(m_spBuffer, 
                (AVPixelFormat)pFrame->format, pFrame->width, pFrame->height,
                m_pLinesizes, m_ppPlanes) != sCtx.success() ){
        return sCtx.error("分配帧内存异常");
    }

    const uint8_t **src_data = (const uint8_t **)pFrame->data;
    const int *src_linesize = pFrame->linesize;
    av_image_copy(
            m_ppPlanes, m_pLinesizes,
            src_data, src_linesize, 
            (AVPixelFormat)pFrame->format, pFrame->width, pFrame->height);

    //
    // 通过搜索linesize里面的值，来判断究竟有多少plane, 便于处理数据，这里面要注意，是否存在
    //  planar audio，并且通道数超过8？如果存在这种情况，则这里的数据是存在丢失的
    //
    int nPlanes = 0;
    for( int i=0; i<AV_NUM_DATA_POINTERS && pFrame->data[i]; i++ ) {
        nPlanes = i+1;
    }
    m_avFrame.sampleMeta.sampleType = EAvSampleType::AvSampleType_Video;
    m_avFrame.sampleMeta.sampleFormat = CAvSampleType::convert((AVPixelFormat)pFrame->format);
    m_avFrame.sampleMeta.videoWidth = pFrame->width;
    m_avFrame.sampleMeta.videoHeight = pFrame->height;
    m_avFrame.nWidth = pFrame->width;
    m_avFrame.nHeight = pFrame->height;
    m_avFrame.nPlanes = nPlanes;
    m_avFrame.ppPlanes = m_ppPlanes;
    m_avFrame.pPlaneLineSizes = m_pLinesizes;
    AVRational r = pStreaming->time_base;
    m_avFrame.timeRate = r.den/r.num;
    m_avFrame.timeStamp = pFrame->pts;
    m_avFrame.streamingId = iStreamingId;
    return sCtx.success();
}

int CAvFrame::writeAudioSampleFrame(AVStream* pStreaming, int iStreamingId, AVFrame* pFrame){
    if( allocAudioSampleDataBuffer(m_spBuffer, 
                (AVSampleFormat)pFrame->format, pFrame->channels, pFrame->nb_samples,
                m_pLinesizes, m_ppPlanes) != sCtx.success() ){
        return sCtx.error("分配帧内存异常");
    }

    av_samples_copy(
            m_ppPlanes, pFrame->extended_data, 0, 0, 
            pFrame->nb_samples, pFrame->channels, (AVSampleFormat)pFrame->format);

    //TODO: 如何计算？
    m_pLinesizes[0] = pFrame->nb_samples * pFrame->channels * av_get_bytes_per_sample((AVSampleFormat)pFrame->format);
    m_pLinesizes[1] = m_pLinesizes[0];

    //
    // 通过搜索linesize里面的值，来判断究竟有多少plane, 便于处理数据，这里面要注意，是否存在
    //  planar audio，并且通道数超过8？如果存在这种情况，则这里的数据是存在丢失的
    //
    int nPlanes = 0;
    for( int i=0; i<AV_NUM_DATA_POINTERS && pFrame->data[i]; i++ ) {
        nPlanes = i+1;
    }

    m_avFrame.sampleMeta.sampleType = EAvSampleType::AvSampleType_Audio;
    m_avFrame.sampleMeta.sampleFormat = CAvSampleType::convert((AVSampleFormat)pFrame->format);
    m_avFrame.sampleMeta.audioRate = pFrame->sample_rate;
    m_avFrame.sampleMeta.audioChannels = pFrame->channels;
    m_avFrame.nWidth = pFrame->nb_samples;
    m_avFrame.nHeight = 1;
    m_avFrame.nPlanes = nPlanes;
    m_avFrame.nPlanes = nPlanes;
    m_avFrame.ppPlanes = m_ppPlanes;
    m_avFrame.pPlaneLineSizes = m_pLinesizes;
    AVRational r = pStreaming->time_base;
    m_avFrame.timeRate = r.den/r.num;
    m_avFrame.timeStamp = pFrame->pts;
    m_avFrame.streamingId = iStreamingId;
    return sCtx.success();
}

const PAvFrame* CAvFrame::getFramePtr() {
    return &m_avFrame;
}

int CAvFrame::getDataBuffer(SDeviceMemory& spDataBuffer) {
    spDataBuffer = m_spBuffer;
    return sCtx.success();
}

//
// 从ffmpeg中拷贝过来的代码，目录在：lavfutils.c
//  修改内存分配部分代码
//
int ff_load_image(uint8_t *data[4], int linesize[4],
                  int *w, int *h, enum AVPixelFormat *pix_fmt,
                  const char *filename, void *log_ctx, SDeviceMemory& spDataBuffer)
{
    AVInputFormat *iformat = NULL;
    AVFormatContext *format_ctx = NULL;
    const AVCodec *codec;
    AVCodecContext *codec_ctx = NULL;
    AVCodecParameters *par;
    AVFrame *frame = NULL;
    int ret = 0;
    AVPacket pkt;
    AVDictionary *opt=NULL;

    iformat = av_find_input_format("image2pipe");
    if ((ret = avformat_open_input(&format_ctx, filename, iformat, NULL)) < 0) {
        av_log(log_ctx, AV_LOG_ERROR,
               "Failed to open input file '%s'\n", filename);
        return ret;
    }

    if ((ret = avformat_find_stream_info(format_ctx, NULL)) < 0) {
        av_log(log_ctx, AV_LOG_ERROR, "Find stream info failed\n");
        goto end;
    }

    par = format_ctx->streams[0]->codecpar;
    codec = avcodec_find_decoder(par->codec_id);
    if (!codec) {
        av_log(log_ctx, AV_LOG_ERROR, "Failed to find codec\n");
        ret = AVERROR(EINVAL);
        goto end;
    }

    codec_ctx = avcodec_alloc_context3(codec);
    if (!codec_ctx) {
        av_log(log_ctx, AV_LOG_ERROR, "Failed to alloc video decoder context\n");
        ret = AVERROR(ENOMEM);
        goto end;
    }

    ret = avcodec_parameters_to_context(codec_ctx, par);
    if (ret < 0) {
        av_log(log_ctx, AV_LOG_ERROR, "Failed to copy codec parameters to decoder context\n");
        goto end;
    }

    av_dict_set(&opt, "thread_type", "slice", 0);
    if ((ret = avcodec_open2(codec_ctx, codec, &opt)) < 0) {
        av_log(log_ctx, AV_LOG_ERROR, "Failed to open codec\n");
        goto end;
    }

    if (!(frame = av_frame_alloc()) ) {
        av_log(log_ctx, AV_LOG_ERROR, "Failed to alloc frame\n");
        ret = AVERROR(ENOMEM);
        goto end;
    }

    ret = av_read_frame(format_ctx, &pkt);
    if (ret < 0) {
        av_log(log_ctx, AV_LOG_ERROR, "Failed to read frame from file\n");
        goto end;
    }

    ret = avcodec_send_packet(codec_ctx, &pkt);
    av_packet_unref(&pkt);
    if (ret < 0) {
        av_log(log_ctx, AV_LOG_ERROR, "Error submitting a packet to decoder\n");
        goto end;
    }

    ret = avcodec_receive_frame(codec_ctx, frame);
    if (ret < 0) {
        av_log(log_ctx, AV_LOG_ERROR, "Failed to decode image from file\n");
        goto end;
    }

    *w       = frame->width;
    *h       = frame->height;
    *pix_fmt = (AVPixelFormat)frame->format;
    //if ((ret = av_image_alloc(data, linesize, *w, *h, *pix_fmt, 16)) < 0)
    //    goto end;
    //
    // 分配图片内存(修改拷贝过来的内核函数，替换为自己的内存分配方式)
    //
    if( CAvFrame::allocImageDataBuffer(spDataBuffer, 
                *pix_fmt, *w, *h,
                linesize, data) != sCtx.success() ){
        goto end;
    }
    ret = 0;

    av_image_copy(data, linesize, (const uint8_t **)frame->data, frame->linesize, *pix_fmt, *w, *h);

end:
    avcodec_free_context(&codec_ctx);
    avformat_close_input(&format_ctx);
    av_frame_free(&frame);
    av_dict_free(&opt);

    if (ret < 0)
        av_log(log_ctx, AV_LOG_ERROR, "Error loading image file '%s'\n", filename);
    return ret;
}

//#define ENABLE_JPEG_LIBRARY
#ifdef ENABLE_JPEG_LIBRARY 

static bool isJpegFile(const char* szFile) {
    if(szFile == nullptr) {
        return false;
    }

    string strFile = szFile;
    size_t it = strFile.find_last_of(".");
    if(it == string::npos) {
        return false;
    }

    string strExt = strFile.substr(it+1);
    transform(strExt.begin(),strExt.end(),strExt.begin(),::tolower);
    if( strExt.compare("jpg") == 0 ||
        strExt.compare("jpeg") == 0 ) {
        return true;
    }
    return false;
}


#include "jconfig.h"
#include "jpeglib.h"
#include <setjmp.h>

// 这些和错误处理有关，不用管
struct my_error_mgr {
    struct jpeg_error_mgr pub;  /* "public" fields */

    jmp_buf setjmp_buffer;  /* for return to caller */
};

typedef struct my_error_mgr * my_error_ptr;

METHODDEF(void) my_error_exit(j_common_ptr cinfo)
{
    my_error_ptr myerr = (my_error_ptr)cinfo->err;
    (*cinfo->err->output_message) (cinfo);
    longjmp(myerr->setjmp_buffer, 1);
}

/*
 * We call the libjpeg API from within a separate function, because modifying
 * the local non-volatile jpeg_decompress_struct instance below the setjmp()
 * return point and then accessing the instance after setjmp() returns would
 * return in undefined behavior that may potentially overwrite all or part of
 * the structure.
 */
static int read_JPEG_file(const char *filename, CAvFrame* pAvFrame)
{
  /* This struct contains the JPEG decompression parameters and pointers to
   * working space (which is allocated as needed by the JPEG library).
   */
  struct jpeg_decompress_struct cinfo;

  /* We use our private extension JPEG error handler.
   * Note that this struct must live as long as the main JPEG parameter
   * struct, to avoid dangling-pointer problems.
   */
  struct my_error_mgr jerr;
  /* More stuff */
  FILE *infile;                 /* source file */
  JSAMPARRAY buffer;            /* Output row buffer */
  int row_stride;               /* physical row width in output buffer */

  /* In this example we want to open the input file before doing anything else,
   * so that the setjmp() error recovery below can assume the file is open.
   * VERY IMPORTANT: use "b" option to fopen() if you are on a machine that
   * requires it in order to read binary files.
   */

  if ((infile = fopen(filename, "rb")) == NULL) {
    fprintf(stderr, "can't open %s\n", filename);
    return 0;
  }

  /* Step 1: allocate and initialize JPEG decompression object */

  /* We set up the normal JPEG error routines, then override error_exit. */
  cinfo.err = jpeg_std_error(&jerr.pub);
  jerr.pub.error_exit = my_error_exit;
  /* Establish the setjmp return context for my_error_exit to use. */
  if (setjmp(jerr.setjmp_buffer)) {
    /* If we get here, the JPEG code has signaled an error.
     * We need to clean up the JPEG object, close the input file, and return.
     */
    jpeg_destroy_decompress(&cinfo);
    fclose(infile);
    return 0;
  }
  /* Now we can initialize the JPEG decompression object. */
  jpeg_create_decompress(&cinfo);

  /* Step 2: specify data source (eg, a file) */

  jpeg_stdio_src(&cinfo, infile);

  /* Step 3: read file parameters with jpeg_read_header() */

  (void)jpeg_read_header(&cinfo, TRUE);
  /* We can ignore the return value from jpeg_read_header since
   *   (a) suspension is not possible with the stdio data source, and
   *   (b) we passed TRUE to reject a tables-only JPEG file as an error.
   * See libjpeg.txt for more info.
   */

    PAvFrame* pFrame = &pAvFrame->m_avFrame;
    pFrame->nHeight = cinfo.image_height;
    pFrame->nWidth = cinfo.image_width;
    pFrame->streamingId = 0;
    pFrame->timeRate = 0;
    pFrame->timeStamp = 0;
    pFrame->sampleMeta.sampleType = EAvSampleType::AvSampleType_Video;
    pFrame->sampleMeta.sampleFormat = EAvSampleFormat::AvSampleFormat_Video_RGB;
    pFrame->sampleMeta.videoHeight = cinfo.image_height;
    pFrame->sampleMeta.videoWidth = cinfo.image_width;
    pFrame->ppPlanes = pAvFrame->m_ppPlanes;
    pFrame->pPlaneLineSizes = pAvFrame->m_pLinesizes;
    pFrame->nPlanes = 1;
    pFrame->ppPlanes[0] = new unsigned char[cinfo.image_height*cinfo.image_width*3];
    pFrame->pPlaneLineSizes[0] = cinfo.image_width * 3;
    pAvFrame->m_spPlanes.take(pFrame->ppPlanes, [](unsigned char** ptr){
        if(ptr[0] != nullptr){
            delete[] ptr[0];
            ptr[0] = nullptr;
        }
    });
  /* Step 4: set parameters for decompression */

  /* In this example, we don't need to change any of the defaults set by
   * jpeg_read_header(), so we do nothing here.
   */

  /* Step 5: Start decompressor */

  (void)jpeg_start_decompress(&cinfo);
  /* We can ignore the return value since suspension is not possible
   * with the stdio data source.
   */

  /* We may need to do some setup of our own at this point before reading
   * the data.  After jpeg_start_decompress() we have the correct scaled
   * output image dimensions available, as well as the output colormap
   * if we asked for color quantization.
   * In this example, we need to make an output work buffer of the right size.
   */
  /* JSAMPLEs per row in output buffer */
  row_stride = cinfo.output_width * cinfo.output_components;
  /* Make a one-row-high sample array that will go away when done with image */
  buffer = (*cinfo.mem->alloc_sarray)
                ((j_common_ptr)&cinfo, JPOOL_IMAGE, row_stride, 1);

  /* Step 6: while (scan lines remain to be read) */
  /*           jpeg_read_scanlines(...); */

  /* Here we use the library's state variable cinfo->output_scanline as the
   * loop counter, so that we don't have to keep track ourselves.
   */
  while (cinfo.output_scanline < cinfo.output_height) {
    
    /* jpeg_read_scanlines expects an array of pointers to scanlines.
     * Here the array is only one element long, but you could ask for
     * more than one scanline at a time if that's more convenient.
     */
    (void)jpeg_read_scanlines(&cinfo, buffer, 1);

    /* Assume put_scanline_someplace wants a pointer and sample count. */
    //put_scanline_someplace(buffer[0], row_stride);       
    unsigned char* pData = pFrame->ppPlanes[0] + row_stride * (cinfo.output_scanline-1);
    memcpy(pData, buffer[0], row_stride);
  }

  /* Step 7: Finish decompression */

  (void)jpeg_finish_decompress(&cinfo);
  /* We can ignore the return value since suspension is not possible
   * with the stdio data source.
   */

  /* Step 8: Release JPEG decompression object */

  /* This is an important step since it will release a good deal of memory. */
  jpeg_destroy_decompress(&cinfo);

  /* After finish_decompress, we can close the input file.
   * Here we postpone it until after no more JPEG errors are possible,
   * so as to simplify the setjmp error logic above.  (Actually, I don't
   * think that jpeg_destroy can do an error exit, but why assume anything...)
   */
  fclose(infile);

  /* At this point you may want to check to see whether any corrupt-data
   * warnings occurred (test whether jerr.pub.num_warnings is nonzero).
   */

  /* And we're done! */
  return 1;
}


static int write_JPEG_file(const char *filename, int quality, unsigned char* image_buffer, int image_width, int image_height)
{
	/* This struct contains the JPEG compression parameters and pointers to
	* working space (which is allocated as needed by the JPEG library).
	* It is possible to have several such structures, representing multiple
	* compression/decompression processes, in existence at once.  We refer
	* to any one struct (and its associated working data) as a "JPEG object".
	*/
	struct jpeg_compress_struct cinfo;
	/* This struct represents a JPEG error handler.  It is declared separately
	* because applications often want to supply a specialized error handler
	* (see the second half of this file for an example).  But here we just
	* take the easy way out and use the standard error handler, which will
	* print a message on stderr and call exit() if compression fails.
	* Note that this struct must live as long as the main JPEG parameter
	* struct, to avoid dangling-pointer problems.
	*/
	struct jpeg_error_mgr jerr;
	/* More stuff */
	FILE *outfile;                /* target file */
	JSAMPROW row_pointer[1];      /* pointer to JSAMPLE row[s] */
	int row_stride;               /* physical row width in image buffer */

	/* Step 1: allocate and initialize JPEG compression object */

	/* We have to set up the error handler first, in case the initialization
	* step fails.  (Unlikely, but it could happen if you are out of memory.)
	* This routine fills in the contents of struct jerr, and returns jerr's
	* address which we place into the link field in cinfo.
	*/
	cinfo.err = jpeg_std_error(&jerr);
	/* Now we can initialize the JPEG compression object. */
	jpeg_create_compress(&cinfo);

	/* Step 2: specify data destination (eg, a file) */
	/* Note: steps 2 and 3 can be done in either order. */

	/* Here we use the library-supplied code to send compressed data to a
	* stdio stream.  You can also write your own code to do something else.
	* VERY IMPORTANT: use "b" option to fopen() if you are on a machine that
	* requires it in order to write binary files.
	*/
	if ((outfile = fopen(filename, "wb")) == NULL) {
		fprintf(stderr, "can't open %s\n", filename);
		return 0;
	}
	jpeg_stdio_dest(&cinfo, outfile);

	/* Step 3: set parameters for compression */

	/* First we supply a description of the input image.
	* Four fields of the cinfo struct must be filled in:
	*/
	cinfo.image_width = image_width;      /* image width and height, in pixels */
	cinfo.image_height = image_height;
	cinfo.input_components = 3;           /* # of color components per pixel */
	cinfo.in_color_space = JCS_RGB;       /* colorspace of input image */
	/* Now use the library's routine to set default compression parameters.
	* (You must set at least cinfo.in_color_space before calling this,
	* since the defaults depend on the source color space.)
	*/
	jpeg_set_defaults(&cinfo);
	/* Now you can set any non-default parameters you wish to.
	* Here we just illustrate the use of quality (quantization table) scaling:
	*/
	jpeg_set_quality(&cinfo, quality, TRUE /* limit to baseline-JPEG values */);

	/* Step 4: Start compressor */

	/* TRUE ensures that we will write a complete interchange-JPEG file.
	* Pass TRUE unless you are very sure of what you're doing.
	*/
	jpeg_start_compress(&cinfo, TRUE);

	/* Step 5: while (scan lines remain to be written) */
	/*           jpeg_write_scanlines(...); */

	/* Here we use the library's state variable cinfo.next_scanline as the
	* loop counter, so that we don't have to keep track ourselves.
	* To keep things simple, we pass one scanline per call; you can pass
	* more if you wish, though.
	*/
	row_stride = image_width * 3; /* JSAMPLEs per row in image_buffer */

	while (cinfo.next_scanline < cinfo.image_height) {
		/* jpeg_write_scanlines expects an array of pointers to scanlines.
		* Here the array is only one element long, but you could pass
		* more than one scanline at a time if that's more convenient.
		*/
		row_pointer[0] = &image_buffer[cinfo.next_scanline * row_stride];
		(void)jpeg_write_scanlines(&cinfo, row_pointer, 1);
	}

	/* Step 6: Finish compression */

	jpeg_finish_compress(&cinfo);
	/* After finish_compress, we can close the output file. */
	fclose(outfile);

	/* Step 7: release JPEG compression object */

	/* This is an important step since it will release a good deal of memory. */
	jpeg_destroy_compress(&cinfo);

	/* And we're done! */
    return 1;
}

static int writeJpegFile(const char* szName, const SAvFrame& spFrame) {

    const PAvFrame* pFrame = spFrame->getFramePtr(); 

    PAvSample targetSample;
    targetSample.sampleType = EAvSampleType::AvSampleType_Video;
    targetSample.sampleFormat = EAvSampleFormat::AvSampleFormat_Video_RGB;
    targetSample.videoHeight = pFrame->nHeight;
    targetSample.videoWidth = pFrame->nWidth;

    SAvNetwork spConverter;
    if( SAvFactory::getAvFactory()->openAvFrameConverter(targetSample, spConverter) != sCtx.success() ) {
        return sCtx.error("创建格式转化器失败");
    }

    SAvFrame spOut;
    if( spConverter->pipeIn(spFrame, spOut) != sCtx.success() ) {
        return sCtx.error("图片格式转化失败");
    }

    pFrame = spOut->getFramePtr();
    if( write_JPEG_file(szName, 90, pFrame->ppPlanes[0], pFrame->nWidth, pFrame->nHeight) ) {
        return true;
    }
    return sCtx.error("保存图片失败");
}
#endif


int CAvFrame::loadImage(const char* szFileName, SAvFrame& spFrame) {
    CPointer<CAvFrame> sp;
    CObject::createObject(sp);

    #ifdef ENABLE_JPEG_LIBRARY
    if( isJpegFile(szFileName) ) {
        if( read_JPEG_file(szFileName, sp) ) {
            spFrame.setPtr(sp.getPtr());
            return sCtx.success();
        }
        return sCtx.error();
    }
    #endif//ENABLE_JPEG_LIBRARY

    int width, height;
    AVPixelFormat pixelFormat;
    if( ff_load_image(
                sp->m_ppPlanes, sp->m_pLinesizes, 
                &width, &height, &pixelFormat,
                szFileName, nullptr, sp->m_spBuffer) < 0 ) {
        return sCtx.error("读取图片文件失败");
    }

    PAvFrame* pFrame = &sp->m_avFrame;
    pFrame->nHeight = height;
    pFrame->nWidth = width;
    pFrame->streamingId = 0;
    pFrame->timeRate = 0;
    pFrame->timeStamp = 0;
    pFrame->sampleMeta.sampleType = EAvSampleType::AvSampleType_Video;
    pFrame->sampleMeta.sampleFormat = CAvSampleType::convert(pixelFormat);
    pFrame->sampleMeta.videoHeight = height;
    pFrame->sampleMeta.videoWidth = width;
    pFrame->pPlaneLineSizes = sp->m_pLinesizes;
    pFrame->ppPlanes = sp->m_ppPlanes;
    pFrame->nPlanes = 0;
    for( int i=0; i<AV_NUM_DATA_POINTERS && pFrame->ppPlanes[i]; i++ ) {
        pFrame->nPlanes = i+1;
    }
    spFrame.setPtr(sp.getPtr());
    return sCtx.success();
}

int CAvFrame::saveImage(const char* szFileName, const SAvFrame& spFrame) {
    #ifdef ENABLE_JPEG_LIBRARY
    if(isJpegFile(szFileName)) {
        return writeJpegFile(szFileName, spFrame);
    }
    #endif//#ifdef ENABLE_JPEG_LIBRARY

    const PAvFrame* pFrame = spFrame ? spFrame->getFramePtr() : nullptr;
    if(pFrame == nullptr) {
        return sCtx.error("图片中无法写入无效的帧数据");
    }
    ((PAvFrame*)pFrame)->timeStamp = 0;

    AVOutputFormat *oformat = NULL;
    oformat = av_guess_format("image2pipe", szFileName, NULL);

    // 分配AVFormatContext对象
    AVFormatContext* pFormatContext = nullptr;
    if( avformat_alloc_output_context2(&pFormatContext, oformat, NULL, szFileName) < 0 ) {
        return sCtx.error("写入上下文创建失败");
    }
    CTaker<AVFormatContext*> spFormatCtx(pFormatContext, [](AVFormatContext* pCtx){
        avformat_free_context(pCtx);
    });

    // 创建流
    PAvStreaming avStreaming;
    avStreaming.streamingId = 0;
    avStreaming.timeRate = 25;
    avStreaming.timeDuration = 0;
    avStreaming.frameMeta = pFrame->sampleMeta;
    CPointer<CAvOutStreaming> spStreaming;
    CObject::createObject(spStreaming);
    if(spStreaming->init(pFormatContext, &avStreaming) != sCtx.success()) {
        return sCtx.error("初始化视频流失败");
    }

    if( spStreaming->open(pFormatContext) != sCtx.success() ) {
        return sCtx.error("打开图片对应的流失败");
    }

    av_dump_format(pFormatContext, 0, szFileName, 1);

    // 创建并初始化一个和该url相关的AVIOContext
    if( avio_open(&pFormatContext->pb, szFileName, AVIO_FLAG_WRITE) < 0){
        return sCtx.error("无法打开输出文件");
    }
    CTaker<AVIOContext*> spIOContext(pFormatContext->pb, [](AVIOContext* pCtx) {
        avio_close(pCtx);
    });

    //写入头部信息
    if( avformat_write_header(pFormatContext, NULL) < 0 ) {
        return sCtx.error("写入图片文件头信息失败");
    };
 
    //写入图片信息
    if( spStreaming->writeFrame(pFormatContext, spFrame) != sCtx.success() ) {
        return sCtx.error("向图片文件写入帧数据失败");
    }

    //写入尾部信息
    if( av_write_trailer(pFormatContext) < 0 ) {
        return sCtx.error("写入图片文件尾部信息失败");
    };
 
    return sCtx.success();
}

int CAvFrame::allocImageDataBuffer(
                    SDeviceMemory& spDataBuffer, 
                    AVPixelFormat pixFormat, int nWidth, int nHeight, 
                    int pLinesizes[AV_NUM_DATA_POINTERS], 
                    uint8_t *ppPlanes[AV_NUM_DATA_POINTERS] ) 
{
    int size = av_image_get_buffer_size(pixFormat, nWidth, nHeight, BUFFER_ALIGN);
    if(size <= 0) {
        return sCtx.error("图像格式异常，无法计算需要分配的图像缓冲大小");
    }

    SDeviceMemory spBuffer = SDeviceMemory::createDeviceMemory(SDevice::cpu(), size);
    if( !spBuffer ) {
        return sCtx.error("分配图像缓冲失败");
    }
    
    if( av_image_fill_linesizes(pLinesizes, pixFormat, nWidth) < 0) {
        return sCtx.error("计算图片Linesize失败");
    }

    int nBufferSize = spBuffer.size();
    int nRequireSize = av_image_fill_pointers(ppPlanes, pixFormat, nHeight, (uint8_t*)spBuffer.data(), pLinesizes);
    if( nRequireSize != spBuffer.size() ) {
        return sCtx.error("分配的缓冲区和需要的缓冲区大小不一致");
    }
    spDataBuffer = spBuffer;
    return sCtx.success();
}

int CAvFrame::allocAudioSampleDataBuffer(
                    SDeviceMemory& spDataBuffer, 
                    AVSampleFormat sampleFormat, int nb_channels, int nb_samples,
                    int pLinesizes[AV_NUM_DATA_POINTERS], 
                    uint8_t *ppPlanes[AV_NUM_DATA_POINTERS] ){
    int size = av_samples_get_buffer_size(pLinesizes, nb_channels, nb_samples, sampleFormat, BUFFER_ALIGN);
    if(size <= 0) {
        return sCtx.error("图像格式异常，无法计算需要分配的图像缓冲大小");
    }

    SDeviceMemory spBuffer = SDeviceMemory::createDeviceMemory(SDevice::cpu(), size);
    if( !spBuffer ) {
        return sCtx.error("分配图像缓冲失败");
    }
    
    for(int i=0; i<AV_NUM_DATA_POINTERS; i++) {
        ppPlanes[i] = nullptr;
    }

    uint8_t *pDataBuffer = (uint8_t*)spBuffer.data();
    int nRequireSize = av_samples_fill_arrays(
        ppPlanes, pLinesizes, pDataBuffer, 
        nb_channels, nb_samples, sampleFormat, BUFFER_ALIGN);
    if( nRequireSize != spBuffer.size() ) {
        return sCtx.error("分配的缓冲区和需要的缓冲区大小不一致");
    }
    spDataBuffer = spBuffer;
    return sCtx.success();
}

FFMPEG_NAMESPACE_LEAVE