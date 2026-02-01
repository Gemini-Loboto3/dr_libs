/*
dr_at3 Basic Tests

This test checks basic functionality of dr_at3 without crashing. It verifies:
1. Consistency between memory and file initialization
2. Consistency between f32 and s16 output
3. Basic properties match between init modes
4. Container detection (RIFF WAV, OMA)
5. Seeking functionality
*/
#include "at3_common.c"

#define FILE_NAME_WIDTH 40
#define NUMBER_WIDTH    10
#define TABLE_MARGIN    2

int open_decoders(drat3* pDecoderMemory, drat3* pDecoderFile, const char* pFilePath, void** ppFileData, size_t* pFileSize)
{
    size_t dataSize;
    void* pData;

    /* Load file into memory */
    pData = dr_open_and_read_file(pFilePath, &dataSize);
    if (pData == NULL) {
        printf("Failed to open file \"%s\"\n", pFilePath);
        return -1;
    }

    /* Initialize memory decoder */
    if (drat3_init_memory(pData, dataSize, NULL, pDecoderMemory) != DRAT3_SUCCESS) {
        free(pData);
        printf("Failed to init AT3 decoder from memory \"%s\"\n", pFilePath);
        return -1;
    }

    /* Initialize file decoder */
    if (drat3_init_file(pFilePath, NULL, pDecoderFile) != DRAT3_SUCCESS) {
        drat3_uninit(pDecoderMemory);
        free(pData);
        printf("Failed to init AT3 decoder from file \"%s\"\n", pFilePath);
        return -1;
    }

    *ppFileData = pData;
    *pFileSize = dataSize;
    return 0;
}

int validate_basic_properties(drat3* pAt3Memory, drat3* pAt3File)
{
    if (pAt3Memory->channels != pAt3File->channels) {
        printf("Channel counts differ: memory=%u, file=%u\n", pAt3Memory->channels, pAt3File->channels);
        return 1;
    }

    if (pAt3Memory->sampleRate != pAt3File->sampleRate) {
        printf("Sample rates differ: memory=%u, file=%u\n", pAt3Memory->sampleRate, pAt3File->sampleRate);
        return 1;
    }

    if (pAt3Memory->totalPCMFrameCount != pAt3File->totalPCMFrameCount) {
        printf("Total frame counts differ: memory=%llu, file=%llu\n",
               (unsigned long long)pAt3Memory->totalPCMFrameCount,
               (unsigned long long)pAt3File->totalPCMFrameCount);
        return 1;
    }

    return 0;
}

int validate_decoding_f32(drat3* pAt3Memory, drat3* pAt3File)
{
    int result = 0;
    drat3_uint64 totalFramesMemory = 0;
    drat3_uint64 totalFramesFile = 0;

    for (;;) {
        drat3_uint64 iSample;
        drat3_uint64 framesMemory;
        drat3_uint64 framesFile;
        float pcmMemory[4096];
        float pcmFile[4096];
        drat3_uint64 framesToRead = sizeof(pcmMemory) / sizeof(pcmMemory[0]) / pAt3Memory->channels;

        framesMemory = drat3_read_pcm_frames_f32(pAt3Memory, framesToRead, pcmMemory);
        framesFile = drat3_read_pcm_frames_f32(pAt3File, framesToRead, pcmFile);

        totalFramesMemory += framesMemory;
        totalFramesFile += framesFile;

        if (framesMemory != framesFile) {
            printf("Frame counts differ: memory=%llu, file=%llu (at total memory=%llu, file=%llu)\n",
                   (unsigned long long)framesMemory, (unsigned long long)framesFile,
                   (unsigned long long)totalFramesMemory, (unsigned long long)totalFramesFile);
            result = 1;
            break;
        }

        /* Compare samples */
        for (iSample = 0; iSample < framesMemory * pAt3Memory->channels; iSample++) {
            if (pcmMemory[iSample] != pcmFile[iSample]) {
                printf("Sample mismatch at frame %llu, sample %llu: memory=%f, file=%f\n",
                       (unsigned long long)totalFramesMemory, (unsigned long long)iSample,
                       pcmMemory[iSample], pcmFile[iSample]);
                result = 1;
                break;
            }
        }

        if (result != 0) {
            break;
        }

        if (framesMemory == 0) {
            break;
        }
    }

    return result;
}

