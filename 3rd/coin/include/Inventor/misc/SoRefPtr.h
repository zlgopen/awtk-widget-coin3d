#ifndef COIN_SOREFPTR_H
#define COIN_SOREFPTR_H

/**************************************************************************\
 * Copyright (c) Kongsberg Oil & Gas Technologies AS
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * 
 * Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 * 
 * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 * 
 * Neither the name of the copyright holder nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
\**************************************************************************/

#include <algorithm>

template <typename T>
class SoRefPtr {
public:
  SoRefPtr(void) noexcept : ptr(NULL) { }

  explicit SoRefPtr(T * p) : ptr(p)
  {
    if (this->ptr) this->ptr->ref();
  }

  SoRefPtr(const SoRefPtr & other) : ptr(other.ptr)
  {
    if (this->ptr) this->ptr->ref();
  }

  SoRefPtr(SoRefPtr && other) noexcept : ptr(other.ptr)
  {
    other.ptr = NULL;
  }

  ~SoRefPtr(void)
  {
    if (this->ptr) this->ptr->unref();
  }

  SoRefPtr & operator=(SoRefPtr other) noexcept
  {
    this->swap(other);
    return *this;
  }

  void reset(T * p = NULL)
  {
    SoRefPtr tmp(p);
    this->swap(tmp);
  }

  T * get(void) const noexcept { return this->ptr; }
  T & operator*(void) const { return *this->ptr; }
  T * operator->(void) const noexcept { return this->ptr; }
  explicit operator bool(void) const noexcept { return this->ptr != NULL; }

  void swap(SoRefPtr & other) noexcept
  {
    using std::swap;
    swap(this->ptr, other.ptr);
  }

private:
  T * ptr;
};

#endif // !COIN_SOREFPTR_H
