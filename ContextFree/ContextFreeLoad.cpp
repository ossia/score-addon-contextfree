#include "ContextFreeLoad.hpp"

#if defined(_WIN32)

#if !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN
#endif
#if !defined(NOMINMAX)
#define NOMINMAX
#endif
#if !defined(UNICODE)
#define UNICODE 1
#endif
#if !defined(_UNICODE)
#define _UNICODE 1
#endif
// clang-format off
#include <windows.h>
#include <shlwapi.h>
// clang-format on
#else

#include <sys/stat.h>
#include <sys/types.h>

#include <dirent.h>
#include <unistd.h>
#endif

#include <abstractPngCanvas.h>
#include <astexpression.h>
#include <cfdg.h>

#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#if defined(__GNU__) || (defined(__ILP32__) && defined(__x86_64__))
#define NOSYSCTL
#else
#ifndef __linux__
#include <sys/sysctl.h>
#endif
#endif

namespace
{
struct MallocDeleter
{
  void operator()(void* ptr) const { std::free(ptr); }
};
struct DirCloser
{
  void operator()(DIR* ptr) const
  {
    (void)closedir(ptr); // not called if nullptr
  }
};

/* The following code declares classes to read from and write to
 * file descriptore or file handles.
 *
 * See
 *      http://www.josuttis.com/cppcode
 * for details and the latest version.
 *
 * - open:
 *      - integrating BUFSIZ on some systems?
 *      - optimized reading of multiple characters
 *      - stream for reading AND writing
 *      - i18n
 *
 * (C) Copyright Nicolai M. Josuttis 2001.
 * Permission to copy, use, modify, sell and distribute this software
 * is granted provided this copyright notice appears in all copies.
 * This software is provided "as is" without express or implied
 * warranty, and with no claim as to its suitability for any purpose.
 *
 * Version: Jul 28, 2002
 * History:
 *  Jul 28, 2002: bugfix memcpy() => memmove()
 *                fdinbuf::underflow(): cast for return statements
 *  Aug 05, 2001: first public version
 */
class fdoutbuf : public std::streambuf
{
public:
  int fd; // file descriptor
  // constructor
  fdoutbuf(int _fd)
      : fd(_fd)
  {
  }

protected:
  // write one character
  virtual int_type overflow(int_type c)
  {
    if(c != EOF)
    {
      char z = c;
      if(write(fd, &z, 1) != 1)
      {
        return EOF;
      }
    }
    return c;
  }
  // write multiple characters
  virtual std::streamsize xsputn(const char* s, std::streamsize num)
  {
    return write(fd, s, num);
  }
};
class fdostream : public std::ostream
{
protected:
  fdoutbuf buf;

public:
  fdostream()
      : std::ostream(0)
      , buf(-1)
  {
  }
  fdostream(int fd)
      : std::ostream(0)
      , buf(fd)
  {
    rdbuf(&buf);
  }
  void setfd(int _fd)
  {
    buf.fd = _fd;
    rdbuf(&buf);
  }
};
}

class OssiaSystem : public AbstractSystem
{
protected:
  virtual void clearAndCR() override { }

public:
  OssiaSystem() { }
  ~OssiaSystem() { }

  void catastrophicError(const char* what) override { }
  istr_ptr tempFileForRead(const FileString& path) override
  {
    return std::make_unique<std::ifstream>(path.c_str(), std::ios::binary);
  }