int validate_decoding_s16(drat3* pAt3Memory, drat3* pAt3File)
{
    int result = 0;
    drat3_uint64 totalFramesMemory = 0;
    drat3_uint64 totalFramesFile = 0;

    for (;;) {
        drat3_uint64 iSample;
        drat3_uint64 framesMemory;
        drat3_uint64 framesFile;
        drat3_int16 pcmMemory[4096];
        drat3_int16 pcmFile[4096];
        drat3_uint64 framesToRead = sizeof(pcmMemory) / sizeof(pcmMemory[0]) / pAt3Memory->channels;

        framesMemory = drat3_read_pcm_frames_s16(pAt3Memory, framesToRead, pcmMemory);
        framesFile = drat3_read_pcm_frames_s16(pAt3File, framesToRead, pcmFile);

        totalFramesMemory += framesMemory;
        totalFramesFile += framesFile;

        if (framesMemory != framesFile) {
            printf("Frame counts differ (s16): memory=%llu, file=%llu\n",
                   (unsigned long long)framesMemory, (unsigned long long)framesFile);
            result = 1;
            break;
        }

        /* Compare samples */
        for (iSample = 0; iSample < framesMemory * pAt3Memory->channels; iSample++) {
            if (pcmMemory[iSample] != pcmFile[iSample]) {
                printf("Sample mismatch (s16) at sample %llu: memory=%d, file=%d\n",
                       (unsigned long long)iSample, pcmMemory[iSample], pcmFile[iSample]);
                result = 1;
                break;
            }
        }

        if (result != 0) {
            break;
        }

        if (framesMemory == 0) {
            break;
        }
    }

    return result;
}

int validate_seeking(drat3* pAt3)
{
    drat3_uint64 totalFrames = pAt3->totalPCMFrameCount;
    drat3_uint64 seekPositions[5];
    int i;

    if (totalFrames == 0) {
        return 0; /* Skip seeking test for empty files */
    }

    /* Test seeking to various positions */
    seekPositions[0] = 0;
    seekPositions[1] = totalFrames / 4;
    seekPositions[2] = totalFrames / 2;
    seekPositions[3] = totalFrames * 3 / 4;
    seekPositions[4] = totalFrames > 0 ? totalFrames - 1 : 0;

    for (i = 0; i < 5; i++) {
        drat3_uint64 cursor;
        drat3_result result;

        result = drat3_seek_to_pcm_frame(pAt3, seekPositions[i]);
        if (result != DRAT3_SUCCESS) {
            printf("Seek to frame %llu failed with error %d\n",
                   (unsigned long long)seekPositions[i], result);
            return 1;
        }

        cursor = drat3_get_cursor_in_pcm_frames(pAt3);
        /* ATRAC frames can be large, allow tolerance */
        if (cursor > seekPositions[i] + 4096) {
            printf("Seek position mismatch: requested=%llu, got=%llu\n",
                   (unsigned long long)seekPositions[i], (unsigned long long)cursor);
            return 1;
        }
    }

    return 0;
}

const char* get_container_name(const drat3_container_info* pInfo)
{
    if (pInfo == NULL) return "Unknown";
    switch (pInfo->container_type) {
        case DRAT3_CONTAINER_RIFF_WAV: return "RIFF WAV";
        case DRAT3_CONTAINER_OMA:      return "OMA";
        default:                        return "Unknown";
    }
}

const char* get_codec_name(drat3* pAt3)
{
    switch (pAt3->codecType) {
        case DRAT3_CODEC_ATRAC3:  return "ATRAC3";
        case DRAT3_CODEC_ATRAC3P: return "ATRAC3+";
        default:                   return "Unknown";
    }
}

int do_test_file(const char* pFilePath)
{
    int result = 0;
    drat3 at3Memory;
    drat3 at3File;
    void* pFileData;
    size_t fileSize;

    dr_printf_fixed_with_margin(FILE_NAME_WIDTH, TABLE_MARGIN, "%s", dr_path_file_name(pFilePath));

    if (open_decoders(&at3Memory, &at3File, pFilePath, &pFileData, &fileSize) != 0) {
        printf("  FAILED (open)\n");
        return 1;
    }

    /* Print container and codec info */
    {
        const drat3_container_info* pInfo = drat3_container_get_info(at3File.pContainer);
        printf("[%s/%s] ", get_container_name(pInfo), get_codec_name(&at3File));
    }

    /* Validate basic properties */
    if (validate_basic_properties(&at3Memory, &at3File) != 0) {
        printf("  FAILED (properties)\n");
        result = 1;
        goto done;
    }

    /* Validate f32 decoding */
    if (validate_decoding_f32(&at3Memory, &at3File) != 0) {
        printf("  FAILED (decode f32)\n");
        result = 1;
        goto done;
    }

    /* Reinitialize for s16 test */
    drat3_uninit(&at3Memory);
    drat3_uninit(&at3File);

    if (drat3_init_memory(pFileData, fileSize, NULL, &at3Memory) != DRAT3_SUCCESS ||
        drat3_init_file(pFilePath, NULL, &at3File) != DRAT3_SUCCESS) {
        printf("  FAILED (reinit)\n");
        result = 1;
        goto done_no_uninit;
    }

    /* Validate s16 decoding */
    if (validate_decoding_s16(&at3Memory, &at3File) != 0) {
        printf("  FAILED (decode s16)\n");
        result = 1;
        goto done;
    }

    /* Test seeking on the file decoder */
    drat3_uninit(&at3File);
    if (drat3_init_file(pFilePath, NULL, &at3File) != DRAT3_SUCCESS) {
        printf("  FAILED (reinit for seek)\n");
        result = 1;
        goto done;
    }

    if (validate_seeking(&at3File) != 0) {
        printf("  FAILED (seeking)\n");
        result = 1;
        goto done;
    }

    printf("  PASSED\n");

done:
    drat3_uninit(&at3Memory);
    drat3_uninit(&at3File);
done_no_uninit:
    free(pFileData);
    return result;
}

