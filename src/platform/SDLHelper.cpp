/**
 *  @file    SDLHelper.cpp
 *  @author  Faaux (github.com/Faaux)
 *  @date    11 February 2018
 */

#include "SDLHelper.h"
#include "engine/Types.h"

#if defined(__WIN32__) || defined(__WINRT__)
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif /*defined(__WIN32__) || defined(__WINRT__)*/

namespace DG
{
/* All of the code is mostly taken from SDLs code but is now using colors to print to Windows
 * Console! */
#if UNICODE
#define WIN_StringToUTF8(S) \
    SDL_iconv_string("UTF-8", "UTF-16LE", (char *)(S), (SDL_wcslen(S) + 1) * sizeof(WCHAR))
#define WIN_UTF8ToString(S) \
    (WCHAR *)SDL_iconv_string("UTF-16LE", "UTF-8", (char *)(S), SDL_strlen(S) + 1)
#else
#define WIN_StringToUTF8(S) SDL_iconv_string("UTF-8", "ASCII", (char *)(S), (SDL_strlen(S) + 1))
#define WIN_UTF8ToString(S) SDL_iconv_string("ASCII", "UTF-8", (char *)(S), SDL_strlen(S) + 1)
#endif

static const char *SDL_priority_prefixes[SDL_NUM_LOG_PRIORITIES] = {
    NULL, "VERBOSE", "DEBUG", "INFO", "WARN", "ERROR", "CRITICAL"};

static const u16 SDL_priority_colors[SDL_NUM_LOG_PRIORITIES] = {0, 8, 10, 15, 14, 12, 12};

#if defined(__WIN32__)
/* Flag tracking the attachment of the console: 0=unattached, 1=attached, -1=error */
static int consoleAttached = 0;

/* Handle to stderr output of console. */
static HANDLE stderrHandle = NULL;
#endif

void LogOutput(void *, int, SDL_LogPriority priority, const char *message)
{
#if defined(__WIN32__) || defined(__WINRT__)
    /* Way too many allocations here, urgh */
    /* Note: One can't call SDL_SetError here, since that function itself logs. */
    {
        char *output;
        size_t length;
        LPTSTR tstr;

#if !defined(HAVE_STDIO_H) && !defined(__WINRT__)
        BOOL attachResult;
        DWORD attachError;
        unsigned long charsWritten;

        /* Maybe attach console and get stderr handle */
        if (consoleAttached == 0)
        {
            attachResult = AttachConsole(ATTACH_PARENT_PROCESS);
            if (!attachResult)
            {
                attachError = GetLastError();
                if (attachError == ERROR_INVALID_HANDLE)
                {
                    OutputDebugString(TEXT("Parent process has no console\r\n"));
                    consoleAttached = -1;
                }
                else if (attachError == ERROR_GEN_FAILURE)
                {
                    OutputDebugString(TEXT("Could not attach to console of parent process\r\n"));
                    consoleAttached = -1;
                }
                else if (attachError == ERROR_ACCESS_DENIED)
                {
                    /* Already attached */
                    consoleAttached = 1;
                }
                else
                {
                    OutputDebugString(TEXT("Error attaching console\r\n"));
                    consoleAttached = -1;
                }
            }
            else
            {
                /* Newly attached */
                consoleAttached = 1;
            }

            if (consoleAttached == 1)
            {
                stderrHandle = GetStdHandle(STD_ERROR_HANDLE);
            }
        }
#endif /* !defined(HAVE_STDIO_H) && !defined(__WINRT__) */

        length = SDL_strlen(SDL_priority_prefixes[priority]) + 2 + SDL_strlen(message) + 1 + 1 + 1;
        output = SDL_stack_alloc(char, length);
        SDL_snprintf(output, length, "%s: %s\r\n", SDL_priority_prefixes[priority], message);
        tstr = WIN_UTF8ToString(output);

        /* Output to debugger */
        OutputDebugString(tstr);

#if !defined(HAVE_STDIO_H) && !defined(__WINRT__)
        /* Screen output to stderr, if console was attached. */
        if (consoleAttached == 1)
        {
            SetConsoleTextAttribute(stderrHandle, SDL_priority_colors[priority]);

            if (!WriteConsole(stderrHandle, tstr, lstrlen(tstr), &charsWritten, NULL))
            {
                OutputDebugString(TEXT("Error calling WriteConsole\r\n"));
                if (GetLastError() == ERROR_NOT_ENOUGH_MEMORY)
                {
                    OutputDebugString(TEXT("Insufficient heap memory to write message\r\n"));
                }
            }
        }
#endif /* !defined(HAVE_STDIO_H) && !defined(__WINRT__) */

        SDL_free(tstr);
        SDL_stack_free(output);
    }
#elif defined(__ANDROID__)
    {
        char tag[32];

        SDL_snprintf(tag, SDL_arraysize(tag), "SDL/%s", GetCategoryPrefix(category));
        __android_log_write(SDL_android_priority[priority], tag, message);
    }
#elif defined(__APPLE__) && defined(SDL_VIDEO_DRIVER_COCOA)
    /* Technically we don't need SDL_VIDEO_DRIVER_COCOA, but that's where this function is defined
     * for now.
     */
    extern void SDL_NSLog(const char *text);
    {
        char *text;

        text = SDL_stack_alloc(char, SDL_MAX_LOG_MESSAGE);
        if (text)
        {
            SDL_snprintf(text, SDL_MAX_LOG_MESSAGE, "%s: %s", SDL_priority_prefixes[priority],
                         message);
            SDL_NSLog(text);
            SDL_stack_free(text);
            return;
        }
    }
#elif defined(__PSP__)
    {
        FILE *pFile;
        pFile = fopen("SDL_Log.txt", "a");
        fprintf(pFile, "%s: %s\n", SDL_priority_prefixes[priority], message);
        fclose(pFile);
    }
#endif
#if HAVE_STDIO_H
    fprintf(stderr, "%s: %s\n", SDL_priority_prefixes[priority], message);
#if __NACL__
    fflush(stderr);
#endif
#endif
}

bool InitSDL()
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "SDL could not initialize! SDL Error: %s\n",
                        SDL_GetError());
        return false;
    }
#if _DEBUG
    SDL_LogSetAllPriority(SDL_LOG_PRIORITY_VERBOSE);
#else
    SDL_LogSetAllPriority(SDL_LOG_PRIORITY_WARN);
#endif
    SDL_LogSetOutputFunction(LogOutput, nullptr);

    // When in fullscreen dont minimize when loosing focus! (Borderless windowed)
    SDL_SetHint(SDL_HINT_VIDEO_MINIMIZE_ON_FOCUS_LOSS, "0");

    SDL_LogVerbose(0, "----- Hardware Information -----");
    SDL_LogVerbose(0, "CPU Cores: %i", SDL_GetCPUCount());
    SDL_LogVerbose(0, "CPU Cache Line Size: %i", SDL_GetCPUCacheLineSize());

    return true;
}
}  // namespace DG
