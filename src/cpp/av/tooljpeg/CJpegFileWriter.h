#pragma once
using namespace sw::av;
class CJpegFileWriter {
public:
    static int saveToFile(const char* szFileName, const SAvFrame& spFrame);
};