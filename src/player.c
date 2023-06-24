#include <string.h>
#include "player.h"

const char VERSION[] = "0.9.0";

bool firstSong = true;
bool refresh = true;
bool equalizerEnabled = true;
bool equalizerBlocks = true;
bool coverEnabled = true;
bool coverBlocks = true;
bool metaDataEnabled = true;
bool timeEnabled = true;
bool drewCover = true;
bool printInfo = false;
bool showList = true;
int equalizerHeight = 8;
int minWidth = 31;
int minHeight = 2;
int coverRow = 0;
int preferredWidth = 0;
int preferredHeight = 0;
int elapsed = 0;
int duration = 0;
int maxListSize = 30;
char *path;
char *tagsPath;
double totalDurationSeconds = 0.0;
PixelData color = { 0, 0, 0 };
PixelData bgColor = { 50, 50, 50 };
TagSettings metadata = {};
char latestVersion[10] = "\0";
struct ResponseData
{
    char *content;
    size_t size;
};

const char githubLatestVersionUrl[] = "https://api.github.com/repos/ravachol/cue/releases/latest";

int calcMetadataHeight()
{
    int term_w, term_h;
    getTermSize(&term_w, &term_h);
    size_t titleLength = strlen(metadata.title);
    int titleHeight = (int)ceil((float)titleLength / term_w);
    size_t artistLength = strlen(metadata.artist);
    int artistHeight = (int)ceil((float)artistLength / term_w);
    size_t albumLength = strlen(metadata.album);
    int albumHeight = (int)ceil((float)albumLength / term_w);
    int yearHeight = 1;

    return titleHeight + artistHeight + albumHeight + yearHeight;
}

void calcPreferredSize()
{
    minHeight = 2 + (equalizerEnabled ? equalizerHeight : 0);    
    calcIdealImgSize(&preferredWidth, &preferredHeight, (equalizerEnabled ? equalizerHeight : 0), calcMetadataHeight(), firstSong);
}

void printCover()
{
    clearRestOfScreen();
    if (coverEnabled)
    {
        color.r = 0;
        color.g = 0;
        color.b = 0;
        displayAlbumArt(path, preferredWidth, preferredHeight, coverBlocks, &color);
        drewCover = true;
        firstSong = false;
    }
    else
    {
        clearRestOfScreen();
        drewCover = false;
    }
}

void setColor()
{
    if (color.r == 0 && color.g == 0 && color.b == 0)
        setDefaultTextColor();
    else
        setTextColorRGB2(color.r, color.g, color.b);
}

void printBasicMetadata(TagSettings *metadata)
{
    if (strlen(metadata->title) > 0)
    {
        PixelData pixel = increaseLuminosity(color, 100);
        setTextColorRGB2(pixel.r, pixel.g, pixel.b);        
        printf(" %s\n", metadata->title);
    }
    setColor();    
    if (strlen(metadata->artist) > 0)
        printf(" %s\n", metadata->artist);
    if (strlen(metadata->album) > 0)
        printf(" %s\n", metadata->album);
    if (strlen(metadata->date) > 0)
    {
        int year = getYear(metadata->date);
        if (year == -1)
            printf(" %s\n", metadata->date);
        else
            printf(" %d\n", year);
    }
    fflush(stdout);
}

void printProgress(double elapsed_seconds, double total_seconds, double total_duration_seconds)
{
    int progressWidth = 31;
    int term_w, term_h;
    getTermSize(&term_w, &term_h);

    if (term_w < progressWidth)
        return;

    // Save the current cursor position
    printf("\033[s");

    int elapsed_hours = (int)(elapsed_seconds / 3600);
    int elapsed_minutes = (int)(((int)elapsed_seconds / 60) % 60);
    int elapsed_seconds_remainder = (int)elapsed_seconds % 60;

    int total_hours = (int)(total_seconds / 3600);
    int total_minutes = (int)(((int)total_seconds / 60) % 60);
    int total_seconds_remainder = (int)total_seconds % 60;

    int progress_percentage = (int)((elapsed_seconds / total_seconds) * 100);

    int total_playlist_hours = (int)(total_duration_seconds / 3600);
    int total_playlist_minutes = (int)(((int)total_duration_seconds / 60) % 60);

    // Clear the current line
    printf("\033[K");
    printf("\r");
    printf(" %02d:%02d:%02d / %02d:%02d:%02d (%d%%) T:%dh%02dm",
           elapsed_hours, elapsed_minutes, elapsed_seconds_remainder,
           total_hours, total_minutes, total_seconds_remainder,
           progress_percentage, total_playlist_hours, total_playlist_minutes);
    fflush(stdout);

    // Restore the cursor position
    printf("\033[u");
    fflush(stdout);
}

void printMetadata(TagSettings *metadata)
{
    if (!metaDataEnabled || printInfo) return;
    usleep(100000);
    setColor();  
    printBasicMetadata(metadata);
}

void printTime()
{
    if (!timeEnabled || printInfo) return;
    setColor();    
    int term_w, term_h;
    getTermSize(&term_w, &term_h);
    if (term_h > minHeight && term_w > minWidth)
        printProgress(elapsed, duration, totalDurationSeconds);
}

