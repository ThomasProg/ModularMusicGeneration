// Source : https://github.com/abique/midi-parser

#include <sys/types.h>
#include <sys/stat.h>
// #include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <fstream>

#ifdef _WIN32
#include <windows.h>
#include <memoryapi.h>

static void win_err_helper(const char *func, const char *path)
{
  const DWORD flags = FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
    FORMAT_MESSAGE_IGNORE_INSERTS;
  const DWORD dw   = GetLastError();
  const DWORD lang = MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT);

  LPVOID lpMsgBuf;
  FormatMessage(flags, NULL, dw, lang, (LPTSTR)&lpMsgBuf, 0, NULL);

  printf("%s(%s): %s\n", func, path, (char *)lpMsgBuf);
  LocalFree(lpMsgBuf);
}

#else
#include <sys/mman.h>
#endif

#include "mp.h"
#include "dump.h"
#include "Debug.h"

static void usage(const char *prog)
{
  printf("usage: %s <file.midi>\n", prog);
}

static void parse_and_dump(struct midi_parser *parser)
{
  enum midi_parser_status status;

  while (1) {
    status = midi_parse(parser);
    switch (status) {
    case MIDI_PARSER_EOB:
      puts("eob");
      return;

    case MIDI_PARSER_ERROR:
      puts("error");
      return;

    case MIDI_PARSER_INIT:
      puts("init");
      break;

    case MIDI_PARSER_HEADER:
      printf("header\n");
      printf("  size: %d\n", parser->header.size);
      printf("  format: %d [%s]\n", parser->header.format, midi_file_format_name(parser->header.format));
      printf("  tracks count: %d\n", parser->header.tracks_count);
      printf("  time division: %d\n", parser->header.time_division);
      break;

    case MIDI_PARSER_TRACK:
    //   puts("track");
    //   printf("  length: %d\n", parser->track.size);
      cout << "track" << '\n';
      cout << "  length: " << parser->track.size << '\n';
      break;

    case MIDI_PARSER_TRACK_MIDI:
    //   puts("track-midi");
      cout << "track-midi" << '\n';
        cout << "  time: " << parser->vtime << '\n';
        cout << "  status: " << (int) parser->midi.status << "[" << midi_status_name(parser->midi.status) << "]" << '\n';
        cout << "  channel: " << (int) parser->midi.channel << '\n';
        cout << "  param1: " << (int) parser->midi.param1 << '\n';
        cout << "  param2: " << (int) parser->midi.param2 << '\n';
    //   printf("  time: %ld\n", parser->vtime);
    //   printf("  status: %d [%s]\n", parser->midi.status, midi_status_name(parser->midi.status));
    //   printf("  channel: %d\n", parser->midi.channel);
    //   printf("  param1: %d\n", parser->midi.param1);
    //   printf("  param2: %d\n", parser->midi.param2);
      break;

    case MIDI_PARSER_TRACK_META:
    //   printf("track-meta\n");
      cout << "track-meta" << '\n';
        cout << "  time: " << parser->vtime << '\n';
        cout << "  type: " << (int) parser->meta.type << "[" << midi_meta_name(parser->meta.type) << "]" << '\n';
        cout << "  length: " << parser->meta.length << '\n';

        if (parser->meta.type == MIDI_META_TRACK_NAME)
        {
          cout << std::string((const char*)parser->meta.bytes, parser->meta.length) << '\n';
        }
        if (parser->meta.type == MIDI_META_TEXT)
        {
          cout << std::string((const char*)parser->meta.bytes, parser->meta.length) << '\n';
        }
    //   printf("  time: %ld\n", parser->vtime);
    //   printf("  type: %d [%s]\n", parser->meta.type, midi_meta_name(parser->meta.type));
    //   printf("  length: %d\n", parser->meta.length);
      break;

    case MIDI_PARSER_TRACK_SYSEX:
    //   puts("track-sysex");
        cout << "track-sysex" << '\n';
        cout << "  time: " << parser->vtime << '\n';
    //   printf("  time: %ld\n", parser->vtime);
      break;

    default:
    //   printf("unhandled state: %d\n", status);
        cout << "unhandled state: " << (int) status << '\n';
      return;
    }
  }
}

int parse_file(const char *path)
{
//   struct stat st;

//   if (stat(path, &st)) {
//     printf("stat(%s): %m\n", path);
//     return 1;
//   }

//   int fd = open(path, O_RDONLY);
//   if (fd < 0) {
//     printf("open(%s): %m\n", path);
//     return 1;
//   }

// #ifdef _WIN32

//   HANDLE fhandle = (HANDLE)_get_osfhandle(fd);

//   if (st.st_size == 0) {
//     printf("file is empty\n");
//     close(fd);
//     return 1;
//   }

//   HANDLE hMapFile = CreateFileMapping(fhandle, NULL, PAGE_READONLY, 0, 0, NULL);

//   if (!hMapFile) {
//     win_err_helper("CreateFileMapping", path);
//     close(fd);
//     return 1;
//   }

//   void *mem = MapViewOfFile(hMapFile, FILE_MAP_READ, 0, 0, 0);

//   if (!mem) {
//     win_err_helper("MapViewOfFile", path);
//     CloseHandle(hMapFile);
//     close(fd);
//     return 1;
//   }

// #else

//   void *mem = mmap(NULL, st.st_size, PROT_READ, MAP_SHARED, fd, 0);
//   if (mem == MAP_FAILED) {
//     printf("mmap(%s): %m\n", path);
//     close(fd);
//     return 1;
//   }

// #endif

    std::ifstream file (path, std::ios::in|std::ios::binary|std::ios::ate);
    // if (file.is_open())
    // {
        size_t size = file.tellg();
        char* memblock = new char [size];
        file.seekg (0, std::ios::beg);
        file.read (memblock, size);
        file.close();
    // }
    // else
    // {
    //     throw std::runtime_error("Couldn't open file : " + std::string(filename));
    // }

  struct midi_parser parser;
  parser.state = MIDI_PARSER_INIT;
  parser.size  = size;
  parser.in    = (const uint8_t*)memblock;

  parse_and_dump(&parser);

delete[] memblock;

// #ifdef _WIN32
//   UnmapViewOfFile(mem);
//   CloseHandle(hMapFile);
// #else
//   munmap(mem, st.st_size);
// #endif
//   close(fd);
  return 0;
}

// int main(int argc, char **argv)
// {
//   if (argc != 2) {
//     usage(argv[0]);
//     return 1;
//   }

//   return parse_file(argv[1]);
// }