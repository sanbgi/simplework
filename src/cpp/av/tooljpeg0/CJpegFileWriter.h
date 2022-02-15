#pragma once
using namespace sw;
class CJpegFileWriter {
public:
    static int saveToFile(const char* szFileName, const SAvFrame& spFrame);
};