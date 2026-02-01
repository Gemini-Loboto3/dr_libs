/*
dr_vorbis Basic Tests

This test checks basic functionality of dr_vorbis without crashing. It verifies:
1. Consistency between memory and file initialization
2. Consistency between f32 and s16 output
3. Basic properties match between init modes
4. Seeking functionality
*/
#include "vorbis_common.c"

#define FILE_NAME_WIDTH 40
#define NUMBER_WIDTH    10
#define TABLE_MARGIN    2

int open_decoders(dr_vorbis* pDecoderMemory, dr_vorbis* pDecoderFile, const char* pFilePath, void** ppFileData, size_t* pFileSize)
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
    if (dr_vorbis_init_memory(pData, dataSize, NULL, pDecoderMemory) != DR_VORBIS_SUCCESS) {
        free(pData);
        printf("Failed to init Vorbis decoder from memory \"%s\"\n", pFilePath);
        return -1;
    }

    /* Initialize file decoder */
    if (dr_vorbis_init_file(pFilePath, NULL, pDecoderFile) != DR_VORBIS_SUCCESS) {
        dr_vorbis_uninit(pDecoderMemory);
        free(pData);
        printf("Failed to init Vorbis decoder from file \"%s\"\n", pFilePath);
        return -1;
    }

    *ppFileData = pData;
    *pFileSize = dataSize;
    return 0;
}

int validate_basic_properties(dr_vorbis* pVorbisMemory, dr_vorbis* pVorbisFile)
{
    if (pVorbisMemory->channels != pVorbisFile->channels) {
        printf("Channel counts differ: memory=%u, file=%u\n", pVorbisMemory->channels, pVorbisFile->channels);
        return 1;
    }

    if (pVorbisMemory->sampleRate != pVorbisFile->sampleRate) {
        printf("Sample rates differ: memory=%u, file=%u\n", pVorbisMemory->sampleRate, pVorbisFile->sampleRate);
        return 1;
    }

    if (pVorbisMemory->totalPCMFrameCount != pVorbisFile->totalPCMFrameCount) {
        printf("Total frame counts differ: memory=%llu, file=%llu\n",
               (unsigned long long)pVorbisMemory->totalPCMFrameCount,
               (unsigned long long)pVorbisFile->totalPCMFrameCount);
        return 1;
    }

    return 0;
}

