#include <string>
#include <iostream>

template <typename M>
class my_lock_guard : public std::lock_guard<M> {
  std::string _file;
  int _line;
  M* _m;

 public:
  my_lock_guard<M>(M& m, const char* file, int line) : std::lock_guard<M>(m), _file{file}, _line{line}, _m(&m) {
    std::cout << "Lock(G): " << _file << ": " << _m << ":" << _line << std::endl;
  }

  ~my_lock_guard() {
    std::cout << "Unlock(G): " << _file << ": " << _m << ":" << _line << std::endl;
  }
};

template <typename M>
class my_unique_lock : public std::unique_lock<M> {
  std::string _file;
  int _line;
  M* _m;

 public:
  my_unique_lock<M>(M& m, const char* file, int line) : std::unique_lock<M>(m), _file{file}, _line{line}, _m{&m} {
    std::cout << "Lock(U): " << _file << ": " << _m << ":" << _line << std::endl;
  }

  ~my_unique_lock() {
    std::cout << "Destroy Lock(U): " << _file << ": " << _m << ":" << _line << std::endl;
  }

  void unlock(int line) {
    std::cout << "unlock(): " << _file << ": " << _m << ":" << line << std::endl;
    std::unique_lock<M>::unlock();
  }

  void lock(int line) {
    std::cout << "lock(): " << _file << ": " << _m << ":" << line << std::endl;
    std::unique_lock<M>::lock();
  }
};
