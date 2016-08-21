#pragma once

#include <malloc.h>
#include <memory>

namespace Core {

class Buffer {
public:
  Buffer() : m_size_(0), m_align_(0), m_buffer_() {
  }

  Buffer(size_t size, size_t align) : m_size_(size), m_align_(align), m_buffer_(AllocateBuffer(size, align)) {
  }

  Buffer(size_t size, size_t align, void* initial_data) : Buffer(size, align) {
    CopyDataToBuffer(initial_data, size);
  }

  template<typename T>
  Buffer(T* initial_data) : Buffer(sizeof(T), alignof(T), initial_data) {
  }

  ~Buffer() = default;

  Buffer(const Buffer& other) = delete;
  Buffer& operator=(const Buffer& other) = delete;

  Buffer(Buffer&& other) = default;
  Buffer& operator=(Buffer&& other) = default;

  void Reset(size_t size, size_t align) {
    m_size_ = size;
    m_align_ = align;
    m_buffer_.reset(AllocateBuffer(size, align));
  }

  void Reset(size_t size, size_t align, void* data) {
    Reset(size, align);
    CopyDataToBuffer(data, size);
  }

  template<typename T>
  void Reset(T* data) {
    Reset(sizeof(T), alignof(T), data);
  }

  size_t GetSize() const {
    return m_size_;
  }

  size_t GetAlign() const {
    return m_align_;
  }

  void* GetBuffer() const {
    if (m_buffer_) {
      return m_buffer_.get();
    }
    return nullptr;
  }

private:
  struct BufferDeleter {
    void operator()(void* ptr) {
      _aligned_free(ptr);
    }
  };

  static void* AllocateBuffer(size_t size, size_t align) {
    return _aligned_malloc(size, align);
  }

  void CopyDataToBuffer(void* data, size_t size) {
    if (data != nullptr) {
      std::memcpy(m_buffer_.get(), data, size);
    }
  }

  size_t m_size_ = 0;
  size_t m_align_ = 0;
  std::unique_ptr<void, BufferDeleter> m_buffer_ = {};
};

}  // namespace Core