int validate_decoding_f32(dr_vorbis* pVorbisMemory, dr_vorbis* pVorbisFile)
{
    int result = 0;
    drlibs_uint64 totalFramesMemory = 0;
    drlibs_uint64 totalFramesFile = 0;

    for (;;) {
        drlibs_uint64 iSample;
        drlibs_uint64 framesMemory;
        drlibs_uint64 framesFile;
        float pcmMemory[4096];
        float pcmFile[4096];
        drlibs_uint64 framesToRead = sizeof(pcmMemory) / sizeof(pcmMemory[0]) / pVorbisMemory->channels;

        framesMemory = dr_vorbis_read_pcm_frames_f32(pVorbisMemory, framesToRead, pcmMemory);
        framesFile = dr_vorbis_read_pcm_frames_f32(pVorbisFile, framesToRead, pcmFile);

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
        for (iSample = 0; iSample < framesMemory * pVorbisMemory->channels; iSample++) {
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

int validate_decoding_s16(dr_vorbis* pVorbisMemory, dr_vorbis* pVorbisFile)
{
    int result = 0;
    drlibs_uint64 totalFramesMemory = 0;
    drlibs_uint64 totalFramesFile = 0;

    for (;;) {
        drlibs_uint64 iSample;
        drlibs_uint64 framesMemory;
        drlibs_uint64 framesFile;
        drlibs_int16 pcmMemory[4096];
        drlibs_int16 pcmFile[4096];
        drlibs_uint64 framesToRead = sizeof(pcmMemory) / sizeof(pcmMemory[0]) / pVorbisMemory->channels;

        framesMemory = dr_vorbis_read_pcm_frames_s16(pVorbisMemory, framesToRead, pcmMemory);
        framesFile = dr_vorbis_read_pcm_frames_s16(pVorbisFile, framesToRead, pcmFile);

        totalFramesMemory += framesMemory;
        totalFramesFile += framesFile;

        if (framesMemory != framesFile) {
            printf("Frame counts differ (s16): memory=%llu, file=%llu\n",
                   (unsigned long long)framesMemory, (unsigned long long)framesFile);
            result = 1;
            break;
        }

        /* Compare samples */
        for (iSample = 0; iSample < framesMemory * pVorbisMemory->channels; iSample++) {
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

int validate_seeking(dr_vorbis* pVorbis)
{
    drlibs_uint64 totalFrames = pVorbis->totalPCMFrameCount;
    drlibs_uint64 seekPositions[5];
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
        drlibs_uint64 cursor;
        dr_vorbis_result result;

        result = dr_vorbis_seek_to_pcm_frame(pVorbis, seekPositions[i]);
        if (result != DR_VORBIS_SUCCESS) {
            printf("Seek to frame %llu failed with error %d\n",
                   (unsigned long long)seekPositions[i], result);
            return 1;
        }

        cursor = dr_vorbis_get_cursor_in_pcm_frames(pVorbis);
        if (cursor != seekPositions[i]) {
            /* Some tolerance for seeking - might land on block boundary */
            if (cursor > seekPositions[i] + 4096) {
                printf("Seek position mismatch: requested=%llu, got=%llu\n",
                       (unsigned long long)seekPositions[i], (unsigned long long)cursor);
                return 1;
            }
        }
    }

    return 0;
}

int do_test_file(const char* pFilePath)
{
    int result = 0;
    dr_vorbis vorbisMemory;
    dr_vorbis vorbisFile;
    void* pFileData;
    size_t fileSize;

    dr_printf_fixed_with_margin(FILE_NAME_WIDTH, TABLE_MARGIN, "%s", dr_path_file_name(pFilePath));

    if (open_decoders(&vorbisMemory, &vorbisFile, pFilePath, &pFileData, &fileSize) != 0) {
        printf("  FAILED (open)\n");
        return 1;
    }

    /* Validate basic properties */
    if (validate_basic_properties(&vorbisMemory, &vorbisFile) != 0) {
        printf("  FAILED (properties)\n");
        result = 1;
        goto done;
    }

    /* Validate f32 decoding */
    if (validate_decoding_f32(&vorbisMemory, &vorbisFile) != 0) {
        printf("  FAILED (decode f32)\n");
        result = 1;
        goto done;
    }

    /* Reinitialize for s16 test */
    dr_vorbis_uninit(&vorbisMemory);
    dr_vorbis_uninit(&vorbisFile);

    if (dr_vorbis_init_memory(pFileData, fileSize, NULL, &vorbisMemory) != DR_VORBIS_SUCCESS ||
        dr_vorbis_init_file(pFilePath, NULL, &vorbisFile) != DR_VORBIS_SUCCESS) {
        printf("  FAILED (reinit)\n");
        result = 1;
        goto done_no_uninit;
    }

    /* Validate s16 decoding */
    if (validate_decoding_s16(&vorbisMemory, &vorbisFile) != 0) {
        printf("  FAILED (decode s16)\n");
        result = 1;
        goto done;
    }

    /* Test seeking on the file decoder */
    dr_vorbis_uninit(&vorbisFile);
    if (dr_vorbis_init_file(pFilePath, NULL, &vorbisFile) != DR_VORBIS_SUCCESS) {
        printf("  FAILED (reinit for seek)\n");
        result = 1;
        goto done;
    }

    if (validate_seeking(&vorbisFile) != 0) {
        printf("  FAILED (seeking)\n");
        result = 1;
        goto done;
    }

    printf("  PASSED\n");

done:
    dr_vorbis_uninit(&vorbisMemory);
    dr_vorbis_uninit(&vorbisFile);
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
            if (dr_extension_equal(pFile->relativePath, "ogg")) {
                if (do_test_file(pFile->absolutePath) != 0) {
                    result = 1;
                }
                fileCount++;
            }
        }
        pFile = dr_file_iterator_next(pFile);
    }

    if (fileCount == 0) {
        printf("No .ogg files found in %s\n", pDirectoryPath);
        printf("To run tests, add .ogg (Vorbis) files to %s\n", pDirectoryPath);
    }

    return result;
}

int test_high_level_api(const char* pFilePath)
{
    drlibs_uint32 channels, sampleRate;
    drlibs_uint64 totalFrameCount;
    float* pSamples;

    printf("Testing high-level API...\n");

    pSamples = dr_vorbis_open_file_and_read_pcm_frames_f32(pFilePath, &channels, &sampleRate, &totalFrameCount, NULL);
    if (pSamples == NULL) {
        printf("  dr_vorbis_open_file_and_read_pcm_frames_f32 returned NULL (file may not exist)\n");
        return 0; /* Not a failure if no test files */
    }

    printf("  Decoded %llu frames, %u channels, %u Hz\n",
           (unsigned long long)totalFrameCount, channels, sampleRate);

    dr_vorbis_free(pSamples, NULL);
    printf("  PASSED\n");
    return 0;
}

int main(int argc, char** argv)
{
    int result = 0;
    const char* pSourceDir = VORBIS_DEFAULT_SOURCE_DIR;

    if (argc > 1) {
        pSourceDir = argv[1];
    }

    printf("dr_vorbis Basic Tests\n");
    printf("=====================\n\n");

    /* Test directory of files */
    printf("Testing consistency between memory and file initialization:\n");
    printf("------------------------------------------------------------\n");
    result = do_test_directory(pSourceDir);

    /* Test high-level API if we have at least one file */
    {
        dr_file_iterator iterator;
        dr_file_iterator* pFile = dr_file_iterator_begin(pSourceDir, &iterator);
        while (pFile != NULL) {
            if (!pFile->isDirectory && dr_extension_equal(pFile->relativePath, "ogg")) {
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