void printLastRow()
{
    int minWidth = 30;
    int term_w, term_h;
    getTermSize(&term_w, &term_h);
    if (term_w < minWidth)
        return;
    setTextColorRGB2(bgColor.r, bgColor.g, bgColor.b);
    printf(" [F1 About] [q Quit] cue v%s", VERSION);
}

// Callback function to write response data
size_t WriteCallback(char *content, size_t size, size_t nmemb, void *userdata)
{
    size_t totalSize = size * nmemb;
    struct ResponseData *responseData = (struct ResponseData *)userdata;
    responseData->content = realloc(responseData->content, responseData->size + totalSize + 1);
    if (responseData->content == NULL)
    {
        printf("Failed to allocate memory for response data.\n");
        return 0;
    }
    memcpy(&(responseData->content[responseData->size]), content, totalSize);
    responseData->size += totalSize;
    responseData->content[responseData->size] = '\0';
    return totalSize;
}

// Function to get the latest version from GitHub
int fetchLatestVersion(int *major, int *minor, int *patch)
{
    CURL *curl = curl_easy_init();
    if (!curl)
    {
        printf("Failed to initialize cURL.\n");
        return -1;
    }

    // Set the request URL
    curl_easy_setopt(curl, CURLOPT_URL, githubLatestVersionUrl);

    // Set the write callback function
    struct ResponseData responseData;
    responseData.content = malloc(1); // Initial memory allocation
    responseData.size = 0;
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseData);

    // Perform the request
    CURLcode res = curl_easy_perform(curl);

    if (res != CURLE_OK)
    {
        printf("Failed to perform cURL request: %s\n", curl_easy_strerror(res));
        curl_easy_cleanup(curl);
        free(responseData.content);
        return -1;
    }

    long response_code;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);

    curl_easy_cleanup(curl);

    if (response_code == 200)
    {
        free(responseData.content);

        return 0;
    }
    else
    {
        free(responseData.content);
        return -1;
    }
}

void showVersion(PixelData color, PixelData secondaryColor)
{
    int major, minor, patch;

    if (latestVersion == '\0' && fetchLatestVersion(&major, &minor, &patch) >= 0)
    {
        sprintf(latestVersion, "%d.%d.%d", major, minor, patch);
    }
    else
    {
        sprintf(latestVersion, VERSION);
    }
    printf("\n");
    printVersion(VERSION, latestVersion, color, secondaryColor);
}

int showPlaylist(int maxHeight)
{
    Node *node = playlist.head;
    int numRows = 0;
    if (node == NULL) return numRows;
    printf("\n");
    numRows++;    
    for (int i = 0; i < maxListSize; i++)
    {
        if (node == NULL) break;
        char filePath[MAXPATHLEN];
        strcpy(filePath, node->song.filePath);
        char *lastSlash = strrchr(filePath, '/');
        char *lastDot = strrchr(filePath, '.');
        printf("\r");
        if (lastSlash != NULL && lastDot != NULL && lastDot > lastSlash) {
            char copiedString[256];
            strncpy(copiedString, lastSlash + 1, lastDot - lastSlash - 1);
            copiedString[lastDot - lastSlash - 1] = '\0';
            if (copiedString != NULL)
                printf(" %s\n", copiedString);
            numRows++;
        }
        node = node->next;        
    }
    printf("\n");
    numRows++;
    return numRows;
}

void printAbout()
{
    usleep(200000);
    printf("\n\r\n");
    clearRestOfScreen();
    if (refresh && printInfo)
    {
        PixelData textColor = increaseLuminosity(color, 100);
        setTextColorRGB2(textColor.r, textColor.g, textColor.b);      
        printAsciiLogo();
        setTextColorRGB2(color.r, color.g, color.b);
        showVersion(textColor,color);
        int versionHeight = 12;
        int emptyRows = (equalizerEnabled ? equalizerHeight : 0) + 
                        (metaDataEnabled ? calcMetadataHeight() : 0) + 
                        (timeEnabled ? 1 : 0) - 
                        versionHeight;
        fflush(stdout);
    }
}

void printEqualizer()
{   
    if (equalizerEnabled && !printInfo)
    {
        printf("\n");        
        int term_w, term_h;
        getTermSize(&term_w, &term_h);
        drawEqualizer(equalizerHeight, term_w -1, equalizerBlocks, color);            
        printLastRow();       
        int jumpAmount = equalizerHeight + 2;
        cursorJump(jumpAmount);
        saveCursorPosition();                   
    }  
}

int printPlayer(const char *songFilepath, TagSettings *metadata, double elapsedSeconds, double songDurationSeconds, PlayList *playlist)
{    
    hideCursor();    
    path = strdup(songFilepath);
    totalDurationSeconds = playlist->totalDuration;
    elapsed = elapsedSeconds;
    duration = songDurationSeconds;

    calcPreferredSize();

    if (preferredWidth <= 0 || preferredHeight <= 0) return -1;

    if (printInfo)
    {
        if (refresh)
        {
            printAbout();  
        }     
        int height = showPlaylist(equalizerHeight + 4);
        if (height == 0)
            printCover();
        printLastRow();        
        int jumpAmount = height;
        cursorJump(jumpAmount);
        saveCursorPosition(); 
    }
    else
    {
        if (refresh)
        {
            printCover();               
            printMetadata(metadata);
        }  
        printTime();
        printEqualizer();
    }
    refresh = false;

    return 0; 
}

void showHelp()
{
    printHelp();
}