int do_test_directory(const char* pDirectoryPath)
{
    dr_file_iterator iterator;
    dr_file_iterator* pFile;
    int result = 0;
    int fileCount = 0;

    pFile = dr_file_iterator_begin(pDirectoryPath, &iterator);
    while (pFile != NULL) {
        if (!pFile->isDirectory) {
            if (dr_extension_equal(pFile->relativePath, "at3") ||
                dr_extension_equal(pFile->relativePath, "oma") ||
                dr_extension_equal(pFile->relativePath, "aa3")) {
                if (do_test_file(pFile->absolutePath) != 0) {
                    result = 1;
                }
                fileCount++;
            }
        }
        pFile = dr_file_iterator_next(pFile);
    }

    if (fileCount == 0) {
        printf("No .at3/.oma/.aa3 files found in %s\n", pDirectoryPath);
        printf("To run tests, add ATRAC3/ATRAC3+ files to %s\n", pDirectoryPath);
    }

    return result;
}

int test_high_level_api(const char* pFilePath)
{
    drat3_uint32 channels, sampleRate;
    drat3_uint64 totalFrameCount;
    float* pSamples;

    printf("Testing high-level API...\n");

    pSamples = drat3_open_file_and_read_pcm_frames_f32(pFilePath, &channels, &sampleRate, &totalFrameCount, NULL);
    if (pSamples == NULL) {
        printf("  drat3_open_file_and_read_pcm_frames_f32 returned NULL (file may not exist)\n");
        return 0; /* Not a failure if no test files */
    }

    printf("  Decoded %llu frames, %u channels, %u Hz\n",
           (unsigned long long)totalFrameCount, channels, sampleRate);

    drat3_free(pSamples, NULL);
    printf("  PASSED\n");
    return 0;
}

int test_container_probing(void)
{
    /* Test container detection with known magic bytes */
    const unsigned char riff_header[] = {'R', 'I', 'F', 'F', 0, 0, 0, 0, 'W', 'A', 'V', 'E'};
    const unsigned char oma_header[] = {'e', 'a', '3', 0x03, 0, 0, 0, 0, 0, 0, 0, 0}; /* OMA needs 12 bytes minimum */
    const unsigned char invalid_header[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    printf("Testing container probing...\n");

    if (drat3_container_probe(riff_header, sizeof(riff_header)) != DRAT3_CONTAINER_RIFF_WAV) {
        printf("  FAILED: RIFF WAV not detected\n");
        return 1;
    }

    if (drat3_container_probe(oma_header, sizeof(oma_header)) != DRAT3_CONTAINER_OMA) {
        printf("  FAILED: OMA not detected\n");
        return 1;
    }

    if (drat3_container_probe(invalid_header, sizeof(invalid_header)) != DRAT3_CONTAINER_UNKNOWN) {
        printf("  FAILED: Invalid header should return UNKNOWN\n");
        return 1;
    }

    printf("  PASSED\n");
    return 0;
}

int main(int argc, char** argv)
{
    int result = 0;
    const char* pSourceDir = AT3_DEFAULT_SOURCE_DIR;

    if (argc > 1) {
        pSourceDir = argv[1];
    }

    printf("dr_at3 Basic Tests\n");
    printf("==================\n\n");

    /* Test container probing */
    result |= test_container_probing();

    /* Test directory of files */
    printf("\nTesting consistency between memory and file initialization:\n");
    printf("------------------------------------------------------------\n");
    result |= do_test_directory(pSourceDir);

    /* Test high-level API if we have at least one file */
    {
        dr_file_iterator iterator;
        dr_file_iterator* pFile = dr_file_iterator_begin(pSourceDir, &iterator);
        while (pFile != NULL) {
            if (!pFile->isDirectory &&
                (dr_extension_equal(pFile->relativePath, "at3") ||
                 dr_extension_equal(pFile->relativePath, "oma") ||
                 dr_extension_equal(pFile->relativePath, "aa3"))) {
                result |= test_high_level_api(pFile->absolutePath);
                break;
            }
            pFile = dr_file_iterator_next(pFile);
        }
    }

    printf("\n");
    if (result == 0) {
        printf("All tests PASSED.\n");
    } else {
        printf("Some tests FAILED.\n");
    }

    return result;
}
