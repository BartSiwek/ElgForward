#pragma once

#include <malloc.h>
#include <memory>

namespace Core {

class Buffer {
 public:
  Buffer() : m_size_(0), m_align_(), m_cpu_buffer_() {
  }

  Buffer(size_t size, size_t align) : m_size_(size), m_align_(align), m_cpu_buffer_(_aligned_malloc(size, align)) {
  }

  Buffer(size_t size, size_t align, void* initial_data) : m_size_(size), m_align_(align), m_cpu_buffer_(_aligned_malloc(size, align)) {
    CopyDataToBuffer(initial_data, size);
  }

  ~Buffer() = default;

  Buffer(const Buffer& other) = delete;
  Buffer& operator=(const Buffer& other) = delete;

  Buffer(Buffer&& other) = default;
  Buffer& operator=(Buffer&& other) = default;

  void Reset(size_t size, size_t align) {
    m_size_ = size;
    m_align_ = align;
    m_cpu_buffer_.reset(_aligned_malloc(size, align));
  }

  void Reset(size_t size, size_t align, void* data) {
    Reset(size, align);
    CopyDataToBuffer(data, size);
  }

  size_t GetSize() const {
    return m_size_;
  }

  size_t GetAlign() const {
    return m_align_;
  }

  void* GetBuffer() const {
    return m_cpu_buffer_.get();
  }

 private:
  struct BufferDeleter {
    void operator()(void* ptr) {
      _aligned_free(ptr);
    }
  };

  void CopyDataToBuffer(void* data, size_t size) {
    if (data != nullptr) {
      std::memcpy(m_cpu_buffer_.get(), data, size);
    }
  }

  size_t m_size_ = 0;
  size_t m_align_ = 0;
  std::unique_ptr<void, BufferDeleter> m_cpu_buffer_ = {};
};

}  // namespace Core
