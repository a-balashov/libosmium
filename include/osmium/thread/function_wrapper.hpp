#ifndef OSMIUM_THREAD_FUNCTION_WRAPPER_HPP
#define OSMIUM_THREAD_FUNCTION_WRAPPER_HPP

/*

This file is part of Osmium (http://osmcode.org/libosmium).

Copyright 2013-2017 Jochen Topf <jochen@topf.org> and others (see README).

Boost Software License - Version 1.0 - August 17th, 2003

Permission is hereby granted, free of charge, to any person or organization
obtaining a copy of the software and accompanying documentation covered by
this license (the "Software") to use, reproduce, display, distribute,
execute, and transmit the Software, and to prepare derivative works of the
Software, and to permit third-parties to whom the Software is furnished to
do so, all subject to the following:

The copyright notices in the Software and this entire statement, including
the above license grant, this restriction and the following disclaimer,
must be included in all copies of the Software, in whole or in part, and
all derivative works of the Software, unless such copies or derivative
works are solely in the form of machine-executable object code generated by
a source language processor.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.

*/

#include <memory>
#include <utility>

namespace osmium {

    namespace thread {

        /**
         * This function wrapper can collect move-only functions unlike
         * std::function which needs copyable functions.
         * Taken from the book "C++ Concurrency in Action".
         */
        class function_wrapper {

            struct impl_base {

                virtual ~impl_base() noexcept = default;

                virtual bool call() {
                    return true;
                }

            }; // struct impl_base

            std::unique_ptr<impl_base> impl;

            template <typename F>
            struct impl_type : impl_base {

                F m_functor;

                explicit impl_type(F&& functor) :
                    m_functor(std::forward<F>(functor)) {
                }

                bool call() override {
                    m_functor();
                    return false;
                }

            }; // struct impl_type

        public:

            // Constructor must not be "explicit" for wrapper
            // to work seemlessly.
            template <typename TFunction>
            // cppcheck-suppress noExplicitConstructor
            function_wrapper(TFunction&& f) :
                impl(new impl_type<TFunction>(std::forward<TFunction>(f))) {
            }

            // The integer parameter is only used to signal that we want
            // the special function wrapper that makes the worker thread
            // shut down.
            explicit function_wrapper(int) :
                impl(new impl_base()) {
            }

            bool operator()() {
                return impl->call();
            }

            function_wrapper() = default;

            function_wrapper(function_wrapper&& other) :
                impl(std::move(other.impl)) {
            }

            function_wrapper& operator=(function_wrapper&& other) {
                impl = std::move(other.impl);
                return *this;
            }

            function_wrapper(const function_wrapper&) = delete;
            function_wrapper& operator=(const function_wrapper&) = delete;

            ~function_wrapper() = default;

            explicit operator bool() const {
                return static_cast<bool>(impl);
            }

        }; // class function_wrapper

    } // namespace thread

} // namespace osmium

#endif // OSMIUM_THREAD_FUNCTION_WRAPPER_HPP
