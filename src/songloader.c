#include "songloader.h"

void loadCover(SongData *songdata)
{
    char path[MAXPATHLEN];

    if (songdata->filePath == NULL)
        return;

    generateTempFilePath(songdata->coverArtPath, "cover", ".jpg");
    int res = extractCoverCommand(songdata->filePath, songdata->coverArtPath);
    if (res < 0)
    {
        getDirectoryFromPath(songdata->filePath, path);  
        char *tmp = NULL;
        off_t size;
        tmp = findLargestImageFileRecursive(path, tmp, &size);

        if (tmp != NULL)
            strcpy(songdata->coverArtPath, tmp);
        else
            return;
    }
    else {
        addToCache(tempCache, songdata->coverArtPath);
    }
    songdata->cover = getBitmap(songdata->coverArtPath);
}

void loadColor(SongData *songdata)
{    
    getCoverColor(songdata->cover, &(songdata->red), &(songdata->green), &(songdata->blue));
}

void loadMetaData(SongData *songdata)
{
    songdata->metadata = malloc(sizeof(TagSettings));
    extractTags(songdata->filePath, songdata->metadata);
}

void loadDuration(SongData *songdata)
{
    songdata->duration = (double*)malloc(sizeof(double)); 
    *(songdata->duration) = getDuration(songdata->filePath);
}

void loadPcmAudio(SongData *songdata)
{
    generateTempFilePath(songdata->pcmFilePath, "temp", ".pcm");
    convertToPcmFile(songdata->filePath, songdata->pcmFilePath);
    addToCache(tempCache, songdata->pcmFilePath);
}

SongData* loadSongData(char *filePath)
{
	SongData *songdata = malloc(sizeof(SongData));
	strcpy(songdata->filePath, filePath);
	loadCover(songdata);
	loadColor(songdata);
	loadMetaData(songdata);
	loadDuration(songdata);
	loadPcmAudio(songdata);

	return songdata;
}

int unloadSongData(SongData *songdata)
{
    FreeImage_Unload(songdata->cover); 
    free(songdata->red);
    free(songdata->green);
    free(songdata->blue);
	free(songdata->metadata);
    free(songdata->duration);
    free(songdata);
}