  ostr_ptr tempFileForWrite(TempType tt, FileString& nameOut) override
  {
#if defined(_WIN32)
    const FileChar* wtempdir = tempFileDirectory();

    std::unique_ptr<wchar_t, MallocDeleter> b{_wtempnam(wtempdir, TempPrefixes[tt])};
    if(!b)
      return nullptr;
    FileString bcopy = b.get();
    bcopy.append(TempSuffixes[tt]);
    ostr_ptr f = std::make_unique<std::ofstream>(
        bcopy, std::ios::binary | std::ios::trunc | std::ios::out);
    nameOut.assign(bcopy);

    return f;
#else
    std::string t(tempFileDirectory());
    if(t.back() != '/')
      t.push_back('/');
    t.append(TempPrefixes[tt]);
    t.append("XXXXXX");
    t.append(TempSuffixes[tt]);

    ostr_ptr f;

    std::unique_ptr<char, MallocDeleter> b(strdup(t.c_str()));
    int tfd = mkstemps(b.get(), (int)std::strlen(TempSuffixes[tt]));
    if(tfd != -1)
    {
      f = std::make_unique<fdostream>(tfd);
      nameOut.assign(b.get());
    }

    return f;
#endif
  }
  const FileChar* tempFileDirectory() override
  {
    struct stat sb;
    const char* tmpenv = getenv("TMPDIR");
    if(!tmpenv || stat(tmpenv, &sb) || !S_ISDIR(sb.st_mode))
      tmpenv = getenv("TEMP");
    if(!tmpenv || stat(tmpenv, &sb) || !S_ISDIR(sb.st_mode))
      tmpenv = getenv("TMP");
    if(!tmpenv || stat(tmpenv, &sb) || !S_ISDIR(sb.st_mode))
      tmpenv = "/tmp/"; // give up
    return tmpenv;
  }
  std::vector<FileString> findTempFiles() override
  {
    std::vector<FileString> ret;

#if defined(_WIN32)
    const FileChar* tempdir = tempFileDirectory();

    std::array<wchar_t, 32768> wtempdir;
    if(wcscpy_s(wtempdir.data(), wtempdir.size(), tempFileDirectory())
       || !::PathAppendW(wtempdir.data(), TempPrefixAll)
       || wcsncat_s(wtempdir.data(), wtempdir.size(), L"*", 1))
      return ret;

    ::WIN32_FIND_DATAW ffd;
    std::unique_ptr<void, FindCloser> fff(::FindFirstFileW(wtempdir.data(), &ffd));
    if(fff.get() == INVALID_HANDLE_VALUE)
    {
      fff.release(); // Don't call FindClose() if invalid
      return ret;    // Return empty
    }

    do
    {
      ret.emplace_back(tempdir);
      if(ret.back().back() != L'\\')
        ret.back().append(L"\\");
      ret.back().append(ffd.cFileName);
    } while(::FindNextFileW(fff.get(), &ffd));
#else
    const char* dirname = tempFileDirectory();
    std::size_t len = std::strlen(TempPrefixAll);
    std::unique_ptr<DIR, DirCloser> dirp(opendir(dirname));
    if(!dirp)
      return ret;
    while(dirent* der = readdir(dirp.get()))
    {
      if(std::strncmp(TempPrefixAll, der->d_name, len) == 0)
      {
        ret.emplace_back(dirname);
        if(ret.back().back() != '/')
          ret.back().push_back('/');
        ret.back().append(der->d_name);
      }
    }

#endif
    return ret;
  }

  std::string relativeFilePath(const std::string& base, const std::string& rel) override
  {
    std::string s = base;

    std::string::size_type i = s.rfind('/');
    if(i == std::string::npos)
    {
      return rel;
    }
    i += 1;
    s.replace(i, s.length() - i, rel);
    return s;
  }

