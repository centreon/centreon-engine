/*
** Copyright 2011-2014 Centreon
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
**
** For more information : contact@centreon.com
*/

#include "com/centreon/engine/logging/file.hh"
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include "com/centreon/concurrency/locker.hh"
#include "com/centreon/exceptions/basic.hh"
#include "com/centreon/misc/stringifier.hh"

using namespace com::centreon::engine::logging;

/**
 *  Default constructor.
 *
 *  @param[in] file            The file to used.
 *  @param[in] is_sync         Enable synchronization.
 *  @param[in] show_pid        Enable show pid.
 *  @param[in] show_timestamp  Enable show timestamp.
 *  @param[in] show_thread_id  Enable show thread id.
 *  @param[in] max_size        Maximum file size.
 */
file::file(FILE* file,
           bool is_sync,
           bool show_pid,
           time_precision show_timestamp,
           bool show_thread_id,
           long long max_size)
    : backend(is_sync, show_pid, show_timestamp, show_thread_id),
      _max_size(max_size),
      _out(file),
      _size(0) {}

/**
 *  Constructor with file path name.
 *
 *  @param[in] path            The path of the file to used.
 *  @param[in] is_sync         Enable synchronization.
 *  @param[in] show_pid        Enable show pid.
 *  @param[in] show_timestamp  Enable show timestamp.
 *  @param[in] show_thread_id  Enable show thread id.
 *  @param[in] max_size        Maximum file size.
 */
file::file(std::string const& path,
           bool is_sync,
           bool show_pid,
           time_precision show_timestamp,
           bool show_thread_id,
           long long max_size)
    : backend(is_sync, show_pid, show_timestamp, show_thread_id),
      _max_size(max_size),
      _path(path),
      _out(NULL),
      _size(0) {
  open();
}

/**
 *  Default destructor.
 */
file::~file() throw() {
  close();
}

/**
 *  Close file.
 */
void file::close() throw() {
  concurrency::locker lock(&_lock);

  if (!_out || _out == stdout || _out == stderr)
    return;

  int ret;
  do {
    ret = fclose(_out);
  } while (ret == -1 && errno == EINTR);
  _out = NULL;
}

/**
 *  Get filename.
 *
 *  @return The filename string.
 */
std::string const& file::filename() const throw() {
  return (_path);
}

/**
 *  Write message into the file.
 *  @remark This method is thread safe.
 *
 *  @param[in] type     Logging types.
 *  @param[in] verbose  Verbosity level.
 *  @param[in] msg      The message to write.
 *  @param[in] size     The message's size.
 */
void file::log(unsigned long long types,
               unsigned int verbose,
               char const* msg,
               unsigned int size) throw() {
  (void)types;
  (void)verbose;
  (void)size;

  misc::stringifier header;
  _build_header(header);

  // Split msg by line.
  misc::stringifier buffer;
  unsigned int i(0);
  unsigned int last(0);
  while (msg[i]) {
    if (msg[i] == '\n') {
      buffer << header;
      buffer.append(msg + last, i - last) << "\n";
      last = i + 1;
    }
    ++i;
  }
  if (last != i) {
    buffer << header;
    buffer.append(msg + last, i - last) << "\n";
  }

  concurrency::locker lock(&_lock);
  if (_out) {
    // Size control.
    if ((_max_size > 0) && (_size + buffer.size() > _max_size))
      _max_size_reached();
    _size += buffer.size();

    // Physical write.
    size_t ret;
    do {
      clearerr(_out);
      ret = fwrite(buffer.data(), buffer.size(), 1, _out);
    } while (ret != 1 && ferror(_out) && errno == EINTR);

    // Flush data if is necessary.
    while (_is_sync && fflush(_out) < 0 && errno == EINTR)
      ;
  }
}

/**
 *  Open file.
 */
void file::open() {
  concurrency::locker lock(&_lock);

  if (_out && _path.empty())
    return;

  if (!(_out = fopen(_path.c_str(), "a")))
    throw(basic_error() << "failed to open file '" << _path
                        << "': " << strerror(errno));
  _size = ftell(_out);

  return;
}

/**
 *  Close and open file.
 */
void file::reopen() {
  concurrency::locker lock(&_lock);

  if (!_out || _out == stdout || _out == stderr)
    return;

  int ret;
  do {
    ret = fclose(_out);
  } while (ret == -1 && errno == EINTR);

  if (!(_out = fopen(_path.c_str(), "a")))
    throw(basic_error() << "failed to open file '" << _path
                        << "': " << strerror(errno));
  _size = ftell(_out);

  return;
}

/**
 *  Method called when max size is reached.
 */
void file::_max_size_reached() {
  close();
  remove(_path.c_str());
  open();
  return;
}