  int deleteTempFile(const FileString& name) override
  {
#if defined(_WIN32)
    return _wremove(name.c_str());
#else
    return remove(name.c_str());
#endif
  }
  std::size_t getPhysicalMemory() override
  {
#ifdef _WIN32
    {
      MEMORYSTATUSEX status;
      status.dwLength = sizeof(status);
      GlobalMemoryStatusEx(&status);
      if(!SystemIs64bit && status.ullTotalPhys > 2147483648ULL)
        return (size_t)2147483648ULL;
      return (size_t)status.ullTotalPhys;
    }
#else
#ifdef NOSYSCTL
    return 0;
#elif defined(__linux__)
#if defined(_SC_PHYS_PAGES) && defined(_SC_PAGESIZE)
    std::uint64_t size
        = sysconf(_SC_PHYS_PAGES) * static_cast<std::uint64_t>(sysconf(_SC_PAGESIZE));
    if(size > MaximumMemory)
      size = MaximumMemory;
    return static_cast<std::size_t>(size);
#else
    return 0;
#endif
#else // not __linux__ and not NOSYSCTL
    int mib[2];
#ifdef CTL_HW
    mib[0] = CTL_HW;
#else
    return 0;
#endif
#if defined(HW_MEMSIZE)
    mib[1] = HW_MEMSIZE;    // OSX
    std::uint64_t size = 0; // 64-bit
#elif defined(HW_PHYSMEM64)
    mib[1] = HW_PHYSMEM64;  // NetBSD, OpenBSD
    std::uint64_t size = 0; // 64-bit
#elif defined(HW_REALMEM)
    mib[1] = HW_REALMEM;   // FreeBSD
    unsigned int size = 0; // 32-bit
#elif defined(HW_PHYSMEM)
    mib[1] = HW_PHYSMEM;   // DragonFly BSD
    unsigned int size = 0; // 32-bit
#else
    std::uint64_t size = 0; // need to define this anyway
    return 0;
#endif
    std::size_t len = sizeof(size);
    if(sysctl(mib, 2, &size, &len, NULL, 0) == 0)
    {
      if(size > MaximumMemory)
        size = MaximumMemory;
      return static_cast<std::size_t>(size);
    }
    return 0;
#endif // __linux__ || NOSYSCTL
#endif
  }

  std::wstring normalize(const std::string& s) override
  {
    return QString::fromStdString(s)
        .normalized(QString::NormalizationForm{})
        .toStdWString();
  }

  void message(const char* fmt, ...) override { }
  void syntaxError(const CfdgError& err) override { }
  bool error(bool errorOccurred = true) override { return true; }
  bool isGuiProgram() override { return true; }
  void stats(const Stats&) override { }
  void orphan() override { }
};

struct options
{
  std::string input;
  std::string definitions;
  int width = 1280;
  int height = 720;
  int minSize = 0;
  int borderSize = 1;
  int animationFrames = 0;
};

struct OssiaCanvas : abstractPngCanvas
{
  using abstractPngCanvas::abstractPngCanvas;
  void output(const char* outfilename, int frame = -1) override { }

  using abstractPngCanvas::mData;
};

RenderResult contextfree_render_file(std::string_view path, int w, int h, int variation)
{
  options opts;
  opts.input = path;
  opts.width = w;
  opts.height = h;

  OssiaSystem system{};
  auto design
      = CFDG::ParseFile(opts.input.c_str(), &system, variation, opts.definitions);
  if(!design)
    return {};

  design->usesAlpha = true;
  design->usesColor = true;

  auto renderer = design->renderer(
      design, opts.width, opts.height, opts.minSize, variation, opts.borderSize);
  if(!renderer)
    return {};

  opts.width = renderer->m_width;
  opts.height = renderer->m_height;

  renderer->run(nullptr, false);

  auto canvas = std::make_unique<OssiaCanvas>(
      "", true, opts.width, opts.height, aggCanvas::PixelFormat::RGBA8_Blend, false,
      opts.animationFrames, variation, false, renderer.get(), 1, 1);

  if(canvas->mWidth != opts.width || canvas->mHeight != opts.height)
  {
    renderer->resetSize(canvas->mWidth, canvas->mHeight);
    opts.width = renderer->m_width;
    opts.height = renderer->m_height;
  }
  renderer->draw(canvas.get());

  return {
      .bytes = std::move(canvas->mData),
      .fmt = QImage::Format::Format_ARGB32,
      .width = opts.width,
      .height = opts.height};
}
